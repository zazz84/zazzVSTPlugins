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

#define M_PI 3.14159265f

//==============================================================================
class OnePoleFilter
{
public:
	OnePoleFilter() = default;
	~OnePoleFilter() = default;

	inline void release() noexcept
	{
		m_samplePeriod = 0.00002f;
		m_sampleLast = 0.0f;
		m_a0 = 1.0f;
	};

protected:
	float m_samplePeriod = 0.00002f;
	float m_sampleLast = 0.0f;
	float m_a0 = 1.0f;
};

//==============================================================================
class OnePoleLowPassFilter : public OnePoleFilter
{
public:
	OnePoleLowPassFilter() = default;
	~OnePoleLowPassFilter() = default;

	inline void init(const int sampleRate) noexcept
	{
		m_samplePeriod = 1.0f / static_cast<float>(sampleRate);
	};
	inline void set(const float frequency) noexcept
	{
		m_a0 = frequency * M_PI * m_samplePeriod;
	};
	inline void setCoef(const float b1)
	{
		m_a0 = 1.0f - b1;
	};
	inline float process(const float sample) noexcept
	{
		return m_sampleLast = m_a0 * (sample - m_sampleLast) + m_sampleLast;
	};
};