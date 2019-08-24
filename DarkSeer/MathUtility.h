#pragma once
#include <type_traits>
inline namespace MathUtility
{
        template <typename IntegerType>
        constexpr typename std::enable_if<!std::is_floating_point<IntegerType>::value, IntegerType>::type DivideRoundUp(
            IntegerType dividend,
            IntegerType divisor)
        {
                if constexpr (std::is_signed<IntegerType>::value)
                        return DivideRoundUpI(dividend, divisor);
                else constexpr
					return DivideRoundUpU(dividend, divisor);
        }

        template <typename IntegerType>
        constexpr typename std::enable_if<!std::is_floating_point<IntegerType>::value && !std::is_signed<IntegerType>::value,
                                          IntegerType>::type
        DivideRoundUpU(IntegerType dividend, IntegerType divisor)
        {
                return 1 + ((dividend - 1) / divisor);
        }
        template <typename IntegerType>
        constexpr typename std::enable_if<!std::is_floating_point<IntegerType>::value && std::is_signed<IntegerType>::value,
                                          IntegerType>::type
        DivideRoundUpI(IntegerType dividend, IntegerType divisor)
        {
                if ((dividend < 0) ^ (divisor < 0))
                {
                        return dividend / divisor;
                }
                return 1 + ((dividend - 1) / divisor);
        }

} // namespace MathUtility