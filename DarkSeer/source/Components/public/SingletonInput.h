#pragma once
#include "../private/InputBuffer.h"

struct SingletonInput
{
        struct requires_constructor_tag;
        struct requires_destructor_tag;

        WNDPROC                m_parentWndProc;
        std::tuple<long, long> m_previousAbsoluteMousePos;
        inline InputBuffer*    GetInputBuffer()
        {
                return &m_inputBuffer;
        }

    private:
        InputBuffer m_inputBuffer;
};