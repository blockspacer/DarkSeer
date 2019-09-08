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

    protected:
        __m256i m_keyCodeSetInternal;
        friend struct InputFrame;
};

// set of keys that must be held down but is considered OnFirstKeyPress only when the activator key was released on the previous
// input frame
struct ActivatedKeyCodeSet : public KeyCodeSet
{
        inline ActivatedKeyCodeSet(std::initializer_list<KeyCode> keyCodes, KeyCode activatorKey) : KeyCodeSet(keyCodes)
        {
                m_keyCodeSetInternal = _mm256_or_si256(m_keyCodeSetInternal, MM256FlagsLUT[to_underlying_type(activatorKey)]);
                m_activatorKeyCodeInternal = activatorKey;
        }

    protected:
        KeyCode m_activatorKeyCodeInternal;
        friend struct InputBuffer;
};

struct alignas(CACHE_LINE) InputFrame
{
        KeyStateLow   m_keyState;         //				32	B
        POINT         m_absoluteMousePos; //				8	B
        POINT         m_mouseDeltas;      //				8	B
        KeyCode       m_keyCode;          // buttonId //	2	B
        int16_t       m_scrollDelta;      //				2	B
        KeyTransition m_transitionState;  // up or down //	1	B

        static constexpr auto DATA_SIZE = sizeof(m_keyState) + sizeof(m_absoluteMousePos) + sizeof(m_mouseDeltas) +
                                          sizeof(m_keyCode) + sizeof(m_scrollDelta) + sizeof(m_transitionState);
        static_assert(DATA_SIZE <= CACHE_LINE);
        std::enable_if<DATA_SIZE - CACHE_LINE != 0, char>::type m_padding[CACHE_LINE - DATA_SIZE];

        inline void SetKeyHeld(KeyCode keyCode)
        {
                auto dbg = MM256FlagsLUT[to_underlying_type(keyCode)];
                reinterpret_cast<__m256i&>(m_keyState) =
                    _mm256_or_si256(reinterpret_cast<__m256i&>(m_keyState), MM256FlagsLUT[to_underlying_type(keyCode)]);
        }
        inline void SetKeyReleased(KeyCode keyCode)
        {
                reinterpret_cast<__m256i&>(m_keyState) =
                    _mm256_andnot_si256(MM256FlagsLUT[to_underlying_type(keyCode)], reinterpret_cast<__m256i&>(m_keyState));
        }
        inline bool IsKeyPressFrame(KeyCode keyCode) const
        {
                return m_keyCode == keyCode && m_transitionState == KeyTransition::Down;
        }
        inline bool IsKeyReleaseFrame(KeyCode keyCode) const
        {
                return m_keyCode == keyCode && m_transitionState == KeyTransition::Up;
        }
        inline bool IsKeySetHeld(KeyCodeSet keyCodeSet) const
        {
                return IsKeyCodeHeldInternal(keyCodeSet.m_keyCodeSetInternal);
        }
        inline bool IsKeyHeld(KeyCode keyCode) const
        {
                return IsKeyCodeHeldInternal(MM256FlagsLUT[to_underlying_type(keyCode)]);
        }

    private:
        inline bool IsKeyCodeHeldInternal(__m256i query) const
        {
                // bitwise and all 256 bits in parallel
                auto masked = _mm256_and_si256(query, reinterpret_cast<const __m256i&>(m_keyState));
                // piecewise compare the 256 bits in 64 bit chunks
                auto compared = _mm256_cmpeq_epi64(masked, query);
                // check if all 64 bit chunks resulted in true
                return static_cast<bool>(_mm256_testc_si256(compared, _mm256_set1_epi8('\xff')));
        }
};
constexpr auto SizeofInputFrame = sizeof(InputFrame);
//================================================================
// Multithreaded Circular Buffer (Single Producer - Single Consumer)
struct InputBuffer
{
    public:
        struct iterator_proxy;
        struct iterator
        {
            private:
                using value_type      = iterator_proxy;
                using difference_type = int64_t;
                using pointer         = iterator_proxy*;
                using reference       = iterator_proxy&;

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
                        return iterator(m_owner, m_currIndex - rhs);
                }
                inline iterator operator+(int64_t rhs) const
                {
                        return iterator(m_owner, m_currIndex + rhs);
                }
                inline const iterator_proxy& operator*() const
                {
                        return reinterpret_cast<const iterator_proxy&>(*this);
                }
                inline const iterator_proxy* operator->() const
                {
                        return reinterpret_cast<const iterator_proxy*>(this);
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
        struct iterator_proxy : iterator
        {
            private:
                inline const InputFrame& previous_input_frame() const
                {
                        return m_owner.m_inputFrames[m_currIndex - 1];
                }
                inline const InputFrame& this_input_frame() const
                {
                        return m_owner.m_inputFrames[InputBuffer::MASK & m_currIndex];
                }

            public:
                inline const InputFrame& operator*() const
                {
                        return this_input_frame();
                }
                inline const InputFrame* operator->() const
                {
                        return &this_input_frame();
                }
                inline bool IsKeyPressFrameBegin(KeyCode keyCode) const
                {
                        return this_input_frame().IsKeyPressFrame(keyCode) && !previous_input_frame().IsKeyHeld(keyCode);
                }
                inline bool IsKeyPressFrame(KeyCode keyCode) const
                {
                        return this_input_frame().IsKeyPressFrame(keyCode);
                }
                inline bool IsKeyReleaseFrame(KeyCode keyCode) const
                {
                        return this_input_frame().IsKeyReleaseFrame(keyCode);
                }
                inline bool IsKeyHeld(KeyCode keyCode) const
                {
                        return this_input_frame().IsKeyHeld(keyCode);
                }
                inline bool IsKeyReleased(KeyCode keyCode) const
                {
                        return !this_input_frame().IsKeyHeld(keyCode);
                }
                inline bool IsKeySetBeginPress(KeyCodeSet keyCodeSet) const
                {
                        return this_input_frame().IsKeySetHeld(keyCodeSet) && !previous_input_frame().IsKeySetHeld(keyCodeSet);
                }
                inline bool IsKeySetBeginPress(ActivatedKeyCodeSet keyCodeSet) const
                {
                        return this_input_frame().IsKeySetHeld(keyCodeSet) &&
                               !previous_input_frame().IsKeyHeld(keyCodeSet.m_activatorKeyCodeInternal);
                }
                inline bool IsKeySetHeld(KeyCodeSet keyCodeSet) const
                {
                        return this_input_frame().IsKeySetHeld(keyCodeSet);
                }
        };
        //================================================================
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
        // producer thread mutators/accessors
        void push_back(InputFrame inputFrame);
        void emplace_back(POINT mouseDeltas, KeyCode buttonSignature, int16_t scrollDelta, KeyTransition transitionState);
        // inline const InputFrame& back() const
        //{
        //        return m_inputFrames[m_bottom - 1 & MASK];
        //}
        inline KeyStateLow GetPrevFrameState() const
        {
                return m_inputFrames[m_bottom - 1 & MASK].m_keyState;
        }
        inline POINT GetPrevAbsMousePos() const
        {
                return m_inputFrames[m_bottom - 1 & MASK].m_absoluteMousePos;
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
