#pragma once

inline namespace Enums
{
#undef ENUM
#define ENUM(E, V) E,
        // dummy enum to get number of mouse signatures
        enum class DummyMouseCodesEnum
        {
#include <SCANCODES64_1.ENUM>
                COUNT
        };
#undef ENUM
#define ENUM(E, V) E,
        enum class KeyCode : uint16_t
        {
#include <SCANCODES64_1.ENUM>
#include <SCANCODES64_2.ENUM>
#include <SCANCODES64_3.ENUM>
#include <SCANCODES64_4.ENUM>
#include <SCANCODES64_5.ENUM>
#include <SCANCODES64_6.ENUM>
                COUNT
        };
        static bool operator!(KeyCode lhs)
        {
                return lhs == KeyCode::Null;
        }
        static bool operator==(KeyCode lhs, bool rhs)
        {
                return static_cast<bool>(to_underlying_type(lhs)) == rhs;
        }
#undef ENUM
        // constexpr uint8_t  INPUT_NUM_MOUSE_SCANCODES = to_underlying_type(DummyMouseCodesEnum::COUNT);
        // constexpr uint16_t INPUT_NUM_KEYBOARD_SCANCODE_SIGNATURES =
        //    to_underlying_type(KeyCode::COUNT) - INPUT_NUM_MOUSE_SCANCODES;
        constexpr uint16_t INPUT_NUM_KEYBOARD_SCANCODES = to_underlying_type(KeyCode::COUNT) / 3;
#define ENUM(E, V) #E,
        constexpr const char* keyCodeToString[to_underlying_type(KeyCode::COUNT) + 1]{
#include <SCANCODES64_1.ENUM>
#include <SCANCODES64_2.ENUM>
#include <SCANCODES64_3.ENUM>
#include <SCANCODES64_4.ENUM>
#include <SCANCODES64_5.ENUM>
#include <SCANCODES64_6.ENUM>
            "KeyCode::COUNT"};
#undef ENUM

#define ENUM(E, V) bool E : 1;
        struct alignas(32) KeyStateLow
        {
#include <SCANCODES64_1.ENUM>
#include <SCANCODES64_2.ENUM>
#include <SCANCODES64_3.ENUM>
#include <SCANCODES64_4.ENUM>
        };
        struct alignas(16) KeyStateHigh
        {
#include <SCANCODES64_5.ENUM>
#include <SCANCODES64_6.ENUM>
        };

#undef ENUM
        // must be size 2 bytes to use the Transition state to store scroll delta if scroll button signature is set
        enum class KeyTransition : int8_t
        {
                Down,
                Up
        };
        constexpr const char* transitionStateToString[2]{"Down", "Up"};

        inline static constexpr const char* TransitionStateToString(KeyTransition keyTransition)
        {
                return transitionStateToString[to_underlying_type(keyTransition)];
        }
		inline static constexpr const char* KeyCodeToString(KeyCode keyCode)
		{
                return keyCodeToString[to_underlying_type(keyCode)];
		}
} // namespace Enums