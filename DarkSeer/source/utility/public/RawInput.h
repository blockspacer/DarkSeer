#pragma once
#include <MemoryDefines.h>

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

        struct alignas(CACHE_LINE) InputFrame
        {
                KeyState               m_pressState;      //				48	B
                std::tuple<long, long> m_mouseDeltas;     //				8	B
                KeyCode                m_buttonSignature; // buttonId //	2	B
                int16_t                m_scrollDelta;     //				2	B
                TransitionState        m_transitionState; // up or down //	1	B
                char                   m_padding[3];

                InputFrame() :
                    m_pressState(),
                    m_mouseDeltas(),
                    m_buttonSignature(),
                    m_scrollDelta(),
                    m_transitionState(),
                    m_padding()
                {}
        };

        struct InputBuffer
        {
            public:
                struct iterator
                {
                    private:
                        using value_type      = InputFrame;
                        using difference_type = int64_t;
                        using pointer         = InputFrame*;
                        using reference       = InputFrame&;

                        const InputBuffer& m_owner;
                        int64_t            m_currIndex;

                        inline iterator(const InputBuffer& owner, int64_t index) : m_owner(owner), m_currIndex(index)
                        {}
                        inline iterator(const iterator& other) : m_owner(other.m_owner), m_currIndex(other.m_currIndex)
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
                                return iterator(m_owner, m_currIndex - rhs);
                        }
                        inline iterator operator+(int64_t rhs) const
                        {
                                return iterator(m_owner, m_currIndex + rhs);
                        }
                        inline const InputFrame& operator*() const
                        {
                                return m_owner.m_inputFrames[InputBuffer::MASK & m_currIndex];
                        }
                        inline const InputFrame* operator->() const
                        {
                                return &m_owner.m_inputFrames[InputBuffer::MASK & m_currIndex];
                        }
                        inline bool operator==(const iterator& other) const
                        {
                                return m_currIndex == other.m_currIndex;
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
                        friend struct InputBuffer;
                };

            private:
                //================================================================
                // Multithreaded Circular Buffer (Single Producer - Single Consumer)
                static constexpr uint64_t MAX_INPUT_FRAMES_PER_FRAME = saturatePowerOf2(10000U);
                static constexpr uint64_t MASK                       = MAX_INPUT_FRAMES_PER_FRAME - 1;
                InputFrame*               m_inputFrames;
                volatile int64_t          m_bottom;
                volatile int64_t          m_top;
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
                void push_back(InputFrame inputFrame);
                void emplace_back(std::tuple<long, long> mouseDeltas,
                                  KeyCode                buttonSignature,
                                  int16_t                scrollDelta,
                                  TransitionState        transitionState);
                //================================================================
                // consumer thread access
                void Signal();

                inline iterator begin() const
                {
                        return iterator(*this, m_currFrametop);
                }
                inline iterator end() const
                {
                        return iterator(*this, m_currFrameBottom);
                }
                inline bool empty() const
                {
                        return begin() == end();
                }

                // previous consumer function, kept for reference
                [[deprecated]] void ProcessAll();

            private:
                inline InputFrame& back()
                {
                        return m_inputFrames[m_bottom - 1 & MASK];
                }
                //================================================================
        };

        inline namespace Globals
        {
                inline InputBuffer g_inputBuffer;
        }
} // namespace RawInput