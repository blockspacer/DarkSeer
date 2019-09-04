#include <SingletonInput.h>
#include "InputBuffer.h"

InputBuffer::InputBuffer() :
    m_inputFrames(),
    m_bottom(),
    m_top(),
    m_currFrameBottom(),
    m_currFrameTop()
{}


void InputBuffer::push_back(InputFrame inputFrame)
{
        switch (inputFrame.m_transitionState)
        {
                case KeyTransition::Up:
                        inputFrame.m_pressState.KeyUp(inputFrame.m_buttonSignature);
                        break;
                case KeyTransition::Down:
                        inputFrame.m_pressState.KeyDown(inputFrame.m_buttonSignature);
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
                        inputFramesCurrent.m_pressState.KeyUp(buttonSignature);
                        break;
                case KeyTransition::Down:
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

void InputBuffer::Signal()
{
        m_top                                   = m_currFrameBottom;
        const_cast<int64_t&>(m_currFrameBottom) = m_bottom;
        const_cast<int64_t&>(m_currFrameTop)    = m_top;
}