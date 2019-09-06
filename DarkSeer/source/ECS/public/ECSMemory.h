#pragma once
#include <ECSMetaFunctions.h>
inline char*                              ECSMemory    = 0;
inline char*                              ECSNextAlloc = 0;
inline std::vector<std::function<void()>> ECSSingletonDestructors;
inline void                               ECSInitialize()
{
        ECSMemory    = static_cast<char*>(malloc(MiB * 64));
        ECSNextAlloc = ECSMemory;
}
inline void ECSShutDown()
{
        for (auto& singletonDestructors : ECSSingletonDestructors)
                singletonDestructors();

        free(ECSMemory);
}

// Takes a pointer and assigns it an allocated address from ECS memory
//	if the type pointed-to-type has a tag "RequiresConstructor":
//		it will also call the objects constructor and later will call its destructor
//		when ECSShutDown is called
template <typename T>
inline void ECSAllocateSingleton(T*& objPtr)
{
        objPtr = reinterpret_cast<T*>(ECSNextAlloc);
        ECSNextAlloc += sizeof(T);
        if constexpr (requires_constructor<T>::value)
        {
                construct(objPtr);
                ECSSingletonDestructors.push_back(std::bind([objPtr]() { destroy(objPtr); }));
        }
}