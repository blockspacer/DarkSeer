#pragma once
#include <Windows.h>
#include <stdint.h>
#include <atomic>
#include <iostream>
#include <numeric>
#include <tuple>
#include "Math.h"
#include "MemoryDefines.h"

inline namespace RawInput
{
        inline void RegisterDefaultRawInputDevices()
        {
                RAWINPUTDEVICE Rid[2];

                Rid[0].usUsagePage = 0x01;
                Rid[0].usUsage     = 0x02;
                Rid[0].dwFlags     = 0;
                // RIDEV_NOLEGACY; // adds HID mouse and also ignores legacy mouse messages
                Rid[0].hwndTarget = 0;

                Rid[1].usUsagePage = 0x01;
                Rid[1].usUsage     = 0x06;
                Rid[1].dwFlags     = 0;
                // RIDEV_NOLEGACY; // adds HID keyboard and also ignores legacy keyboard messages
                Rid[1].hwndTarget = 0;

                if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
                {
                        // registration failed. Call GetLastError for the cause of the error
                }
        }

        inline namespace Enums
        {
#undef ENUM
#define ENUM(E, V) INPUT_##E,
                // dummy enum to get number of mouse signatures
                enum class DummyEnum
                {
#include "MOUSE_SCANCODES.ENUM"
                        NUM_MOUSE_SIGNATURES
                };
#undef ENUM
#define ENUM(E, V) INPUT_##E,
                enum ButtonSignature : uint16_t
                {
#include "SCANCODES_FLAG0.ENUM"
#include "SCANCODES_FLAG1.ENUM"
#include "SCANCODES_FLAG2.ENUM"

#include "MOUSE_SCANCODES.ENUM"
                        INPUT_NUM_SCANCODE_SIGNATURES
                };
#undef ENUM
                constexpr uint8_t  INPUT_NUM_MOUSE_SCANCODES = (uint8_t)DummyEnum::NUM_MOUSE_SIGNATURES;
                constexpr uint16_t INPUT_NUM_KEYBOARD_SCANCODE_SIGNATURES =
                    INPUT_NUM_SCANCODE_SIGNATURES - INPUT_NUM_MOUSE_SCANCODES;
                constexpr uint16_t INPUT_NUM_KEYBOARD_SCANCODES = INPUT_NUM_KEYBOARD_SCANCODE_SIGNATURES / 3;
#define ENUM(E, V) #E,
                constexpr const char* buttonSignatureToString[INPUT_NUM_SCANCODE_SIGNATURES + 1]{
#include "SCANCODES_FLAG0.ENUM"
#include "SCANCODES_FLAG1.ENUM"
#include "SCANCODES_FLAG2.ENUM"

#include "MOUSE_SCANCODES.ENUM"
                    "INPUT_NUM_SCANCODE_SIGNATURE_ENUMS"};
#undef ENUM

#define ENUM(E, V) bool INPUT_##E : 1;
                struct Key
                {
                        static constexpr auto _SZ64 = 48 /*sizeof(Key)*/ / sizeof(uint64_t);
#include "SCANCODES_FLAG0.ENUM"
#include "SCANCODES_FLAG1.ENUM"
#include "SCANCODES_FLAG2.ENUM"

#include "MOUSE_SCANCODES.ENUM"

                        inline void KeyDown(ButtonSignature button)
                        {
                                uint64_t(&pressStateAlias)[_SZ64] = (uint64_t(&)[_SZ64]) * this;
                                uint64_t mask                     = 1ULL << ((uint64_t)button & (64ULL - 1ULL));
                                pressStateAlias[button >> 6] |= mask;
                        }
                        inline void KeyUp(ButtonSignature button)
                        {
                                uint64_t(&pressStateAlias)[_SZ64] = (uint64_t(&)[_SZ64]) * this;
                                uint64_t mask                     = 1ULL << ((uint64_t)button & (64ULL - 1ULL));
                                pressStateAlias[button >> 6] &= ~mask;
                        }
                };
#undef ENUM
                // must be size 2 bytes to use the Transition state to store scroll delta if scroll button signature is set
                enum TransitionState : int8_t
                {
                        INPUT_transitionStateDown,
                        INPUT_transitionStateUp,
                        INPUT_numTransitionStates
                };
                constexpr const char* transitionStateToString[INPUT_numTransitionStates]{"down", "up"};
        } // namespace Enums

        struct alignas(CACHE_LINE) InputFrame
        {
                Key                    m_pressState;      //				48	B
                std::tuple<long, long> m_mouseDeltas;     //				8	B
                ButtonSignature        m_buttonSignature; // buttonId //	2	B
                int16_t                m_scrollDelta;     //				2	B
                TransitionState        m_transitionState; // up or down //	1	B
                char                   m_padding[3];
        };

        struct InputBuffer
        {
            private:
                Key m_lastPressState;
#ifdef STRICT
                WNDPROC m_parentWndProc;
#else
                FARPROC m_parentWndProc
#endif
                HWND m_parentHWND;
                //================================================================
                static constexpr uint64_t MAX_INPUT_FRAMES_PER_FRAME = saturatePowerOf2(10000U);
                static constexpr uint64_t MASK                       = MAX_INPUT_FRAMES_PER_FRAME - 1;
                InputFrame*               m_InputFrames;
                volatile int64_t          m_bottom;
                volatile int64_t          m_top;
                //================================================================
                static LRESULT CALLBACK InputWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);

            public:
                void Initialize(HWND hwnd);

                void ShutDown();

                void Push(InputFrame inputFrame);

                void PopAll();
        };

        inline namespace Globals
        {
                inline InputBuffer g_inputBuffer;
        }
} // namespace RawInput