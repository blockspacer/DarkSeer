#pragma once
#include <Console.h>
#include <SingletonInput.h>

inline namespace Engine
{
        inline namespace EngineGlobals
        {
                volatile inline bool g_engineShutdown = false;
                inline uint64_t      g_frameCounter   = 0;
                inline std::thread   g_engineThread;
        } // namespace EngineGlobals

        inline void EngineMain()
        {
                bool consoleActivated = false;
                while (!g_engineShutdown)
                {
                        SingletonInput* singlInput = g_userEntityAdmin.GetSingletonInput();
                        singlInput->m_inputBuffer.Signal();

                        for (auto& inputFrame : singlInput->m_inputBuffer)
                        {
                                if (inputFrame.m_pressState.ShiftLeft && inputFrame.IsKeyPress(KeyCode::C) && !Console::IsActive())
                                {
                                        Console::Begin();

                                        float vector[3];

                                        std::cout << "enter vector values:\n";
                                        vector[0] = Console::GetFloatNoFail("x:");
                                        vector[1] = Console::GetFloatNoFail("y:");
                                        vector[2] = Console::GetFloatNoFail("z:");
                                        std::cout << "[ " << vector[0] << ", " << vector[1] << ", " << vector[2] << " ]\n";

                                        Console::End();
                                }
                        }
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