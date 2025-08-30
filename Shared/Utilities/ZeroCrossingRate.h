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

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

class ZeroCrossingRate
{
public:
	ZeroCrossingRate() = default;
	~ZeroCrossingRate() = default;

	inline void init(const int sampleRate, const float hightPassFrequency = 40.0f, const float lowPassFrequency = 440.0f) noexcept
	{
		m_hpFilter.init(sampleRate);
		m_lpFilter.init(sampleRate);

		m_hpFilter.setHighPass(hightPassFrequency, 0.707f);
		m_lpFilter.setLowPass(lowPassFrequency, 0.707f);
	};
	inline void set(const float hightPassFrequency, const float lowPassFrequency) noexcept
	{
		m_hpFilter.setHighPass(hightPassFrequency, 0.707f);
		m_lpFilter.setLowPass(lowPassFrequency, 0.707f);
	}
	inline int process(const float in) noexcept
	{		
		const float inFiltered = m_hpFilter.processDF1(m_lpFilter.processDF1(in));
		
		if (m_inLast > 0.0f && inFiltered < 0.0f || m_inLast < 0.0f && inFiltered > 0.0f)
		{
			m_rateSamples = m_samplesSinceLast;
			m_samplesSinceLast = 0;
		}
		else
		{
			m_samplesSinceLast++;
		}

		m_inLast = inFiltered;

		return m_rateSamples;			// returns zero crossing rate in samples
	}

private:
	BiquadFilter m_hpFilter, m_lpFilter;
	float m_inLast = 0.0f;
	int m_samplesSinceLast = 0;
	int m_rateSamples = 0;
};