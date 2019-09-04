#include <SingletonTimer.h>
#include <TimerUtility.h>

namespace TimerUtil
{
        void ResetTimers(SingletonTimer* singlTimer)
        {
                singlTimer->m_deltaTime     = SingletonTimer::value_type::zero();
                singlTimer->m_totalTime     = SingletonTimer::value_type::zero();
                singlTimer->m_prevTotalTime = SingletonTimer::value_type::zero();
        }

        void Signal(SingletonTimer* singlTimer)
        {
                singlTimer->m_prevTotalTime = singlTimer->m_totalTime;
                singlTimer->m_totalTime     = std::chrono::duration_cast<SingletonTimer::value_type>(
                    std::chrono::high_resolution_clock::now() - singlTimer->m_start);
                singlTimer->m_deltaTime = singlTimer->m_totalTime - singlTimer->m_prevTotalTime;
        }
} // namespace TimerUtil