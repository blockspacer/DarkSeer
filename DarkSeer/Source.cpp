#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>
#include <thread>

#include "Utility/EngineWindows.h"
#include "Utility/Math.h"
#include "Utility/RawInput.h"
#include "Utility/Console.h"
#include "Utility/Engine.h"

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
 
// D3D12 extension library.
#include "3rdParty/d3dx12.h"
// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft::WRL;

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>

#undef min
#undef max

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
