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
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

class CorrelationMeterComponent : public juce::Component
{
public:
	CorrelationMeterComponent()
	{
		m_smoother.init(30.0f);
		m_smoother.set(2.0f);
	};
	~CorrelationMeterComponent()
	{
		m_smoother.release();
	};

	static const int SMALL_LOD_HEIGHT_LIMIT = 53;

	inline void paint(juce::Graphics& g) override
	{
		//SIZE: n x 2

		//g.fillAll(juce::Colour::fromRGB(90, 90, 100));

		const auto width = getWidth();
		const auto height = getHeight();

		const auto pixelSize = height / 2;

		const auto meterWidth = width - pixelSize;
		const auto meterHeight = pixelSize / 2;

		// Draw meter background
		juce::Rectangle<int> bounds;
		bounds.setSize(meterWidth, meterHeight);

		const auto xPos = pixelSize / 2;
		const auto yPos = pixelSize;

		bounds.setPosition(xPos, yPos);

		g.setColour(juce::Colours::black);
		g.setOpacity(0.5f);
		g.fillRect(bounds);

		// Draw values
		g.setFont(static_cast<float>(pixelSize / 2));
		g.setColour(juce::Colours::white);
		g.setOpacity(0.5f);
		
		const auto yPosText = pixelSize / 3;

		bounds.setSize(pixelSize, pixelSize);
		bounds.setPosition(xPos, yPosText);
		g.drawText("-1", bounds, juce::Justification::topLeft);

		bounds.setPosition((width - pixelSize) / 2 , yPosText);
		g.drawText("0", bounds, juce::Justification::centredTop);

		bounds.setPosition(width - xPos - pixelSize, yPosText);
		g.drawText("1", bounds, juce::Justification::topRight);
		
		// Draw meter value
		const float correlationSmooth = m_smoother.process(m_correlation);

		const auto peakWidth = static_cast<int>(Math::remap(Math::fabsf(correlationSmooth), 0.0f, 1.0f, 0.0f, static_cast<float>(meterWidth / 2)));
		
		bounds.setSize(peakWidth, meterHeight);

		if (m_correlation > 0.0f)
		{		
			bounds.setPosition(width / 2, yPos);
		}
		else
		{
			bounds.setPosition(width / 2 - peakWidth, yPos);
		}

		g.setColour(juce::Colours::white);
		g.setOpacity(1.0f);
		g.fillRect(bounds);
	}
	inline void set(const float correlation)
	{
		m_correlation = correlation;
	}

private:
	OnePoleLowPassFilter m_smoother;
	float m_correlation = 0.0f;
};