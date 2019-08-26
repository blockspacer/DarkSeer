#include "RawInput.h"
#include <Windows.h>
#include <assert.h>
#include <commctrl.h>
void RawInput::InputBuffer::Initialize(HWND hwnd)
{
        memset(&m_lastPressState, 0, sizeof(m_lastPressState));

        m_InputFrames = (InputFrame*)_aligned_malloc(sizeof(InputFrame) * MAX_INPUT_FRAMES_PER_FRAME, 64);
        m_bottom      = 0;
        m_top         = 0;

        m_parentHWND = hwnd;

        m_parentWndProc = (decltype(m_parentWndProc))SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)InputBuffer::InputWndProc);

        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void RawInput::InputBuffer::ShutDown()
{
        _aligned_free(m_InputFrames);
}

void RawInput::InputBuffer::Push(InputFrame inputFrame)
{
        switch (inputFrame.m_transitionState)
        {
                case TransitionState::INPUT_transitionStateUp:
                        inputFrame.m_pressState.KeyUp(inputFrame.m_buttonSignature);
                        break;
                case TransitionState::INPUT_transitionStateDown:
                        inputFrame.m_pressState.KeyDown(inputFrame.m_buttonSignature);
                        break;
        }
        m_lastPressState = inputFrame.m_pressState;

        const auto b = m_bottom;
        while (b - m_top >= MAX_INPUT_FRAMES_PER_FRAME)
                Sleep(0);

        m_InputFrames[b & MASK] = inputFrame;

        // ensure the inputFrame is written before b+1 is published to other threads.
        // on x86/64, a compiler barrier is enough.
        std::atomic_thread_fence(std::memory_order_seq_cst);

        m_bottom++;
}

void RawInput::InputBuffer::PopAll()
{
        const auto      b           = m_bottom;
        const auto      t           = m_top;
        const auto      frame_count = b - t;
        const auto      end         = t + frame_count;
        const auto      begin       = t;
        static uint64_t prevFrame   = -1;
        for (auto i = begin; i < end; i++)
        {
                InputFrame inputFrame = m_InputFrames[i & MASK];

                auto [x, y] = inputFrame.m_mouseDeltas;

                if (x || y)
                        std::cout << "[" << x << "," << y << "]\t";
                if (inputFrame.m_buttonSignature)
                {
                        std::cout << buttonSignatureToString[inputFrame.m_buttonSignature] << "("
                                  << transitionStateToString[inputFrame.m_transitionState] << ")"
                                  << "\t";
                }
                if (inputFrame.m_scrollDelta)
                        std::cout << "(" << inputFrame.m_scrollDelta << ")";
        }
        if (frame_count)
                std::cout << "\n";

        m_top += frame_count;
}

LRESULT CALLBACK InputBuffer::InputWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
        InputFrame inputFrame;
        memset(((char*)&inputFrame) + sizeof(Key), 0, sizeof(InputFrame) - sizeof(Key));
        inputFrame.m_pressState = g_inputBuffer.m_lastPressState;

        switch (message)
        {
                case WM_INPUT:
                {
                        RAWINPUT rawInputFrame;
                        UINT     dwSize;
                        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 0, &dwSize, sizeof(RAWINPUTHEADER));
                        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &rawInputFrame, &dwSize, sizeof(RAWINPUTHEADER));

                        if (rawInputFrame.header.dwType == RIM_TYPEKEYBOARD)
                        {
                                uint16_t        button_signature_flag = rawInputFrame.data.keyboard.Flags >> 1;
                                TransitionState transition_state = (TransitionState)(rawInputFrame.data.keyboard.Flags & 1);
                                inputFrame.m_buttonSignature =
                                    (ButtonSignature)(rawInputFrame.data.keyboard.MakeCode +
                                                      button_signature_flag * INPUT_NUM_KEYBOARD_SCANCODES);
                                inputFrame.m_transitionState = transition_state;

                                g_inputBuffer.Push(inputFrame);
                        }
                        else if (rawInputFrame.header.dwType == RIM_TYPEMOUSE)
                        {
                                // relative mouse movement
                                if (!(rawInputFrame.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
                                {
                                        inputFrame.m_mouseDeltas =
                                            std::tuple{rawInputFrame.data.mouse.lLastX, rawInputFrame.data.mouse.lLastY};
                                        g_inputBuffer.Push(inputFrame);
                                }
                                // absolute mouse movment
                                else if (rawInputFrame.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP)
                                {
                                        const int width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                                        const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

                                        int x = static_cast<int>((float(rawInputFrame.data.mouse.lLastX) / 65535.0f) * width);
                                        int y = static_cast<int>((float(rawInputFrame.data.mouse.lLastY) / 65535.0f) * height);

                                        inputFrame.m_mouseDeltas = std::tuple{x, y};
                                        g_inputBuffer.Push(inputFrame);
                                }
                        }
                        break;
                }
                case WM_LBUTTONDOWN:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseLeft;
                        g_inputBuffer.Push(inputFrame);
                        break;
                }
                case WM_LBUTTONUP:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseLeft;
                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                        g_inputBuffer.Push(inputFrame);
                        break;
                }
                case WM_RBUTTONDOWN:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseRight;
                        g_inputBuffer.Push(inputFrame);
                        break;
                }
                case WM_RBUTTONUP:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseRight;
                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                        g_inputBuffer.Push(inputFrame);
                        break;
                }
                case WM_MBUTTONDOWN:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseMiddle;
                        g_inputBuffer.Push(inputFrame);
                        break;
                }
                case WM_MBUTTONUP:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseMiddle;
                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                        g_inputBuffer.Push(inputFrame);
                        break;
                }
                case WM_MOUSEWHEEL:
                {
                        std::underlying_type<TransitionState>::type mouseDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                        inputFrame.m_buttonSignature                           = INPUT_mouseScrollVertical;
                        inputFrame.m_scrollDelta                               = mouseDelta;
                        g_inputBuffer.Push(inputFrame);
                        break;
                }
                case WM_MOUSEHWHEEL:
                {
                        std::underlying_type<TransitionState>::type mouseDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                        inputFrame.m_buttonSignature                           = INPUT_mouseScrollHorizontal;
                        inputFrame.m_scrollDelta                               = mouseDelta;
                        g_inputBuffer.Push(inputFrame);
				}
                case WM_XBUTTONDOWN:
                {
                        switch (GET_XBUTTON_WPARAM(wParam))
                        {
                                case XBUTTON1:
                                {
                                        inputFrame.m_buttonSignature = INPUT_mouseForward;
                                        g_inputBuffer.Push(inputFrame);
                                        break;
                                }
                                case XBUTTON2:
                                {
                                        inputFrame.m_buttonSignature = INPUT_mouseBack;
                                        g_inputBuffer.Push(inputFrame);
                                        break;
                                }
                        }
                        break;
                }
                case WM_XBUTTONUP:
                {
                        switch (GET_XBUTTON_WPARAM(wParam))
                        {
                                case XBUTTON1:
                                {
                                        inputFrame.m_buttonSignature = INPUT_mouseForward;
                                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                                        g_inputBuffer.Push(inputFrame);
                                        break;
                                }
                                case XBUTTON2:
                                {
                                        inputFrame.m_buttonSignature = INPUT_mouseBack;
                                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                                        g_inputBuffer.Push(inputFrame);
                                        break;
                                }
                        }
                        break;
                }
                case WM_KILLFOCUS:
                {
                        // set all press states to released
                        memset(&inputFrame.m_pressState, 0, sizeof(inputFrame.m_pressState));
                        g_inputBuffer.m_lastPressState = inputFrame.m_pressState;
                        g_inputBuffer.Push(inputFrame);
                        break;
                }
        }

        return CallWindowProcA(g_inputBuffer.m_parentWndProc, g_inputBuffer.m_parentHWND, message, wParam, lParam);
}