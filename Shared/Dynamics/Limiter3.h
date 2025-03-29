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
		Peak() : m_samples(0), m_step(0.0)
		{
		};
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

		m_peaks = new Peak[size];
		m_attackSizeMax = size;
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
	inline void setAttackTime(const float attackMS)
	{
		const auto tmp = attackMS * 0.001f * (float)m_sampleRate;
		m_attackSize = (int)(tmp);
		m_attackFactor = 1.0f / tmp;

		m_envelopeFollower.setAttackSize(1);
	}
	inline void setAttackSize(const int attackSize)
	{
		m_attackSize = attackSize;
		m_attackFactor = 1.0f / (float)attackSize;

		m_envelopeFollower.setAttackSize(1);
	}
	inline void setReleaseTime(const float releaseMS)
	{
		m_envelopeFollower.setReleaseTime(releaseMS);
	}
	inline void setThreshold(const float threshold)
	{
		m_threshold = threshold;
		m_thresholddB = Math::gainTodB(threshold);
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

		// Detect peak
		const float inAbs = std::fabsf(in);
		if (inAbs > m_threshold)
		{
			const float attenuatedB = Math::gainTodB(inAbs) - m_thresholddB;
			const float step = attenuatedB * m_attackFactor;
			
			Peak& peak = m_peaks[m_peaksWritteIndex];
			m_peaksWritteIndex++;
			if (m_peaksWritteIndex >= m_attackSizeMax)
			{
				m_peaksWritteIndex -= m_attackSizeMax;
			}

			peak.m_samples = m_attackSize;
			peak.m_step = step;
			peak.m_value = 0.0f;
		}
		
		// Process peaks
		float max = 0.0f;
		//const int validPeaksCount = m_firstValidPeakIndex <= m_peaksWritteIndex ? m_peaksWritteIndex - m_firstValidPeakIndex + 1 : m_firstValidPeakIndex + m_attackSizeMax - m_peaksWritteIndex - 1;

		//for (int i = 0; i < validPeaksCount; i++)
		for (int i = 0; i < m_attackSizeMax; i++)
		{
			//const int idx = (i + m_firstValidPeakIndex) % m_attackSizeMax;
			
			//Peak& peak = m_peaks[idx];
			Peak& peak = m_peaks[i];

			// Find maximum
			const float value = peak.increase();
			if (peak.m_samples > 0 && value > max)
			{
				max = value;
			}

			// Store first invalid peak
			/*if (peak.m_samples <= 0)
			{
				m_firstValidPeakIndex = idx;
			}*/
		}

		// Apply release
		const float maxSmooth = m_envelopeFollower.process(max);
		const float gain = Math::dBToGain(-maxSmooth);

		// Get min gain
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
		
		delete[] m_peaks;
		m_peaks = nullptr;
		
		m_envelopeFollower.release();

		m_gainMin = 1.0;
		m_threshold = 1.0;
		m_thresholddB = 0.0f;
		m_attackFactor = 1.0f;
		m_sampleRate = 48000;
		m_attackSize = 0;
		m_attackSizeMax = 0;
		m_peaksWritteIndex = 0;
		m_firstValidPeakIndex = 0;
	}

private:
	CircularBuffer m_buffer;
	Peak* m_peaks = nullptr;
	BranchingEnvelopeFollower<float> m_envelopeFollower;

	float m_gainMin = 1.0f;	
	float m_threshold = 1.0;
	float m_thresholddB = 0.0f;
	float m_attackFactor = 1.0f;
	int m_sampleRate = 48000;
	int m_attackSize = 0;
	int m_attackSizeMax = 0;
	int m_peaksWritteIndex = 0;
	int m_firstValidPeakIndex = 0;
};