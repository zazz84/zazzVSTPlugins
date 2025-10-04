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

#include <vector>

#include <JuceHeader.h>

class WaveformDisplayComponent : public juce::Component
{
public:
	WaveformDisplayComponent() = default;
	~WaveformDisplayComponent() = default;

	void setAudioBuffer(const juce::AudioBuffer<float>& buffer)
	{
		m_audioBuffer = buffer;
		m_rightSampleIndex = buffer.getNumSamples();
		repaint();
	}
	void setRegions(const std::vector<int> regions, const std::vector<int> validRegionsIdx)
	{
		m_regions = regions;
		m_validRegionsIdx = validRegionsIdx;
		
		repaint();
	}
	void setVerticalZoom(const float verticalZoom)
	{
		m_verticalZoom = verticalZoom;
	}
	void setHorizontalZoom(const int leftRegionIndex, const int rightRegionIndex)
	{
		m_leftRegionIndex = leftRegionIndex;
		m_rightRegionIndex = rightRegionIndex;
		
		const int size = m_regions.size();
		if (leftRegionIndex >= 0 && leftRegionIndex < size)
		{
			m_leftSampleIndex = m_regions[m_leftRegionIndex];
		}

		if (rightRegionIndex >= 0 && rightRegionIndex < size)
		{
			m_rightSampleIndex = m_regions[rightRegionIndex];
		}
		
		repaint();
	}
	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);
		g.setColour(juce::Colours::whitesmoke);

		const float width = (float)getWidth();
		const float height = (float)getHeight();
		const float heightHalf = 0.5f * height;

		// Draw zero line
		g.setColour(juce::Colours::whitesmoke);
		g.drawLine(0.0f, heightHalf, width, heightHalf, 1.0f);

		// Draw waveform
		if (m_audioBuffer.getNumSamples() != 0)
		{
			auto* channelData = m_audioBuffer.getReadPointer(0); // take first channel

			juce::Path path;
			path.preallocateSpace(3 * (int)width); // reserve space for efficiency

			// Start path at left edge
			path.startNewSubPath(0, heightHalf);

			// Step through samples, mapping them to pixel positions
			for (int x = 0; x < (int)width; ++x)
			{
				// Find sample corresponding to this pixel (nearest-neighbour downsampling)
				const auto sampleIndex = juce::jmap<int>(x, 0, (int)width, m_leftSampleIndex, m_rightSampleIndex - 1);
				const float level = m_verticalZoom * channelData[sampleIndex];

				// Map sample value (-1..1) to vertical pixel position
				float y = juce::jmap(level, -1.0f, 1.0f, height, 0.0f);

				path.lineTo((float)x, y);
			}

			g.strokePath(path, juce::PathStrokeType(1.0f));
		}

		// Draw regions
		if (m_regions.size() != 0)
		{
			const int regionsCount = m_rightRegionIndex - m_leftRegionIndex;
			const float factor = width / (float)(m_rightSampleIndex - m_leftSampleIndex);

			// Draw all regions
			g.setColour(juce::Colours::red);

			for (int region = m_leftRegionIndex; region <= m_rightRegionIndex; region++)
			{
				const float x = factor * (float)(m_regions[region] - m_leftSampleIndex);

				g.drawLine(x, 0.0f, x, height, 1.0f);
			}

			// Draw valid regions
			for (int id = 0; id < m_validRegionsIdx.size(); id++)
			{
				const int region = m_validRegionsIdx[id];

				if (region >= m_leftRegionIndex && region <= m_rightRegionIndex)
				{
					const float x = factor * (float)(m_regions[region] - m_leftSampleIndex);

					g.setColour(juce::Colours::whitesmoke);
					g.drawLine(x, 0.0f, x, height, 3.0f);

					// Dont draw for the last region
					if (region != m_rightRegionIndex)
					{
						constexpr float recWidth = 70.0f;
						
						// Region index
						juce::Rectangle<int> rectangle(x + 5.0f, heightHalf + 5.0f, recWidth, 20.0f);

						g.setColour(juce::Colours::grey);
						g.fillRect(rectangle);

						g.setColour(juce::Colours::black);
						g.drawText(juce::String((float)region, 0), rectangle, juce::Justification::centred);

						// Sample index
						rectangle.setBounds(x + 5.0f, heightHalf + 30.0f, recWidth, 20.0f);
						g.setColour(juce::Colours::grey);
						g.fillRect(rectangle);

						g.setColour(juce::Colours::black);
						const int sampleIndex = m_regions[region];
						g.drawText(juce::String((float)sampleIndex, 0), rectangle, juce::Justification::centred);

						// Sample value
						if (m_audioBuffer.getNumSamples() >= sampleIndex)
						{
							rectangle.setBounds(x + 5.0f, heightHalf + 55.0f, recWidth, 20.0f);
							g.setColour(juce::Colours::grey);
							g.fillRect(rectangle);

							auto* channelData = m_audioBuffer.getReadPointer(0);

							g.setColour(juce::Colours::black);
							g.drawText(juce::String(juce::Decibels::gainToDecibels(channelData[sampleIndex]), 1), rectangle, juce::Justification::centred);
						}

						// Region length
						rectangle.setBounds(x + 5.0f, heightHalf + 80.0f, recWidth, 20.0f);
						g.setColour(juce::Colours::grey);
						g.fillRect(rectangle);

						g.setColour(juce::Colours::black);
						const int regionLength = m_regions[region + 1] - m_regions[region];
						g.drawText(juce::String((float)regionLength, 0), rectangle, juce::Justification::centred);
					}
				}
			}

			// Draw last line
			g.setColour(juce::Colours::whitesmoke);
			g.drawLine(width, 0.0f, width, height, 3.0f);
		}
	}

private:
	juce::AudioBuffer<float> m_audioBuffer;

	std::vector<int> m_regions;
	std::vector<int> m_validRegionsIdx;

	float m_verticalZoom = 1.0f;

	int m_leftSampleIndex = 0;
	int m_rightSampleIndex = 0;

	int m_leftRegionIndex = 0;
	int m_rightRegionIndex = 0;
};