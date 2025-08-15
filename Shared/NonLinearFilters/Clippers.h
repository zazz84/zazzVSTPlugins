/*
 * Copyright (C) 2025 Filip Cenzak (filip.c@centrum.cz)
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
//#include <algorithm>
//#include <mmintrin.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class Clippers
{

public:
	inline static float HardClip(const float in, const float threshold)
	{
		return Math::clamp(in, -threshold, threshold);
	}

	inline static float SoftClip(const float in, float const threshold)
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

	inline static float FoldBack(const float in, float const threshold)
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

	inline static float HalfWave(const float in, float const threshold)
	{
		return Math::clamp(in, 0.0f, threshold);
	}

	inline static float ABS(const float in, const float threshold)
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
};

//==============================================================================
class SlopeClipper
{
public:
	SlopeClipper() = default;
	~SlopeClipper() = default;

	inline void init(const int sampleRate)
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
constexpr auto SoftClipperThresholdPositive = 0.9f;
constexpr auto SoftClipperThresholdNegative = -0.9f;
constexpr auto SoftClipperThresholdRatio = 0.3f;

class SoftClipper
{
public:
	SoftClipper() = default;
	~SoftClipper() = default;

	inline void init(const int sampleRate)
	{
		m_softClipperThresholdRatio = SoftClipperThresholdRatio * 48000.0f / static_cast<float>(sampleRate);
	};

	inline float process(const float in)
	{
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