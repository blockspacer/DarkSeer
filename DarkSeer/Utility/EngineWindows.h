#pragma once
inline namespace EngineWindows
{
        inline namespace Globals
        {
                inline HINSTANCE g_hInstance              = 0;
                inline LPSTR     g_pCmdLine               = 0;
                inline int       g_nCmdShow               = 0;
                inline auto      g_screenWidth            = GetSystemMetrics(SM_CXSCREEN);
                inline auto      g_screenHeight           = GetSystemMetrics(SM_CYSCREEN);
                inline bool      g_windowsMessageShutdown = false;
                LRESULT CALLBACK g_mainWindowProc(HWND m_hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        } // namespace Globals

        struct WindowCreationDescriptor
        {
                char    WindowTitle[32];
                char    WindowClassName[32];
                HWND    m_hwnd;
                WNDPROC windowProc;
                int     x;
                int     y;
                int     width;
                int     height;
                HWND    parent;
                HMENU   menu;
                DWORD   style;
                DWORD   exstyle;
                HBRUSH  hbrBackground;
        };

        struct Window
        {
            private:
                HWND m_hwnd;

            public:
                inline Window(HWND m_hwnd) : m_hwnd(m_hwnd)
                {}
                inline void Show()
                {
                        bool test = ShowWindow(m_hwnd, SW_SHOWDEFAULT);
                }
                inline HWND GetHWND()
                {
                        return m_hwnd;
                }
        };

        struct CreateWindow
        {
            private:
                WindowCreationDescriptor windowCreationDescriptor;

            public:
                inline CreateWindow()
                {
                        memset(&windowCreationDescriptor, 0, sizeof(windowCreationDescriptor));
                        windowCreationDescriptor.windowProc = g_mainWindowProc;
                        windowCreationDescriptor.style      = WS_OVERLAPPED;
                        windowCreationDescriptor.x          = CW_USEDEFAULT;
                        windowCreationDescriptor.y          = CW_USEDEFAULT;
                        windowCreationDescriptor.width      = CW_USEDEFAULT;
                        windowCreationDescriptor.height     = CW_USEDEFAULT;
                        windowCreationDescriptor.style      = WS_OVERLAPPED;
                        strcpy_s(windowCreationDescriptor.WindowClassName, std::to_string(__COUNTER__).c_str());
                }
                inline CreateWindow& Title(const char* title)
                {
                        strcpy_s(windowCreationDescriptor.WindowTitle, title);
                        return *this;
                }
                inline CreateWindow& Position(int x, int y)
                {
                        windowCreationDescriptor.x = x;
                        windowCreationDescriptor.y = y;
                        return *this;
                }
                inline CreateWindow& Position(percent<float, float> pos)
                {
                        windowCreationDescriptor.x = std::lroundf((std::get<0>(pos.floats) / 100) * g_screenWidth);
                        windowCreationDescriptor.y = std::lroundf((std::get<1>(pos.floats) / 100) * g_screenHeight);
                        return *this;
                }
                inline CreateWindow& WindProc(WNDPROC wndProc)
                {
                        windowCreationDescriptor.windowProc = wndProc;
                        return *this;
                }
                inline CreateWindow& Size(int width, int height)
                {
                        windowCreationDescriptor.width  = width;
                        windowCreationDescriptor.height = height;
                        return *this;
                }
                inline CreateWindow& Size(percent<float, float> size)
                {
                        windowCreationDescriptor.width  = static_cast<int>((std::get<0>(size.floats) / 100) * g_screenWidth);
                        windowCreationDescriptor.height = static_cast<int>((std::get<1>(size.floats) / 100) * g_screenHeight);
                        return *this;
                }
                inline CreateWindow& BackgroundColor(unsigned r, unsigned g, unsigned b)
                {
                        COLORREF rgb                           = 0 | r | (g << 1) | (b << 2);
                        windowCreationDescriptor.hbrBackground = CreateSolidBrush(rgb);
                        return *this;
                }
                inline Window Finalize()
                {
                        WNDCLASS wc          = {};
                        wc.lpfnWndProc       = windowCreationDescriptor.windowProc;
                        wc.hInstance         = g_hInstance;
                        wc.lpszClassName     = windowCreationDescriptor.WindowClassName;
                        auto standard_cursor = LoadCursor(0, IDC_ARROW);
                        wc.hCursor           = standard_cursor;
                        wc.style             = CS_HREDRAW | CS_VREDRAW;
                        wc.hbrBackground     = windowCreationDescriptor.hbrBackground;


                        DWORD err;
                        if (!RegisterClassA(&wc))
                                err = GetLastError();
                        int pause = 0;

                        auto m_hwnd = CreateWindowExA(windowCreationDescriptor.exstyle,         // Optional window styles.
                                                      windowCreationDescriptor.WindowClassName, // Window class
                                                      windowCreationDescriptor.WindowTitle,     // Window text
                                                      WS_OVERLAPPEDWINDOW,                      // Window style

                                                      windowCreationDescriptor.x,
                                                      windowCreationDescriptor.y,
                                                      windowCreationDescriptor.width,
                                                      windowCreationDescriptor.height,

                                                      windowCreationDescriptor.parent, // Parent window
                                                      windowCreationDescriptor.menu,   // Menu
                                                      g_hInstance,                     // Instance handle
                                                      0                                // Additional application data
                        );
                        return Window(m_hwnd);
                }
        };

        inline void MessageLoop()
        {
                while (!g_windowsMessageShutdown)
                {
                        MSG msg{};
                        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
                        {
                                TranslateMessage(&msg);
                                DispatchMessageA(&msg);
                        }
                }
        }

        inline void RequestMessageLoopExit()
        {
                g_windowsMessageShutdown = true;
        }
} // namespace Windows