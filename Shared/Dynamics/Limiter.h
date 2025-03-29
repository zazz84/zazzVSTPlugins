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
		const float factor = 0.001f * (float)m_sampleRate;

		m_attackSize = (int)(attackMS * factor);
		m_releaseSize = (int)(releaseMS * factor);
		m_threshold = threshold;
	}
	inline void setAttackTime(const float attackMS)
	{
		m_attackSize = (int)(attackMS * 0.001f * (float)m_sampleRate);
	}
	inline void setAttackSize(const int attackSize)
	{
		m_attackSize = attackSize;
	}
	inline void setReleaseTime(const float releaseMS)
	{
		m_releaseSize = (int)(releaseMS * 0.001f * (float)m_sampleRate);
	}
	inline void setThreshold(const float threshold)
	{
		m_threshold = threshold;
	}
	inline float getGainMin()
	{
		const float gainMin = m_gainMin;
		m_gainMin = 1.0f;
		return gainMin;
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

		// Get min gain
		if (m_interpolationMultiplier < m_gainMin)
		{
			m_gainMin = m_interpolationMultiplier;
		}

		//Out
		return m_interpolationMultiplier * inDelayed;
	};
	inline void release()
	{
		m_buffer.release();

		m_gainMin = 1.0f;
		m_currentPeak = 0.0f;
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

	float m_gainMin = 1.0f;
	float m_currentPeak = 0.0f;
	float m_interpolationMultiplier = 1.0f;
	float m_interpolationSpeed = 0.0f;
	float m_threshold = 1.0f;
	int m_sampleRate = 48000;
	int m_samplesToPeak = 0;
	int m_attackSize = 0;
	int m_releaseSize = 0;
};