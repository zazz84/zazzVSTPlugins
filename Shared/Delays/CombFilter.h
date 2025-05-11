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

#include "../../../zazzVSTPlugins/Shared/Delays/AllPassFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

//==============================================================================
// [1] 1962_Schroeder_Natural Sounding Artificial Reverb.pdf
// [2] Pirkle_Designing Audio Effect Plugins In C - Figure 17.6

class CombFilter : public AllPassFilter
{
public:
	CombFilter() = default;
	~CombFilter() = default;

	// Set m_feedback based on desired RT60 time
	inline void setTime(const float rt60, const float sampleRate)
	{
		const float size = (float)getSize();
		const float exponent = (-3.0f * size) / (rt60 * sampleRate);
		m_feedback = powf(10.0f, exponent);
	}
	inline float process(const float in) noexcept
	{
		const float delayOut = read();
		write(in + m_feedback * delayOut);
		return delayOut;
	};
};

//==============================================================================
// [1] https://ccrma.stanford.edu/~jos/pasp/Lowpass_Feedback_Comb_Filter.html
// [2] Pirkle_Designing Audio Effect Plugins In C - Figure 17.16

class LowPassCombFilter : public CombFilter
{
public:
	LowPassCombFilter() = default;
	~LowPassCombFilter() = default;

	inline void init(const int size, const int sampleRate)
	{
		__super::init(size);
		m_filter.init(sampleRate);
	};
	inline void set(const int size, const float feedback, const float frequency)
	{
		__super::set(size, feedback);
		m_filter.set(frequency);
	};
	inline void setFrequency(const float frequency)
	{
		m_filter.set(frequency);
	};
	inline void setDamping(const float damping)
	{
		m_filter.setCoef(damping);
	};
	inline float process(const float in) noexcept
	{
		const float delayOut = read();
		write(in - m_feedback * m_filter.process(delayOut));
		return delayOut;
	};
	inline void release()
	{
		__super::release();
		m_filter.release();
	};

private:
	OnePoleLowPassFilter m_filter;
};