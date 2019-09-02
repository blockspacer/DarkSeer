#include <Admin.h>
#include <ECSMemory.h>
#include <InputUtility.h>
#include <DSWindows.h>
#include <Console.h>

#include <SingletonInput.h>
SingletonInput* Admin::GetSingletonInput()
{
        return m_singletonInput;
}

void Admin::Initialize()
{
        InitializeConsole();
        Console::DisableQuickEdit();

        RegisterDefaultRawInputDevices();

        ECSInitialize();
        m_singletonInput = static_cast<SingletonInput*>(ECSAllocateSingleton(sizeof(SingletonInput)));

        auto mainWindow = CreateWindow().Title("DarkSeer").Size(percent(50, 50)).Position(percent(25, 25)).Finalize();
        mainWindow.Show();
		InputUtil::InitializeInputBuffer(m_singletonInput, mainWindow.GetHWND());
}

void Admin::Shutdown()
{
        InputUtil::ReleaseInputBufferMemory(m_singletonInput);
}
