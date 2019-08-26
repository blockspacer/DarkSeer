#include "RawInput.h"
#include <commctrl.h>
void RawInputE::RawInputBufferE::Initialize(HWND hwnd)
{
        memset(&m_lastPressState, 0, sizeof(m_lastPressState));

        m_InputFrames = (InputFrameE*)_aligned_malloc(sizeof(InputFrameE) * MAX_INPUT_FRAMES_PER_FRAME, 64);
        m_bottom      = 0;
        m_top         = 0;

        m_parentHWND           = hwnd;
        //WNDPROC defaultWndProc = (WNDPROC)GetWindowLongPtrA(hwnd, GWLP_WNDPROC);
		//SubClassWindow
  //      SetWindowSubclass(hwnd, RawInputBufferE::ProcessMessage,
        //m_parentWndProc = (WNDPROC)SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)RawInputBufferE::ProcessMessage);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

void RawInputE::RawInputBufferE::ShutDown()
{
        _aligned_free(m_InputFrames);
}

void RawInputE::RawInputBufferE::Push(InputFrameE inputFrame)
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

void RawInputE::RawInputBufferE::PopAll()
{
        const auto      b           = m_bottom;
        const auto      t           = m_top;
        const auto      frame_count = b - t;
        const auto      end         = t + frame_count;
        const auto      begin       = t;
        static uint64_t prevFrame   = -1;
        for (auto i = begin; i < end; i++)
        {
                InputFrameE inputFrame = m_InputFrames[i & MASK];

                auto [x, y] = inputFrame.m_mouseDeltas;

                if (x || y)
                        std::cout << "[" << x << "," << y << "]\t";
                if (inputFrame.m_buttonSignature)
                        std::cout << buttonSignatureToString[inputFrame.m_buttonSignature] << "("
                                  << transitionStateToString[inputFrame.m_transitionState] << ")"
                                  << "\t";
        }
        if (frame_count)
                std::cout << "\n\n\n";

        m_top += frame_count;
}


LRESULT RawInputBufferE::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
        InputFrameE inputFrame;
        memset(((char*)&inputFrame) + sizeof(PressState), 0, sizeof(InputFrameE) - sizeof(PressState));
        inputFrame.m_pressState = g_inputBufferE.m_lastPressState;
        switch (message)
        {
                case WM_INPUT:
                {
                        std::cout << "WHY";
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

                                g_inputBufferE.Push(inputFrame);
                        }
                        else if (rawInputFrame.header.dwType == RIM_TYPEMOUSE)
                        {
                                // relative mouse movement
                                if (!(rawInputFrame.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
                                {
                                        inputFrame.m_mouseDeltas =
                                            std::tuple{rawInputFrame.data.mouse.lLastX, rawInputFrame.data.mouse.lLastY};
                                        g_inputBufferE.Push(inputFrame);
                                }
                                // absolute mouse movment
                                else if (rawInputFrame.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP)
                                {
                                        const int width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                                        const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

                                        int x = static_cast<int>((float(rawInputFrame.data.mouse.lLastX) / 65535.0f) * width);
                                        int y = static_cast<int>((float(rawInputFrame.data.mouse.lLastY) / 65535.0f) * height);

                                        inputFrame.m_mouseDeltas = std::tuple{x, y};
                                        g_inputBufferE.Push(inputFrame);
                                }
                        }
                        break;
                }
                case WM_LBUTTONDOWN:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseLeft;
                        g_inputBufferE.Push(inputFrame);
                        break;
                }
                case WM_LBUTTONUP:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseLeft;
                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                        g_inputBufferE.Push(inputFrame);
                        break;
                }
                case WM_RBUTTONDOWN:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseRight;
                        g_inputBufferE.Push(inputFrame);
                        break;
                }
                case WM_RBUTTONUP:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseRight;
                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                        g_inputBufferE.Push(inputFrame);
                        break;
                }
                case WM_MBUTTONDOWN:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseMiddle;
                        g_inputBufferE.Push(inputFrame);
                        break;
                }
                case WM_MBUTTONUP:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseMiddle;
                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                        g_inputBufferE.Push(inputFrame);
                        break;
                }
                case WM_MOUSEWHEEL:
                {
                        std::underlying_type<TransitionState>::type mouseDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                        inputFrame.m_buttonSignature                           = INPUT_mouseScrollVertical;
                        inputFrame.m_scrollDelta                               = mouseDelta;
                        g_inputBufferE.Push(inputFrame);
                        break;
                }
                case WM_XBUTTONDOWN:
                {
                        switch (GET_XBUTTON_WPARAM(wParam))
                        {
                                case XBUTTON1:
                                {
                                        inputFrame.m_buttonSignature = INPUT_mouseForward;
                                        g_inputBufferE.Push(inputFrame);
                                        break;
                                }
                                case XBUTTON2:
                                {
                                        inputFrame.m_buttonSignature = INPUT_mouseBack;
                                        g_inputBufferE.Push(inputFrame);
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
                                        g_inputBufferE.Push(inputFrame);
                                        break;
                                }
                                case XBUTTON2:
                                {
                                        inputFrame.m_buttonSignature = INPUT_mouseBack;
                                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                                        g_inputBufferE.Push(inputFrame);
                                        break;
                                }
                        }
                        break;
                }
                case WM_KILLFOCUS:
                {
                        // set all press states to released
                        memset(&inputFrame.m_pressState, 0, sizeof(inputFrame.m_pressState));
                        g_inputBufferE.m_lastPressState = inputFrame.m_pressState;
                        g_inputBufferE.Push(inputFrame);
                        break;
                }
                default:
                        return CallWindowProcA(
                            g_inputBufferE.m_parentWndProc, g_inputBufferE.m_parentHWND, message, wParam, lParam);
        }
}