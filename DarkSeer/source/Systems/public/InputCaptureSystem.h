#pragma once

struct InputCaptureSystem
{
    private:
        static LRESULT CALLBACK InputWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);
    public:
        void Initialize();
        void Shutdown();
};