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

class WaveformDisplayComponent : public juce::Component, private juce::ChangeListener
{
public:
	WaveformDisplayComponent() : thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
	{
		formatManager.registerBasicFormats();
		thumbnail.addChangeListener(this);
	}

	~WaveformDisplayComponent()
	{
		release();
	}

	void loadFile(const juce::File& file)
	{
		thumbnail.clear();
		auto* reader = formatManager.createReaderFor(file);
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

	juce::AudioFormatManager formatManager;
	juce::AudioThumbnailCache thumbnailCache;
	juce::AudioThumbnail thumbnail;

	float m_verticalZoomFactor = 1.0f;
};