#include <Console.h>
#include <EngineWindows.h>
#include <RawInput.h>
#include <Engine.h>

int WINAPI WinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE, _In_ LPSTR _pCmdLine, _In_ int _nCmdShow)
{
        InitializeConsole();
        DisableQuickEdit();

        g_hInstance = _hInstance;
        g_pCmdLine  = _pCmdLine;
        g_nCmdShow  = _nCmdShow;

        RegisterDefaultRawInputDevices();

        auto mainWindow = CreateWindow().Title("DarkSeer").Size(percent(50, 50)).Position(percent(25, 25)).Finalize();
        g_inputBuffer.Initialize(mainWindow.GetHWND());
        LaunchEngine();

        mainWindow.Show();

        MessageLoop();

        g_inputBuffer.ShutDown();
        ShutdownEngine();

        return 0;
}
