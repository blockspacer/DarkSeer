#pragma once
#include <SingletonSystemManager.h>
struct SingletonInput;
struct SingletonWindow;
struct SingletonConsole;

enum SystemIndex : int
{
        invalid = -1
};

namespace SystemConceptMetaFunctions
{
        template <class SystemConcept, class = void>
        struct has_entity_admin : std::false_type
        {};
        template <class SystemConcept>
        struct has_entity_admin<SystemConcept, std::void_t<decltype(SystemConcept::PostUpdate(std::declval<EntityAdmin*>()))>>
            : std::true_type
        {};
        template <class SystemConcept, class = void>
        struct has_initialize : std::false_type
        {};
        template <class SystemConcept>
        struct has_initialize<SystemConcept, std::void_t<decltype(SystemConcept::Initialize(std::declval<EntityAdmin*>()))>>
            : std::true_type
        {};
        template <class SystemConcept, class = void>
        struct has_shutdown : std::false_type
        {};
        template <class SystemConcept>
        struct has_shutdown<SystemConcept, std::void_t<decltype(SystemConcept::Shutdown(std::declval<EntityAdmin*>()))>>
            : std::true_type
        {};
        template <class SystemConcept, class = void>
        struct has_pre_update : std::false_type
        {};
        template <class SystemConcept>
        struct has_pre_update<SystemConcept, std::void_t<decltype(SystemConcept::PreUpdate(std::declval<EntityAdmin*>()))>>
            : std::true_type
        {};
        template <class SystemConcept, class = void>
        struct has_update : std::false_type
        {};
        template <class SystemConcept>
        struct has_update<SystemConcept, std::void_t<decltype(SystemConcept::Update(std::declval<EntityAdmin*>()))>>
            : std::true_type
        {};
        template <class SystemConcept, class = void>
        struct has_fixed_update : std::false_type
        {};
        template <class SystemConcept>
        struct has_fixed_update<SystemConcept, std::void_t<decltype(SystemConcept::FixedUpdate(std::declval<EntityAdmin*>()))>>
            : std::true_type
        {};
        template <class SystemConcept, class = void>
        struct has_tick_rate : std::false_type
        {};
        template <class SystemConcept>
        struct has_tick_rate<SystemConcept, std::void_t<decltype(SystemConcept::TickRate)>> : std::true_type
        {};
        template <class SystemConcept, class = void>
        struct has_post_update : std::false_type
        {};
        template <class SystemConcept>
        struct has_post_update<SystemConcept, std::void_t<decltype(SystemConcept::PostUpdate(std::declval<EntityAdmin*>()))>>
            : std::true_type
        {};
} // namespace SystemConceptMetaFunctions

struct EntityAdmin
{
    private:
        SingletonInput*         m_singletonInput;
        SingletonSystemManager* m_singletonSystemManager;
        SingletonWindow*        m_singletonWindow;
        SingletonConsole*       m_singletonConsole;
        SingletonTimer*         m_singletonTimer;

        template <typename TSystemConcept>
        inline void AttachSystem_DEBUG_INJECT_META_DATA(SingletonSystemManager* singlSystemManager)
        {
#ifdef _DEBUG
                using namespace SystemConceptMetaFunctions;

                if constexpr (has_initialize<TSystemConcept>::value)
                        singlSystemManager->m_DBG_inlitializeIndex.push_back(singlSystemManager->m_initializeFunctions.size() -
                                                                             1);
                else
                        singlSystemManager->m_DBG_inlitializeIndex.push_back(static_cast<int>(SystemIndex::invalid));

                if constexpr (has_pre_update<TSystemConcept>::value)
                        singlSystemManager->m_DBG_preUpdateIndex.push_back(singlSystemManager->m_preUpdateFunctions.size() - 1);
                else
                        singlSystemManager->m_DBG_preUpdateIndex.push_back(static_cast<int>(SystemIndex::invalid));

                if constexpr (has_update<TSystemConcept>::value)
                        singlSystemManager->m_DBG_updateIndex.push_back(singlSystemManager->m_updateFunctions.size() - 1);
                else
                        singlSystemManager->m_DBG_updateIndex.push_back(static_cast<int>(SystemIndex::invalid));

                if constexpr (has_fixed_update<TSystemConcept>::value)
                        singlSystemManager->m_DBG_fixedUpdateIndex.push_back(singlSystemManager->m_fixedUpdateFunctions.size() -
                                                                             1);
                else
                        singlSystemManager->m_DBG_fixedUpdateIndex.push_back(static_cast<int>(SystemIndex::invalid));

                if constexpr (has_post_update<TSystemConcept>::value)
                        singlSystemManager->m_DBG_postUpdateIndex.push_back(singlSystemManager->m_postUpdateFunctions.size() -
                                                                            1);
                else
                        singlSystemManager->m_DBG_postUpdateIndex.push_back(static_cast<int>(SystemIndex::invalid));

                if constexpr (has_shutdown<TSystemConcept>::value)
                        singlSystemManager->m_DBG_shutdownIndex.push_back(singlSystemManager->m_shutdownFunctions.size() - 1);
                else
                        singlSystemManager->m_DBG_shutdownIndex.push_back(static_cast<int>(SystemIndex::invalid));
#endif
        }

        static void SystemUpdateLoop(EntityAdmin*            entityAdmin,
                                     SingletonSystemManager* singlSystemManager,
                                     SingletonTimer*         singlTimer);

        static void LaunchSystemUpdateLoopInternal(EntityAdmin*            entityAdmin,
                                                   SingletonTimer*         singlTimer,
                                                   SingletonSystemManager* singlSystemManager);
    public:
        SingletonInput*         GetSingletonInput();
        SingletonWindow*        GetSingletonWindow();
        SingletonConsole*       GetSingletonConsole();
        SingletonSystemManager* GetSingletonSystemManager();
        SingletonTimer*         GetSingletonTimer();

        // If any of the follow function signatures are static members of the template paramater type:
        //	Update(EntityAdmin*)
        //	PreUpdate(EntityAdmin*)
        //	PostUpdate(EntityAdmin*)
        //	Shutdown(EntityAdmin*)
        //	ComponentsInitialize(EntityAdmin*)
        //	FixedUpdate(EntityAdmin*) /* requires static constexpr TickRate */
        // their function will be pushed into the singlSystemManager and called in the appropriate order after SystemsLaunch is
        // called on the singlSystemManager
        template <typename TSystemConcept>
        inline void SystemsAttach(SingletonSystemManager* singlSystemManager)
        {
                using namespace SystemConceptMetaFunctions;

                if constexpr (has_initialize<TSystemConcept>::value)
                        singlSystemManager->m_initializeFunctions.push_back(TSystemConcept::Initialize);
                if constexpr (has_pre_update<TSystemConcept>::value)
                        singlSystemManager->m_preUpdateFunctions.push_back(TSystemConcept::PreUpdate);
                if constexpr (has_update<TSystemConcept>::value)
                        singlSystemManager->m_updateFunctions.push_back(TSystemConcept::Update);
                if constexpr (has_fixed_update<TSystemConcept>::value)
                {
                        static_assert(has_tick_rate<TSystemConcept>::value,
                                      "FixedUpdate requires a static constexpr TickRate member");
                        static_assert(std::is_same<decltype(TSystemConcept::TickRate), const SingletonTimer::value_type>::value,
                                      "TickRate must be a SingletonSystemManager::value_type type");
                        static_assert(TSystemConcept::TickRate > SingletonTimer::value_type::zero(),
                                      "TickRate must be a value greater than zero");

                        singlSystemManager->m_fixedUpdateFunctions.push_back(TSystemConcept::FixedUpdate);
                        singlSystemManager->m_fixedUpdateTickRates.push_back(TSystemConcept::TickRate);
                        //singlSystemManager->m_fixedUpdateTotalTimes.push_back(SingletonTimer::value_type(0));
                }
                if constexpr (has_post_update<TSystemConcept>::value)
                        singlSystemManager->m_postUpdateFunctions.push_back(TSystemConcept::PostUpdate);
                if constexpr (has_shutdown<TSystemConcept>::value)
                        singlSystemManager->m_shutdownFunctions.push_back(TSystemConcept::ComponentsShutdown);

                AttachSystem_DEBUG_INJECT_META_DATA<TSystemConcept>(singlSystemManager);
        }
        void SystemsLaunch(SingletonTimer* singlTimer, SingletonSystemManager* singlSystemManager);
        void SystemsShutdown(SingletonSystemManager* singlSystemManager);

		void WindowsInitialize(SingletonInput* singlInput, SingletonWindow* singlWindow);
        void WindowsLaunch(SingletonWindow* singlWindow);
        void WindowsShutdown(SingletonWindow* singlWindow);

        void ComponentsInitialize();
        void ComponentsShutdown();
};
inline EntityAdmin g_userEntityAdmin;