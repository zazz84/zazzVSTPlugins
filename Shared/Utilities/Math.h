/*
 * Copyright (C) 2026 Filip Cenzak (filip.c@centrum.cz)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <cmath>

namespace Math
{
	//==============================================================================
	__forceinline float fabsf(const float value) noexcept
    {
		uint32_t i;
		std::memcpy(&i, &value, sizeof(float));
		i &= 0x7FFFFFFFu;

		float out;
		std::memcpy(&out, &i, sizeof(float));
		return out;
    }

	//==============================================================================
	__forceinline constexpr float clamp(const float value, const float min, const float max) noexcept
	{
		const float t = value < min ? min : value;
		return t > max ? max : t;
	}

	//==============================================================================
	__forceinline constexpr int clampInt(const int value, const int min, const int max) noexcept
	{
		const int t = value < min ? min : value;
		return t > max ? max : t;
	}

	//==============================================================================
	__forceinline constexpr float fminf(const float a, const float b) noexcept
	{
		return a > b ? b : a;
	}

	//==============================================================================
	__forceinline constexpr float fmaxf(const float a, const float b) noexcept
	{
		return a > b ? a : b;
	}

	//==============================================================================
	__forceinline constexpr float sign(const float value) noexcept
	{
		return value > 0.0f ? 1.0f : -1.0f;
	}
	
	//==============================================================================
	// This is a fast approximation to log2()
	// Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;
	__forceinline float log2Approx(const float value) noexcept
	{
		float Y, F;
		int E;
		F = frexpf(Math::fabsf(value), &E);
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
	__forceinline float log10Approx(const float value) noexcept
	{
		return Math::log2Approx(value) * 0.3010299956639812f;
	}

	//==============================================================================
	// powf(10.f,x) is exactly exp(log(10.0f)*x)
	__forceinline float pow10(const float value) noexcept
	{
		return std::expf(2.302585092994046f * value);
	}

	//==============================================================================
	__forceinline float dBToGain(float dB) noexcept
	{
		constexpr float DB_TO_GAIN = 0.05f;
		return dB > -100.0f ? Math::pow10(DB_TO_GAIN * dB) : 0.0f;
	}

	//==============================================================================
	__forceinline float gainTodBAprox(float gain) noexcept
	{
		return gain > 0.00001f ? 20.0f * Math::log10Approx(gain) : -100.0f;
	}

	//==============================================================================
	// Fast proximation for input (0.0f, 1.0f), where -45db = 0.0f
	__forceinline float gain01TodBApprox(const float gain) noexcept
	{
		return 5.0f - (5.0f / (gain + 0.1f));
	}

	//==============================================================================
	// No safety checks
	// inMin must be < inMax
	__forceinline float remap(float value, float inMin, float inMax, float outMin, float outMax) noexcept
	{
		if (value <= inMin)
		{
			return outMin;
		}
		else if (value >= inMax)
		{
			return outMax;
		}
		else
		{
			return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
		}
	};

	//==============================================================================
	__forceinline float frequencyToMel(const float frequency) noexcept
	{
		return 1127.0f * std::log(1.0f + frequency / 700.0f);
	};

	//==============================================================================
	__forceinline float melToFrequency(const float mel) noexcept
	{
		return 700.0f * (std::expf(mel / 1127.0f) - 1.0f);
	}

	//==============================================================================
	__forceinline float shiftFrequency(float frequency, float semitones) noexcept
	{
		return frequency * std::exp2f(semitones / 12.0f);
	}

	//==============================================================================
	__forceinline float noteToFrequency(int midiNoteNumber) noexcept
	{
		return 440.0f * std::exp2f((float)(midiNoteNumber - 69) / 12.0f);
	}
	//==============================================================================
	__forceinline bool almostEquals(const float a, const float b, const float epsilon = 0.001f) noexcept
	{
		return fabsf(a - b) < epsilon;
	}
	//==============================================================================
	__forceinline constexpr float getAmplitudeAttenuation(const float distance, const float factor, const float innerRange) noexcept
	{
		if (distance <= innerRange)
		{
			return 1.0f;
		}
		else
		{
			return factor / (distance - innerRange + factor);
		}
	}
}