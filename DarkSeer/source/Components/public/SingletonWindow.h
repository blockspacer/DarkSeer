#pragma once
struct SingletonWindow
{
        HWND m_mainHwnd;
        long m_clientWidth;
        long m_clientHeight;
        bool m_dispatchMessages;
};