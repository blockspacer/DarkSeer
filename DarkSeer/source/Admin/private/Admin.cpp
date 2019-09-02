#include <Admin.h>
#include <ECSMemory.h>
#include <InputUtility.h>
#include <DSWindows.h>
#include <Console.h>

#include <SingletonInput.h>
#include <SingletonWindow.h>
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
        InitializeConsole();
        Console::DisableQuickEdit();

        ECSInitialize();
        m_singletonInput = static_cast<SingletonInput*>(ECSAllocateSingleton(sizeof(SingletonInput)));
        m_singletonWindow = static_cast<SingletonWindow*>(ECSAllocateSingleton(sizeof(SingletonWindow)));

        auto mainWindow = CreateWindow().Title("DarkSeer").Size(percent(50, 50)).Position(percent(25, 25)).Finalize();
        m_singletonWindow->m_mainHwnd = mainWindow.m_hwnd;
        m_singletonWindow->m_mainWndProc = mainWindow.m_wndproc;
        mainWindow.Show();

        InputUtil::InitializeInputBuffer(m_singletonInput);
        InputUtil::InitializeInputWndProc(m_singletonInput, m_singletonWindow);
}

void Admin::Shutdown()
{
        InputUtil::ReleaseInputBufferMemory(m_singletonInput);
}
