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
                        g_inputBuffer.BeginFrame();
                        auto inputFrames = g_inputBuffer.GetInputFrames();
                        for (auto& itr : inputFrames)
                        {
                                auto [x, y] = itr.m_mouseDeltas;

                                if (x || y)
                                        std::cout << "[" << x << "," << y << "]\t";
                                if (itr.m_buttonSignature)
                                {
                                        std::cout << buttonSignatureToString[itr.m_buttonSignature] << "("
                                                  << transitionStateToString[itr.m_transitionState] << ")"
                                                  << "\t";
                                }
                                if (itr.m_scrollDelta)
                                        std::cout << "(" << itr.m_scrollDelta << ")";
                        }

                        if (inputFrames.GetFrameCount())
                                std::cout << std::endl;
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