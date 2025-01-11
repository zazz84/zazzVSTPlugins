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

#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

class Correlation
{
public:
	Correlation() = default;
	~Correlation()
	{
		release();
	};

	inline void init(const int sampleRate)
	{
		m_smoother.init(sampleRate);
		m_smoother.set(5.0f);
	}
	inline float process(const float inLeft, const float inRight)
	{
		//const float phase = inRight != 0.0f ? std::atan(inLeft / inRight) : 0.0f;
		const float phase = inLeft * inRight >= 0.0f ? 1.0f : -1.0f;
		
		return m_smoother.process(phase);
	}
	inline void release()
	{
		m_smoother.release();
	}

private:
	OnePoleLowPassFilter m_smoother;
};