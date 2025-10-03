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

class WaveformDisplayComponent : public juce::Component, private juce::ChangeListener
{
public:
	WaveformDisplayComponent() : thumbnailCache(5), thumbnail(512, m_formatManager, thumbnailCache)
	{
		m_formatManager.registerBasicFormats();
		thumbnail.addChangeListener(this);
	}

	~WaveformDisplayComponent()
	{
		release();
	}

	void loadFile(const juce::File& file)
	{
		thumbnail.clear();
		auto* reader = m_formatManager.createReaderFor(file);
		if (reader != nullptr)
		{
			thumbnail.setSource(new juce::FileInputSource(file));
			repaint();
		}
	}

	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);

		g.setColour(juce::Colours::white);
		if (thumbnail.getTotalLength() > 0.0)
		{
			const float aproximatedPeak = thumbnail.getApproximatePeak();
			thumbnail.drawChannels(g, getLocalBounds(), 0.0, thumbnail.getTotalLength(), 1.0f / aproximatedPeak);
		}
		else
		{
			g.setFont(20.0f);
			g.drawText("No File Loaded", getLocalBounds(), juce::Justification::centred);
		}
	}

	void release()
	{
		thumbnailCache.clear();
		thumbnail.clear();
	}

private:
	void changeListenerCallback(juce::ChangeBroadcaster*) override
	{
		repaint();
	}

	juce::AudioFormatManager m_formatManager;
	juce::AudioThumbnailCache thumbnailCache;
	juce::AudioThumbnail thumbnail;

	float m_verticalZoomFactor = 1.0f;
};

//==============================================================================
class WaveformComponent : public juce::Component
{
public:
	WaveformComponent() {}

	void setAudioBuffer(const juce::AudioBuffer<float>& buffer)
	{
		audioBuffer = buffer;
		m_rightIndex = buffer.getNumSamples();
		repaint();
	}

	void setVerticalZoom(const float verticalZoom)
	{
		m_verticalZoom = verticalZoom;
	}

	void setHorizontalZoom(const int leftIndex, const int rightIndex)
	{
		m_leftIndex = leftIndex;
		m_rightIndex = rightIndex;
		repaint();
	}

	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);
		g.setColour(juce::Colours::whitesmoke);

		if (audioBuffer.getNumSamples() == 0)
			return;

		auto width = getWidth();
		auto height = getHeight();

		// Draw zero line
		g.drawLine(0.0f, height / 2, width, height / 2, 1.0f);

		//int numSamples = audioBuffer.getNumSamples();
		auto* channelData = audioBuffer.getReadPointer(0); // take first channel

		juce::Path path;
		path.preallocateSpace(3 * width); // reserve space for efficiency

		// Start path at left edge
		path.startNewSubPath(0, height * 0.5f);

		// Step through samples, mapping them to pixel positions
		for (int x = 0; x < width; ++x)
		{
			// Find sample corresponding to this pixel (nearest-neighbour downsampling)
			//auto sampleIndex = juce::jmap<int>(x, 0, width, 0, numSamples - 1);
			auto sampleIndex = juce::jmap<int>(x, 0, width, m_leftIndex, m_rightIndex - 1);
			float level = channelData[sampleIndex];

			// Map sample value (-1..1) to vertical pixel position
			float y = juce::jmap(m_verticalZoom * level, -1.0f, 1.0f, (float)height, 0.0f);

			path.lineTo((float)x, y);
		}

		g.strokePath(path, juce::PathStrokeType(1.0f));
	}

private:
	juce::AudioBuffer<float> audioBuffer;
	float m_verticalZoom = 1.0f;

	int m_leftIndex = 0;
	int m_rightIndex = 0;
};

//==============================================================================
class RegionsComponent : public juce::Component
{
public:
	RegionsComponent() {}

	void set(const std::vector<int> regions, const std::vector<int> validRegionsIdx)
	{
		m_regions = regions;
		m_validRegionsIdx = validRegionsIdx;
		repaint();
	}

	void setHorizontalZoom(const int leftIndex, const int rightIndex)
	{
		m_leftIndex = leftIndex;
		m_rightIndex = rightIndex;
		repaint();
	}

	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);
		g.setColour(juce::Colours::whitesmoke);

		if (m_regions.size() == 0)
		{
			return;
		}

		jassert(m_leftIndex >= 0 && m_leftIndex < m_regions.size());
		jassert(m_rightIndex >= 0 && m_rightIndex < m_regions.size());

		const int samplesOffst = m_regions[m_leftIndex];
		const int samples = m_regions[m_rightIndex] - samplesOffst;
		const int width = getWidth();
		const int regionsCount = m_rightIndex - m_leftIndex;
		const float factor = (float)width / (float)samples;
		const int bottom = getHeight();

		// Draw all regions
		g.setColour(juce::Colours::red);

		for (int region = m_leftIndex; region <= m_rightIndex; region++)
		{
			const int x = factor * (m_regions[region] - samplesOffst);

			g.drawLine((float)x, 0.0f, (float)x, (float)bottom, 1.0f);
		}

		// Draw valid regions
		g.setColour(juce::Colours::whitesmoke);

		for (int id = 0; id < m_validRegionsIdx.size(); id++)
		{
			const int region = m_validRegionsIdx[id];
			
			if (region >= m_leftIndex && region <= m_rightIndex)
			{
				const int x = factor * (m_regions[region] - samplesOffst);

				g.drawLine((float)x, 0.0f, (float)x, (float)bottom, 3.0f);
			}
		}

		// Draw last line
		g.drawLine((float)width, 0.0f, (float)width, (float)bottom, 3.0f);
	}

private:
	std::vector<int> m_regions;
	std::vector<int> m_validRegionsIdx;
	
	int m_leftIndex = 0;
	int m_rightIndex = 0;
};