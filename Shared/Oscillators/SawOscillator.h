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

class SawOscillator
{
public:
	SawOscillator() = default;
	~SawOscillator() = default;

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
	}
	inline void set(float frequency)
	{
		m_step =  2.0f * frequency / (float)m_sampleRate;
	}
	inline float process()
	{
		m_phase += m_step;
		
		if (m_phase >= 1.0f)
		{
			m_phase -= 2.0f;
		}

		return m_phase;
	}
	inline void release()
	{
		m_sampleRate = 48000;
		m_step = 0.0f;
		m_phase = 0.0f;
	};
	
private:
	int m_sampleRate = 48000;
	float m_step = 0.0f;
	float m_phase = 0.0f;
};