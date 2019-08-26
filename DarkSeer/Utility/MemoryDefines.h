#pragma once
#include <new>
inline namespace MemoryDefines
{
        constexpr auto KiB        = 1024;
        constexpr auto MiB        = KiB * 1024;
        constexpr auto GiB        = MiB * 1024;
        constexpr auto CACHE_LINE = std::hardware_destructive_interference_size;
} // namespace MemoryDefines
