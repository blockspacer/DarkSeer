#pragma once
#include "../private/InputBuffer.h"

struct SingletonInput
{
        struct requires_constructor_tag;
        struct requires_destructor_tag;
        
        WNDPROC                m_parentWndProc;
        std::tuple<long, long> m_previousAbsoluteMousePos;
        InputBuffer            m_inputBuffer;
};