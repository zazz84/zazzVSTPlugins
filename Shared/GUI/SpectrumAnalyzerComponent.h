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
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

enum
{
	fftOrder = 9,
	//fftOrder = 12,
	fftSize = 1 << fftOrder,
	scopeSize = 16
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
		// Values smoothing
		for (int i = 0; i < scopeSize; i++)
		{
			m_filters[i].init(30.0f);
			m_filters[i].set(1.0f);

			m_peaks[i].init(30.0f);
			m_peaks[i].set(0.0f, 1000.0f, 2000.0f);
		}
	}
		
	void setScopeData(const float scopeData[scopeSize])
	{
		for (int i = 0; i < scopeSize; i++)
		{
			m_scopeData[i] = scopeData[i];
		}
	}

	inline void paint(juce::Graphics& g) override
	{
		// Draw background
		g.fillAll(darkColor);

		// Get pixel size
		const auto pixelSize = getWidth() / 18;

		// Draw horizontal lines
		g.setColour(highlightColor);
		g.setOpacity(0.5f);
		
		const float left = static_cast<float>(pixelSize);
		const float right = static_cast<float>(pixelSize * 17);

		for (int i = 0; i < 7; i++)
		{
			g.drawHorizontalLine((i + 1) * pixelSize, left, right);
		}

		// Draw y scale numbers
		g.setColour(juce::Colours::white);
		g.setOpacity(1.0f);
		g.setFont(10.0f);

		juce::Rectangle<int> textBox;
		textBox.setSize(pixelSize, pixelSize);

		for (int i = 0; i < 7; i++)
		{
			textBox.setPosition(right, (i * pixelSize) + (pixelSize / 2));
			g.drawText(juce::String(-12 * i), textBox, juce::Justification::centred, true);
		}

		// Draw x scale numbers
		if (pixelSize > 20)
		{
			const int posY = 7 * pixelSize;

			for (int i = 0; i < scopeSize; i++)
			{
				textBox.setPosition((i + 1) * pixelSize, posY);

				g.drawText(frequencies[i], textBox, juce::Justification::centred, true);
			}
		}
		
		// Draw bars + peaks
		g.setColour(highlightColor);

		const int maxBarHeight = 6 * pixelSize;
		const int barWidth = pixelSize - 1;
		
		int posX = pixelSize;
		juce::Rectangle<int> bounds;

		for (int scopeIndex = 0; scopeIndex < scopeSize; scopeIndex++)
		{
			const auto gainSmooth = m_filters[scopeIndex].process(m_scopeData[scopeIndex]);
			constexpr auto gainCompenation = 6.0f;														// +6dB is gain amplitude compensation for Hann window
			const auto dB = Math::gainTodB(gainSmooth) + gainCompenation;
					
			const auto barHeight = static_cast<int>(Math::remap(dB, -80.0f, 0.0f, 0.0f, maxBarHeight));

			bounds.setSize(barWidth, barHeight);
			bounds.setPosition(posX, maxBarHeight - barHeight + pixelSize);
			g.setOpacity(0.8f);
			g.fillRect(bounds);

			// Draw peak
			const float ratio = static_cast<float>(barHeight) / static_cast<float>(maxBarHeight);
			const auto peakHeight = maxBarHeight * m_peaks[scopeIndex].process(ratio);					// Smooth ratio so it works with plugin resizing

			bounds.setSize(barWidth, 2);
			bounds.setPosition(posX, maxBarHeight - peakHeight + pixelSize);
			g.setOpacity(1.0f);
			g.fillRect(bounds);

			posX += pixelSize;
		}
	}

private:
	OnePoleLowPassFilter m_filters[scopeSize];
	HoldEnvelopeFollower<float> m_peaks[scopeSize];
	float m_scopeData[scopeSize];

	const juce::String frequencies[scopeSize] = { "100", "200", "300", "400", "500", "600", "700", "800", "1k", "2k", "4k", "6k", "8k", "10k", "12k", "16k" };

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerComponent)
};