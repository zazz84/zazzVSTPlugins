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

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

class AllPassFilter
{
public:
	AllPassFilter() {};

	inline void init(const int size)
	{ 
		m_buffer.init(size);
	};
	inline void set(const float feedback, const int size) 
	{ 
		m_feedback = feedback;
		m_buffer.set(size);
	};
	inline void setFeedback(const float feedback)
	{
		m_feedback = feedback;
	}
	float process(const float in) noexcept
	{
		const float delayOut = m_buffer.read();
		m_buffer.write(in - m_feedback * delayOut);

		return delayOut + m_feedback * in;
	};
	inline void release()
	{
		m_buffer.release();
		m_feedback = 0.0f;
	}

private:
	CircularBuffer m_buffer;
	float m_feedback = 0.0f;
};

//==============================================================================

class AllPassFilterSimple : public CircularBuffer
{
public:
	AllPassFilterSimple() = default;
	~AllPassFilterSimple() = default;

	inline float process(const float in) noexcept
	{
		const float delayOut = read();
		write(in + 0.5f * delayOut);
		return 0.5f * (delayOut - in);
	};
	inline void processReplace(float& in) noexcept
	{
		/*const float delayOut = read();
		write(in + 0.5f * delayOut);
		in =  0.5f * (delayOut - in);*/

		// Same as code above but written differently
		const float delayOut = 0.5f * read();
		const float delayIn = in - delayOut;
		write(delayIn);
		in *= 0.5f;
		in += delayOut;
	};
};