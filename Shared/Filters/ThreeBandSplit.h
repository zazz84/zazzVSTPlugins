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

#include "../../../zazzVSTPlugins/Shared/Filters/LinkwitzRileyFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/FirstOrderAllPassFilter.h"

class ThreeBandSplit
{
public:
	ThreeBandSplit() = default;
	~ThreeBandSplit() = default;

	inline void init(int sampleRate)
	{
		m_lowMidFilter.init(sampleRate);
		m_midHighFilter.init(sampleRate);
		m_lowAllPass.init(sampleRate);
	};
	inline void set(float frequency1, float frequenyc2)
	{
		m_lowMidFilter.set(frequency1);
		m_lowAllPass.set(frequenyc2);
		m_midHighFilter.set(frequenyc2);
	};
	inline void process(float sample, float& low, float& mid, float& high)
	{
		// First split
		low = m_lowMidFilter.processLP(sample);
		low = m_lowAllPass.process(low);

		const float midHighLeft = m_lowMidFilter.processHP(sample);

		// Second split
		mid = m_midHighFilter.processLP(midHighLeft);
		high = m_midHighFilter.processHP(midHighLeft);
	};

private:
	LinkwitzRileyFilter m_lowMidFilter;
	LinkwitzRileyFilter m_midHighFilter;
	FirstOrderAllPassFilter m_lowAllPass;
};
