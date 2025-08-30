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

#include "../../../zazzVSTPlugins/Shared/Oscillators/SinOscillator.h"
#include "../../../zazzVSTPlugins/Shared/Oscillators/SawOscillator.h"

class SinSawOscillator
{
public:
	SinSawOscillator() = default;
	~SinSawOscillator() = default;

	inline void init(const int sampleRate)
	{
		m_sinOscillator.init(sampleRate);
		m_sawOscillator.init(sampleRate);
	}
	inline void set(float frequency, float ratio)
	{
		m_sinOscillator.set(frequency);
		m_sawOscillator.set(frequency);

		m_ratio = ratio;
	}
	inline float process()
	{
		const float sin = m_sinOscillator.process();
		const float saw = m_sawOscillator.process();

		return sin * (1.0f - m_ratio) + saw * m_ratio;
	}
	inline void release()
	{
		m_sinOscillator.release();
		m_sawOscillator.release();
		m_ratio = 0.0f;
	};

private:
	SinOscillator m_sinOscillator;
	SawOscillator m_sawOscillator;
	float m_ratio = 0.0f;
};
