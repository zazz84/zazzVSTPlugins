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
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

class Limiter2
{
public:
	Limiter2() {};

	inline void init(const int sampleRate, const int size)
	{
		m_sampleRate = sampleRate;
		m_buffer.init(size);
		m_envelopeFollower.init(sampleRate);
	}
	inline void set(const float attackMS, const float releaseMS, const float threshold)
	{
		m_attackSize = (int)(attackMS * 0.001f * (float)m_sampleRate);
		m_thresholdMultiplier = 1.0f / threshold;
		m_envelopeFollower.set(attackMS, releaseMS);
	}
	inline void setAttackTime(const float attackMS)
	{
		m_attackSize = (int)(attackMS * 0.001f * (float)m_sampleRate);
		m_envelopeFollower.setAttackSize(m_attackSize);
	}
	inline void setAttackSize(const int attackSize)
	{
		m_attackSize = attackSize;
		m_envelopeFollower.setAttackSize(attackSize);
	}
	inline void setReleaseTime(const float releaseMS)
	{
		m_envelopeFollower.setReleaseTime(releaseMS);
	}
	inline void setThreshold(const float threshold)
	{
		m_thresholdMultiplier = 1.0f / threshold;
	}
	inline float getGainMin()
	{
		const float gainMin = m_gainMin;
		m_gainMin = 1.0f;
		return gainMin;
	}
	inline float process(float in)
	{
		// Handle buffer
		const float inDelayed = m_buffer.readDelay(m_attackSize);
		m_buffer.write(in);

		// Get envelope
		const float inAbs = std::fabsf(in);
		const float aboveThresholdNormalized = m_thresholdMultiplier * inAbs;
		const float envelope = std::max(1.0f, m_envelopeFollower.process(aboveThresholdNormalized));

		// Get min gain
		const float gain = 1 / envelope;
		if (gain < m_gainMin)
		{
			m_gainMin = gain;
		}

		// apply attenuation for output
		return gain * inDelayed;
	};
	inline void release()
	{
		m_buffer.release();
		m_envelopeFollower.release();

		m_gainMin = 1.0f;
		m_thresholdMultiplier = 1.0f;
		m_sampleRate = 48000;
		m_attackSize = 0;
	}

private:
	CircularBuffer m_buffer;
	BranchingEnvelopeFollower<float> m_envelopeFollower;

	float m_gainMin = 1.0f;
	float m_thresholdMultiplier = 1.0f;
	int m_sampleRate = 48000;
	int m_attackSize = 0;
};