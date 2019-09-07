#pragma once
struct KeyCodeSet
{
        inline KeyCodeSet(std::initializer_list<KeyCode> keyCodes)
        {
                m_keyCodeSetInternal = _mm256_setzero_si256();
                for (auto& keyCode : keyCodes)
                        m_keyCodeSetInternal =
                            _mm256_or_si256(m_keyCodeSetInternal, MM256FlagsLUT[to_underlying_type(keyCode)]);
        }

    private:
        __m256i m_keyCodeSetInternal;
        friend struct InputFrame;
};

struct alignas(CACHE_LINE) InputFrame
{
        KeyStateLow            m_keyStateLow;     //				32	B
        KeyStateHigh           m_keyStateHigh;    //				16	B
        std::tuple<long, long> m_mouseDeltas;     //				8	B
        KeyCode                m_buttonSignature; // buttonId //	2	B
        int16_t                m_scrollDelta;     //				2	B
        KeyTransition          m_transitionState; // up or down //	1	B

        static constexpr auto DATA_SIZE = sizeof(m_keyStateLow) + sizeof(m_keyStateHigh) + sizeof(m_mouseDeltas) +
                                          sizeof(m_buttonSignature) + sizeof(m_scrollDelta) + sizeof(m_transitionState);
        static_assert(DATA_SIZE <= CACHE_LINE);
        std::enable_if<DATA_SIZE - CACHE_LINE != 0, char>::type m_padding[CACHE_LINE - DATA_SIZE];

        inline bool IsKeyPressFrame(KeyCode keyCode) const
        {
                return m_buttonSignature == keyCode && m_transitionState == KeyTransition::Down;
        }
        inline bool IsKeyReleaseFrame(KeyCode keyCode) const
        {
                return m_buttonSignature == keyCode && m_transitionState == KeyTransition::Up;
        }
        inline void SetKeyHeld(KeyCode keyCode)
        {
                reinterpret_cast<__m256i&>(m_keyStateLow) =
                    _mm256_or_si256(reinterpret_cast<__m256i&>(m_keyStateLow), MM256FlagsLUT[to_underlying_type(keyCode)]);
        }
        inline void SetKeyReleased(KeyCode keyCode)
        {
                reinterpret_cast<__m256i&>(m_keyStateLow) =
                    _mm256_andnot_si256(MM256FlagsLUT[to_underlying_type(keyCode)], reinterpret_cast<__m256i&>(m_keyStateLow));
        }
        inline bool IsKeyCodeSetHeld(KeyCodeSet keyCodeSet) const
        {
                auto query = keyCodeSet.m_keyCodeSetInternal;
                // bitwise and all 256 bits in parallel
                auto masked = _mm256_and_si256(query, reinterpret_cast<const __m256i&>(m_keyStateLow));
                // piecewise compare the 256 bits in 64 bit chunks
                auto compared = _mm256_cmpeq_epi64(masked, query);
                // check if all 64 bit chunks resulted in true
                auto result = _mm256_testc_si256(compared, _mm256_set1_epi8(0xff));
                return static_cast<bool>(result);
        }

        inline bool IsKeyHeld(KeyCode keyCode) const
        {
                auto query = MM256FlagsLUT[to_underlying_type(keyCode)];
                // bitwise and all 256 bits in parallel
                auto masked = _mm256_and_si256(query, reinterpret_cast<const __m256i&>(m_keyStateLow));
                // piecewise compare the 256 bits in 64 bit chunks
                auto compared = _mm256_cmpeq_epi64(masked, query);
                // check if all 64 bit chunks resulted in true
                auto result = _mm256_testc_si256(compared, _mm256_set1_epi8(0xff));
                return static_cast<bool>(result);
        }

        InputFrame() :
            m_keyStateHigh(),
            m_keyStateLow(),
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
                // using value_type      = InputFrame;
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

                inline bool IsKeyBeginPressFrame(KeyCode keyCode) const
                {
                        return m_owner.m_inputFrames[m_currIndex].IsKeyPressFrame(keyCode) &&
                               !m_owner.m_inputFrames[m_currIndex - 1].IsKeyHeld(keyCode);
                }
                inline bool IsKeyPressFrame(KeyCode keyCode) const
                {
                        return (**this).IsKeyPressFrame(keyCode);
                }
                inline bool IsKeyReleaseFrame(KeyCode keyCode) const
                {
                        return (**this).IsKeyReleaseFrame(keyCode);
                }
                inline bool IsKeyHeld(KeyCode keyCode) const
                {
                        return (**this).IsKeyHeld(keyCode);
                }
                inline bool IsKeyReleased(KeyCode keyCode) const
                {
                        return !(**this).IsKeyHeld(keyCode);
                }
                friend struct InputBuffer;
        };

        //================================================================
        // Multithreaded Circular Buffer (Single Producer - Single Consumer)
        static constexpr uint64_t MAX_INPUT_FRAMES_PER_FRAME = saturatePowerOf2(10000U);
        static constexpr uint64_t MASK                       = MAX_INPUT_FRAMES_PER_FRAME - 1;
        InputFrame*               m_inputFrames;
        volatile int64_t          m_bottom;
        volatile int64_t          m_top;
        //================================================================
        // Signal variables (per-input-update freeze frame captures of m_bottom and m_top)
        const int64_t m_currFrameBottom;
        const int64_t m_currFrameTop;
        //================================================================

        //================================================================
        // producer thread mutators/accessors
        void                     push_back(InputFrame inputFrame);
        void                     emplace_back(std::tuple<long, long> mouseDeltas,
                                              KeyCode                buttonSignature,
                                              int16_t                scrollDelta,
                                              KeyTransition          transitionState);
        inline const InputFrame& back() const
        {
                return m_inputFrames[m_bottom - 1 & MASK];
        }
        //================================================================
        // captures the current m_top and m_bottom for use with consumer thread accessors (call once per input frame)
        void Signal();
        // consumer thread accessors
        inline iterator begin() const
        {
                return iterator(*this, m_currFrameTop);
        }
        inline iterator end() const
        {
                return iterator(*this, m_currFrameBottom);
        }
        inline bool empty() const
        {
                return begin() == end();
        }
        //================================================================
    private:
        InputBuffer();
        ~InputBuffer();
        friend struct SingletonInput;
};

KeyCodeSet CreateKeyCodeSet(std::initializer_list<KeyCode> keyCodes);