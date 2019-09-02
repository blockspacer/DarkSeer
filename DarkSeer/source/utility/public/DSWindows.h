//#pragma once
//inline namespace DSWindows
//{
//        inline namespace Globals
//        {
//                inline HINSTANCE g_hInstance              = 0;
//                inline auto      g_screenWidth            = GetSystemMetrics(SM_CXSCREEN);
//                inline auto      g_screenHeight           = GetSystemMetrics(SM_CYSCREEN);
//                inline bool      g_windowsMessageShutdown = false;
//                LRESULT CALLBACK g_mainWindowProc(HWND m_hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//        } // namespace Globals
//
//        struct WindowCreationDescriptor
//        {
//                char    WindowTitle[32];
//                char    WindowClassName[32];
//                HWND    m_hwnd;
//                WNDPROC wndProc;
//                int     x;
//                int     y;
//                int     width;
//                int     height;
//                HWND    parent;
//                HMENU   menu;
//                DWORD   style;
//                DWORD   exstyle;
//                HBRUSH  hbrBackground;
//        };
//
//        struct CreatedWindow
//        {
//                HWND    m_hwnd;
//                WNDPROC m_wndproc;
//
//                inline CreatedWindow(HWND hwnd, WNDPROC wndproc) : m_hwnd(hwnd), m_wndproc(wndproc)
//                {}
//                inline void Show()
//                {
//                        bool test = ShowWindow(m_hwnd, SW_SHOWDEFAULT);
//                }
//        };
//
//        struct WindowProxy
//        {
//            private:
//                WindowCreationDescriptor m_windowProxyDescriptor;
//
//            public:
//                inline WindowProxy()
//                {
//                        memset(&m_windowProxyDescriptor, 0, sizeof(m_windowProxyDescriptor));
//                        m_windowProxyDescriptor.wndProc = g_mainWindowProc;
//                        m_windowProxyDescriptor.style      = WS_OVERLAPPED;
//                        m_windowProxyDescriptor.x          = CW_USEDEFAULT;
//                        m_windowProxyDescriptor.y          = CW_USEDEFAULT;
//                        m_windowProxyDescriptor.width      = CW_USEDEFAULT;
//                        m_windowProxyDescriptor.height     = CW_USEDEFAULT;
//                        m_windowProxyDescriptor.style      = WS_OVERLAPPED;
//                        strcpy_s(m_windowProxyDescriptor.WindowClassName, std::to_string(__COUNTER__).c_str());
//                }
//                inline WindowProxy& Title(const char* title)
//                {
//                        strcpy_s(m_windowProxyDescriptor.WindowTitle, title);
//                        return *this;
//                }
//                inline WindowProxy& Position(int x, int y)
//                {
//                        m_windowProxyDescriptor.x = x;
//                        m_windowProxyDescriptor.y = y;
//                        return *this;
//                }
//                inline WindowProxy& Position(percent<float, float> pos)
//                {
//                        m_windowProxyDescriptor.x = std::lroundf((std::get<0>(pos.floats) / 100) * g_screenWidth);
//                        m_windowProxyDescriptor.y = std::lroundf((std::get<1>(pos.floats) / 100) * g_screenHeight);
//                        return *this;
//                }
//                inline WindowProxy& WindProc(WNDPROC wndProc)
//                {
//                        m_windowProxyDescriptor.wndProc = wndProc;
//                        return *this;
//                }
//                inline WindowProxy& Size(int width, int height)
//                {
//                        m_windowProxyDescriptor.width  = width;
//                        m_windowProxyDescriptor.height = height;
//                        return *this;
//                }
//                inline WindowProxy& Size(percent<float, float> size)
//                {
//                        m_windowProxyDescriptor.width  = static_cast<int>((std::get<0>(size.floats) / 100) * g_screenWidth);
//                        m_windowProxyDescriptor.height = static_cast<int>((std::get<1>(size.floats) / 100) * g_screenHeight);
//
//                        return *this;
//                }
//                inline WindowProxy& BackgroundColor(unsigned r, unsigned g, unsigned b)
//                {
//                        COLORREF rgb                           = 0 | r | (g << 1) | (b << 2);
//                        m_windowProxyDescriptor.hbrBackground = CreateSolidBrush(rgb);
//                        return *this;
//                }
//                inline CreatedWindow Create()
//                {
//                        WNDCLASS wc          = {};
//                        wc.lpfnWndProc       = m_windowProxyDescriptor.wndProc;
//                        wc.hInstance         = g_hInstance;
//                        wc.lpszClassName     = m_windowProxyDescriptor.WindowClassName;
//                        auto standard_cursor = LoadCursor(0, IDC_ARROW);
//                        wc.hCursor           = standard_cursor;
//                        wc.style             = CS_HREDRAW | CS_VREDRAW;
//                        wc.hbrBackground     = m_windowProxyDescriptor.hbrBackground;
//
//
//                        DWORD err;
//                        if (!RegisterClassA(&wc))
//                                err = GetLastError();
//                        int pause = 0;
//
//                        auto hwnd = CreateWindowExA(m_windowProxyDescriptor.exstyle,         // Optional window styles.
//                                                      m_windowProxyDescriptor.WindowClassName, // CreatedWindow class
//                                                      m_windowProxyDescriptor.WindowTitle,     // CreatedWindow text
//                                                      WS_OVERLAPPEDWINDOW,                      // CreatedWindow style
//
//                                                      m_windowProxyDescriptor.x,
//                                                      m_windowProxyDescriptor.y,
//                                                      m_windowProxyDescriptor.width,
//                                                      m_windowProxyDescriptor.height,
//
//                                                      m_windowProxyDescriptor.parent, // Parent window
//                                                      m_windowProxyDescriptor.menu,   // Menu
//                                                      g_hInstance,                     // Instance handle
//                                                      0                                // Additional application data
//                        );
//                        return CreatedWindow(hwnd, m_windowProxyDescriptor.wndProc);
//                }
//        };
//
//        inline void MessageLoop()
//        {
//                while (!g_windowsMessageShutdown)
//                {
//                        MSG msg{};
//                        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
//                        {
//                                TranslateMessage(&msg);
//                                DispatchMessageA(&msg);
//                        }
//                }
//        }
//
//        inline void RequestMessageLoopExit()
//        {
//                g_windowsMessageShutdown = true;
//        }
//} // namespace DSWindows