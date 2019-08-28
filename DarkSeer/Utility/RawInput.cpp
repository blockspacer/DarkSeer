#include "RawInput.h"
#include <Windows.h>
#include <assert.h>
#include <commctrl.h>

RawInput::InputBuffer::InputBuffer() :
    m_inputFrames(),
    m_bottom(),
    m_top(),
    m_lastPressState(),
    m_parentWndProc(),
    m_parentHWND(),
    m_currFrameBottom(),
    m_currFrametop()
{}

void RawInput::InputBuffer::Initialize(HWND hwnd)
{
        m_inputFrames = (InputFrame*)_aligned_malloc(sizeof(InputFrame) * MAX_INPUT_FRAMES_PER_FRAME, 64);
        m_bottom      = 0;
        m_top         = 0;

        memset(&m_lastPressState, 0, sizeof(m_lastPressState));

        m_parentHWND    = hwnd;
        m_parentWndProc = (decltype(m_parentWndProc))SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)InputBuffer::InputWndProc);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void RawInput::InputBuffer::ShutDown()
{
        _aligned_free(m_inputFrames);
}

void RawInput::InputBuffer::push_back(InputFrame inputFrame)
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

        m_inputFrames[b & MASK] = inputFrame;

        // ensure the inputFrame is written before b+1 is published to other threads.
        // on x86/64, a compiler barrier is enough.
        std::atomic_thread_fence(std::memory_order_seq_cst);
        m_bottom++;
}

void RawInput::InputBuffer::emplace_back(std::tuple<long, long> mouseDeltas,
                                         ButtonSignature        buttonSignature,
                                         int16_t                scrollDelta,
                                         TransitionState        transitionState)
{
        const auto b = m_bottom;
        while (b - m_top >= MAX_INPUT_FRAMES_PER_FRAME)
                Sleep(0);

        InputFrame&       inputFramesCurrent  = m_inputFrames[b & MASK];
        const InputFrame& inputFramesPrevious = m_inputFrames[(b - 1) & MASK];
        inputFramesCurrent.m_pressState       = inputFramesPrevious.m_pressState;
        switch (transitionState)
        {
                case TransitionState::INPUT_transitionStateUp:
                        inputFramesCurrent.m_pressState.KeyUp(buttonSignature);
                        break;
                case TransitionState::INPUT_transitionStateDown:
                        inputFramesCurrent.m_pressState.KeyDown(buttonSignature);
                        break;
        }
        inputFramesCurrent.m_mouseDeltas     = mouseDeltas;
        inputFramesCurrent.m_buttonSignature = buttonSignature;
        inputFramesCurrent.m_scrollDelta     = scrollDelta;
        inputFramesCurrent.m_transitionState = transitionState;
        // ensure the inputFrame is written before b+1 is published to other threads.
        // on x86/64, a compiler barrier is enough.
        std::atomic_thread_fence(std::memory_order_seq_cst);
        m_bottom++;
}

void RawInput::InputBuffer::ProcessAll()
{
        const auto b = m_bottom;
        const auto t = m_top;
        for (auto i = t; i < b; i++)
        {
                InputFrame inputFrame = m_inputFrames[i & MASK];

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
        if (b - t)
                std::cout << "\n";

        m_top = b;
}

void RawInput::InputBuffer::BeginFrame()
{
        m_top                                   = m_currFrameBottom;
        const_cast<int64_t&>(m_currFrameBottom) = m_bottom;
        const_cast<int64_t&>(m_currFrametop)    = m_top;
}

LRESULT CALLBACK InputBuffer::InputWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
        InputFrame inputFrame;
        memset(((char*)&inputFrame) + sizeof(KeyState), 0, sizeof(InputFrame) - sizeof(KeyState));
        inputFrame.m_pressState = g_inputBuffer.m_lastPressState;

        switch (message)
        {
                case WM_INPUT:
                {
                        RAWINPUT rawInputFrame{};
                        UINT     dwSize{};
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

                                g_inputBuffer.push_back(inputFrame);
                        }
                        else if (rawInputFrame.header.dwType == RIM_TYPEMOUSE)
                        {
                                // relative mouse movement
                                if (!(rawInputFrame.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
                                {
                                        inputFrame.m_mouseDeltas =
                                            std::tuple{rawInputFrame.data.mouse.lLastX, rawInputFrame.data.mouse.lLastY};
                                        g_inputBuffer.push_back(inputFrame);
                                }
                                // absolute mouse movment
                                else if (rawInputFrame.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP)
                                {
                                        const int width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                                        const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

                                        int x = static_cast<int>((float(rawInputFrame.data.mouse.lLastX) / 65535.0f) * width);
                                        int y = static_cast<int>((float(rawInputFrame.data.mouse.lLastY) / 65535.0f) * height);

                                        inputFrame.m_mouseDeltas = std::tuple{x, y};
                                        g_inputBuffer.push_back(inputFrame);
                                }
                        }
                        break;
                }
                case WM_LBUTTONDOWN:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseLeft;
                        g_inputBuffer.push_back(inputFrame);
                        break;
                }
                case WM_LBUTTONUP:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseLeft;
                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                        g_inputBuffer.push_back(inputFrame);
                        break;
                }
                case WM_RBUTTONDOWN:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseRight;
                        g_inputBuffer.push_back(inputFrame);
                        break;
                }
                case WM_RBUTTONUP:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseRight;
                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                        g_inputBuffer.push_back(inputFrame);
                        break;
                }
                case WM_MBUTTONDOWN:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseMiddle;
                        g_inputBuffer.push_back(inputFrame);
                        break;
                }
                case WM_MBUTTONUP:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseMiddle;
                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                        g_inputBuffer.push_back(inputFrame);
                        break;
                }
                case WM_MOUSEWHEEL:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseScrollVertical;
                        inputFrame.m_scrollDelta     = GET_WHEEL_DELTA_WPARAM(wParam);
                        g_inputBuffer.push_back(inputFrame);
                        break;
                }
                case WM_MOUSEHWHEEL:
                {
                        inputFrame.m_buttonSignature = INPUT_mouseScrollHorizontal;
                        inputFrame.m_scrollDelta     = GET_WHEEL_DELTA_WPARAM(wParam);
                        g_inputBuffer.push_back(inputFrame);
                        break;
                }
                case WM_XBUTTONDOWN:
                {
                        switch (GET_XBUTTON_WPARAM(wParam))
                        {
                                case XBUTTON1:
                                {
                                        inputFrame.m_buttonSignature = INPUT_mouseForward;
                                        g_inputBuffer.push_back(inputFrame);
                                        break;
                                }
                                case XBUTTON2:
                                {
                                        inputFrame.m_buttonSignature = INPUT_mouseBack;
                                        g_inputBuffer.push_back(inputFrame);
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
                                        g_inputBuffer.push_back(inputFrame);
                                        break;
                                }
                                case XBUTTON2:
                                {
                                        inputFrame.m_buttonSignature = INPUT_mouseBack;
                                        inputFrame.m_transitionState = INPUT_transitionStateUp;
                                        g_inputBuffer.push_back(inputFrame);
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
                        g_inputBuffer.push_back(inputFrame);
                        break;
                }
        }

        return CallWindowProcA(g_inputBuffer.m_parentWndProc, g_inputBuffer.m_parentHWND, message, wParam, lParam);
}