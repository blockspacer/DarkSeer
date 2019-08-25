#pragma once
#include <Windows.h>
#include <stdint.h>
#include <numeric>
#include <tuple>
#include "Math.h"
inline namespace RawInputE
{
        inline namespace Enums
        {
#define ENUM(E,V) INPUT_##E,
                enum ButtonSignature : uint16_t
                {
#include "SCANCODES_FLAG0.ENUM"
#include "SCANCODES_FLAG1.ENUM"
#include "SCANCODES_FLAG2.ENUM"
                        INPUT_NUM_KEYBOARD_SCANCODE_SIGNATURES,
#include "MOUSE_SCANCODES.ENUM"
                        INPUT_NUM_SCANCODE_SIGNATURES
                };
#undef ENUM
                constexpr uint16_t INPUT_NUM_KEYBOARD_SCANCODES = INPUT_NUM_KEYBOARD_SCANCODE_SIGNATURES / 3;
                constexpr uint8_t  INPUT_NUM_MOUSE_SCANCODES =
                    INPUT_NUM_SCANCODE_SIGNATURES - INPUT_NUM_KEYBOARD_SCANCODE_SIGNATURES - 1;
#define ENUM(E,V) #E,
                constexpr const char* buttonSignatureToString[INPUT_NUM_SCANCODE_SIGNATURES + 1]{
#include "SCANCODES_FLAG0.ENUM"
#include "SCANCODES_FLAG1.ENUM"
#include "SCANCODES_FLAG2.ENUM"
                    "INPUT_NUM_SCANCODE_SIGNATURES",
#include "MOUSE_SCANCODES.ENUM"
                    "INPUT_NUM_SCANCODE_SIGNATURE_ENUMS"};
#undef ENUM
                // must be size 2 bytes to use the Transition state to store scroll delta if scroll button signature is set
                enum TransitionState : uint16_t
                {
                        INPUT_transitionStateDown,
                        INPUT_transitionStateUp
                };
                constexpr const char* transitionStateToString[2]{"down", "up"};
        } // namespace Enums

        struct InputFrameE
        {
                InputFrameE()
                {}
                InputFrameE(long arg0, long arg1, TransitionState arg2, ButtonSignature arg3)
                {
                        m_mouseDeltas     = {arg0, arg1};
                        m_transitionState = arg2;
                        m_buttonSignature = arg3;
                }
                std::tuple<long, long> m_mouseDeltas;
                TransitionState        m_transitionState; // up or down
                ButtonSignature        m_buttonSignature; // buttonId
        };

        struct RawInputBufferE
        {
            private:
                static constexpr auto m_SZ_in          = 10000ULL;
                static constexpr auto m_lcm            = std::lcm(sizeof(InputFrameE), sizeof(__m256i));
                static constexpr auto m_frameChunkSize = m_lcm / sizeof(InputFrameE);
                static constexpr auto m_256ChunkSize   = m_lcm / sizeof(__m256i);
                // saturate SZ and SZ_M256 to their least common multiple sizes
                static constexpr auto m_SZ      = Math::DivideRoundUp(m_SZ_in, m_frameChunkSize) * m_frameChunkSize;
                static constexpr auto m_SZ_M256 = (m_SZ * sizeof(InputFrameE)) / sizeof(__m256i);

                static constexpr unsigned m_NUM_SWAP_BUFFERS = saturatePowerOf2(2);
                // m_NUM_SWAP_BUFFERS must be power of 2, swap() uses mersenne prime modulation

                volatile __m256i*     m_inputFramesM256;
                volatile InputFrameE* m_inputFrames;

                volatile uint64_t m_bufferSizes[m_NUM_SWAP_BUFFERS];
                unsigned          m_readSwapBufferIndex  = 0;
                unsigned          m_writeSwapBufferIndex = 1;

                void swap()
                {
                        m_readSwapBufferIndex  = m_writeSwapBufferIndex;
                        m_writeSwapBufferIndex = (m_writeSwapBufferIndex + 1ULL) & (m_NUM_SWAP_BUFFERS - 1);
                }

                bool try_push(const InputFrameE& InputFrame)
                {
                        const auto bufferWriteIndex = m_bufferSizes[m_writeSwapBufferIndex];

                        if (bufferWriteIndex == m_SZ)
                                return false;

                        auto&      inputFrameSz = const_cast<uint64_t&>(m_bufferSizes[m_writeSwapBufferIndex]);
                        const auto local_offset = (m_writeSwapBufferIndex * m_SZ) + inputFrameSz;

                        auto& inputFramePrev = const_cast<InputFrameE&>(m_inputFrames[local_offset - 1]);
                        auto& inputFrameCurr = const_cast<InputFrameE&>(m_inputFrames[local_offset]);
                        // mouse move		:	push_back or concat_back
                        if (!InputFrame.m_buttonSignature)
                        {
                                // array is empty						:		always push_back
                                if (!inputFrameSz)
                                {
                                        inputFrameCurr = InputFrame;
                                        inputFrameSz++;
                                }
                                // last push was a button press			:		push_back
                                else if (inputFramePrev.m_buttonSignature)
                                {
                                        inputFrameCurr = InputFrame;
                                        inputFrameSz++;
                                }
                                // last push was a mouse move			:		concat_back
                                else
                                {
                                        inputFramePrev.m_mouseDeltas += InputFrame.m_mouseDeltas;
                                }
                        }
                        // button press		:	always push_back
                        else
                        {
                                inputFrameCurr = InputFrame;
                                inputFrameSz++;
                        }
                        return true;
                }

                bool try_push_old(InputFrameE& rawInputFrame)
                {
                        const auto bufferWriteIndex = m_bufferSizes[m_writeSwapBufferIndex];

                        if (bufferWriteIndex == m_SZ)
                                return false;

                        // assert(bufferWriteIndex < m_SZ);

                        // UINT dwSize = 0;

                        // RAWINPUT rawInputFrame;
                        // GetRawInputData((HRAWINPUT)rawInputFrame, RID_INPUT, 0, &dwSize, sizeof(RAWINPUTHEADER));
                        // GetRawInputData((HRAWINPUT)rawInputFrame, RID_INPUT, &rawInputFrame, &dwSize,
                        // sizeof(RAWINPUTHEADER)); if (rawInputFrame.header.dwType == RIM_TYPEKEYBOARD)
                        //{
                        //        if (rawInputFrame.data.keyboard.MakeCode & 0xFF00)
                        //                std::cout << "E-CODE\n";

                        //        short button_signature_flag      = rawInputFrame.data.keyboard.Flags >> 1;
                        //        bool  transition_state = rawInputFrame.data.keyboard.Flags & 1;

                        //        std::cout << "SPEC_KEY_OFFSET:\t" << button_signature_flag << "\n";
                        //        std::cout << "TRANSITION_STATE:\t" << transition_state << "\n";
                        //        std::cout << "VKEY:\t" << rawInputFrame.data.keyboard.VKey << "\n";
                        //        std::cout << "FLAGS:\t" << rawInputFrame.data.keyboard.Flags << "\n";
                        //        std::cout << "SCAN_CODE:\t" << rawInputFrame.data.keyboard.MakeCode << "\n";
                        //        std::cout << "WINDOWS_MESSAGE:\t" << rawInputFrame.data.keyboard.Message << "\n";
                        //        std::cout << buttonSignatureToString[rawInputFrame.data.keyboard.MakeCode +
                        //                                      button_signature_flag * INPUT_NUM_KEYBOARD_SCANCODES]
                        //                  << "\n";
                        //        std::cout << std::endl;
                        //}
                        // if (rawInputFrame.header.dwType == RIM_TYPEMOUSE)
                        //{
                        //        InputFrameE _inputFrame;
                        //        memset(&_inputFrame, 0, sizeof(InputFrameE));
                        //        _inputFrame.m_mouseDeltas =
                        //            std::tuple{rawInputFrame.data.mouse.lLastX, rawInputFrame.data.mouse.lLastY};
                        //        const int16_t _buttonFlags = rawInputFrame.data.mouse.usButtonData;
                        //        std::cout << rawInputFrame.data.mouse.ulRawButtons << std::endl;
                        //        std::cout << _buttonFlags << std::endl;
                        //        std::cout << rawInputFrame.data.mouse.usButtonFlags << std::endl;
                        //        std::cout << rawInputFrame.header.wParam << std::endl;
                        //        std::cout << std::endl;
                        //        push_impl(_inputFrame);
                        //}

                        // return true;
                }

            public:
                // single consumer functions:
                void process_writes()
                {
                        const unsigned num_to_read  = m_bufferSizes[m_readSwapBufferIndex];
                        const unsigned local_offset = m_readSwapBufferIndex * m_SZ;

                        for (unsigned i = 0; i < num_to_read; i++)
                        {
                                const auto& thisFrame = const_cast<InputFrameE&>(m_inputFrames[local_offset + i]);
                                const auto [x, y]     = thisFrame.m_mouseDeltas;
                                std::cout << "x:\t" << x << std::endl;
                                std::cout << "y:\t" << y << std::endl;
                                std::cout << "buttonSignature:\t" << buttonSignatureToString[thisFrame.m_buttonSignature]
                                          << std::endl;
                                if (thisFrame.m_buttonSignature != INPUT_mouseScrollVertical)
                                        std::cout << "buttonFlag:\t" << transitionStateToString[thisFrame.m_transitionState]
                                                  << std::endl;
                                else
                                        std::cout << "mouseDelta:\t" << ((short)thisFrame.m_transitionState) << std::endl;

                        }
                        m_bufferSizes[m_readSwapBufferIndex] = 0;
                        Sleep(5000);
                }

                // single producer functions:
                void write(InputFrameE& rawInputFrame)
                {
                       /* std::cout << "x:\t" << std::get<0>(rawInputFrame.m_mouseDeltas) << std::endl;
                        std::cout << "y:\t" << std::get<1>(rawInputFrame.m_mouseDeltas) << std::endl;
                        std::cout << "buttonFlag:\t" << buttonSignatureToString[rawInputFrame.m_buttonSignature] << std::endl;
                        if (rawInputFrame.m_buttonSignature != INPUT_mouseScrollVertical)
                                std::cout << "buttonFlag:\t" << transitionStateToString[rawInputFrame.m_transitionState]
                                          << std::endl;
                        else
                                std::cout << "mouseDelta:\t" << ((short)rawInputFrame.m_transitionState) << std::endl;*/

                        if (!try_push(rawInputFrame))
                        {
                                while (m_bufferSizes[m_readSwapBufferIndex] != 0)
                                {
                                        Sleep(0);
                                }
                                // TODO // Run a job while we wait?

                                //
                                swap();
                                try_push(rawInputFrame);
                                return;
                        };
                        if (m_bufferSizes[m_readSwapBufferIndex] == 0)
                                swap();

                        Sleep(0);
                }

                void initialize()
                {
                        m_inputFramesM256 = (volatile __m256i*)_aligned_malloc(m_SZ_M256 * sizeof(__m256i) * m_NUM_SWAP_BUFFERS,
                                                                               alignof(__m256i));

                        m_inputFrames = reinterpret_cast<volatile InputFrameE*>(m_inputFramesM256);

                        reset();
                }

                void shutdown()
                {
                        _aligned_free(const_cast<__m256i*>(m_inputFramesM256));
                }

                void reset()
                {
                        auto inputFrames     = const_cast<InputFrameE*>(m_inputFrames);
                        auto inputFrames256i = reinterpret_cast<__m256i*>(inputFrames);

                        for (auto i = 0; i < m_SZ_M256 * m_NUM_SWAP_BUFFERS; i++)
                                inputFrames256i[i] = _mm256_setzero_si256();
                }

                void invalidate()
                {
                        auto inputFrames     = const_cast<InputFrameE*>(m_inputFrames);
                        auto inputFrames256i = reinterpret_cast<__m256i*>(inputFrames);

                        for (auto i = 0; i < m_SZ_M256; i++)
                                inputFrames256i[i] = _mm256_set1_epi32(-1);
                }
        };

        inline namespace Globals
        {
                 inline RawInputBufferE g_inputBufferE;
        }
} // namespace RawInputE