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

class FourBandSplit
{
public:
	FourBandSplit() = default;
	~FourBandSplit() = default;

	inline void init(int sampleRate)
	{
		m_band1Filter.init(sampleRate);
		m_band2Filter.init(sampleRate);
		m_band3Filter.init(sampleRate);

		m_band1AllPass.init(sampleRate);
		m_band3AllPass.init(sampleRate);
	};
	inline void set(float frequency1, float frequenyc2, float frequency3)
	{
		m_band1Filter.set(frequency1);
		m_band2Filter.set(frequenyc2);
		m_band3Filter.set(frequency3);

		m_band1AllPass.set(frequency1);
		m_band3AllPass.set(frequency3);
	};
	inline void process(float sample, float& band1, float& band2, float& band3, float& band4)
	{
		float bandLow = m_band2Filter.processLP(sample);
		float bandHigh = m_band2Filter.processHP(sample);

		bandLow = m_band3AllPass.process(bandLow);
		bandHigh = m_band1AllPass.process(bandHigh);

		band1 = m_band1Filter.processLP(bandLow);
		band2 = m_band1Filter.processHP(bandLow);

		band3 = m_band3Filter.processLP(bandHigh);
		band4 = 0.5f * m_band3Filter.processHP(bandHigh);
	};

private:
	LinkwitzRileyFilter m_band1Filter;
	LinkwitzRileyFilter m_band2Filter;
	LinkwitzRileyFilter m_band3Filter;

	FirstOrderAllPassFilter m_band1AllPass;
	FirstOrderAllPassFilter m_band3AllPass;
};
