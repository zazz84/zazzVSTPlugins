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

// https://docs.juce.com/master/tutorial_spectrum_analyser.html

#pragma once

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

enum
{
	fftOrder = 11,				// [1]
	fftSize = 1 << fftOrder,	// [2]
	scopeSize = 64				// [3]
};

class SpectrumAnalyzerComponent : public juce::Component
{
public:
	SpectrumAnalyzerComponent()
	{
		init();
	}
	~SpectrumAnalyzerComponent() = default;

	inline void init()
	{
		for (int i = 0; i < scopeSize; i++)
		{
			m_filters[i].init(30.0f);
			m_filters[i].set(1.0f);
		}
	}
		
	void setScopeData(const float scopeData[scopeSize])
	{
		for (int i = 0; i < scopeSize; i++)
		{
			m_scopeData[i] = scopeData[i];
		}

		//memcpy(m_scopeData, scopeData, sizeof(scopeData));
	}

	inline void paint(juce::Graphics& g) override
	{
		const auto width = getWidth();
		const auto height = getHeight();

		const auto bandWidth = (width - (scopeSize + 1)) / scopeSize;

		auto xPos = 1;
		g.setColour(juce::Colours::yellow);

		juce::Rectangle<int> bounds;

		for (int scopeIndex = 0; scopeIndex < scopeSize; scopeIndex++)
		{
			const auto gainSmooth = m_filters[scopeIndex].process(m_scopeData[scopeIndex]);
			constexpr auto gainCompenation = 6.0f;			// +6dB is gain amplitude compensation for Hann window
			const auto dB = Math::gainTodB(gainSmooth) + gainCompenation;
					
			const auto bandHeight = static_cast<int>(Math::remap(dB, -60.0f, 6.0f, 0.0f, height));

			bounds.setSize(bandWidth, bandHeight);
			bounds.setPosition(xPos, height - bandHeight);
			g.fillRect(bounds);

			xPos += bandWidth + 1;
		}
	}

private:
	OnePoleLowPassFilter m_filters[scopeSize];
	float m_scopeData[scopeSize];

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerComponent)
};