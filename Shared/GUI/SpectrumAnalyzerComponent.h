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
	//fftOrder = 9,
	fftOrder = 10,
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

	enum Type
	{
		Max,
		LeftAndRight,
		MS,
		Mono
	};
	inline void init()  noexcept
	{
		// Values smoothing
		for (int i = 0; i < scopeSize; i++)
		{
			m_filters[0][i].init(30.0f);
			m_filters[1][i].init(30.0f);
			m_filters[0][i].set(1.0f);
			m_filters[1][i].set(1.0f);

			m_peaks[0][i].init(30.0f);
			m_peaks[1][i].init(30.0f);
			m_peaks[0][i].set(0.0f, 1000.0f, 2000.0f);
			m_peaks[1][i].set(0.0f, 1000.0f, 2000.0f);
		}

		std::fill(std::begin(m_scopeDataL), std::end(m_scopeDataL), 0.0f);
		std::fill(std::begin(m_scopeDataR), std::end(m_scopeDataR), 0.0f);
	};
	inline void setScopeDataL(const float scopeData[scopeSize])  noexcept
	{
		for (int i = 0; i < scopeSize; i++)
		{
			m_scopeDataL[i] = scopeData[i];
		}
	};
	inline void setScopeDataR(const float scopeData[scopeSize])  noexcept
	{
		for (int i = 0; i < scopeSize; i++)
		{
			m_scopeDataR[i] = scopeData[i];
		}
	};
	inline void setType(const Type type)  noexcept
	{
		m_type = type;
	};
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
			textBox.setPosition(static_cast<int>(right), (i * pixelSize) + (pixelSize / 2));
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
		const int maxBarHeight = 6 * pixelSize;
		const int barWidth = pixelSize - 1;

		int posX = pixelSize;
		juce::Rectangle<int> bounds;

		if (m_type == Type::Mono)
		{
			for (int scopeIndex = 0; scopeIndex < scopeSize; scopeIndex++)
			{
				const auto gainSmoothL = m_filters[0][scopeIndex].process(m_scopeDataL[scopeIndex]);
				constexpr auto gainCompenation = 6.0f;														// +6dB is gain amplitude compensation for Hann window
				const auto dBL = Math::gainTodB(gainSmoothL) + gainCompenation;

				const auto barHeightL = static_cast<int>(Math::remap(dBL, -80.0f, 0.0f, 0.0f, maxBarHeight));

				// Draw bars

				bounds.setSize(barWidth, barHeightL);
				bounds.setPosition(posX, maxBarHeight - barHeightL + pixelSize);
				g.setColour(highlightColor);
				g.setOpacity(0.8f);
				g.fillRect(bounds);

				// Draw peak
				const float ratioL = static_cast<float>(barHeightL) / static_cast<float>(maxBarHeight);
				const auto peakHeightL = static_cast<int>(static_cast<float>(maxBarHeight) * m_peaks[0][scopeIndex].process(ratioL));					// Smooth ratio so it works with plugin resizing
		
				// Draw bars
				bounds.setSize(barWidth, 2);
				bounds.setPosition(posX, maxBarHeight - peakHeightL + pixelSize);
				g.setColour(highlightColor);
				g.setOpacity(1.0f);
				g.fillRect(bounds);

				posX += pixelSize;
			}
		}
		else
		{
			for (int scopeIndex = 0; scopeIndex < scopeSize; scopeIndex++)
			{
				const auto gainSmoothL = m_filters[0][scopeIndex].process(m_scopeDataL[scopeIndex]);
				const auto gainSmoothR = m_filters[1][scopeIndex].process(m_scopeDataR[scopeIndex]);
				constexpr auto gainCompenation = 6.0f;														// +6dB is gain amplitude compensation for Hann window
				const auto dBL = Math::gainTodB(gainSmoothL) + gainCompenation;
				const auto dBR = Math::gainTodB(gainSmoothR) + gainCompenation;

				const auto barHeightL = static_cast<int>(Math::remap(dBL, -80.0f, 0.0f, 0.0f, maxBarHeight));
				const auto barHeightR = static_cast<int>(Math::remap(dBR, -80.0f, 0.0f, 0.0f, maxBarHeight));

				// Draw bars
				if (m_type == Type::Max)
				{
					const auto barHeightAvg = (barHeightL + barHeightR) / 2;

					bounds.setSize(barWidth, barHeightAvg);
					bounds.setPosition(posX, maxBarHeight - barHeightAvg + pixelSize);
					g.setColour(highlightColor);
					g.setOpacity(0.8f);
					g.fillRect(bounds);
				}
				else if (m_type == Type::LeftAndRight || m_type == Type::MS)
				{
					const auto barWidthHalf = barWidth / 2;

					// Left
					bounds.setSize(barWidthHalf, barHeightL);
					bounds.setPosition(posX, maxBarHeight - barHeightL + pixelSize);
					g.setColour(highlightColor);
					g.setOpacity(0.8f);
					g.fillRect(bounds);

					// Right
					bounds.setSize(barWidthHalf, barHeightR);
					bounds.setPosition(posX + barWidthHalf, maxBarHeight - barHeightR + pixelSize);
					g.setColour(highlightAnalogousColor);
					g.setOpacity(0.8f);
					g.fillRect(bounds);
				}

				// Draw peak
				const float ratioL = static_cast<float>(barHeightL) / static_cast<float>(maxBarHeight);
				const float ratioR = static_cast<float>(barHeightR) / static_cast<float>(maxBarHeight);
				const auto peakHeightL = static_cast<int>(static_cast<float>(maxBarHeight) * m_peaks[0][scopeIndex].process(ratioL));					// Smooth ratio so it works with plugin resizing
				const auto peakHeightR = static_cast<int>(static_cast<float>(maxBarHeight) * m_peaks[1][scopeIndex].process(ratioR));					// Smooth ratio so it works with plugin resizing

				// Draw bars
				if (m_type == Type::Max)
				{
					const auto peakHeightAvg = (peakHeightL + peakHeightR) / 2;

					bounds.setSize(barWidth, 2);
					bounds.setPosition(posX, maxBarHeight - peakHeightAvg + pixelSize);
					g.setColour(highlightColor);
					g.setOpacity(1.0f);
					g.fillRect(bounds);
				}
				else if (m_type == Type::LeftAndRight || m_type == Type::MS)
				{
					const auto barWidthHalf = barWidth / 2;

					// Left
					bounds.setSize(barWidthHalf, 2);
					bounds.setPosition(posX, maxBarHeight - peakHeightL + pixelSize);
					g.setColour(highlightColor);
					g.setOpacity(1.0f);
					g.fillRect(bounds);

					// Right
					bounds.setPosition(posX + barWidthHalf, maxBarHeight - peakHeightR + pixelSize);
					g.setColour(highlightAnalogousColor);
					g.setOpacity(1.0f);
					g.fillRect(bounds);
				}

				posX += pixelSize;
			}
		}
	};

private:
	OnePoleLowPassFilter m_filters[2][scopeSize];
	HoldEnvelopeFollower<float> m_peaks[2][scopeSize];
	float m_scopeDataL[scopeSize];
	float m_scopeDataR[scopeSize];
	Type m_type = Type::LeftAndRight;

	const juce::String frequencies[scopeSize] = { "100", "200", "300", "400", "500", "600", "700", "800", "1k", "2k", "4k", "6k", "8k", "10k", "12k", "16k" };

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);
	juce::Colour highlightComplementaryColor = juce::Colour::fromRGB(115, 115, 217);
	juce::Colour highlightAnalogousColor = juce::Colour::fromRGB(217, 166, 115);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzerComponent)
};