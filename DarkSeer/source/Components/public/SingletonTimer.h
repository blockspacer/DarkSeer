#pragma once
struct SingletonTimer
{
        using value_type = std::chrono::milliseconds;
        std::chrono::high_resolution_clock::time_point m_start;
        value_type                                     m_deltaTime;
        value_type                                     m_totalTime;
        value_type                                     m_prevTotalTime;
        int                                            m_fixedUpdateIndex;
};