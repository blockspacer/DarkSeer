#pragma once
struct SingletonInput;
struct SingletonWindow;

namespace InputUtil
{
        void InitializeInputBuffer(SingletonInput* singlInput);
        void InitializeInputWndProc(SingletonInput* singlInput, const SingletonWindow* singlWindow);
        void ReleaseInputBufferMemory(SingletonInput* singlInput);
        void RegisterDefaultRawInputDevices();
        void LaunchMessageLoop(SingletonWindow* singlWindow);
} // namespace InputUtil