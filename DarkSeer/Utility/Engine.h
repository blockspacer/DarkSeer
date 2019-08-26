#pragma once
#include <stdint.h>
#include <thread>

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
                while (!g_engineShutdown)
                {
                        g_inputBuffer.PopAll();
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