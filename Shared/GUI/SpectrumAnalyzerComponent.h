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
	fftOrder = 9,				// [1]
	fftSize = 1 << fftOrder,	// [2]
	scopeSize = 16				// [3]
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
		const auto width = getWidth() - 50;
		const auto height = getHeight() - 50;

		const auto bandWidth = (width - (scopeSize + 1 + 2)) / scopeSize;
		g.setFont(10.0f);
		// Draw the scale
		g.setColour(juce::Colours::white); // Set the line color
		const auto count = 8;
		const auto lineHeight = height / (count - 1);
		int y = 0;

		const auto scaleStep = (0 + 80) / count;

		int value = 0;

		juce::Rectangle<int> textBox;
		textBox.setSize(25, lineHeight);

		for (int i = 0; i < count; i++)
		{
			textBox.setPosition(width + 25, y - lineHeight / 2);

			g.drawText(juce::String(value), textBox, juce::Justification::centred, true);

			g.drawLine(width, y, width + 20, y, 1.0f);

			y += lineHeight;
			value -= scaleStep;
		}

		// Frequencies
		int freqPosX = 0;
		int freqValue = 92;

		textBox.setSize(bandWidth, 25);

		for (int i = 0; i < scopeSize; i++)
		{
			textBox.setPosition(freqPosX, height + 25);

			g.drawText(juce::String(0.5f * (fftDataIndexes[i] + fftDataIndexes[i + 1]) * freqValue), textBox, juce::Justification::centred, true);

			freqPosX += bandWidth + 1;
		}
		

		// Bars
		auto xPos = 1;
		g.setColour(juce::Colours::yellow);

		juce::Rectangle<int> bounds;

		for (int scopeIndex = 0; scopeIndex < scopeSize; scopeIndex++)
		{
			const auto gainSmooth = m_filters[scopeIndex].process(m_scopeData[scopeIndex]);
			constexpr auto gainCompenation = 6.0f;			// +6dB is gain amplitude compensation for Hann window
			const auto dB = Math::gainTodB(gainSmooth) + gainCompenation;
					
			const auto bandHeight = static_cast<int>(Math::remap(dB, -80.0f, 0.0f, 0.0f, height));

			bounds.setSize(bandWidth, bandHeight);
			bounds.setPosition(xPos, height - bandHeight);
			g.fillRect(bounds);

			xPos += bandWidth + 1;
		}
	}

private:
	OnePoleLowPassFilter m_filters[scopeSize];
	float m_scopeData[scopeSize];

	int fftDataIndexes[scopeSize+1] = {0, 1, 2, 3, 4, 5, 6, 8, 10, 15, 30, 50, 70, 90, 120, 180, 220};

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerComponent)
};