#pragma once

inline namespace TaggedPrimitives
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

        template <typename E>
        constexpr auto to_underlying_type(E e) -> typename std::underlying_type<E>::type
        {
                return static_cast<typename std::underlying_type<E>::type>(e);
        }

		template <typename T>
        inline T& non_const_ref(const T& t)
        {
                return const_cast<T&>(t);
        }
} // namespace TaggedIntegrals