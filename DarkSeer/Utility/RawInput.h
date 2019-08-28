#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>
#include <atomic>
#include <iostream>
#include <memory>
#include <numeric>
#include <tuple>
#include <cassert>
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
                struct KeyState
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
                KeyState               m_pressState;      //				48	B
                std::tuple<long, long> m_mouseDeltas;     //				8	B
                ButtonSignature        m_buttonSignature; // buttonId //	2	B
                int16_t                m_scrollDelta;     //				2	B
                TransitionState        m_transitionState; // up or down //	1	B
                char                   m_padding[3];
        };

        struct InputBuffer
        {
            private:
                //================================================================
                // Multithreaded Circular Buffer (Single Producer - Single Consumer)
                static constexpr uint64_t MAX_INPUT_FRAMES_PER_FRAME = saturatePowerOf2(10000U);
                static constexpr uint64_t MASK                       = MAX_INPUT_FRAMES_PER_FRAME - 1;
                InputFrame*               m_inputFrames;
                volatile int64_t          m_bottom;
                volatile int64_t          m_top;

                KeyState                  m_lastPressState;
                //================================================================
                // Wnd proc
                WNDPROC        m_parentWndProc;
                HWND           m_parentHWND;
                static LRESULT CALLBACK InputWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);
                //================================================================
                // range loop
                const int64_t m_currFrameBottom;
                const int64_t m_currFrametop;

            public:
                //================================================================
                InputBuffer();
                void Initialize(HWND hwnd);
                void ShutDown();
                //================================================================
                // producer thread access
                void Push(InputFrame inputFrame);
                //================================================================
                // consumer thread access
                void            BeginFrame();

                struct iterator
                {
                    private:
                        using iterator_category = std::random_access_iterator_tag;
                        using value_type        = InputFrame;
                        using difference_type   = int64_t;
                        using pointer           = InputFrame*;
                        using reference         = InputFrame&;

                        const InputFrame*& m_ownerInputFramesAlias;
                        const int64_t      m_top;
                        const int64_t      m_bottom;
                        int64_t            m_currIndex;

                        inline iterator(const InputFrame*& ownerInputFrames, int64_t top, int64_t bottom, int64_t index) :
                            m_ownerInputFramesAlias(ownerInputFrames),
                            m_top(top),
                            m_bottom(bottom),
                            m_currIndex(index)
                        {}
                        inline iterator(const iterator& other) :
                            m_ownerInputFramesAlias(other.m_ownerInputFramesAlias),
                            m_top(other.m_top),
                            m_bottom(other.m_bottom),
                            m_currIndex(other.m_currIndex)
                        {}

                    public:
                        // prefix
                        inline iterator& operator++()
                        {
                                m_currIndex++;
                                return *this;
                        }
                        inline iterator& operator--()
                        {
                                m_currIndex--;
                                return *this;
                        }
                        // postfix
                        inline iterator operator++(int)
                        {
                                auto temp = iterator(*this);
                                m_currIndex++;
                                return temp;
                        }
                        inline iterator operator--(int)
                        {
                                auto temp = iterator(*this);
                                m_currIndex--;
                                return temp;
                        }
                        //
                        inline iterator& operator+=(int64_t offset)
                        {
                                m_currIndex += offset;
                                return *this;
                        }
                        inline iterator& operator-=(int64_t offset)
                        {
                                m_currIndex -= offset;
                                return *this;
                        }
                        inline iterator operator-(int64_t rhs) const
                        {
                                return iterator(m_ownerInputFramesAlias, m_top, m_bottom, m_currIndex - rhs);
                        }
                        inline iterator operator+(int64_t rhs) const
                        {
                                return iterator(m_ownerInputFramesAlias, m_top, m_bottom, m_currIndex + rhs);
                        }
                        const InputFrame& operator*() const
                        {
                                assert(m_currIndex >= m_top && m_currIndex <= m_bottom);
                                return m_ownerInputFramesAlias[InputBuffer::MASK & m_currIndex];
                        }
                        inline bool operator!=(const iterator& other) const
                        {
                                return m_currIndex != other.m_currIndex;
                        }
                        inline bool operator<=(const iterator& other) const
                        {
                                return m_currIndex <= other.m_currIndex;
                        }
                        inline bool operator>=(const iterator& other) const
                        {
                                return m_currIndex >= other.m_currIndex;
                        }
                        inline bool operator<(const iterator& other) const
                        {
                                return m_currIndex < other.m_currIndex;
                        }
                        inline bool operator>(const iterator& other) const
                        {
                                return m_currIndex > other.m_currIndex;
                        }
                        friend class InputBuffer;
                };
                inline iterator begin() const
                {
                        return iterator(
                            const_cast<const InputFrame*&>(m_inputFrames), m_currFrametop, m_currFrameBottom, m_currFrametop);
                }
                inline iterator end() const
                {
                        return iterator(const_cast<const InputFrame*&>(m_inputFrames),
                                        m_currFrametop,
                                        m_currFrameBottom,
                                        m_currFrameBottom);
                }

				// previous consumer function, kept for reference
                [[deprecated]] void ProcessAll();
                //================================================================
        };

        inline namespace Globals
        {
                inline InputBuffer g_inputBuffer;
        }
} // namespace RawInput