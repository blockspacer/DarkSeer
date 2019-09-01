#pragma once
#include "RawInput.h"
                                             
inline namespace Engine
{
        inline namespace EngineGlobals
        {
                volatile bool g_engineShutdown = false;
                uint64_t      g_frameCounter   = 0;
                std::thread   g_engineThread;
        } // namespace EngineGlobals

        inline void EngineMain()
        {
                bool consoleActivated = false;
                while (!g_engineShutdown)
                {
                        g_inputBuffer.Signal();

                        for (auto& itr : g_inputBuffer)
                        {
                                // auto [x, y] = itr.m_mouseDeltas;

                                // if (x || y)
                                //        std::cout << "[" << x << "," << y << "]\t";
                                // if (itr.m_buttonSignature)
                                //{
                                //        std::cout
                                //            << buttonSignatureToString[itr.m_buttonSignature] << "("
                                //            <<
                                //            transitionStateToString[static_cast<std::underlying_type<TransitionState>::type>(
                                //                   itr.m_transitionState)]
                                //            << ")"
                                //            << "\t";
                                //}
                                // if (itr.m_scrollDelta)
                                //        std::cout << "(" << itr.m_scrollDelta << ")";
                                if (itr.m_pressState.ShiftLeft && itr.m_buttonSignature == KeyCode::C && itr.m_transitionState == TransitionState::Down && !consoleActivated)
                                {
                                        consoleActivated = true;
                                        std::cin.clear();
                                        std::cin.ignore(std::numeric_limits<std::streamsize>::max());

                                        std::cout << "enter vector values:\n";
                                        float vector[3];
                                        std::cout << "x:";
                                        std::cin >> vector[0];
                                        std::cout << "y:";
                                        std::cin >> vector[1];
                                        std::cout << "z:";
                                        std::cin >> vector[2];
                                        std::cout << "Vector: " << vector[0] << "," << vector[1] << "," << vector[2]
                                                  << std::endl;
                                        consoleActivated = false;
                                }
                        }
                        // if (!g_inputBuffer.empty())
                        //        std::cout << std::endl;
                }

                g_frameCounter++;
        }

        inline void LaunchEngine()
        {
                g_engineThread = std::thread(EngineMain);
        }

        inline void ShutdownEngine()
        {
                g_engineShutdown = true;
                g_engineThread.join();
        }
} // namespace Engine