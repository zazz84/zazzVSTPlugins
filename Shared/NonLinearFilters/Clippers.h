#pragma once

#include <math.h>

class Clippers
{
public:
	inline static float HardClip(float in, float threshold)
	{
		if (in > threshold)
		{
			return threshold;
		}
		else if (in < -threshold)
		{
			return -threshold;
		}
		else
		{
			return in;
		}
	}

	inline static float SoftClip(float in, float threshold)
	{
		const float thresholdHalf = 0.5f * threshold;
		const float sign = in > 0.0f ? 1.0f : -1.0f;
		const float inAbs = in * sign;

		if (inAbs < thresholdHalf)
		{
			return sign * inAbs;
		}
		else if (inAbs < threshold + thresholdHalf)
		{
			return sign * 0.5f * (inAbs + thresholdHalf);
		}
		else
		{
			return sign * threshold;
		}
	}

	inline static float FoldBack(float in, float threshold)
	{
		if (in > threshold || in < -threshold)
		{
			in = std::fabsf(std::fabsf(std::fmodf(in - threshold, 4.0f * threshold)) - 2.0f * threshold) - threshold;
		}

		return in;
	}

	inline static float HalfWave(float in, float threshold)
	{
		if (in < 0.0f)
		{
			return 0.0f;
		}
		else if (in < threshold)
		{
			return in;
		}
		else
		{
			return threshold;
		}
	}

	inline static float ABS(float in, float threshold)
	{
		const float inAbs = std::fabsf(in);

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

class SlopeClipper
{
public:
	SlopeClipper() {};

	inline void init(int sampleRate)
	{
		constexpr float ratioDefault = 0.15f * 48000.0f;
		m_ratio = ratioDefault / (float)sampleRate;
	}

	inline float process(float in, float threshold)
	{
		const float out = m_inLast;

		if (in > 0.0f)
		{
			if (in > threshold)
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
			if (in < -threshold)
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