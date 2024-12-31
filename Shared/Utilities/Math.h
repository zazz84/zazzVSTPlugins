#pragma once

#include <cstdint>
#include <type_traits>

namespace Math
{

	//==============================================================================
    inline float fabsf(float value)
    {
        constexpr uint32_t mask = 0x7FFFFFFF;                          // Mask to clear the sign bit
        uint32_t intValue = *reinterpret_cast<uint32_t*>(&value);      // Reinterpret float as uint32_t
        intValue &= mask;                                              // Clear the sign bit
        return *reinterpret_cast<float*>(&intValue);                   // Reinterpret back to float
    }

	//==============================================================================
    template <typename T>
    inline T abs(T value)
    {
        static_assert(std::is_floating_point_v<T>, "fastAbs only supports floating-point types.");

        using IntType = typename std::conditional_t<sizeof(T) == 4, uint32_t,
                        typename std::conditional_t<sizeof(T) == 8, uint64_t, uint128_t>>;

        constexpr IntType mask = static_cast<IntType>(~(static_cast<IntType>(1) << (sizeof(T) * 8 - 1)));
        IntType intValue = *reinterpret_cast<IntType*>(&value);                                             // Reinterpret as integer
        intValue &= mask;                                                                                   // Clear the sign bit
        return *reinterpret_cast<T*>(&intValue);                                                            // Reinterpret back to floating-point
    }
}