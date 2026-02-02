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

#include <cstring>

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

//==============================================================================
//[1] https://ccrma.stanford.edu/~jos/Delay/Schroeder_Allpass_Filters.html
//[2] https://www.earlevel.com/main/1997/01/19/a-bit-about-reverb/

// In literature and scientific papers, you can encounter two different ways all pass filter is implemented. See [1] and [2]. But their output is identical.
// When m_feedback > 0.0f, all except the first echo have positive sign. When m_feedback < 0.0f, echoes alternate between positive and negative sign, except the first echo

class AllPassFilter : public CircularBuffer
{
public:
	AllPassFilter() = default;
	~AllPassFilter() = default;

	inline void set(const int size, const float feedback = 0.5f)
	{
		__super::set(size);
		m_feedback = feedback;
	};
	inline void setSize(const int size)
	{
		__super::set(size);
	};
	inline void setFeedback(const float feedback)
	{
		m_feedback = feedback;
	};
	inline void release()
	{
		__super::release();
		
		m_feedback = 0.5f;
	}
	inline float process(const float in) noexcept
	{
		const float delayOut =  read();
		const float delayIn = in + m_feedback * delayOut;
		write(delayIn);
		return delayOut - m_feedback * delayIn;
	};
	inline void processBlock(float* buffer, const unsigned int samples)
	{
		// Get linear buffer
		auto* linearBuffer = getLinearBuffer();

		// Process
		for (unsigned int sample = 0; sample < samples; sample++)
		{
			float& delayOut = linearBuffer[sample];
			const float delayIn = buffer[sample] + m_feedback * delayOut;
			buffer[sample] = delayOut - m_feedback * delayIn;
			delayOut = delayIn;
		}

		// Store linear buffer
		moveLinearBufferToCircularBuffer();
	};

protected:
	float m_feedback = 0.5f;
};

//==============================================================================
// [1] https://www.dsprelated.com/freebooks/pasp/Freeverb_Allpass_Approximation.html
// [2] 1962_Schroeder_Natural Sounding Artificial Reverb.pdf

// TODO
class SchroederAllPassFilter : public CircularBuffer
{
	SchroederAllPassFilter() = default;
	~SchroederAllPassFilter() = default;
};

//==============================================================================
// Not a true all pass filter
// Made by mistake because I am fucking idiot
// More like comp filter with dry path controled by m_feedback

class AllPassFilter2 : public AllPassFilter
{
public:
	AllPassFilter2() = default;
	~AllPassFilter2() = default;

	inline float process(const float in) noexcept
	{
		const float delayOut = read();
		write(in + 0.5f * delayOut);
		return 0.5f * (delayOut - in);
	};
};