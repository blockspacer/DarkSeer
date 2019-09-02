#pragma once
#include "../private/CircularInputBuffer.h"

struct SingletonInput
{
        std::tuple<long, long> m_previousAbsoluteMousePos;
        WNDPROC                m_parentWndProc;
        HWND                   m_parentHWND;
        InputBuffer            m_inputBuffer;
};