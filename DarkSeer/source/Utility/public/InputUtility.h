#pragma once
struct SingletonInput;
struct SingletonWindow;

namespace InputUtil
{
        void InitializeInputWndProc(SingletonInput* singlInput, const SingletonWindow* singlWindow);
        void RegisterDefaultRawInputDevices();
} // namespace InputUtil