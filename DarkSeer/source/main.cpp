#include <DSWindows.h>
#include <Engine.h>
#include <InputUtility.h>
#include <SingletonWindow.h>

//// Directx Initialize
// EnableDebugLayer();
// auto MainAdapter = GetAdapter(false);
// auto MainDevice  = CreateDevice(MainAdapter);

int WINAPI WinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE, _In_ LPSTR _pCmdLine, _In_ int _nCmdShow)
{

        g_userAdmin.Initialize();

        return 0;
}
