#pragma once
struct SingletonTimer
{
        struct requires_constructor_tag;
        struct requires_destructor_tag;

        using value_type = std::chrono::milliseconds;
        std::chrono::high_resolution_clock::time_point m_start;
        value_type                                     m_deltaTime;
        value_type                                     m_totalTime;
        value_type                                     m_fixedTotalTime;
        value_type                                     m_prevTotalTime;

    private:
        std::vector<SingletonTimer::value_type>        m_fixedUpdateTotalTimes;
        friend struct EntityAdmin;
};