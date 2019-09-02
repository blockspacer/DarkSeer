#pragma once
struct SingletonInput;

namespace InputUtil
{
        void    InitializeInputBuffer(SingletonInput* singlInput, HWND hwnd);
        void    ReleaseInputBufferMemory(SingletonInput* singlInput);
        void    RegisterDefaultRawInputDevices();
        LRESULT CALLBACK InputWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);
} // namespace InputUtil