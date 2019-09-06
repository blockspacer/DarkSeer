#pragma once
#include <SingletonTimer.h>
struct EntityAdmin;

struct SingletonSystemManager_DEBUG_METADATA
{
#ifdef _DEBUG
        std::vector<long long>   m_DBG_fixedUpdateIndex;
        std::vector<long long>   m_DBG_updateIndex;
        std::vector<long long>   m_DBG_preUpdateIndex;
        std::vector<long long>   m_DBG_postUpdateIndex;
        std::vector<long long>   m_DBG_inlitializeIndex;
        std::vector<long long>   m_DBG_shutdownIndex;
        std::vector<std::string> m_DBG_systemName;
#endif
};

struct SingletonSystemManager : SingletonSystemManager_DEBUG_METADATA
{
        struct requires_constructor_tag;
        struct requires_destructor_tag;

        bool        m_runSystems;
        std::thread m_systemManagerThread;

        std::vector<void (*)(EntityAdmin*)> m_initializeFunctions;
        std::vector<void (*)(EntityAdmin*)> m_shutdownFunctions;

        std::vector<void (*)(EntityAdmin*)>     m_fixedUpdateFunctions;
        std::vector<SingletonTimer::value_type> m_fixedUpdateTickRates;

        std::vector<void (*)(EntityAdmin*)> m_updateFunctions;
        std::vector<void (*)(EntityAdmin*)> m_postUpdateFunctions;
        std::vector<void (*)(EntityAdmin*)> m_preUpdateFunctions;
};