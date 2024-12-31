#pragma once

#include <cstdint>
#include <type_traits>

#include <cmath>

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

	//==============================================================================
	inline float clamp(float val, float minval, float maxval)
	{
		const float t = val < minval ? minval : val;
		return t > maxval ? maxval : t;
	}

	//==============================================================================
	// This is a fast approximation to log2()
	// Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;
	inline float log2(float x)
	{
		float Y, F;
		int E;
		F = frexpf(Math::fabsf(x), &E);
		Y = 1.23149591368684f;
		Y *= F;
		Y += -4.11852516267426f;
		Y *= F;
		Y += 6.02197014179219f;
		Y *= F;
		Y += -3.13396450166353f;
		Y += E;
			
		return Y;
	}

	//==============================================================================
	// log10f is exactly log2(x) / log2(10.0f)
	inline float log10(const float x)
	{
		return Math::log2(x) * 0.3010299956639812f;
	}

	//==============================================================================
	//powf(10.f,x) is exactly exp(log(10.0f)*x)
	/*inline float pow10(const float x)
	{
		return std::expf(2.302585092994046f * x);
	}*/

	// Fast power-of-10 approximation, with RMS error of 1.77%. 
	// This approximation developed by Nicol Schraudolph (Neural Computation vol 11, 1999).  
	// Adapted for 32-bit floats by Lippold Haken of Haken Audio, April 2010.
	// Set float variable's bits to integer expression.
	// f=b^f is approximated by
	//   (int)f = f*0x00800000*log(b)/log(2) + 0x3F800000-60801*8
	// f=10^f is approximated by
	//   (int)f = f*27866352.6 + 1064866808.0
	inline float pow10(float f)
	{
		*(int *)&f = f * 27866352.6f + 1064866808.0f;
		return f;
	}

	//==============================================================================
	inline float dBToGain(float dB)
	{
		//return dB > -100.0f ? std::pow(10.0f, (1.0f / 20.0f) * dB) : 0.0f;
		return dB > -100.0f ? Math::pow10((1.0f / 20.0f) * dB) : 0.0f;
	}

	//==============================================================================
	inline float gainTodB(float gain)
	{
		//return gain > 0.00001f ? 20.0f * std::log10(gain) : -100.0f;
		return gain > 0.00001f ? 20.0f * Math::log10(gain) : -100.0f;
	}
}