#pragma once
struct SingletonWindow;
struct SingletonSystemManager;
struct SingletonTimer;
#include <SingletonSystemManager.h>

namespace SystemUtil
{
        enum SystemIndex : int
        {
                invalid = -1
        };

        void SystemUpdateLoop(SingletonSystemManager* singlSystemManager, SingletonTimer* singlTimer);
        void LaunchSystemUpdateLoop(SingletonTimer* singlTimer, SingletonSystemManager* singlSystemManager);
        void Shutdown(SingletonWindow* singlWindow, SingletonSystemManager* singlSystemManager);

        namespace SystemConceptMetaFunctions
        {
                template <class SystemConcept, class = void>
                struct has_entity_admin : std::false_type
                {};
                template <class SystemConcept>
                struct has_entity_admin<SystemConcept, std::void_t<decltype(SystemConcept::PostUpdate())>> : std::true_type
                {};
                template <class SystemConcept, class = void>
                struct has_initialize : std::false_type
                {};
                template <class SystemConcept>
                struct has_initialize<SystemConcept, std::void_t<decltype(SystemConcept::Initialize())>> : std::true_type
                {};
                template <class SystemConcept, class = void>
                struct has_shutdown : std::false_type
                {};
                template <class SystemConcept>
                struct has_shutdown<SystemConcept, std::void_t<decltype(SystemConcept::Shutdown())>> : std::true_type
                {};
                template <class SystemConcept, class = void>
                struct has_pre_update : std::false_type
                {};
                template <class SystemConcept>
                struct has_pre_update<SystemConcept, std::void_t<decltype(SystemConcept::PreUpdate())>> : std::true_type
                {};
                template <class SystemConcept, class = void>
                struct has_update : std::false_type
                {};
                template <class SystemConcept>
                struct has_update<SystemConcept, std::void_t<decltype(SystemConcept::Update())>> : std::true_type
                {};
                template <class SystemConcept, class = void>
                struct has_fixed_update : std::false_type
                {};
                template <class SystemConcept>
                struct has_fixed_update<SystemConcept, std::void_t<decltype(SystemConcept::FixedUpdate())>> : std::true_type
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
                struct has_post_update<SystemConcept, std::void_t<decltype(SystemConcept::PostUpdate())>> : std::true_type
                {};
        } // namespace SystemConceptMetaFunctions
        using namespace SystemConceptMetaFunctions;

        template <typename TSystemConcept>
        inline void AttachSystem_DEBUG_INJECT_META_DATA(SingletonSystemManager* singlSystemManager)
        {
#ifdef _DEBUG
                if constexpr (has_initialize<TSystemConcept>::value)
                        singlSystemManager->DEBUG_METADATA.m_inlitializeIndex.push_back(
                            singlSystemManager->m_initializeFunctions.size() - 1);
                else
                        singlSystemManager->DEBUG_METADATA.m_inlitializeIndex.push_back(static_cast<int>(SystemIndex::invalid));

                if constexpr (has_pre_update<TSystemConcept>::value)
                        singlSystemManager->DEBUG_METADATA.m_preUpdateIndex.push_back(
                            singlSystemManager->m_preUpdateFunctions.size() - 1);
                else
                        singlSystemManager->DEBUG_METADATA.m_preUpdateIndex.push_back(static_cast<int>(SystemIndex::invalid));

                if constexpr (has_update<TSystemConcept>::value)
                        singlSystemManager->DEBUG_METADATA.m_updateIndex.push_back(
                            singlSystemManager->m_updateFunctions.size() - 1);
                else
                        singlSystemManager->DEBUG_METADATA.m_updateIndex.push_back(static_cast<int>(SystemIndex::invalid));

                if constexpr (has_fixed_update<TSystemConcept>::value)
                        singlSystemManager->DEBUG_METADATA.m_fixedUpdateIndex.push_back(
                            singlSystemManager->m_fixedUpdateFunctions.size() - 1);
                else
                        singlSystemManager->DEBUG_METADATA.m_fixedUpdateIndex.push_back(static_cast<int>(SystemIndex::invalid));

                if constexpr (has_post_update<TSystemConcept>::value)
                        singlSystemManager->DEBUG_METADATA.m_postUpdateIndex.push_back(
                            singlSystemManager->m_postUpdateFunctions.size() - 1);
                else
                        singlSystemManager->DEBUG_METADATA.m_postUpdateIndex.push_back(static_cast<int>(SystemIndex::invalid));

                if constexpr (has_shutdown<TSystemConcept>::value)
                        singlSystemManager->DEBUG_METADATA.m_shutdownIndex.push_back(
                            singlSystemManager->m_shutdownFunctions.size() - 1);
                else
                        singlSystemManager->DEBUG_METADATA.m_shutdownIndex.push_back(static_cast<int>(SystemIndex::invalid));
#endif
        }
        template <typename TSystemConcept>
        inline void AttachSystem(SingletonSystemManager* singlSystemManager, EntityAdmin* admin)
        {
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
                        singlSystemManager->m_fixedUpdateTotalTimes.push_back(SingletonTimer::value_type(0));
                }
                if constexpr (has_post_update<TSystemConcept>::value)
                        singlSystemManager->m_postUpdateFunctions.push_back(TSystemConcept::PostUpdate);
                if constexpr (has_shutdown<TSystemConcept>::value)
                        singlSystemManager->m_shutdownFunctions.push_back(TSystemConcept::ShutDown);

                AttachSystem_DEBUG_INJECT_META_DATA<TSystemConcept>(singlSystemManager);
        }
} // namespace SystemUtil
