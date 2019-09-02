#pragma once
inline namespace DSWindows
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
                WNDPROC wndProc;
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
                HWND    m_hwnd;
                WNDPROC m_wndproc;

                inline Window(HWND hwnd, WNDPROC wndproc) : m_hwnd(hwnd), m_wndproc(wndproc)
                {}
                inline void Show()
                {
                        bool test = ShowWindow(m_hwnd, SW_SHOWDEFAULT);
                }
        };

        struct CreateWindow
        {
            private:
                WindowCreationDescriptor windowProxyDescriptor;

            public:
                inline CreateWindow()
                {
                        memset(&windowProxyDescriptor, 0, sizeof(windowProxyDescriptor));
                        windowProxyDescriptor.wndProc = g_mainWindowProc;
                        windowProxyDescriptor.style      = WS_OVERLAPPED;
                        windowProxyDescriptor.x          = CW_USEDEFAULT;
                        windowProxyDescriptor.y          = CW_USEDEFAULT;
                        windowProxyDescriptor.width      = CW_USEDEFAULT;
                        windowProxyDescriptor.height     = CW_USEDEFAULT;
                        windowProxyDescriptor.style      = WS_OVERLAPPED;
                        strcpy_s(windowProxyDescriptor.WindowClassName, std::to_string(__COUNTER__).c_str());
                }
                inline CreateWindow& Title(const char* title)
                {
                        strcpy_s(windowProxyDescriptor.WindowTitle, title);
                        return *this;
                }
                inline CreateWindow& Position(int x, int y)
                {
                        windowProxyDescriptor.x = x;
                        windowProxyDescriptor.y = y;
                        return *this;
                }
                inline CreateWindow& Position(percent<float, float> pos)
                {
                        windowProxyDescriptor.x = std::lroundf((std::get<0>(pos.floats) / 100) * g_screenWidth);
                        windowProxyDescriptor.y = std::lroundf((std::get<1>(pos.floats) / 100) * g_screenHeight);
                        return *this;
                }
                inline CreateWindow& WindProc(WNDPROC wndProc)
                {
                        windowProxyDescriptor.wndProc = wndProc;
                        return *this;
                }
                inline CreateWindow& Size(int width, int height)
                {
                        windowProxyDescriptor.width  = width;
                        windowProxyDescriptor.height = height;
                        return *this;
                }
                inline CreateWindow& Size(percent<float, float> size)
                {
                        windowProxyDescriptor.width  = static_cast<int>((std::get<0>(size.floats) / 100) * g_screenWidth);
                        windowProxyDescriptor.height = static_cast<int>((std::get<1>(size.floats) / 100) * g_screenHeight);

                        return *this;
                }
                inline CreateWindow& BackgroundColor(unsigned r, unsigned g, unsigned b)
                {
                        COLORREF rgb                           = 0 | r | (g << 1) | (b << 2);
                        windowProxyDescriptor.hbrBackground = CreateSolidBrush(rgb);
                        return *this;
                }
                inline Window Finalize()
                {
                        WNDCLASS wc          = {};
                        wc.lpfnWndProc       = windowProxyDescriptor.wndProc;
                        wc.hInstance         = g_hInstance;
                        wc.lpszClassName     = windowProxyDescriptor.WindowClassName;
                        auto standard_cursor = LoadCursor(0, IDC_ARROW);
                        wc.hCursor           = standard_cursor;
                        wc.style             = CS_HREDRAW | CS_VREDRAW;
                        wc.hbrBackground     = windowProxyDescriptor.hbrBackground;


                        DWORD err;
                        if (!RegisterClassA(&wc))
                                err = GetLastError();
                        int pause = 0;

                        auto hwnd = CreateWindowExA(windowProxyDescriptor.exstyle,         // Optional window styles.
                                                      windowProxyDescriptor.WindowClassName, // Window class
                                                      windowProxyDescriptor.WindowTitle,     // Window text
                                                      WS_OVERLAPPEDWINDOW,                      // Window style

                                                      windowProxyDescriptor.x,
                                                      windowProxyDescriptor.y,
                                                      windowProxyDescriptor.width,
                                                      windowProxyDescriptor.height,

                                                      windowProxyDescriptor.parent, // Parent window
                                                      windowProxyDescriptor.menu,   // Menu
                                                      g_hInstance,                     // Instance handle
                                                      0                                // Additional application data
                        );
                        return Window(hwnd, windowProxyDescriptor.wndProc);
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
} // namespace DSWindows