#include <DSWindows.h>
#include <Engine.h>
#include <InputUtility.h>

int WINAPI WinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE, _In_ LPSTR _pCmdLine, _In_ int _nCmdShow)
{
        g_hInstance = _hInstance;
        g_pCmdLine  = _pCmdLine;
        g_nCmdShow  = _nCmdShow;

        InputUtil::RegisterDefaultRawInputDevices();

        g_userAdmin.Initialize();

        // Directx Initialize
        EnableDebugLayer();
        auto MainAdapter = GetAdapter(false);
        auto MainDevice  = CreateDevice(MainAdapter);

        LaunchEngine();
        MessageLoop();

        ShutdownEngine();

        return 0;
}
