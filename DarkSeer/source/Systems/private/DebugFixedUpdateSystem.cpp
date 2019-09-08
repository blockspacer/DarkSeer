#include <DebugFixedUpdateSystem.h>

#include <SingletonTimer.h>
#include <SingletonSystemManager.h>
void DebugFixedUpdateSystem::FixedUpdate(EntityAdmin* entityAdmin)
{
        auto singlTimer         = entityAdmin->GetSingletonTimer();
        auto singlSystemManager = entityAdmin->GetSingletonSystemManager();
        std::cout << "[t]\t" << singlTimer->m_totalTime.count() << std::endl;
        std::cout << "[P]\t" << singlTimer->m_fixedTotalTime.count() << std::endl;
        std::cout << std::endl;
}
