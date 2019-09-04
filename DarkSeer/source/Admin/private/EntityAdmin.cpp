#include <Console.h>
#include <ECSMemory.h>
#include <Engine.h>
#include <EntityAdmin.h>

#include <InputUtility.h>
#include <SystemUtility.h>
#include <WindowUtility.h>

#include <SingletonConsole.h>
#include <SingletonInput.h>
#include <SingletonSystemManager.h>
#include <SingletonWindow.h>

void EntityAdmin::LaunchSystems(SingletonSystemManager* singlSystemManager)
{
        singlSystemManager->m_runSystems = true;
        std::atomic_thread_fence(std::memory_order_seq_cst);
        new (&singlSystemManager->m_systemManagerThread)
            std::thread(SystemUtil::LaunchSystemUpdateLoop, m_singletonTimer, m_singletonSystemManager);
}

void EntityAdmin::LaunchWnproc(SingletonWindow* singlWindow)
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

struct SystemConceptB
{
        static constexpr SingletonTimer::value_type TickRate = SingletonTimer::value_type(100);

        inline static void FixedUpdate()
        {
                std::cout << "[t]\t" << g_userEntityAdmin.GetSingletonTimer()->m_totalTime.count() << std::endl;
                std::cout << "[P]\t"
                          << g_userEntityAdmin.GetSingletonSystemManager()
                                 ->m_fixedUpdateTotalTimes[g_userEntityAdmin.GetSingletonTimer()->m_fixedUpdateIndex]
                                 .count()
                          << std::endl;
                std::cout << std::endl;
        }
};

void EntityAdmin::Initialize()
{
        ECSInitialize();
        m_singletonInput         = static_cast<SingletonInput*>(ECSAllocateSingleton(sizeof(SingletonInput)));
        m_singletonWindow        = static_cast<SingletonWindow*>(ECSAllocateSingleton(sizeof(SingletonWindow)));
        m_singletonConsole       = static_cast<SingletonConsole*>(ECSAllocateSingleton(sizeof(SingletonConsole)));
        m_singletonSystemManager = static_cast<SingletonSystemManager*>(ECSAllocateSingleton(sizeof(SingletonSystemManager)));
        m_singletonTimer         = static_cast<SingletonTimer*>(ECSAllocateSingleton(sizeof(SingletonTimer)));

        InitializeConsole();
        Console::DisableQuickEdit();

        InputUtil::RegisterDefaultRawInputDevices();
        WindowUtil::CreateAndShowMainWindow(m_singletonWindow);
        InputUtil::InitializeInputBuffer(m_singletonInput);
        InputUtil::InitializeInputWndProc(m_singletonInput, m_singletonWindow);
        SystemUtil::AttachSystem<SystemConceptB>(m_singletonSystemManager);

        LaunchSystems(m_singletonSystemManager);
        LaunchWnproc(m_singletonWindow);
        PostShutDown(m_singletonInput);
}

void EntityAdmin::Update()
{}

void EntityAdmin::ShutDown()
{
        m_singletonSystemManager->m_runSystems = false;
        m_singletonWindow->m_dispatchMessages  = false;
        m_singletonSystemManager->m_systemManagerThread.join();
}

void EntityAdmin::PostShutDown(SingletonInput* singlInput)
{
        InputUtil::ReleaseInputBufferMemory(singlInput);
}