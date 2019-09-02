#pragma once
#include "../private/CircularInputBuffer.h"

struct SingletonInput
{
        WNDPROC                m_parentWndProc;
        std::tuple<long, long> m_previousAbsoluteMousePos;
        InputBuffer            m_inputBuffer;
};