//// Directx Initialize
// EnableDebugLayer();
// auto MainAdapter = GetAdapter(false);
// auto MainDevice  = CreateDevice(MainAdapter);
#include "../resource.h"
#include <Console.h>
#include <SingletonSystemManager.h>
#include <SingletonTimer.h>
#include <SingletonWindow.h>

struct ConsoleSystem
{
        static void Initialize(EntityAdmin* entityAdmin)
        {
                InitializeConsole();
                Console::DisableQuickEdit();
        }
};

struct SystemConceptB
{
        static constexpr SingletonTimer::value_type TickRate = SingletonTimer::value_type(100);

        static void FixedUpdate(EntityAdmin* entityAdmin)
        {
                std::cout << "[t]\t" << entityAdmin->GetSingletonTimer()->m_totalTime.count() << std::endl;
                std::cout << "[P]\t"
                          << entityAdmin->GetSingletonSystemManager()
                                 ->m_fixedUpdateTotalTimes[g_userEntityAdmin.GetSingletonTimer()->m_fixedUpdateIndex]
                                 .count()
                          << std::endl;
                std::cout << std::endl;
        }
};

int WINAPI WinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE, _In_ LPSTR _pCmdLine, _In_ int _nCmdShow)
{
        g_userEntityAdmin.Initialize();

        auto singlSysManager = g_userEntityAdmin.GetSingletonSystemManager();
        auto singlTimer      = g_userEntityAdmin.GetSingletonTimer();

        g_userEntityAdmin.AttachSystem<ConsoleSystem>(singlSysManager);
        g_userEntityAdmin.AttachSystem<SystemConceptB>(singlSysManager);
        g_userEntityAdmin.LaunchSystemUpdateLoop(singlTimer, singlSysManager);
		//ConsoleSystem::Initialize;

        //g_userEntityAdmin.AttachSystem<SystemConceptB>(singlSysManager);
        //g_userEntityAdmin.LaunchSystemUpdateLoop(singlTimer, singlSysManager);

        // InitializeConsole();
        // Console::DisableQuickEdit();

        // InputUtil::RegisterDefaultRawInputDevices();
        // WindowUtil::CreateAndShowMainWindow(m_singletonWindow);
        // InputUtil::InitializeInputBuffer(m_singletonInput);
        // InputUtil::InitializeInputWndProc(m_singletonInput, m_singletonWindow);

        // LaunchSystems(m_singletonSystemManager);
        // LaunchWnproc(m_singletonWindow);
        // PostShutDown(m_singletonInput);

        //// input system shutdown
        // entityAdmin->GetSingletonWindow()->m_dispatchMessages = false;
        //// input system postshutdown
        // InputUtil::ReleaseInputBufferMemory(singlInput);

        return 0;
}
