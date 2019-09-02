#include <InputUtility.h>

#include <SingletonInput.h>

void InputUtil::InitializeInputBuffer(SingletonInput* singlInput, HWND hwnd)
{
        // allocate input frames
        singlInput->m_inputBuffer.m_inputFrames =
            (InputFrame*)_aligned_malloc(sizeof(InputFrame) * InputBuffer::MAX_INPUT_FRAMES_PER_FRAME, 64);
        assert(singlInput->m_inputBuffer.m_inputFrames);

        // set circular buffer bottom/top to 0
        singlInput->m_inputBuffer.m_bottom                         = 0;
        singlInput->m_inputBuffer.m_top                            = 0;
        non_const_ref(singlInput->m_inputBuffer.m_currFrameBottom) = 0;
        non_const_ref(singlInput->m_inputBuffer.m_currFrameTop)    = 0;
        // set previous press state to 0 for all buttons
#pragma warning(suppress : 6385)
        memset(&singlInput->m_inputBuffer.m_inputFrames[-1 & InputBuffer::MASK].m_pressState, 0, sizeof(KeyState));

        // add input capture window proc to the window handle's previous window proc
        singlInput->m_inputBuffer.m_parentHWND    = hwnd;
        singlInput->m_inputBuffer.m_parentWndProc = (decltype(singlInput->m_inputBuffer.m_parentWndProc))SetWindowLongPtrA(
            hwnd, GWLP_WNDPROC, (LONG_PTR)InputBuffer::InputWndProc);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void InputUtil::ReleaseInputBufferMemory(SingletonInput* singlInput)
{
        _aligned_free(singlInput->m_inputBuffer.m_inputFrames);
}
