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

#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class GainMeterComponent : public juce::Component
{
public:
	GainMeterComponent()
	{
		constexpr int sampleRate = 20;
		
		m_smoother.init(sampleRate);
		m_smoother.set(3.0f);

		m_maxEnvelope.init(sampleRate);
		m_minEnvelope.init(sampleRate);

		m_maxEnvelope.set(0.0f, 300.0f, 300.0f);
		m_minEnvelope.set(0.0f, 300.0f, 300.0f);
	}
	~GainMeterComponent()
	{
	}

	static const int SMALL_LOD_WIDTH_LIMIT = 53;

	inline void paint(juce::Graphics& g) override
	{
		//SIZE: 2 x n

		const auto width = getWidth();
		const auto height = getHeight();

		const auto pixelSize = width / 2;

		const auto meterWidth = pixelSize;
		const auto meterWidthHalf = meterWidth / 2;
		const auto meterHeight = height - pixelSize;
		const auto meterHeightHalf = meterHeight / 2;

		juce::Rectangle<int> bounds;
		bounds.setSize(meterWidth, meterHeight);

		const auto xPos = pixelSize / 2;
		const auto yPos = pixelSize / 2;

		bounds.setPosition(xPos, yPos);

		g.setColour(juce::Colours::black);
		g.setOpacity(0.5f);
		g.fillRect(bounds);

		constexpr float minLevel = -18.0f;
		constexpr float maxLevel = 18.0f;

		const float level = Math::remap(m_level, minLevel, maxLevel, -1.0f, 1.0f);

		// Draw peaks
		const float peakMax = m_maxEnvelope.process(std::fmaxf(0.0f, level));
		const float peakMin = m_minEnvelope.process(-1.0f * std::fminf(0.0f, level));

		// Draw max peak
		constexpr int lineheight = 4;

		const auto heightMax = static_cast<int>(static_cast<float>(meterHeightHalf) * peakMax);
		bounds.setSize(meterWidth, lineheight);
		bounds.setPosition(xPos, yPos + meterHeightHalf - heightMax - lineheight);
		g.setColour(highlightColor);
		g.setOpacity(1.0f);
		g.fillRect(bounds);

		// Draw max peak value
		bounds.setSize(meterWidth, meterWidthHalf);
		bounds.setPosition(xPos, yPos + meterHeightHalf - heightMax - lineheight - meterWidthHalf);
		g.setFont(0.7f * static_cast<float>(meterWidthHalf));
		g.drawText("+" + juce::String(peakMax * maxLevel, 1), bounds, juce::Justification::centred);

		// Draw min peak
		const auto heightMin = static_cast<int>(static_cast<float>(meterHeightHalf) * peakMin);
		bounds.setSize(pixelSize, lineheight);
		bounds.setPosition(xPos, yPos + meterHeightHalf + heightMin);
		g.setColour(juce::Colours::white);
		g.setOpacity(1.0f);
		g.fillRect(bounds);

		// Draw min peak value
		bounds.setSize(meterWidth, meterWidthHalf);
		bounds.setPosition(xPos, yPos + meterHeightHalf + heightMin + lineheight);
		g.drawText(juce::String(peakMin * minLevel, 1), bounds, juce::Justification::centred);

		// Draw bar
		//const float levelSmooth = m_smoother.process(level);
		const float levelSmooth = level;

		if (levelSmooth > 0.0f)
		{
			const int height = static_cast<int>(static_cast<float>(meterHeightHalf) * levelSmooth);
			bounds.setSize(pixelSize, height);
			bounds.setPosition(xPos, yPos + (meterHeightHalf - height));
			g.setColour(highlightColor);
		}
		else
		{
			const auto height = static_cast<int>(static_cast<float>(meterHeightHalf) * (-levelSmooth));
			bounds.setSize(pixelSize, height);
			bounds.setPosition(xPos, yPos + meterHeightHalf);
			g.setColour(juce::Colours::white);
		}

		g.setOpacity(0.5f);
		g.fillRect(bounds);
	}
	inline void setLevel(const float level)
	{
		m_level = level;
	}

private:
	OnePoleLowPassFilter m_smoother;
	HoldEnvelopeFollower<float> m_maxEnvelope;
	HoldEnvelopeFollower<float> m_minEnvelope;

	float m_level = 0.0f;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);
};