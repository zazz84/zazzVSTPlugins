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

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class FrequencyMeterComponent : public juce::Component
{
public:
	FrequencyMeterComponent() = default;
	~FrequencyMeterComponent() = default;

	inline void paint(juce::Graphics& g) override
	{
		//SIZE: n x 2

		const auto width = getWidth();
		const auto height = getHeight();

		const auto pixelSize = height / 2;
		const auto pixelSizeHalf = height / 4;

		const auto meterWidth = width - pixelSize;
		const auto meterHeight = pixelSize;

		// Draw meter background
		juce::Rectangle<int> bounds;
		
		bounds.setSize(meterWidth, meterHeight);

		const auto posX = pixelSizeHalf;
		const auto posY = pixelSizeHalf;

		bounds.setPosition(posX, posY);

		g.setColour(lightColor);
		g.setOpacity(1.0f);
		g.fillRect(bounds);

		// Draw limit regions
		const auto m_frequency20Mel = Math::frequenyToMel(20.0f);
		const auto m_frequency20kMel = Math::frequenyToMel(20000.0f);

		const auto limitMinX = static_cast<int>(Math::remap(m_frequencyMinMel, m_frequency20Mel, m_frequency20kMel, static_cast<float>(posX), static_cast<float>(posX + meterWidth)));
		const auto limitMaxX = static_cast<int>(Math::remap(m_frequencyMaxMel, m_frequency20Mel, m_frequency20kMel, static_cast<float>(posX), static_cast<float>(posX + meterWidth)));

		// Min limit
		bounds.setSize(limitMinX - posX, meterHeight);
		bounds.setPosition(posX, posY);
		g.setColour(darkColor);
		g.setOpacity(0.5f);
		g.fillRect(bounds);

		// Max limit
		bounds.setSize(posX + meterWidth - limitMaxX, meterHeight);
		bounds.setPosition(limitMaxX, posY);
		g.setColour(darkColor);
		g.setOpacity(0.5f);
		g.fillRect(bounds);

		// Draw marker
		const auto centerX = static_cast<int>(Math::remap(m_frequencyMel, m_frequency20Mel, m_frequency20kMel, static_cast<float>(posX), static_cast<float>(posX + meterWidth)));
		bounds.setSize(4, meterHeight);
		bounds.setPosition(centerX - 2, posY);
		g.setColour(highlightColor);
		g.setOpacity(1.0f);
		g.fillRect(bounds);
	
		// Draw marker value
		g.setFont(static_cast<float>(pixelSizeHalf / 2));
		g.setColour(juce::Colours::white);
		g.setOpacity(1.0f);
		
		bounds.setSize(pixelSize, pixelSizeHalf);
		bounds.setPosition(centerX - pixelSizeHalf, pixelSize + pixelSizeHalf);

		if (m_frequency < 1000.0f)
		{
			g.drawText(juce::String(static_cast<int>(m_frequency)) + " Hz", bounds, juce::Justification::centred);
		}
		else
		{
			g.drawText(juce::String(0.001f * m_frequency, 1) + " kHz", bounds, juce::Justification::centred);
		}

		// Draw scale values		
		// 20Hz
		bounds.setSize(pixelSize, pixelSizeHalf);
		bounds.setPosition(0, 0);
		g.drawText("20 Hz", bounds, juce::Justification::centred);

		// 20kHz
		bounds.setSize(pixelSize, pixelSizeHalf);
		bounds.setPosition(width - pixelSize, 0);
		g.drawText("20 kHz", bounds, juce::Justification::centred);
	}
	inline void set(const float frequency, const float frequencyMin, const float frequencyMax)
	{
		m_frequencyMin = frequencyMin;
		m_frequencyMinMel = Math::frequenyToMel(frequencyMin);

		m_frequencyMax = Math::fmaxf(frequencyMin, frequencyMax);
		m_frequencyMaxMel = Math::frequenyToMel(m_frequencyMax);

		m_frequency = Math::clamp(frequency, frequencyMin, m_frequencyMax);
		m_frequencyMel = Math::frequenyToMel(m_frequency);
	}

private:
	float m_frequency = 20.0f;
	float m_frequencyMin = 20.0f;
	float m_frequencyMax = 20000.0f;
	float m_frequencyMel = 20.0f;
	float m_frequencyMinMel = 20.0f;
	float m_frequencyMaxMel = 20000.0f;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);
};