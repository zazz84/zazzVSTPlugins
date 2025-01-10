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

#include <cmath>

class PeakDetector
{
public:
	PeakDetector() {};

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
	};
	inline void set(const float holdTimeMS, const float releaseTimeMS)
	{
		m_holdTimeSamples = static_cast<int>(0.001f * holdTimeMS * static_cast<float>(m_sampleRate));
		m_decayCoefficient = std::expf(-1.0f / (0.001f * releaseTimeMS * (float)m_sampleRate));
	};
	inline float process(const float in)
	{
		// Compute the absolute value of the input
		const float inAbs = (in > 0.0f) ? in : -in;

		// Update the peak value
		if (inAbs > m_out)
		{
			m_out = inAbs;							// Instant rise to the new peak value
			m_holdCounter = m_holdTimeSamples;		// Reset hold counter
		}
		else
		{
			if (m_holdCounter > 0)
			{
				m_holdCounter--; // Hold the envelope
			}
			else
			{
				m_out *= m_decayCoefficient;		// Exponential decay
			}
		}

		return m_out;
	};
	inline void release()
	{
		m_sampleRate = 48000;
		m_decayCoefficient = 0.0f;
		m_out = 0.0f;
	}

private:
	float m_decayCoefficient = 0.0f;
	float m_out = 0.0f;
	int m_sampleRate = 48000;
	int m_holdTimeSamples = 0;
	int m_holdCounter = 0;
};