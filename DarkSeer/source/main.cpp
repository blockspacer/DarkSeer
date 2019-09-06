//// Directx Initialize
// EnableDebugLayer();
// auto MainAdapter = GetAdapter(false);
// auto MainDevice  = CreateDevice(MainAdapter);
//#include "../resource1.h"
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
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

        g_userEntityAdmin.Initialize();
        g_userEntityAdmin.ShutDown();
        //auto singlSysManager = g_userEntityAdmin.GetSingletonSystemManager();
        //auto singlTimer      = g_userEntityAdmin.GetSingletonTimer();

        //g_userEntityAdmin.AttachSystem<ConsoleSystem>(singlSysManager);
        //g_userEntityAdmin.AttachSystem<SystemConceptB>(singlSysManager);
        //g_userEntityAdmin.LaunchSystemUpdateLoop(singlTimer, singlSysManager);

        return 0;
}
