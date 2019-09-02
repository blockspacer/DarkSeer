#include <Admin.h>
#include <Console.h>
#include <DSWindows.h>
#include <ECSMemory.h>
#include <Engine.h>
#include <InputUtility.h>
#include <WindowUtility.h>

#include <SingletonInput.h>
#include <SingletonWindow.h>

void Admin::LaunchMessageLoop()
{
        while (m_singletonWindow->m_dispatchMessages)
        {
                MSG msg{};
                while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
                {
                        TranslateMessage(&msg);
                        DispatchMessageA(&msg);
                }
        }
}

SingletonInput* Admin::GetSingletonInput()
{
        return m_singletonInput;
}

SingletonWindow* Admin::GetSingletonWindow()
{
        return m_singletonWindow;
}

void Admin::Initialize()
{
        InputUtil::RegisterDefaultRawInputDevices();

        InitializeConsole();
        Console::DisableQuickEdit();

        ECSInitialize();
        m_singletonInput  = static_cast<SingletonInput*>(ECSAllocateSingleton(sizeof(SingletonInput)));
        m_singletonWindow = static_cast<SingletonWindow*>(ECSAllocateSingleton(sizeof(SingletonWindow)));

        auto window = WindowUtil::WindowProxy().Title("DarkSeer").Size(percent(50, 50)).Position(percent(25, 25)).Create();
        m_singletonWindow->m_mainHwnd = window.m_hwnd;
		ShowWindow(m_singletonWindow->m_mainHwnd, SW_SHOW);

        InputUtil::InitializeInputBuffer(m_singletonInput);
        InputUtil::InitializeInputWndProc(m_singletonInput, m_singletonWindow);

        LaunchEngine();
        LaunchMessageLoop();
        PostShutDown();
}

void Admin::ShutDown()
{
        ShutdownEngine();
        m_singletonWindow->m_dispatchMessages = false;
}

void Admin::PostShutDown()
{
        InputUtil::ReleaseInputBufferMemory(m_singletonInput);
}
