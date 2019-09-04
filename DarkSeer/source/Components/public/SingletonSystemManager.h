#pragma once
#include <SingletonTimer.h>
struct SingletonSystemManager
{
        volatile bool m_runSystems;
        std::thread   m_systemManagerThread;

        std::vector<void (*)()> m_initializeFunctions;
        std::vector<void (*)()> m_shutdownFunctions;

        std::vector<void (*)()>                 m_fixedUpdateFunctions;
        std::vector<SingletonTimer::value_type> m_fixedUpdateTickRates;
        std::vector<SingletonTimer::value_type> m_fixedUpdateTotalTimes;

        std::vector<void (*)()> m_postUpdateFunctions;
        std::vector<void (*)()> m_updateFunctions;
        std::vector<void (*)()> m_preUpdateFunctions;

#ifdef _DEBUG
        struct DEBUG_METADATA
        {
                std::vector<long long>   m_fixedUpdateIndex;
                std::vector<long long>   m_updateIndex;
                std::vector<long long>   m_preUpdateIndex;
                std::vector<long long>   m_postUpdateIndex;
                std::vector<long long>   m_inlitializeIndex;
                std::vector<long long>   m_shutdownIndex;
                std::vector<std::string> m_systemName;
        } DEBUG_METADATA;
#endif
};