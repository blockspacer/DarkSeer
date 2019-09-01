#pragma once
#include <intrin.h>

inline namespace Math
{
        inline unsigned Accumulate(const __m256i* const buffer, unsigned sz)
        {
                __m256i accumulate = buffer[0];
                for (unsigned i = 1; i < sz; i++)
                        accumulate = _mm256_add_epi32(accumulate, buffer[i]);

                return accumulate.m256i_i32[0] + accumulate.m256i_i32[1] + accumulate.m256i_i32[2] + accumulate.m256i_i32[3] +
                       accumulate.m256i_i32[4] + accumulate.m256i_i32[5] + accumulate.m256i_i32[6] + accumulate.m256i_i32[7];
        }

        template <typename IntegerType>
        inline constexpr
            typename std::enable_if<!std::is_floating_point<IntegerType>::value && !std::is_signed<IntegerType>::value,
                                          IntegerType>::type
        DivideRoundUpU(IntegerType dividend, IntegerType divisor)
        {
                return 1 + ((dividend - 1) / divisor);
        }
        template <typename IntegerType>
        inline constexpr
            typename std::enable_if<!std::is_floating_point<IntegerType>::value && std::is_signed<IntegerType>::value,
                                          IntegerType>::type
        DivideRoundUpI(IntegerType dividend, IntegerType divisor)
        {
                if ((dividend < 0) ^ (divisor < 0))
                {
                        return dividend / divisor;
                }
                return 1 + ((dividend - 1) / divisor);
        }
        template <typename IntegerType>
        inline constexpr typename std::enable_if<!std::is_floating_point<IntegerType>::value, IntegerType>::type DivideRoundUp(
            IntegerType dividend,
            IntegerType divisor)
        {
                if constexpr (std::is_signed<IntegerType>::value)
                        return DivideRoundUpI(dividend, divisor);
                else
                        return DivideRoundUpU(dividend, divisor);
        }


        // saturatePowerOf2 is from the link below
        // https://www.geeksforgeeks.org/smallest-power-of-2-greater-than-or-equal-to-n/
        inline constexpr unsigned saturatePowerOf2(unsigned n)
        {
                n--;
                n |= n >> 1;
                n |= n >> 2;
                n |= n >> 4;
                n |= n >> 8;
                n |= n >> 16;
                n++;
                return n;
                // This code is contributed by SHUBHAMSINGH10
        }
} // namespace Math

template <typename... Ts, unsigned I>
constexpr void plusequalsimpl(std::tuple<Ts...>& lhs, const std::tuple<Ts...>& rhs, std::index_sequence<I>)
{
        std::get<I>(lhs) += std::get<I>(rhs);
}
template <typename... Ts, unsigned I, unsigned... Is>
constexpr typename std::enable_if<sizeof...(Is)>::type plusequalsimpl(std::tuple<Ts...>&       lhs,
                                                                      const std::tuple<Ts...>& rhs,
                                                                      std::index_sequence<I, Is...>)
{
        std::get<I>(lhs) += std::get<I>(rhs);
        plusequalsimpl(lhs, rhs, std::index_sequence<Is...>{});
}
template <typename... Ts>
constexpr std::tuple<Ts...>& operator+=(std::tuple<Ts...>& lhs, const std::tuple<Ts...>& rhs)
{
        plusequalsimpl(lhs, rhs, std::make_index_sequence<sizeof...(Ts)>{});
        return lhs;
}