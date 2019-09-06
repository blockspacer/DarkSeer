#include <Console.h>
#include <ECSMemory.h>
#include <Engine.h>
#include <EntityAdmin.h>

#include <InputUtility.h>
#include <SystemUtility.h>
#include <TimerUtility.h>
#include <WindowUtility.h>

#include <SingletonConsole.h>
#include <SingletonInput.h>
#include <SingletonSystemManager.h>
#include <SingletonWindow.h>

SingletonInput* EntityAdmin::GetSingletonInput()
{
        return m_singletonInput;
}
SingletonWindow* EntityAdmin::GetSingletonWindow()
{
        return m_singletonWindow;
}
SingletonConsole* EntityAdmin::GetSingletonConsole()
{
        return m_singletonConsole;
}
SingletonSystemManager* EntityAdmin::GetSingletonSystemManager()
{
        return m_singletonSystemManager;
}
SingletonTimer* EntityAdmin::GetSingletonTimer()
{
        return m_singletonTimer;
}

void EntityAdmin::SystemUpdateLoop(EntityAdmin*            entityAdmin,
                                   SingletonSystemManager* singlSystemManager,
                                   SingletonTimer*         singlTimer)
{
        for (auto& preUpdate : singlSystemManager->m_preUpdateFunctions)
                preUpdate(entityAdmin);
        for (auto& update : singlSystemManager->m_updateFunctions)
                update(entityAdmin);

        // fixed update
        for (auto i = 0; i < singlSystemManager->m_fixedUpdateFunctions.size(); i++)
        {
                singlTimer->m_fixedUpdateIndex = i;

                SingletonTimer::value_type::rep numSteps =
                    singlTimer->m_deltaTime.count() / singlSystemManager->m_fixedUpdateTickRates[i].count();

                SingletonTimer::value_type timerDifference =
                    singlTimer->m_totalTime - ((numSteps * singlSystemManager->m_fixedUpdateTickRates[i]) +
                                               singlSystemManager->m_fixedUpdateTotalTimes[i]);

                SingletonTimer::value_type::rep numAccumulationSteps =
                    timerDifference.count() / singlSystemManager->m_fixedUpdateTickRates[i].count();

                numSteps += numAccumulationSteps;

                for (auto j = 1; j <= numSteps; j++)
                {
                        singlSystemManager->m_fixedUpdateTotalTimes[i] += singlSystemManager->m_fixedUpdateTickRates[i];
                        singlSystemManager->m_fixedUpdateFunctions[i](entityAdmin);
                }

                // simulate frame going long
                std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000));
        }

        for (auto& postUpdate : singlSystemManager->m_postUpdateFunctions)
                postUpdate(entityAdmin);
}

void EntityAdmin::LaunchSystemUpdateLoopInternal(EntityAdmin*            entityAdmin,
                                                 SingletonTimer*         singlTimer,
                                                 SingletonSystemManager* singlSystemManager)
{
        for (const auto& initialize : singlSystemManager->m_initializeFunctions)
        {
                initialize(entityAdmin);
        }
        singlTimer->m_start              = std::chrono::high_resolution_clock::now();
        singlSystemManager->m_runSystems = true;
        while (singlSystemManager->m_runSystems)
        {
                TimerUtil::Signal(singlTimer);
                SystemUpdateLoop(entityAdmin, singlSystemManager, singlTimer);
        }
        for (const auto& shutdown : singlSystemManager->m_shutdownFunctions)
        {
                shutdown(entityAdmin);
        }
}

void EntityAdmin::LaunchSystemUpdateLoop(SingletonTimer* singlTimer, SingletonSystemManager* singlSystemManager)
{
        singlSystemManager->m_systemManagerThread =
            std::thread(LaunchSystemUpdateLoopInternal, this, singlTimer, singlSystemManager);

        auto singlWindow = m_singletonWindow;

        singlWindow->m_dispatchMessages = true;
        while (singlWindow->m_dispatchMessages)
        {
                MSG msg{};
                while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
                {
                        TranslateMessage(&msg);
                        DispatchMessageA(&msg);
                }
        }
}

void EntityAdmin::Initialize()
{
        ECSInitialize();

        m_singletonInput         = static_cast<SingletonInput*>(ECSAllocateSingleton(sizeof(SingletonInput)));
        m_singletonWindow        = static_cast<SingletonWindow*>(ECSAllocateSingleton(sizeof(SingletonWindow)));
        m_singletonConsole       = static_cast<SingletonConsole*>(ECSAllocateSingleton(sizeof(SingletonConsole)));
        m_singletonSystemManager = static_cast<SingletonSystemManager*>(ECSAllocateSingleton(sizeof(SingletonSystemManager)));
        m_singletonTimer         = static_cast<SingletonTimer*>(ECSAllocateSingleton(sizeof(SingletonTimer)));
        new (m_singletonInput) SingletonInput();
        new (m_singletonSystemManager) SingletonSystemManager();
}