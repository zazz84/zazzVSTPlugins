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

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

class MovingAverage : CircularBuffer
{
public:
	MovingAverage() = default;
	~MovingAverage() = default;

	inline void init(const unsigned int circularBufferSize) noexcept
	{	
		CircularBuffer::init(circularBufferSize);
		m_normalize = 1.0f / circularBufferSize;
	}
	inline void set(const unsigned int circularBufferSize) noexcept
	{
		CircularBuffer::set(circularBufferSize);
		m_normalize = 1.0f / circularBufferSize;
	};
	inline float process(float in)
	{
		// subtract oldest sample
		m_sum -= read();

		// write new sample
		write(in);

		// add new sample to sum
		m_sum += in;

		return m_sum * m_normalize;
	}

private:
	float   m_sum = 0.0f;
	float   m_normalize = 0.0f;
};