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

#include "../../../zazzVSTPlugins/Shared/Dynamics/PeakDetector.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class PeakLevelMeterComponent : public juce::Component
{
public:
	PeakLevelMeterComponent()
	{
		m_peakDetector.init(30);
		m_peakDetector.set(300.0f, 800.0f);

		m_peakDetectorFast.init(30);
		m_peakDetectorFast.set(0.0f, 100.0f);
	}
	~PeakLevelMeterComponent()
	{
		m_peakDetector.release();
		m_peakDetectorFast.release();
	}

	static const int SMALL_LOD_WIDTH_LIMIT = 53;

	inline void paint(juce::Graphics& g) override
	{
		//SIZE: 2 x n

		g.fillAll(juce::Colour::fromRGB(90, 90, 100));

		const auto width = getWidth();
		const auto height = getHeight();

		const auto pixelSize = width / 2;
		const auto meterHeight = width > SMALL_LOD_WIDTH_LIMIT ? height - 2 * pixelSize : height - pixelSize;
		
		juce::Rectangle<int> bounds;
		bounds.setSize(pixelSize, meterHeight);

		const auto xPos = pixelSize / 2;
		const auto yPos = width > SMALL_LOD_WIDTH_LIMIT ? pixelSize + pixelSize / 2 : pixelSize / 2;

		bounds.setPosition(xPos, yPos);

		g.setColour(juce::Colours::black);
		g.setOpacity(0.5f);
		g.fillRect(bounds);

		g.setColour(juce::Colours::white);
		g.setOpacity(1.0f);

		const float levelSmooth = m_peakDetector.process(m_level);
		const float levelSmoothdB = Math::gainTodB(levelSmooth);

		if (levelSmooth > 1.0f)
		{
			g.setColour(juce::Colours::red);
		}
		else
		{
			g.setColour(juce::Colours::white);
		}

		const auto peakSmoothHeight = static_cast<int>(Math::remap(levelSmoothdB, -60.0f, 6.0f, 0.0f, static_cast<float>(meterHeight)));
		bounds.setSize(pixelSize, pixelSize / 10);
		bounds.setPosition(xPos, yPos + meterHeight - peakSmoothHeight);
		g.fillRect(bounds);

		const auto peakFastHeight = static_cast<int>(Math::remap(Math::gainTodB(m_peakDetectorFast.process(m_level)), -60.0f, 6.0f, 0.0f, static_cast<float>(meterHeight)));
		bounds.setSize(pixelSize, peakFastHeight);
		bounds.setPosition(xPos, yPos + meterHeight - peakFastHeight);
		g.setColour(juce::Colours::white);
		g.setOpacity(0.5f);
		g.fillRect(bounds);

		if (width > SMALL_LOD_WIDTH_LIMIT)
		{
			// Number
			if (levelSmooth > 1.0f)
			{
				g.setColour(juce::Colours::red);
			}
			else
			{
				g.setColour(juce::Colours::white);
			}

			bounds.setSize(pixelSize + pixelSize, pixelSize);
			bounds.setPosition(0, pixelSize / 2);

			juce::String text = levelSmoothdB > -60.0f ? juce::String(levelSmoothdB, 1) + " dB" : "-inf";

			g.setOpacity(1.0f);
			g.setFont(0.45f * static_cast<float>(pixelSize));
			g.setOpacity(1.0f);
			g.drawText(text, bounds, juce::Justification::centred, false);
		}
	}
	inline void setLevel(const float level)
	{
		m_level = level;
	}

private:
	PeakDetector m_peakDetector;
	PeakDetector m_peakDetectorFast;
	float m_level = 0.0f;
};
