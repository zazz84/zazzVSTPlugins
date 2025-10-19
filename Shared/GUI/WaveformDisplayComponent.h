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

#include "../../../zazzVSTPlugins/Shared/GUI/GroupLabelComponent.h"

struct Region
{
	int m_sampleIndex = -1;
	bool m_isValid = false;
	int m_length = 0;
};

class WaveformDisplayComponent : public juce::Component
{
public:
	WaveformDisplayComponent(juce::String name) : m_nameGroupComponent(name)
	{
		// Group labeles
		addAndMakeVisible(m_nameGroupComponent);
		addAndMakeVisible(m_zoomGroupComponent);
		
		// Zoom lables
		addAndMakeVisible(m_zoomRegionLeftLabel);
		m_zoomRegionLeftLabel.setEditable(true, true, false);
		m_zoomRegionLeftLabel.setJustificationType(juce::Justification::centred);
		m_zoomRegionLeftLabel.setText("-1", juce::dontSendNotification);

		m_zoomRegionLeftLabel.onTextChange = [this]
		{
			const size_t size = m_regions.size();
			
			// Reset to invalid when not regions are detected
			if (size == 0)
			{
				m_zoomRegionLeftLabel.setText("-1", juce::dontSendNotification);
				return;
			}

			const int leftRegion = m_zoomRegionLeftLabel.getText().getIntValue();
			const int regionsMax = (int)m_regions.size() - 1;

			// Reset if wrong input
			if (leftRegion < 0 || leftRegion > regionsMax)
			{
				m_zoomRegionLeftLabel.setText("0", juce::dontSendNotification);
			}

			// Limit if large than right region
			const int rightRegion = m_zoomRegionRightLabel.getText().getIntValue();

			if (leftRegion > rightRegion)
			{
				m_zoomRegionLeftLabel.setText(juce::String((float)rightRegion, 0), juce::dontSendNotification);
			}

			setHorizontalZoom();
		};

		addAndMakeVisible(m_zoomRegionRightLabel);
		m_zoomRegionRightLabel.setEditable(true, true, false);  // user can click & type
		m_zoomRegionRightLabel.setJustificationType(juce::Justification::centred);
		m_zoomRegionRightLabel.setText("-1", juce::dontSendNotification);

		m_zoomRegionRightLabel.onTextChange = [this]
		{
			const size_t size = m_regions.size();
			
			// Reset to invalid when not regions are detected
			if (size == 0)
			{
				m_zoomRegionRightLabel.setText("-1", juce::dontSendNotification);
				return;
			}

			const int rightRegion = m_zoomRegionRightLabel.getText().getIntValue();
			const int regionsMax = (int)size - 1;

			// Reset if wrong input
			if (rightRegion < 0 || rightRegion > regionsMax)
			{
				m_zoomRegionRightLabel.setText(juce::String((float)(regionsMax)), juce::dontSendNotification);
			}

			// Limit if large than right region
			const int leftRegion = m_zoomRegionLeftLabel.getText().getIntValue();

			if (leftRegion > rightRegion)
			{
				m_zoomRegionRightLabel.setText(juce::String((float)(std::min(regionsMax, leftRegion + 1)), 0), juce::dontSendNotification);
			}

			setHorizontalZoom();
		};

		// Scroll buttons
		addAndMakeVisible(&m_scrollLeftButton);
		m_scrollLeftButton.setButtonText("<");
		m_scrollLeftButton.onClick = [this]
		{
			int zoomLeft = m_zoomRegionLeftLabel.getText().getIntValue();
			int zoomRight = m_zoomRegionRightLabel.getText().getIntValue();
			const bool regionsIsEmpty = m_zoomRegionRightLabel.getText().isEmpty();
			const int regionsMax = (int)m_regions.size() - 1;

			if (zoomLeft > zoomRight || zoomLeft == -1 || zoomRight == -1 || regionsMax == 0 || regionsIsEmpty)
			{
				return;
			}

			// Set zoom
			const int size = zoomRight - zoomLeft + 1;

			zoomLeft = std::max(0, zoomLeft - size);
			zoomRight = std::max(0, zoomRight - size);

			m_zoomRegionLeftLabel.setText(juce::String((float)zoomLeft, 0), juce::dontSendNotification);
			m_zoomRegionRightLabel.setText(juce::String((float)zoomRight, 0), juce::dontSendNotification);

			setHorizontalZoom();
		};

		addAndMakeVisible(&m_scrollRightButton);
		m_scrollRightButton.setButtonText(">");
		m_scrollRightButton.onClick = [this]
		{
			int zoomLeft = m_zoomRegionLeftLabel.getText().getIntValue();
			int zoomRight = m_zoomRegionRightLabel.getText().getIntValue();
			const bool regionsIsEmpty = m_zoomRegionRightLabel.getText().isEmpty();
			const int regionsMax = (int)m_regions.size() - 1;

			if (zoomLeft > zoomRight || zoomLeft == -1 || zoomRight == -1 || regionsMax == 0 || regionsIsEmpty)
			{
				return;
			}

			// Set zoom
			const int size = zoomRight - zoomLeft + 1;

			zoomRight = std::min(regionsMax, zoomRight + size);
			zoomLeft = std::min(regionsMax, zoomLeft + size);

			m_zoomRegionLeftLabel.setText(juce::String((float)zoomLeft, 0), juce::dontSendNotification);
			m_zoomRegionRightLabel.setText(juce::String((float)zoomRight, 0), juce::dontSendNotification);

			setHorizontalZoom();
		};

		addAndMakeVisible(&m_resetZoomButton);
		m_resetZoomButton.setButtonText("R");
		m_resetZoomButton.onClick = [this]
		{			
			m_zoomRegionLeftLabel.setText("0", juce::NotificationType::dontSendNotification);
			m_zoomRegionRightLabel.setText(juce::String((float)(m_regions.size() - 1), 0), juce::NotificationType::dontSendNotification);

			setHorizontalZoom();
		};

		addAndMakeVisible(&m_showDetailsButton);
		m_showDetailsButton.setButtonText("D");
		m_showDetailsButton.setClickingTogglesState(true);
		m_showDetailsButton.setToggleState(true, juce::NotificationType::dontSendNotification);
		m_showDetailsButton.onClick = [this]
		{
			repaint();
		};

	}
	~WaveformDisplayComponent() = default;

	void setAudioBuffer(const juce::AudioBuffer<float> buffer)
	{
		m_audioBuffer = buffer;
		m_leftSampleIndex = 0;
		m_rightSampleIndex = buffer.getNumSamples();

		if (m_rightSampleIndex != 0)
		{
			const float verticalZoom = 1.0f / buffer.getMagnitude(0, buffer.getNumSamples());
			m_verticalZoom = verticalZoom;
		}
		else
		{
			m_verticalZoom = 1.0f;
		}

		// Clear
		m_leftRegionIndex = -1;
		m_rightRegionIndex = -1;

		m_zoomRegionLeftLabel.setText("-1", juce::dontSendNotification);
		m_zoomRegionRightLabel.setText("-1", juce::dontSendNotification);

		m_regions.clear();

		repaint();
	}
	void setRegions(const std::vector<Region> regions)
	{
		const size_t size = regions.size();
		if (size == 0)
		{
			return;
		}
		
		m_regions = regions;

		m_leftRegionIndex = 0;
		m_rightRegionIndex = (int)(size - 1);

		m_zoomRegionLeftLabel.setText("0", juce::dontSendNotification);	
		m_zoomRegionRightLabel.setText(juce::String((float)m_rightRegionIndex, 0), juce::dontSendNotification);
		
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
		
		const int size = (int)m_regions.size();
		if (leftRegionIndex >= 0 && leftRegionIndex < size)
		{
			m_leftSampleIndex = m_regions[m_leftRegionIndex].m_sampleIndex;
		}

		if (rightRegionIndex >= 0 && rightRegionIndex < size)
		{
			m_rightSampleIndex = m_regions[rightRegionIndex].m_sampleIndex + m_regions[rightRegionIndex].m_length;
		}
		
		repaint();
	}
	void resized() override
	{
		const auto width = getWidth();
		const auto height = getHeight();
		const auto pixelSize = height / 11;
		const auto pixelSize2 = pixelSize + pixelSize;
		const auto pixelSize4 = pixelSize2 + pixelSize2;
		const auto pixelSize8 = pixelSize4 + pixelSize4;

		// Set size
		m_nameGroupComponent.setSize(width, pixelSize);
		m_zoomGroupComponent.setSize(pixelSize4, pixelSize);

		m_zoomRegionLeftLabel.setSize(pixelSize, pixelSize);
		m_zoomRegionRightLabel.setSize(pixelSize, pixelSize);
		m_scrollLeftButton.setSize(pixelSize, pixelSize);
		m_scrollRightButton.setSize(pixelSize, pixelSize);
		m_resetZoomButton.setSize(pixelSize, pixelSize);
		m_showDetailsButton.setSize(pixelSize, pixelSize);

		// Set position
		const auto column1 = 0;	
		const auto column4 = width / 2;
		const auto column2 = column4 - pixelSize2;
		const auto column3 = column4 - pixelSize;
		const auto column5 = column4 + pixelSize;
		const auto column6 = column5 + pixelSize2;
		const auto column7 = column6 + pixelSize8;

		const auto row1 = 0;
		const auto row2 = row1 + pixelSize;
		const auto row3 = row2 + pixelSize8;
		const auto row4 = row3 + pixelSize;

		m_nameGroupComponent.setTopLeftPosition(column1, row1);
		m_zoomGroupComponent.setTopLeftPosition(column2, row3);

		m_scrollLeftButton.setTopLeftPosition		(column2, row4);
		m_zoomRegionLeftLabel.setTopLeftPosition	(column3, row4);
		m_zoomRegionRightLabel.setTopLeftPosition	(column4, row4);
		m_scrollRightButton.setTopLeftPosition		(column5, row4);
		m_resetZoomButton.setTopLeftPosition		(column6, row4);
		m_showDetailsButton.setTopLeftPosition		(column7, row4);
	}
	void paint(juce::Graphics& g) override
	{
		// Size N / 11	
		const auto width = getWidth();
		const auto height = getHeight();
		const auto pixelSize = height / 11;
		const auto pixelSize5 = 5 * pixelSize;
		const auto pixelSize8 = 8 * pixelSize;
		const auto pixelSize9 = pixelSize8 + pixelSize;

		// Misc
		const auto showDetails = m_showDetailsButton.getToggleState();

		// Draw background
		juce::Rectangle<float> backgroundRectabgle;

		backgroundRectabgle.setSize((float)width, (float)pixelSize8);
		backgroundRectabgle.setPosition(0.0f, (float)pixelSize);

		g.setColour(juce::Colours::black);
		g.fillRect(backgroundRectabgle);
		
		// Draw zero line
		g.setColour(juce::Colours::whitesmoke);
		g.drawLine(0.0f, (float)pixelSize5, (float)width, (float)pixelSize5, 1.0f);

		// Draw waveform
		if (m_audioBuffer.getNumSamples() != 0)
		{
			auto* channelData = m_audioBuffer.getReadPointer(0); // take first channel

			juce::Path path;
			path.preallocateSpace(3 * (int)width); // reserve space for efficiency

			// Start path at left edge
			path.startNewSubPath(0.0f, (float)pixelSize5);

			// Step through samples, mapping them to pixel positions
			for (int x = 0; x < (int)width; ++x)
			{
				// Find sample corresponding to this pixel (nearest-neighbour downsampling)
				const auto sampleIndex = juce::jmap<int>(x, 0, (int)width, m_leftSampleIndex, m_rightSampleIndex - 1);
				const float level = m_verticalZoom * channelData[sampleIndex];

				// Map sample value (-1..1) to vertical pixel position
				float y = juce::jmap(level, -1.0f, 1.0f, (float)pixelSize9, (float)pixelSize);

				path.lineTo((float)x, y);
			}

			g.strokePath(path, juce::PathStrokeType(1.0f));
		}

		// Draw regions
		if (m_regions.size() != 0)
		{
			const int regionsCount = m_rightRegionIndex - m_leftRegionIndex;
			const float factor = (float)width / (float)(m_rightSampleIndex - m_leftSampleIndex);

			// Draw all regions
			for (int region = m_leftRegionIndex; region <= m_rightRegionIndex; region++)
			{
				const float x = factor * (float)(m_regions[region].m_sampleIndex - m_leftSampleIndex);

				if (m_regions[region].m_isValid)
				{
					g.setColour(juce::Colours::whitesmoke);
					g.drawLine(x, (float)pixelSize, x, (float)pixelSize9, 3.0f);

					// Draw details
					if (showDetails == true)
					{
						constexpr float recWidth = 70.0f;

						// Region index
						juce::Rectangle<float> rectangle(x + 5.0f, (float)pixelSize5 + 5.0f, recWidth, 20.0f);

						g.setColour(juce::Colours::grey);
						g.fillRect(rectangle);

						g.setColour(juce::Colours::black);
						g.drawText(juce::String((float)region, 0), rectangle, juce::Justification::centred);

						// Sample index
						rectangle.setBounds(x + 5.0f, (float)pixelSize5 + 30.0f, recWidth, 20.0f);
						g.setColour(juce::Colours::grey);
						g.fillRect(rectangle);

						g.setColour(juce::Colours::black);
						const int sampleIndex = m_regions[region].m_sampleIndex;
						g.drawText(juce::String((float)sampleIndex, 0), rectangle, juce::Justification::centred);

						// Sample value
						if (m_audioBuffer.getNumSamples() >= sampleIndex)
						{
							rectangle.setBounds(x + 5.0f, (float)pixelSize5 + 55.0f, recWidth, 20.0f);
							g.setColour(juce::Colours::grey);
							g.fillRect(rectangle);

							auto* channelData = m_audioBuffer.getReadPointer(0);

							g.setColour(juce::Colours::black);
							g.drawText(juce::String(juce::Decibels::gainToDecibels(channelData[sampleIndex]), 1), rectangle, juce::Justification::centred);
						}

						// Region length
						rectangle.setBounds(x + 5.0f, (float)pixelSize5 + 80.0f, recWidth, 20.0f);
						g.setColour(juce::Colours::grey);
						g.fillRect(rectangle);

						g.setColour(juce::Colours::black);
						g.drawText(juce::String((float)m_regions[region].m_length, 0), rectangle, juce::Justification::centred);
					}
				}
				else
				{
					g.setColour(juce::Colours::red);
				}

				g.drawLine(x, (float)pixelSize, x, (float)pixelSize9, 1.0f);
			}

			// Draw last line
			g.setColour(juce::Colours::whitesmoke);
			g.drawLine((float)width, (float)pixelSize, (float)width, (float)pixelSize9, 3.0f);
		}
	}

	// TODO - Refactor
	void setHorizontalZoom()
	{
		const int leftRegion = m_zoomRegionLeftLabel.getText().getIntValue();
		const int rightRegion = m_zoomRegionRightLabel.getText().getIntValue();

		const int regionsIndexMax = (int)m_regions.size() - 1;

		if (leftRegion == -1 || leftRegion > regionsIndexMax || rightRegion == -1 || rightRegion > regionsIndexMax)
		{
			return;
		}

		setHorizontalZoom(leftRegion, rightRegion);
	}

private:
	juce::AudioBuffer<float> m_audioBuffer;

	std::vector<Region> m_regions;

	GroupLabelComponent m_nameGroupComponent;
	GroupLabelComponent m_zoomGroupComponent{ "Zoom" };
	juce::Label m_zoomRegionLeftLabel;
	juce::Label m_zoomRegionRightLabel;
	juce::TextButton m_scrollLeftButton;
	juce::TextButton m_scrollRightButton;
	juce::TextButton m_resetZoomButton;
	juce::TextButton m_showDetailsButton;

	float m_verticalZoom = 1.0f;
	int m_leftSampleIndex = 0;
	int m_rightSampleIndex = 0;
	int m_leftRegionIndex = 0;
	int m_rightRegionIndex = 0;
};