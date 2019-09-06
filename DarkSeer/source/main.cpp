//// Directx ComponentsInitialize
// EnableDebugLayer();
// auto MainAdapter = GetAdapter(false);
// auto MainDevice  = CreateDevice(MainAdapter);
//#include "../resource1.h"
#include <Console.h>
#include <SingletonInput.h>
#include <SingletonSystemManager.h>
#include <SingletonTimer.h>
#include <SingletonWindow.h>
struct ConsoleSystem
{
        static void Initialize(EntityAdmin* entityAdmin)
        {
                ConsoleUtil::InitializeConsole();
                ConsoleUtil::DisableQuickEdit();
        }
};

struct DebugFixedUpdateSystem
{
        static constexpr SingletonTimer::value_type TickRate = SingletonTimer::value_type(100);

        static void FixedUpdate(EntityAdmin* entityAdmin)
        {
                auto singlTimer         = entityAdmin->GetSingletonTimer();
                auto singlSystemManager = entityAdmin->GetSingletonSystemManager();
                std::cout << "[t]\t" << singlTimer->m_totalTime.count() << std::endl;
                std::cout << "[P]\t" << singlTimer->m_fixedTotalTime.count() << std::endl;
                std::cout << std::endl;
        }
};

struct InputSystem
{
        static void Initialize(EntityAdmin* entityAdmin)
        {}
        static void PreUpdate(EntityAdmin* entityAdmin)
        {
                auto singlInput = entityAdmin->GetSingletonInput();
                singlInput->m_inputBuffer.Signal();
                for (auto& inputFrame : singlInput->m_inputBuffer)
                {
                        std::string echo;
                        if (inputFrame.IsKeyPress(KeyCode::C))
                        {
                                ConsoleUtil::Begin();
                                std::cout << "enter a string\n";
                                std::cin >> echo;
                                for (auto& itr : echo)
                                        itr = std::toupper(itr);
                                std::cout << echo << std::endl;
                                std::cout << "enter anything to continue\n";
                                std::cin >> echo;
                                ConsoleUtil::End();
                        }
                }
        }
};


#include <InputUtility.h>
#include <WindowUtility.h>

int WINAPI WinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE, _In_ LPSTR _pCmdLine, _In_ int _nCmdShow)
{
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

        g_userEntityAdmin.ComponentsInitialize();

        auto singlInput  = g_userEntityAdmin.GetSingletonInput();
        auto singlWindow = g_userEntityAdmin.GetSingletonWindow();
        g_userEntityAdmin.WindowsInitialize(singlInput, singlWindow);

        auto singlSysManager = g_userEntityAdmin.GetSingletonSystemManager();
        auto singlTimer      = g_userEntityAdmin.GetSingletonTimer();
        g_userEntityAdmin.SystemsAttach<InputSystem>(singlSysManager);
        g_userEntityAdmin.SystemsAttach<ConsoleSystem>(singlSysManager);
        g_userEntityAdmin.SystemsAttach<DebugFixedUpdateSystem>(singlSysManager);
        g_userEntityAdmin.SystemsLaunch(singlTimer, singlSysManager);
        g_userEntityAdmin.WindowsLaunch(singlWindow);
        g_userEntityAdmin.SystemsShutdown(singlSysManager);
        g_userEntityAdmin.ComponentsShutdown();
        return 0;
}
