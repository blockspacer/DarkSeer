#include <SystemAdmin.h>

#include <SingletonSystemManager.h>
#include <SingletonTimer.h>
#include <SingletonWindow.h>
#include <TimerUtility.h>

void SystemManager::ShutDown(EntityAdmin* entityAdmin)
{
        entityAdmin->GetSingletonSystemManager()->m_runSystems = false;
        entityAdmin->GetSingletonSystemManager()->m_systemManagerThread.join();
}
