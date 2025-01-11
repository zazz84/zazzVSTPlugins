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

#pragma once

#include <cmath>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

class Balance
{
public:
	Balance() = default;
	~Balance()
	{
		release();
	};

	inline void init(const int sampleRate)
	{
		m_smoother.init(sampleRate);
		m_smoother.set(1.0f);
	}
	inline float processBlock(juce::AudioBuffer<float>& buffer)
	{
		if (buffer.getNumChannels() != 2)
		{
			return 0.0f;
		}

		auto* LChannel = buffer.getWritePointer(0);
		auto* RChannel = buffer.getWritePointer(1);
		
		float out = 0.0f;

		for (int sample = 0; sample < buffer.getNumSamples(); sample++)
		{
			const float left = Math::fabsf(LChannel[sample]);
			const float right = Math::fabsf(RChannel[sample]);

			const float sum = left + right;

			out = sum > 0.0001f ? m_smoother.process((right - left) / sum) : m_smoother.process(0.0f);
		}
		
		return out;
	}
	inline void release()
	{
		m_smoother.release();
	}

private:
	OnePoleLowPassFilter m_smoother;
};
