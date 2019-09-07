#pragma once

inline namespace Enums
{
#undef ENUM
#define ENUM(E, V) E,
        // dummy enum to get number of mouse signatures
        enum class DummyMouseCodesEnum
        {
#include <Enums/MOUSE_SCANCODES.ENUM>
                COUNT
        };
#undef ENUM
#define ENUM(E, V) E,
        enum class KeyCode : uint16_t
        {
#include <Enums/SCANCODES_FLAG0.ENUM>
#include <Enums/SCANCODES_FLAG1.ENUM>
#include <Enums/SCANCODES_FLAG2.ENUM>

#include <Enums/MOUSE_SCANCODES.ENUM>
                COUNT
        };
#undef ENUM
        constexpr uint8_t  INPUT_NUM_MOUSE_SCANCODES = to_underlying_type(DummyMouseCodesEnum::COUNT);
        constexpr uint16_t INPUT_NUM_KEYBOARD_SCANCODE_SIGNATURES =
            to_underlying_type(KeyCode::COUNT) - INPUT_NUM_MOUSE_SCANCODES;
        constexpr uint16_t INPUT_NUM_KEYBOARD_SCANCODES = INPUT_NUM_KEYBOARD_SCANCODE_SIGNATURES / 3;
#define ENUM(E, V) #E,
        constexpr const char* buttonSignatureToString[to_underlying_type(KeyCode::COUNT) + 1]{
#include <Enums/SCANCODES_FLAG0.ENUM>
#include <Enums/SCANCODES_FLAG1.ENUM>
#include <Enums/SCANCODES_FLAG2.ENUM>

#include <Enums/MOUSE_SCANCODES.ENUM>
            "KeyCode::COUNT"};
#undef ENUM

#define ENUM(E, V) bool E : 1;
        struct KeyState
        {
                inline KeyState()
                {
                        memset(this, 0, sizeof(this));
                }
                static constexpr auto _SZ64 = 48 /*sizeof(Key)*/ / sizeof(uint64_t);
#include <Enums/SCANCODES_FLAG0.ENUM>
#include <Enums/SCANCODES_FLAG1.ENUM>
#include <Enums/SCANCODES_FLAG2.ENUM>

#include <Enums/MOUSE_SCANCODES.ENUM>

                inline void SetKeyDown(KeyCode button)
                {
                        uint64_t(&pressStateAlias)[_SZ64] = (uint64_t(&)[_SZ64]) * this;
                        uint64_t mask                     = 1ULL << ((uint64_t)button & (64ULL - 1ULL));
                        pressStateAlias[to_underlying_type(button) >> 6] |= mask;
                }
                inline void SetKeyUp(KeyCode button)
                {
                        uint64_t(&pressStateAlias)[_SZ64] = (uint64_t(&)[_SZ64]) * this;
                        uint64_t mask                     = 1ULL << ((uint64_t)button & (64ULL - 1ULL));
                        pressStateAlias[to_underlying_type(button) >> 6] &= ~mask;
                }
                inline bool IsKeyDown(KeyCode keyCode) const
                {
                        uint64_t(&pressStateAlias)[_SZ64] = (uint64_t(&)[_SZ64]) * this;
                        uint64_t mask                     = 1ULL << ((uint64_t)keyCode & (64ULL - 1ULL));
                        return static_cast<bool>(pressStateAlias[to_underlying_type(keyCode) >> 6] & mask);
                }
                inline bool IsKeyUp(KeyCode keycode) const
                {
                        return !IsKeyDown(keycode);
                }
        };
#undef ENUM
        // must be size 2 bytes to use the Transition state to store scroll delta if scroll button signature is set
        enum class KeyTransition : int8_t
        {
                Down,
                Up
        };
        constexpr const char* transitionStateToString[2]{"Down", "Up"};
} // namespace Enums