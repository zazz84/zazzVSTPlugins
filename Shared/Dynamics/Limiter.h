#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

class Limiter
{
public:
	Limiter() {};

	inline void init(const int sampleRate, const int size)
	{
		m_sampleRate = sampleRate;
		m_buffer.init(size);
	}
	inline void set(const float attackMS, const float releaseMS, const float threshold)
	{
		m_attackSize = (int)(attackMS * 0.001f * (float)m_sampleRate);
		m_releaseSize = (int)(releaseMS * 0.001f * (float)m_sampleRate);
		m_threshold = threshold;
	}
	inline float process(float in)
	{
		// In
		const float inDelayed = m_buffer.readDelay(m_attackSize);
		m_buffer.write(in);

		// Start attack
		const float inAbs = std::fabsf(in);
		if (inAbs > m_threshold && inAbs > m_currentPeak)
		{
			const float finalMultiplier = m_threshold / inAbs;
			m_interpolationSpeed = (finalMultiplier - m_interpolationMultiplier) / m_attackSize;
			m_currentPeak = inAbs;
			m_samplesToPeak = m_attackSize;
		}

		// Start release
		if (m_samplesToPeak == 0)
		{
			constexpr float finalMultiplier = 1.0f;
			m_interpolationSpeed = (finalMultiplier - m_interpolationMultiplier) / m_releaseSize;
			m_currentPeak = 0.0f;
		}

		m_samplesToPeak--;

		// Update multiplier
		if (m_samplesToPeak > -m_releaseSize)
		{
			m_interpolationMultiplier += m_interpolationSpeed;
		}

		//Out
		return m_interpolationMultiplier * inDelayed;
	};
	inline void release()
	{
		m_buffer.release();

		m_interpolationMultiplier = 1.0f;
		m_interpolationSpeed = 0.0f;
		m_threshold = 1.0f;
		m_sampleRate = 48000;
		m_samplesToPeak = 0;
		m_attackSize = 0;
		m_releaseSize = 0;
	}

private:
	CircularBuffer m_buffer;

	float m_currentPeak = 0.0f;
	float m_interpolationMultiplier = 1.0f;
	float m_interpolationSpeed = 0.0f;
	float m_threshold = 1.0f;
	int m_sampleRate = 48000;
	int m_samplesToPeak = 0;
	int m_attackSize = 0;
	int m_releaseSize = 0;
};