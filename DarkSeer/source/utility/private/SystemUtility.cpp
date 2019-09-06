#include <SystemUtility.h>
#include <TimerUtility.h>

#include <SingletonSystemManager.h>
#include <SingletonTimer.h>
#include <SingletonWindow.h>

namespace SystemUtil
{
        void Shutdown(SingletonWindow* singlWindow, SingletonSystemManager* singlSystemManager)
        {
                singlSystemManager->m_systemManagerThread.join();
                singlWindow->m_dispatchMessages = false;
        }
} // namespace SystemUtil