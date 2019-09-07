#include "InputBuffer.h"
#include <SingletonInput.h>

InputBuffer::InputBuffer() : m_currFrameBottom(0), m_currFrameTop(0)
{
        // allocate input buffer
        m_inputFrames = (InputFrame*)_aligned_malloc(sizeof(InputFrame) * MAX_INPUT_FRAMES_PER_FRAME, 64);
        assert(m_inputFrames);

        // set circular buffer bottom/top to 0
        m_bottom = 0;
        m_top    = 0;
        // set previous press state to 0 for all buttons
#pragma warning(suppress : 6385)
        memset(&m_inputFrames[-1 & InputBuffer::MASK].m_pressState, 0, sizeof(KeyState));
}

InputBuffer::~InputBuffer()
{
        _aligned_free(m_inputFrames);
}


void InputBuffer::push_back(InputFrame inputFrame)
{
        switch (inputFrame.m_transitionState)
        {
                case KeyTransition::Up:
                        inputFrame.m_pressState.SetKeyUp(inputFrame.m_buttonSignature);
                        break;
                case KeyTransition::Down:
                        inputFrame.m_pressState.SetKeyDown(inputFrame.m_buttonSignature);
                        break;
        }

        const auto b = m_bottom;
        while (b - m_top >= MAX_INPUT_FRAMES_PER_FRAME)
                Sleep(0);

        m_inputFrames[b & MASK] = inputFrame;

        // ensure the inputFrame is written before b+1 is published to other threads.
        // on x86/64, a compiler barrier is enough.
        std::atomic_thread_fence(std::memory_order_seq_cst);
        m_bottom++;
}

void InputBuffer::emplace_back(std::tuple<long, long> mouseDeltas,
                               KeyCode                buttonSignature,
                               int16_t                scrollDelta,
                               KeyTransition          transitionState)
{
        const auto b = m_bottom;
        while (b - m_top >= MAX_INPUT_FRAMES_PER_FRAME)
                Sleep(0);

        InputFrame&       inputFramesCurrent  = m_inputFrames[b & MASK];
        const InputFrame& inputFramesPrevious = m_inputFrames[(b - 1) & MASK];
        inputFramesCurrent.m_pressState       = inputFramesPrevious.m_pressState;
        switch (transitionState)
        {
                case KeyTransition::Up:
                        inputFramesCurrent.m_pressState.SetKeyUp(buttonSignature);
                        break;
                case KeyTransition::Down:
                        inputFramesCurrent.m_pressState.SetKeyDown(buttonSignature);
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

void InputBuffer::Signal()
{
        m_top                                   = m_currFrameBottom;
        const_cast<int64_t&>(m_currFrameBottom) = m_bottom;
        const_cast<int64_t&>(m_currFrameTop)    = m_top;
}