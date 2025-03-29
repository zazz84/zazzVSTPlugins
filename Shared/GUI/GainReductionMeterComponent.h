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

class GainReductionMeterComponent : public juce::Component
{
public:
	GainReductionMeterComponent()
	{
		constexpr int sampleRate = 20;

		m_smoother.init(sampleRate);
		m_smoother.set(0.0f, 150.0f, 150.0f);
	}
	~GainReductionMeterComponent()
	{
	}

	static const int SMALL_LOD_WIDTH_LIMIT = 28;

	inline void paint(juce::Graphics& g) override
	{
		//SIZE: 2 x n

		const auto width = getWidth();
		const auto height = getHeight();

		const auto pixelSize = width / 2;
		const auto pixelSizeHalf = width / 4;

		const auto meterWidth = pixelSize;
		const auto meterHeight = height - pixelSize;

		juce::Rectangle<int> bounds;

		const auto xPos = pixelSize / 2;
		auto yPos = 0;

		// Draw meter name
		if (pixelSize > SMALL_LOD_WIDTH_LIMIT)
		{
			bounds.setSize(pixelSize, pixelSizeHalf);
			bounds.setPosition(xPos, yPos);
			g.setColour(juce::Colours::white);
			g.drawText("GR", bounds, juce::Justification::centred);
		}

		// Draw meter background	
		bounds.setSize(meterWidth, meterHeight);

		yPos += pixelSizeHalf;
		bounds.setPosition(xPos, yPos);

		g.setColour(juce::Colours::black);
		g.setOpacity(0.5f);
		g.fillRect(bounds);

		constexpr float gainReductionMin = 0.0f;
		constexpr float gainReductionMax = 36.0f;

		const float levelNormalized = Math::remap(m_level, gainReductionMin, gainReductionMax, 0.0f, 1.0f);

		// Smooth normalized level
		const float levelNormalizedSmooth = m_smoother.process(levelNormalized);

		// Draw marker line
		const auto heightMin = static_cast<int>(static_cast<float>(meterHeight * levelNormalizedSmooth));
		const auto lineHeight = 2;
		bounds.setSize(pixelSize, lineHeight);
		bounds.setPosition(xPos, yPos + heightMin);
		g.setColour(juce::Colours::white);
		g.setOpacity(1.0f);
		g.fillRect(bounds);

		// Draw maximum gain reduction value
		if (pixelSize > SMALL_LOD_WIDTH_LIMIT)
		{
			bounds.setSize(meterWidth, pixelSizeHalf);
			bounds.setPosition(xPos, yPos + meterHeight);
			g.drawText(juce::String(levelNormalizedSmooth * gainReductionMax, 1), bounds, juce::Justification::centred);
		}

		// Draw background gain reduction bar
		bounds.setSize(pixelSize, heightMin);
		bounds.setPosition(xPos, yPos);
		g.setColour(juce::Colours::white);

		g.setOpacity(0.5f);
		g.fillRect(bounds);
	}
	inline void setLevel(const float level)
	{
		m_level = level;
	}

private:
	HoldEnvelopeFollower<float> m_smoother;

	float m_level = 0.0f;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);
};