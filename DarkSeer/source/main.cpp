//// Directx ComponentsInitialize
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

struct SystemFixedUpdateTest
{
        static constexpr SingletonTimer::value_type TickRate = SingletonTimer::value_type(100);

        static void FixedUpdate(EntityAdmin* entityAdmin)
        {
                auto singlTimer         = entityAdmin->GetSingletonTimer();
                auto singlSystemManager = entityAdmin->GetSingletonSystemManager();
                std::cout << "[t]\t" << singlTimer->m_totalTime.count() << std::endl;
                std::cout << "[P]\t" << singlTimer->m_fixedTotalTime.count()
                          << std::endl;
                std::cout << std::endl;
        }
};

int WINAPI WinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE, _In_ LPSTR _pCmdLine, _In_ int _nCmdShow)
{
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

        ConsoleSystem::Initialize(0);
        g_userEntityAdmin.ComponentsInitialize();
        auto singlSysManager = g_userEntityAdmin.GetSingletonSystemManager();
        auto singlTimer      = g_userEntityAdmin.GetSingletonTimer();

        g_userEntityAdmin.SystemsAttach<ConsoleSystem>(singlSysManager);
        g_userEntityAdmin.SystemsAttach<SystemFixedUpdateTest>(singlSysManager);
        g_userEntityAdmin.SystemsLaunch(singlTimer, singlSysManager);
        g_userEntityAdmin.ComponentsShutdown();

        return 0;
}
