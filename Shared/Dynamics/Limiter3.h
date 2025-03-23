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

#include <vector>

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class Limiter3
{
public:
	Limiter3() = default;
	~Limiter3() = default;

	struct Peak
	{
		Peak(int samples, float step) : m_samples(samples), m_step(step)
		{
		};
		float increase()
		{
			m_samples--;
			return m_value += m_step;
		}

		int m_samples;
		float m_step;
		float m_value = 0.0f;
	};

	inline void init(const int sampleRate, const int size)
	{
		m_sampleRate = sampleRate;
		m_buffer.init(size);
		m_envelopeFollower.init(sampleRate);
	}
	inline void set(const float attackMS, const float releaseMS, const float threshold)
	{
		const auto tmp = attackMS * 0.001f * (float)m_sampleRate;
		m_attackSize = (int)(tmp);
		m_attackFactor = 1.0f / tmp;
		
		m_threshold = threshold;
		m_thresholddB = Math::gainTodB(threshold);
		
		m_envelopeFollower.set(0.0f, releaseMS);
	}
	inline float process(float in)
	{
		// Handle buffer
		const float inDelayed = m_buffer.readDelay(m_attackSize);
		m_buffer.write(in);

		// Detect peak
		const float inAbs = std::fabsf(in);
		if (inAbs > m_threshold)
		{
			const float attenuatedB = Math::gainTodB(inAbs) - m_thresholddB;
			const float step = attenuatedB * m_attackFactor;
			
			m_peaks.push_back(Peak(m_attackSize, step));
		}
		
		// Process peaks
		float max = 0.0f;
		for (int i = m_peaks.size() - 1; i >= 0; i--)
		{
			Peak& peak = m_peaks[i];

			// Find maximum
			const float value = peak.increase();
			if (value > max)
			{
				max = value;
			}

			// Remove invalid peaks
			if (peak.m_samples <= 0)
			{
				m_peaks.erase(m_peaks.begin() + i);
			}
		}

		// Apply release
		const float maxSmooth = m_envelopeFollower.process(max);
		const float gain = Math::dBToGain(-maxSmooth);

		// apply attenuation for output
		return gain * inDelayed;
	};
	inline void release()
	{
		m_buffer.release();
		m_peaks.clear();
		m_envelopeFollower.release();

		m_threshold = 1.0;
		m_thresholddB = 0.0f;
		m_sampleRate = 48000;
		m_attackSize = 0;
	}

private:
	CircularBuffer m_buffer;
	std::vector<Peak> m_peaks;
	BranchingEnvelopeFollower<float> m_envelopeFollower;
	
	float m_threshold = 1.0;
	float m_thresholddB = 0.0f;
	float m_attackFactor = 1.0f;
	int m_sampleRate = 48000;
	int m_attackSize = 0;
};