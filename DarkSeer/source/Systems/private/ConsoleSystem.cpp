#include <ConsoleSystem.h>
#include <ConsoleUtility.h>

#include <SingletonConsole.h>
void ConsoleSystem::Initialize(EntityAdmin* entityAdmin)
{
        auto singlConsole                    = entityAdmin->GetSingletonConsole();
        singlConsole->m_prevForegroundWindow = 0;
        ConsoleUtil::InitializeConsole();
        ConsoleUtil::DisableQuickEdit();
}
