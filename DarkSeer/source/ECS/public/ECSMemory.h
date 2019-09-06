#pragma once
inline char* ECSMemory    = 0;
inline char* ECSNextAlloc = 0;

inline void ECSInitialize()
{
        ECSMemory    = static_cast<char*>(malloc(MiB * 64));
        ECSNextAlloc = ECSMemory;
}
inline void ECSShutDown()
{
        free(ECSMemory);
}
inline void* ECSAllocateSingleton(size_t size)
{
        char* temp = ECSNextAlloc;
        ECSNextAlloc += size;
        return static_cast<void*>(temp);
}
