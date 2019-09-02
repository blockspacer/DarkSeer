#pragma once
#include <MemoryDefines.h>
#include "../private/CircularInputBuffer.h"

struct SingletonInput
{
        std::tuple<long, long> m_previousAbsoluteMousePos;
        InputBuffer            m_inputBuffer;
};