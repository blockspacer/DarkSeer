#pragma once
struct SingletonTimer;

namespace TimerUtil
{
        void ResetTimers(SingletonTimer* singlTimer);
        void Signal(SingletonTimer* singlTimer)
        {
                singlTimer->m_prevTotalTime = singlTimer->m_totalTime;
                singlTimer->m_totalTime     = std::chrono::duration_cast<SingletonTimer::value_type>(
                    std::chrono::high_resolution_clock::now() - singlTimer->m_start);
                singlTimer->m_deltaTime = singlTimer->m_totalTime - singlTimer->m_prevTotalTime;
        }
} // namespace TimerUtil