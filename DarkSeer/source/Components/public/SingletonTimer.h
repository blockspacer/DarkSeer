#pragma once
struct SingletonTimer
{
        struct requires_constructor_tag;

        using value_type = std::chrono::milliseconds;
        std::chrono::high_resolution_clock::time_point m_start;
        value_type                                     m_deltaTime;
        value_type                                     m_totalTime;
        value_type                                     m_fixedTotalTime;
        value_type                                     m_prevTotalTime;

        std::vector<SingletonTimer::value_type>        m_fixedUpdateTotalTimes;
};