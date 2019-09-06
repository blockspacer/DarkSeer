#pragma once
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <cstdlib>

#ifdef _DEBUG
#define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define new new
#endif

template <typename T, typename... TArgs>
inline void construct(T* mem, TArgs... args)
{
#pragma push_macro("new")
#undef new
        new (mem) T(args...);
#pragma pop_macro("new")
}
template <typename T>
inline void destroy(T* mem)
{
        mem->~T();
}

#ifdef _DEBUG
#define malloc(size) _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#ifdef _DEBUG
#define _aligned_malloc(size, alignment) _aligned_malloc_dbg(size, alignment, __FILE__, __LINE__)
#endif