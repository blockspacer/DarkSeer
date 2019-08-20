#pragma once
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#if defined _DEBUG
#define ENABLE_LEAK_DETECTION() _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#else
#define ENABLE_LEAK_DETECTION()
#endif

#ifdef _DEBUG
#define placement_new new
#define new new (_CLIENT_BLOCK, __FILE__, __LINE__)
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define placement_new new
#define new new
#endif

#ifdef _DEBUG
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define malloc malloc
#endif	

namespace
{
        struct StaticMemoryLeakSentinel
        {
                inline StaticMemoryLeakSentinel()
                {}
                inline ~StaticMemoryLeakSentinel()
                {

                        _CrtDumpMemoryLeaks();
                }
        } inline static _StaticMemoryLeakSentinelSingleton;
} // namespace