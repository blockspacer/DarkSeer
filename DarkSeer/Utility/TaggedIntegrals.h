#pragma once
#include <tuple>

inline namespace TaggedIntegrals
{
        template <typename... Ts>
        struct percent
        {
                std::tuple<Ts...> floats;
                percent(Ts... ts)
                {
                        floats = std::tuple(ts...);
                }
        };
        template <typename T1>
        percent(T1 t1)->percent<float>;
        template <typename T1, typename T2>
        percent(T1 t1, T2 t2)->percent<float, float>;
        template <typename T1, typename T2, typename T3>
        percent(T1 t1, T2 t2, T3 t3)->percent<float, float, float>;
} // namespace TaggedIntegrals