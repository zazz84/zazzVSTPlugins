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
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

class StereoWidthMeterComponent : public juce::Component
{
public:
	StereoWidthMeterComponent()
	{
		m_smoother.init(60.0f);
		m_smoother.set(1.0f, 100.0f);
	};
	~StereoWidthMeterComponent()
	{
		m_smoother.release();
	};

	static const int SMALL_LOD_HEIGHT_LIMIT = 53;

	inline void paint(juce::Graphics& g) override
	{
		//SIZE: n x 2

		const auto width = getWidth();
		const auto height = getHeight();

		const auto pixelSize = height / 2;
		const auto pixelHalf = pixelSize / 2;
		const auto pixelSize2 = pixelSize + pixelSize;

		const auto meterWidth = width - pixelSize;
		const auto meterHeight = pixelHalf;

		// Draw meter background
		juce::Rectangle<int> bounds;
		bounds.setSize(meterWidth, meterHeight);

		const auto xPos = pixelHalf;
		const auto yPos = pixelSize;

		bounds.setPosition(xPos, yPos);

		g.setColour(juce::Colours::black);
		g.setOpacity(0.5f);
		g.fillRect(bounds);

		// Draw values
		g.setFont(pixelHalf);
		g.setColour(highlightColor);
		g.setOpacity(0.5f);

		const auto yPosText = pixelSize / 3;

		bounds.setSize(pixelSize2, pixelSize);
		bounds.setPosition(xPos, yPosText);
		g.drawText("100%", bounds, juce::Justification::topLeft);

		bounds.setPosition((width - pixelSize2) / 2, yPosText);
		g.drawText("0%", bounds, juce::Justification::centredTop);

		bounds.setPosition(width - xPos - pixelSize2, yPosText);
		g.drawText("100%", bounds, juce::Justification::topRight);

		// Draw meter value
		const float stereoWidthSmooth = m_smoother.process(m_stereoWidth);

		const auto markerWidth = std::max(pixelSize / 4, static_cast<int>(Math::remap(stereoWidthSmooth, 0.0f, 1.0f, 0.0f, static_cast<float>(meterWidth))));

		bounds.setSize(markerWidth, meterHeight);
		bounds.setPosition(xPos + (meterWidth - markerWidth) / 2, yPos);

		g.setColour(highlightColor);
		g.setOpacity(1.0f);
		g.fillRect(bounds);
	}
	inline void set(const float stereoWidth)
	{
		m_stereoWidth = stereoWidth;
	}

private:
	BranchingEnvelopeFollower<float> m_smoother;
	float m_stereoWidth = 0.0f;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);
};