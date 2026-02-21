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

class SinOscillator
{
public:
	SinOscillator() = default;
	~SinOscillator() = default;

	static constexpr float PI2 = 2.0f * 3.14159265358979f;

	inline void init(const int sampleRate) noexcept
	{
		m_samplingPeriod = 1.0f / sampleRate;
	}
	inline void set(float frequency, float phase = 0.0f) noexcept
	{
		m_step = PI2 * frequency * m_samplingPeriod;

		// Set phase if defined
		if (phase > 0.0f)
		{
			m_phase = phase;
		}
	}
	inline float process() noexcept
	{
		m_phase += m_step;
		
		if (m_phase >= PI2)
		{
			m_phase -= PI2;
		}

		return std::sinf(m_phase);
	}
	inline void release() noexcept
	{
		m_step = 0.0f;
		m_phase = 0.0f;
		m_samplingPeriod = 1.0f / 48000.0f;
	};
	
private:
	float m_step = 0.0f;
	float m_phase = 0.0f;
	float m_samplingPeriod = 1.0f / 48000.0f;
};