#pragma once
inline namespace Random
{
        inline thread_local std::mt19937 g_tl_defaultRandomEngine;

        std::tuple<uint32_t, uint32_t> FeistelCoder(std::tuple<uint32_t, uint32_t> v)
        {
                uint32_t k[4]{0xA341316C, 0xC8013EA4, 0xAD90777D, 0x7E95761E};
                uint32_t sum   = 0;
                uint32_t delta = 0x9E3779B9;
                for (unsigned i = 0; i < 4; i++)
                {
                        sum += delta;
                        std::get<0>(v) +=
                            ((std::get<1>(v) << 4) + k[0]) ^ (std::get<1>(v) + sum) ^ ((std::get<1>(v) >> 5) + k[1]);
                        std::get<1>(v) +=
                            ((std::get<0>(v) << 4) + k[2]) ^ (std::get<0>(v) + sum) ^ ((std::get<0>(v) >> 5) + k[3]);
                }
                return v;
        }
} // namespace Random