#include "WindowFactories.h"
WindowProxy::WindowProxy()
{
        memset(&m_windowProxyDescriptor, 0, sizeof(m_windowProxyDescriptor));
        m_windowProxyDescriptor.wndProc = 0;
        m_windowProxyDescriptor.style   = WS_OVERLAPPED;
        m_windowProxyDescriptor.x       = CW_USEDEFAULT;
        m_windowProxyDescriptor.y       = CW_USEDEFAULT;
        m_windowProxyDescriptor.width   = CW_USEDEFAULT;
        m_windowProxyDescriptor.height  = CW_USEDEFAULT;
        m_windowProxyDescriptor.style   = WS_OVERLAPPED;
        strcpy_s(m_windowProxyDescriptor.WindowClassName, std::to_string(__COUNTER__).c_str());
}

WindowProxy& WindowProxy::Title(const char* title)
{
        strcpy_s(m_windowProxyDescriptor.WindowTitle, title);
        return *this;
}

WindowProxy& WindowProxy::Position(int x, int y)
{
        m_windowProxyDescriptor.x = x;
        m_windowProxyDescriptor.y = y;
        return *this;
}

WindowProxy& WindowProxy::Position(percent<float, float> pos)
{
        m_windowProxyDescriptor.x = std::lroundf((std::get<0>(pos.floats) / 100) * GetSystemMetrics(SM_CXFULLSCREEN));
        m_windowProxyDescriptor.y = std::lroundf((std::get<1>(pos.floats) / 100) * GetSystemMetrics(SM_CYFULLSCREEN));
        return *this;
}

WindowProxy& WindowProxy::WindProc(WNDPROC wndProc)
{
        m_windowProxyDescriptor.wndProc = wndProc;
        return *this;
}

WindowProxy& WindowProxy::Size(int width, int height)
{
        m_windowProxyDescriptor.width  = width;
        m_windowProxyDescriptor.height = height;
        return *this;
}

WindowProxy& WindowProxy::Size(percent<float, float> size)
{
        m_windowProxyDescriptor.width  = static_cast<int>((std::get<0>(size.floats) / 100) * GetSystemMetrics(SM_CXFULLSCREEN));
        m_windowProxyDescriptor.height = static_cast<int>((std::get<1>(size.floats) / 100) * GetSystemMetrics(SM_CYFULLSCREEN));

        return *this;
}

WindowProxy& WindowProxy::BackgroundColor(unsigned r, unsigned g, unsigned b)
{
        COLORREF rgb                          = 0 | r | (g << 1) | (b << 2);
        m_windowProxyDescriptor.hbrBackground = CreateSolidBrush(rgb);
        return *this;
}

CreatedWindow WindowProxy::Create()
{
        WNDCLASS wc          = {};
        wc.lpfnWndProc       = m_windowProxyDescriptor.wndProc;
        wc.hInstance         = GetModuleHandleA(0);
        wc.lpszClassName     = m_windowProxyDescriptor.WindowClassName;
        auto standard_cursor = LoadCursorA(0, IDC_ARROW);
        wc.hCursor           = standard_cursor;
        wc.style             = CS_HREDRAW | CS_VREDRAW;
        wc.hbrBackground     = m_windowProxyDescriptor.hbrBackground;

        DWORD err;
        if (!RegisterClassA(&wc))
                err = GetLastError();
        int pause = 0;

        auto hwnd = CreateWindowExA(m_windowProxyDescriptor.exstyle,         // Optional window styles.
                                    m_windowProxyDescriptor.WindowClassName, // CreatedWindow class
                                    m_windowProxyDescriptor.WindowTitle,     // CreatedWindow text
                                    WS_OVERLAPPEDWINDOW,                     // CreatedWindow style

                                    m_windowProxyDescriptor.x,
                                    m_windowProxyDescriptor.y,
                                    m_windowProxyDescriptor.width,
                                    m_windowProxyDescriptor.height,

                                    m_windowProxyDescriptor.parent, // Parent window
                                    m_windowProxyDescriptor.menu,   // Menu
                                    GetModuleHandleA(0),            // Instance handle
                                    0                               // Additional application data
        );

        CreatedWindow output;
        output.m_hwnd    = hwnd;
        output.m_wndproc = m_windowProxyDescriptor.wndProc;
        return output;
}