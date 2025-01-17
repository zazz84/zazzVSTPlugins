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

class FirstOrderAllPassFilter
{
public:
	FirstOrderAllPassFilter() {};

	inline void init(const int sampleRate)
	{
		m_SampleRate = sampleRate;
	};
	inline void set(const float frequency)
	{
		const float tmp = tanf(3.141592f * frequency / m_SampleRate);
		m_a1 = (tmp - 1.0f) / (tmp + 1.0f);
	}
	inline float process(const float in)
	{
		const float tmp = m_a1 * in + m_d;
		m_d = in - m_a1 * tmp;
		return tmp;
}	;

protected:
	int m_SampleRate;
	float m_a1 = -1.0f; // all pass filter coeficient
	float m_d = 0.0f;   // history d = x[n-1] - a1y[n-1]
};