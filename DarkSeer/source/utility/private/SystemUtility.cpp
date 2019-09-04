#include <SystemUtility.h>
#include <TimerUtility.h>

#include <SingletonSystemManager.h>
#include <SingletonTimer.h>
#include <SingletonWindow.h>

namespace SystemUtil
{
        void SystemUpdateLoop(SingletonSystemManager* singlSystemManager, SingletonTimer* singlTimer)

        {
                for (auto& preUpdate : singlSystemManager->m_preUpdateFunctions)
                        preUpdate();
                for (auto& update : singlSystemManager->m_updateFunctions)
                        update();

                // fixed update
                for (auto i = 0; i < singlSystemManager->m_fixedUpdateFunctions.size(); i++)
                {
                        singlTimer->m_fixedUpdateIndex = i;

                        SingletonTimer::value_type::rep numSteps =
                            singlTimer->m_deltaTime.count() / singlSystemManager->m_fixedUpdateTickRates[i].count();

                        SingletonTimer::value_type timerDifference =
                            singlTimer->m_totalTime - ((numSteps * singlSystemManager->m_fixedUpdateTickRates[i]) +
                                                       singlSystemManager->m_fixedUpdateTotalTimes[i]);

                        SingletonTimer::value_type::rep numAccumulationSteps =
                            timerDifference.count() / singlSystemManager->m_fixedUpdateTickRates[i].count();

                        numSteps += numAccumulationSteps;

                        for (auto j = 1; j <= numSteps; j++)
                        {
                                singlSystemManager->m_fixedUpdateTotalTimes[i] += singlSystemManager->m_fixedUpdateTickRates[i];
                                singlSystemManager->m_fixedUpdateFunctions[i]();
                        }

                        // simulate frame going long
                        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000));
                }

                for (auto& postUpdate : singlSystemManager->m_postUpdateFunctions)
                        postUpdate();
        }

        void LaunchSystemUpdateLoop(SingletonTimer* singlTimer, SingletonSystemManager* singlSystemManager)
        {
                for (const auto& initialize : singlSystemManager->m_initializeFunctions)
                {
                        initialize();
                }
                singlTimer->m_start              = std::chrono::high_resolution_clock::now();
                singlSystemManager->m_runSystems = true;
                while (singlSystemManager->m_runSystems)
                {
                        TimerUtil::Signal(singlTimer);
                        SystemUtil::SystemUpdateLoop(singlSystemManager, singlTimer);
                }
                for (const auto& shutdown : singlSystemManager->m_shutdownFunctions)
                {
                        shutdown();
                }
        }

        void Shutdown(SingletonWindow* singlWindow, SingletonSystemManager* singlSystemManager)
        {
                singlSystemManager->m_systemManagerThread.join();
                singlWindow->m_dispatchMessages = false;
        }
} // namespace SystemUtil