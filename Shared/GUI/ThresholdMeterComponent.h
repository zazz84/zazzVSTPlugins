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

class ThresholdMeterComponent : public juce::Component
{
public:
	ThresholdMeterComponent()
	{
		m_smootherAmplitude.init(30.0f);
		m_smootherAmplitude.set(0.0f, 200.0f);

		m_smootherOpacity.init(30.0f);
		m_smootherOpacity.set(0.0f, 200.0f);
	}
	~ThresholdMeterComponent() = default;

	inline void paint(juce::Graphics& g) override
	{
		//SIZE: n x 2

		const auto width = getWidth();
		const auto height = getHeight();

		const auto pixelSize = height / 2;
		const auto pixelSizeHalf = height / 4;
		const auto pixelSizeQuarter = height / 8;
		const auto pixelAndHalfSize = pixelSize + pixelSizeHalf;
		const auto pixelSize2 = pixelSize + pixelSize;

		const auto meterWidth = width;
		const auto meterHeight = pixelSize;

		// Draw meter background
		juce::Rectangle<int> bounds;

		bounds.setSize(meterWidth, meterHeight);

		const auto posX = 0;
		const auto posY = pixelSizeHalf;

		bounds.setPosition(posX, posY);

		g.setColour(lightColor);
		g.setOpacity(1.0f);
		g.fillRect(bounds);

		// Draw meter outline
		g.setColour(highlightColor);
		g.setOpacity(0.5f);
		g.drawRect(bounds, 1.0f);

		// Scale
		constexpr float mindB = -60.0f;
		constexpr float maxdB = 6.0f;
		
		// Draw amplitude
		const auto amplitudeWidth = Math::remap(m_amplitude, mindB, maxdB, static_cast<float>(posX), static_cast<float>(posX + meterWidth));
		const auto amplitudeWidthSmooth = m_smootherAmplitude.process(amplitudeWidth);
		bounds.setSize(static_cast<int>(amplitudeWidthSmooth), pixelSize);
		bounds.setPosition(0, pixelSizeHalf);	
		g.setOpacity(m_amplitudeOpacity);
		g.fillRect(bounds);

		// Draw marker
		auto drawMarker = [&](float marker, juce::Colour colour)
		{
			const auto markerX = static_cast<int>(Math::remap(marker, mindB, maxdB, static_cast<float>(posX), static_cast<float>(posX + meterWidth)));
			g.setColour(colour);
			g.setOpacity(0.5f);
			g.drawVerticalLine(markerX, static_cast<float>(pixelSizeHalf), static_cast<float>(pixelAndHalfSize));

			juce::Rectangle<float> circle;
			circle.setSize(static_cast<float>(pixelSizeHalf), static_cast<float>(pixelSizeHalf));
			circle.setPosition(static_cast<float>(markerX - pixelSizeQuarter), 0.0f);
			g.setOpacity(1.0f);
			g.fillEllipse(circle);
		};

		drawMarker(m_threshold, juce::Colours::white);

		if (pixelSize > 25)
		{
			// Draw scale values		
			// -60 dB
			bounds.setSize(pixelSize2, pixelSizeHalf);
			bounds.setPosition(posX, pixelAndHalfSize);
			g.drawText("-60 dB", bounds, juce::Justification::left);

			// +6 dB
			bounds.setSize(pixelSize2, pixelSizeHalf);
			bounds.setPosition(width - pixelSize2, pixelAndHalfSize);
			g.drawText("+6 dB", bounds, juce::Justification::right);
		}
	}
	inline void set(const float amplitude, const float threshold, const bool isOpen)
	{
		m_amplitude = amplitude;
		m_threshold = threshold;

		m_amplitudeOpacity = 0.5f + m_smootherOpacity.process(static_cast<float>(isOpen));
	}

private:
	BranchingEnvelopeFollower<float> m_smootherAmplitude;
	BranchingEnvelopeFollower<float> m_smootherOpacity;

	float m_amplitude = -100.0f;
	float m_amplitudeOpacity = 0.5f;
	float m_threshold = 0.0f;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);
};