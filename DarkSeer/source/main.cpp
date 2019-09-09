//// Directx ComponentsInitialize
// EnableDebugLayer();
// auto MainAdapter = GetAdapter(false);
// auto MainDevice  = CreateDevice(MainAdapter);
//#include "../resource1.h"
#include <ConsoleSystem.h>
#include <DebugFixedUpdateSystem.h>
#include <InputSystem.h>

int WINAPI WinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE, _In_ LPSTR _pCmdLine, _In_ int _nCmdShow)
{
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        try
        {
                g_userEntityAdmin.ComponentsInitialize();
                auto singlInput  = g_userEntityAdmin.GetSingletonInput();
                auto singlWindow = g_userEntityAdmin.GetSingletonWindow();
                auto singlSysManager = g_userEntityAdmin.GetSingletonSystemManager();

                g_userEntityAdmin.WindowsInitialize(singlInput, singlWindow);
                g_userEntityAdmin.SystemsAttach<InputSystem>(singlSysManager);
                g_userEntityAdmin.SystemsAttach<ConsoleSystem>(singlSysManager);
                g_userEntityAdmin.SystemsAttach<DebugFixedUpdateSystem>(singlSysManager);
        }
        catch (const std::exception& except)
        {
                MessageBox(0, (std::string("Initialization error: ") + except.what()).c_str(), 0, 0);
                g_userEntityAdmin.ComponentsShutdown();
                return 0;
		}

        auto singlSysManager = g_userEntityAdmin.GetSingletonSystemManager();
        auto singlTimer      = g_userEntityAdmin.GetSingletonTimer();
        auto singlWindow     = g_userEntityAdmin.GetSingletonWindow();
        g_userEntityAdmin.SystemsLaunch(singlTimer, singlSysManager);
        g_userEntityAdmin.WindowsLaunch(singlWindow);
        g_userEntityAdmin.SystemsShutdown(singlSysManager);
        g_userEntityAdmin.ComponentsShutdown();
        return 0;
}
