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

#include <math.h>
#include <mmintrin.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class Clippers
{
public:
	inline static float Hard(const float in, const float threshold) noexcept
	{
		return Math::clamp(in, -threshold, threshold);
	}
	inline static float HardRatio(const float in, const float threshold, const float ratio)
	{
	  if (in > threshold)
	  {
	    return threshold + (in - threshold) * ratio;
	  }
	  else if (in < -threshold)
	  {
	    return -(threshold + (-in - threshold) * ratio);
	  }

	  return in;
	}
	// Different algorithm than SoftBlock
	inline static float Soft(const float in, float const threshold) noexcept
	{
		const float thresholdHalf = 0.5f * threshold;
		const float inAbs = fabsf(in);

		if (inAbs < thresholdHalf)
		{
			return std::copysignf(inAbs, in);
		}
		else if (inAbs < threshold + thresholdHalf)
		{
			return std::copysignf(0.5f * (inAbs + thresholdHalf), in);
		}
		else
		{
			return std::copysignf(threshold, in);
		}
	}
	inline static float FoldBack(const float in, float const threshold) noexcept
	{
		if (in < threshold && in > -threshold)
		{
			return in;
		}
		else
		{
			const float range = 4.0f * threshold;

			float mod = in - threshold;
			mod -= range * std::floor(mod / range);

			return fabsf(mod - 2.0f * threshold) - threshold;
		}
	}
	inline static float HalfWave(const float in, float const threshold) noexcept
	{
		return Math::clamp(in, 0.0f, threshold);
	}
	inline static float ABS(const float in, const float threshold) noexcept
	{
		const float inAbs = fabsf(in);

		if (inAbs < threshold)
		{
			return inAbs;
		}
		else
		{
			return threshold;
		}
	}

	//==============================================================================
	struct Params
	{
		float threshold = 0.0f;
		float wet = 0.0f;
		float* buffer = nullptr;
		unsigned int samples = 0;
	};

	static void HardBlock(const Params& params) noexcept
	{
		// Constants
		const float thresholdLow = -params.threshold;

		// Only wet
		if (Math::almostEquals(params.wet, 1.0f))
		{
#if JUCE_USE_SSE_INTRINSICS
			int sample = 0;

			// Constants
			const __m128 vLo = _mm_set1_ps(thresholdLow);
			const __m128 vHi = _mm_set1_ps(params.threshold);

			// Process 4 samples per iteration
			for (; sample <= params.samples - 4; sample += 4)
			{
				// Load
				float* pIn = params.buffer + sample;
				const __m128 in = _mm_load_ps(pIn);

				// Hard clip
				const __m128 clipped = _mm_min_ps(_mm_max_ps(in, vLo), vHi);

				// Store
				_mm_store_ps(pIn, clipped);
			}

			// Tail processing (remaining samples)
			for (; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				float clipped = in < thresholdLow ? thresholdLow : in;
				clipped = clipped > params.threshold ? params.threshold : clipped;
				in = clipped;
			}
#else
			for (int sample = 0; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				float clipped = in < thresholdLow ? thresholdLow : in;
				clipped = clipped > params.threshold ? params.threshold : clipped;
				in = clipped;
			}
#endif
		}
		// Wet / dry mix
		else
		{
#if JUCE_USE_SSE_INTRINSICS
			int sample = 0;

			// Constants
			const __m128 vLo = _mm_set1_ps(thresholdLow);
			const __m128 vHi = _mm_set1_ps(params.threshold);
			const __m128 vWet = _mm_set1_ps(params.wet);

			// Process 4 samples per iteration
			for (; sample <= params.samples - 4; sample += 4)
			{
				// Load
				float* pIn = params.buffer + sample;
				const __m128 in = _mm_load_ps(pIn);

				// Hard clip
				const __m128 clipped = _mm_min_ps(_mm_max_ps(in, vLo), vHi);

				// Dry / wet mix
				const __m128 delta = _mm_sub_ps(clipped, in);
				const __m128 out = _mm_add_ps(in, _mm_mul_ps(vWet, delta));

				// Store
				_mm_store_ps(pIn, out);
			}

			// Tail processing (remaining samples)
			for (; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				float clipped = in < thresholdLow ? thresholdLow : in;
				clipped = clipped > params.threshold ? params.threshold : clipped;
				in = in + params.wet * (clipped - in);
			}
#else
			for (int sample = 0; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				float clipped = in < thresholdLow ? thresholdLow : in;
				clipped = clipped > params.threshold ? params.threshold : clipped;
				in = in + params.wet * (clipped - in);
			}
#endif
		}
	}

	static void SoftBlock(const Params& params) noexcept
	{
		if (Math::almostEquals(params.wet, 1.0f))
		{
			// Only wet path
			const float t1 = params.threshold / 3.0f;
			const float t2 = 2.0f * t1;
			const float A = 3.0f / params.threshold;
			constexpr float B = 1.0f / 3.0f;

#if JUCE_USE_SSE_INTRINSICS
			int sample = 0;

			const __m128 vHalf = _mm_set1_ps(0.5f);
			const __m128 vT1 = _mm_set1_ps(t1);
			const __m128 vT2 = _mm_set1_ps(t2);
			const __m128 vThreshold = _mm_set1_ps(params.threshold);
			const __m128 vA = _mm_set1_ps(A);
			const __m128 vB = _mm_set1_ps(B);
			const __m128 vOne = _mm_set1_ps(1.0f);
			const __m128 vTwo = _mm_set1_ps(2.0f);
			const __m128 signMask = _mm_set1_ps(-0.0f); // sign bit

			for (; sample <= params.samples - 4; sample += 4)
			{
				float* pIn = params.buffer + sample;
				const __m128 in = _mm_load_ps(pIn);

				// sign ±1
				const __m128 signIn = _mm_or_ps(_mm_and_ps(in, signMask), vOne);

				// absIn = 0.5 * in * sign
				const __m128 absIn = _mm_mul_ps(signIn, _mm_mul_ps(in, vHalf));

				// Masks for mid/high
				const __m128 maskMid = _mm_and_ps(_mm_cmpge_ps(absIn, vT1), _mm_cmplt_ps(absIn, vT2));
				const __m128 maskHigh = _mm_cmpge_ps(absIn, vT2);
				const __m128 applyMask = _mm_or_ps(maskMid, maskHigh);

				// midClipped = sign * threshold * (1 - B * (2 - A*absIn)^2)
				const __m128 tmp = _mm_sub_ps(vTwo, _mm_mul_ps(vA, absIn));
				const __m128 tmp2 = _mm_mul_ps(tmp, tmp);
				const __m128 midClipped = _mm_mul_ps(signIn, _mm_mul_ps(vThreshold, _mm_sub_ps(vOne, _mm_mul_ps(vB, tmp2))));

				// highClipped = sign * threshold
				const __m128 highClipped = _mm_mul_ps(signIn, vThreshold);

				// Select clipped based on masks
				__m128 clipped = _mm_blendv_ps(midClipped, highClipped, maskHigh);

				// Apply only where absIn >= t1
				const __m128 result = _mm_blendv_ps(in, clipped, applyMask);

				_mm_store_ps(pIn, result);
			}

			// Tail loop
			for (; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				const float signIn = in > 0.0f ? 1.0f : -1.0f;
				const float absIn = signIn * (0.5f * in);

				if (absIn < t1)
					continue;
				else if (absIn < t2)
				{
					const float tmp = 2.0f - A * absIn;
					const float clipped = signIn * params.threshold * (1.0f - B * tmp * tmp);
					in = clipped;
				}
				else
				{
					in = signIn * params.threshold;
				}
			}

#else
			// Scalar-only path
			for (unsigned int sample = 0; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				const float signIn = in > 0.0f ? 1.0f : -1.0f;
				const float absIn = signIn * (0.5f * in);


				if (absIn < t1)
					continue;
				else if (absIn < t2)
				{
					const float tmp = 2.0f - A * absIn;
					const float clipped = signIn * params.threshold * (1.0f - B * tmp * tmp);
					in = clipped;
				}
				else
				{
					in = signIn * params.threshold;
				}
			}
#endif
		}
		else
		{
			// Wet/dry mix path
			const float t1 = params.threshold / 3.0f;
			const float t2 = 2.0f * t1;
			const float A = 3.0f / params.threshold;
			constexpr float B = 1.0f / 3.0f;

#if JUCE_USE_SSE_INTRINSICS
			int sample = 0;

			const __m128 vHalf = _mm_set1_ps(0.5f);
			const __m128 vT1 = _mm_set1_ps(t1);
			const __m128 vT2 = _mm_set1_ps(t2);
			const __m128 vThreshold = _mm_set1_ps(params.threshold);
			const __m128 vA = _mm_set1_ps(A);
			const __m128 vB = _mm_set1_ps(B);
			const __m128 vWet = _mm_set1_ps(params.wet);
			const __m128 vOne = _mm_set1_ps(1.0f);
			const __m128 vTwo = _mm_set1_ps(2.0f);
			const __m128 signMask = _mm_set1_ps(-0.0f);

			for (; sample <= params.samples - 4; sample += 4)
			{
				float* pIn = params.buffer + sample;
				const __m128 in = _mm_load_ps(pIn);

				const __m128 signIn = _mm_or_ps(_mm_and_ps(in, signMask), vOne);
				const __m128 absIn = _mm_mul_ps(signIn, _mm_mul_ps(in, vHalf));

				const __m128 maskMid = _mm_and_ps(_mm_cmpge_ps(absIn, vT1), _mm_cmplt_ps(absIn, vT2));
				const __m128 maskHigh = _mm_cmpge_ps(absIn, vT2);
				const __m128 applyMask = _mm_or_ps(maskMid, maskHigh);

				const __m128 tmp = _mm_sub_ps(vTwo, _mm_mul_ps(vA, absIn));
				const __m128 tmp2 = _mm_mul_ps(tmp, tmp);
				const __m128 midClipped = _mm_mul_ps(signIn, _mm_mul_ps(vThreshold, _mm_sub_ps(vOne, _mm_mul_ps(vB, tmp2))));
				const __m128 highClipped = _mm_mul_ps(signIn, vThreshold);

				__m128 clipped = _mm_blendv_ps(midClipped, highClipped, maskHigh);

				// Dry/wet mix: in + wet * (clipped - in)
				const __m128 delta = _mm_sub_ps(clipped, in);
				const __m128 mixed = _mm_add_ps(in, _mm_mul_ps(vWet, delta));

				const __m128 result = _mm_blendv_ps(in, mixed, applyMask);

				_mm_store_ps(pIn, result);
			}

			// Tail loop
			for (; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				const float signIn = in > 0.0f ? 1.0f : -1.0f;
				const float absIn = signIn * (0.5f * in);

				if (absIn < t1)
					continue;
				else if (absIn < t2)
				{
					const float tmp = 2.0f - A * absIn;
					const float clipped = signIn * params.threshold * (1.0f - B * tmp * tmp);
					in = in + params.wet * (clipped - in);
				}
				else
				{
					const float clipped = signIn * params.threshold;
					in = in + params.wet * (clipped - in);
				}
			}

#else
			// Scalar-only path
			for (unsigned int sample = 0; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				const float signIn = in > 0.0f ? 1.0f : -1.0f;
				const float absIn = signIn * (0.5f * in);

				if (absIn < t1)
					continue;
				else if (absIn < t2)
				{
					const float tmp = 2.0f - A * absIn;
					const float clipped = signIn * params.threshold * (1.0f - B * tmp * tmp);
					in = in + params.wet * (clipped - in);
				}
				else
				{
					const float clipped = signIn * params.threshold;
					in = in + params.wet * (clipped - in);
				}
			}
#endif
		}
	}

	static void HalfWaveBlock(const Params& params) noexcept
	{
		// Early-out: only wet
		if (Math::almostEquals(params.wet, 1.0f))
		{
#if JUCE_USE_SSE_INTRINSICS
			int sample = 0;

			const __m128 vLo = _mm_set1_ps(0.0f);
			const __m128 vHi = _mm_set1_ps(params.threshold);

			for (; sample <= params.samples - 4; sample += 4)
			{
				float* pIn = params.buffer + sample;
				const __m128 in = _mm_load_ps(pIn);

				__m128 clipped = _mm_max_ps(in, vLo);
				clipped = _mm_min_ps(clipped, vHi);

				_mm_store_ps(pIn, clipped);
			}

			// Tail
			for (; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				float clipped = in < 0.0f ? 0.0f : in;
				clipped = clipped > params.threshold ? params.threshold : clipped;
				in = clipped;
			}
#else
			for (unsigned int sample = 0; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				float clipped = in < 0.0f ? 0.0f : in;
				clipped = clipped > params.threshold ? params.threshold : clipped;
				in = clipped;
			}
#endif
		}
		else
		{
#if JUCE_USE_SSE_INTRINSICS
			int sample = 0;

			const __m128 vLo = _mm_set1_ps(0.0f);
			const __m128 vHi = _mm_set1_ps(params.threshold);
			const __m128 vWet = _mm_set1_ps(params.wet);

			for (; sample <= params.samples - 4; sample += 4)
			{
				float* pIn = params.buffer + sample;
				const __m128 in = _mm_load_ps(pIn);

				__m128 clipped = _mm_max_ps(in, vLo);
				clipped = _mm_min_ps(clipped, vHi);

				// Mix: in + wet*(clipped - in)
				const __m128 delta = _mm_sub_ps(clipped, in);
				const __m128 out = _mm_add_ps(in, _mm_mul_ps(vWet, delta));

				_mm_store_ps(pIn, out);
			}

			// Tail
			for (; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				float clipped = in < 0.0f ? 0.0f : in;
				clipped = clipped > params.threshold ? params.threshold : clipped;
				in = in + params.wet * (clipped - in);
			}
#else
			for (unsigned int sample = 0; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				float clipped = in < 0.0f ? 0.0f : in;
				clipped = clipped > params.threshold ? params.threshold : clipped;
				in = in + params.wet * (clipped - in);
			}
#endif
		}
	}

	static void ABSBlock(const Params& params) noexcept
	{
		// Early-out: only wet
		if (Math::almostEquals(params.wet, 1.0f))
		{
#if JUCE_USE_SSE_INTRINSICS
			int sample = 0;

			const __m128 vThreshold = _mm_set1_ps(params.threshold);
			const __m128 signMask = _mm_set1_ps(-0.0f); // to extract abs

			for (; sample <= params.samples - 4; sample += 4)
			{
				float* pIn = params.buffer + sample;
				const __m128 in = _mm_load_ps(pIn);

				// Absolute value
				const __m128 absIn = _mm_andnot_ps(signMask, in);

				// Clip
				const __m128 clipped = _mm_min_ps(absIn, vThreshold);

				_mm_store_ps(pIn, clipped);
			}

			// Scalar tail
			for (; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				const float clipped = std::fabsf(in);
				in = clipped > params.threshold ? params.threshold : clipped;
			}
#else
			for (unsigned int sample = 0; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				const float clipped = std::fabsf(in);
				in = clipped > params.threshold ? params.threshold : clipped;
			}
#endif
		}
		else
		{
#if JUCE_USE_SSE_INTRINSICS
			int sample = 0;

			const __m128 vThreshold = _mm_set1_ps(params.threshold);
			const __m128 signMask = _mm_set1_ps(-0.0f);
			const __m128 vWet = _mm_set1_ps(params.wet);

			for (; sample <= params.samples - 4; sample += 4)
			{
				float* pIn = params.buffer + sample;
				const __m128 in = _mm_load_ps(pIn);

				const __m128 absIn = _mm_andnot_ps(signMask, in);
				const __m128 clipped = _mm_min_ps(absIn, vThreshold);

				// Wet/dry mix: in + wet*(clipped - in)
				const __m128 delta = _mm_sub_ps(clipped, in);
				const __m128 out = _mm_add_ps(in, _mm_mul_ps(vWet, delta));

				_mm_store_ps(pIn, out);
			}

			// Scalar tail
			for (; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				const float clipped = std::fabsf(in);
				in = clipped > params.threshold ? params.threshold : clipped;
			}
#else
			for (unsigned int sample = 0; sample < params.samples; ++sample)
			{
				float& in = params.buffer[sample];
				const float clipped = std::fabsf(in);
				in = clipped > params.threshold ? params.threshold : clipped;
			}
#endif
		}
	}

	static void CrispBlock(const Params& params) noexcept
	{
		constexpr float dB = 4.5f;
		const float thresholdHi = params.threshold * juce::Decibels::decibelsToGain(dB);
		const float thresholdLow = params.threshold * juce::Decibels::decibelsToGain(-dB);

		for (int sample = 0; sample < params.samples; ++sample)
		{
			float& in = params.buffer[sample];
			const float absx = Math::fabsf(in);
			const float sign = in > 0.0f ? 1.0f : -1.0f;
			float clipped = absx;

			if (params.threshold < absx)
			{
				if (absx < thresholdHi)
				{
					clipped = Math::remap(absx, params.threshold, thresholdHi, params.threshold, thresholdLow);
				}
				else
				{
					clipped = thresholdLow;
				}

				// Add noise
				const float range = params.threshold - clipped;
				const float noise = range * ((float)rand() / (float)RAND_MAX);
				clipped += noise;
			}

			clipped *= sign;

			in = in + params.wet * (clipped - in);
		}
	}
};

//==============================================================================
class SlopeClipper
{
public:
	SlopeClipper() = default;
	~SlopeClipper() = default;

	inline void set(const int sampleRate)
	{
		constexpr float ratioDefault = 0.5f * 48000.0f;
		m_ratio = ratioDefault / (float)sampleRate;
	}
	inline float process(const float in, const float threshold)
	{
		const float thresholdAdjusted = 0.5f * threshold;
		const float out = m_inLast;

		if (in > 0.0f)
		{
			if (in > thresholdAdjusted)
			{
				m_inLast += (threshold - m_inLast) * m_ratio;
			}
			else
			{
				m_inLast = in;
			}
		}
		else
		{
			if (in < -thresholdAdjusted)
			{
				m_inLast += (-threshold - m_inLast) * m_ratio;
			}
			else
			{
				m_inLast = in;
			}
		}

		return out;
	};

private:
	float m_ratio = 0.1f;
	float m_inLast = 0.0f;
};

//==============================================================================
class SoftClipper
{
public:
	SoftClipper() = default;
	~SoftClipper() = default;

	inline void init(const int sampleRate)
	{
		constexpr float SoftClipperThresholdRatio = 0.3f;

		m_softClipperThresholdRatio = SoftClipperThresholdRatio * 48000.0f / static_cast<float>(sampleRate);
	};

	inline float process(const float in)
	{
		constexpr float SoftClipperThresholdPositive = 0.9f;
		constexpr float SoftClipperThresholdNegative = -0.9f;

		float out = 0.0f;

		if (in > SoftClipperThresholdPositive)
		{
			out = SoftClipperThresholdPositive + (in - SoftClipperThresholdPositive) * m_softClipperThresholdRatio;
		}
		else if (in < SoftClipperThresholdNegative)
		{
			out = SoftClipperThresholdNegative + (in - SoftClipperThresholdNegative) * m_softClipperThresholdRatio;
		}
		else
		{
			out = in;
		}

		return Math::clamp(out, -1.0f, 1.0f);
	};

private:
	float m_softClipperThresholdRatio = 0.0f;
};