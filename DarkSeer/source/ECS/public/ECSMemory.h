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
//	if the pointed-to-type has a member struct "requires_constructor_tag":
//		it will also call the objects constructor
//	if the pointed-to-type has a member struct "requires_destructor_tag":
//		the types destructor will be called on ECSShutdown
template <typename T>
inline void ECSAllocateSingleton(T*& objPtr)
{
        objPtr = reinterpret_cast<T*>(ECSNextAlloc);
        ECSNextAlloc += sizeof(T);

        if constexpr (requires_constructor<T>::value)
                construct(objPtr);

        if constexpr (requires_destructor<T>::value)
                ECSSingletonDestructors.push_back(std::bind([objPtr]() { destroy(objPtr); }));
}