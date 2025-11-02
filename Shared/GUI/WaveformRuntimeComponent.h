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

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

class WaveformRuntimeComponent : public juce::Component
{
public:
	WaveformRuntimeComponent() = default;
	~WaveformRuntimeComponent() = default;

	void init(const unsigned int size)
	{
		m_buffer.init(size);
	}
	void set(const int displaySize)
	{
		m_displaySize = displaySize;
	}
	void process(const float sample)
	{
		m_buffer.write(sample);
		m_counterSample++;
	}
	bool canRepaint() { return m_repaint; };
	void paint(juce::Graphics& g) override
	{		
		const auto width = getWidth();
		const auto height = getHeight();
		const auto heightHalf = 0.5f * height;

		// Draw background
		g.fillAll(juce::Colours::black);

		// Draw zero line
		g.setColour(juce::Colours::whitesmoke);
		g.drawLine(0.0f, (float)heightHalf, (float)width, (float)heightHalf, 1.0f);

		// Draw waveform
		juce::Path path;
		path.preallocateSpace(3 * (int)width); // reserve space for efficiency

		// Start path at left edge
		path.startNewSubPath(0.0f, (float)heightHalf);

		// Step through samples, mapping them to pixel positions
		for (int x = 0; x < width; x++)
		{
			// Find sample corresponding to this pixel (nearest-neighbour downsampling)
			const auto delaySamples = juce::jmap<int>(x, 0, width, 0, m_displaySize);

			const auto sampleValue = m_buffer.readDelay(delaySamples);

			// Map sample value (-1..1) to vertical pixel position
			float y = juce::jmap(sampleValue, -1.0f, 1.0f, (float)height, 0.0f);

			path.lineTo((float)x, y);
		}

		g.strokePath(path, juce::PathStrokeType(1.0f));

		m_repaint = false;
	}

private:
	CircularBuffer m_buffer;
	unsigned int m_displaySize = 0;
	unsigned int m_counterSample = 0;
	bool m_repaint = false;
};