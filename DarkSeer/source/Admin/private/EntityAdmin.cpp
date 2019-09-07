#include <ECSMemory.h>
#include <EntityAdmin.h>

#include <InputUtility.h>
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
                SingletonTimer::value_type::rep numSteps =
                    singlTimer->m_deltaTime.count() / singlSystemManager->m_fixedUpdateTickRates[i].count();

                SingletonTimer::value_type timerDifference =
                    singlTimer->m_totalTime -
                    ((numSteps * singlSystemManager->m_fixedUpdateTickRates[i]) + singlTimer->m_fixedUpdateTotalTimes[i]);

                SingletonTimer::value_type::rep numAccumulationSteps =
                    timerDifference.count() / singlSystemManager->m_fixedUpdateTickRates[i].count();

                numSteps += numAccumulationSteps;

                for (auto j = 1; j <= numSteps; j++)
                {
                        singlTimer->m_fixedUpdateTotalTimes[i] += singlSystemManager->m_fixedUpdateTickRates[i];
                        singlTimer->m_fixedTotalTime = singlTimer->m_fixedUpdateTotalTimes[i];
                        singlSystemManager->m_fixedUpdateFunctions[i](entityAdmin);
                }
        }

        for (auto& postUpdate : singlSystemManager->m_postUpdateFunctions)
                postUpdate(entityAdmin);
}

void EntityAdmin::LaunchSystemUpdateLoopInternal(EntityAdmin*            entityAdmin,
                                                 SingletonTimer*         singlTimer,
                                                 SingletonSystemManager* singlSystemManager)
{
        singlTimer->m_fixedUpdateTotalTimes.resize(singlSystemManager->m_fixedUpdateFunctions.size());
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

void EntityAdmin::SystemsLaunch(SingletonTimer* singlTimer, SingletonSystemManager* singlSystemManager)
{
        singlSystemManager->m_systemManagerThread =
            std::thread(LaunchSystemUpdateLoopInternal, this, singlTimer, singlSystemManager);
}

void EntityAdmin::SystemsShutdown(SingletonSystemManager* singlSystemManager)
{
        singlSystemManager->m_runSystems = false;
        singlSystemManager->m_systemManagerThread.join();
}

void EntityAdmin::WindowsInitialize(SingletonInput* singlInput, SingletonWindow* singlWindow)
{
        InputUtil::RegisterDefaultRawInputDevices();
        WindowUtil::CreateAndShowMainWindow(m_singletonWindow);
        InputUtil::InitializeInputWndProc(m_singletonInput, m_singletonWindow);
}

void EntityAdmin::WindowsLaunch(SingletonWindow* singlWindow)
{
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

void EntityAdmin::WindowsShutdown(SingletonWindow* singlWindow)
{
        singlWindow->m_dispatchMessages = false;
}

void EntityAdmin::ComponentsInitialize()
{
        ECSInitialize();

        ECSAllocateSingleton(m_singletonInput);
        ECSAllocateSingleton(m_singletonSystemManager);
        ECSAllocateSingleton(m_singletonWindow);
        ECSAllocateSingleton(m_singletonConsole);
        ECSAllocateSingleton(m_singletonTimer);
}

void EntityAdmin::ComponentsShutdown()
{
        ECSShutDown();
}
