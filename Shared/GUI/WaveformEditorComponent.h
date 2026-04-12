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
#include "WaveformDisplayComponent.h"
#include "SpectrogramDisplayComponent.h"

struct Region
{
	static const int FFT_ORDER = 12;
	static const int FFT_SIZE = 1 << FFT_ORDER;

	int m_sampleIndex = -1;
	bool m_isValid = false;
	int m_length = 0;

	float m_fftData[2 * FFT_SIZE];
	float m_difference = 0.0f;
};

class WaveformEditorComponent : public juce::Component
{
public:
	WaveformEditorComponent(juce::String name) : m_nameGroupComponent(name), m_spectrogramComponent(name + " Spectrum")
	{
		addAndMakeVisible(m_waveformDisplayComponent);
		addAndMakeVisible(m_waveformFilteredComponent);
		addAndMakeVisible(m_spectrogramComponent);
		m_spectrogramComponent.setVisible(false);

		// Configure filtered waveform appearance
		m_waveformFilteredComponent.setWaveformColour(juce::Colours::cyan);
		m_waveformFilteredComponent.setWaveformThickness(1.5f);
		m_waveformFilteredComponent.setVisible(false);

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

		addAndMakeVisible(&m_showFilteredButton);
		m_showFilteredButton.setButtonText("F");
		m_showFilteredButton.setClickingTogglesState(true);
		m_showFilteredButton.setToggleState(false, juce::NotificationType::dontSendNotification);
		m_showFilteredButton.onClick = [this]
		{
			m_waveformFilteredComponent.setVisible(m_showFilteredButton.getToggleState());
		};

		addAndMakeVisible(&m_showSpectrogramButton);
		m_showSpectrogramButton.setButtonText("S");
		m_showSpectrogramButton.setClickingTogglesState(true);
		m_showSpectrogramButton.setToggleState(false, juce::NotificationType::dontSendNotification);
		m_showSpectrogramButton.onClick = [this]
		{
			m_spectrogramComponent.setVisible(m_showSpectrogramButton.getToggleState());
			repaint();
		};

		addAndMakeVisible(&m_showRegionMarkersButton);
		m_showRegionMarkersButton.setButtonText("M");
		m_showRegionMarkersButton.setClickingTogglesState(true);
		m_showRegionMarkersButton.setToggleState(true, juce::NotificationType::dontSendNotification);
		m_showRegionMarkersButton.onClick = [this]
		{
			repaint();
		};

		addAndMakeVisible(m_phaseDisplayComponent);
		m_phaseDisplayComponent.setWaveformColour(juce::Colours::greenyellow);
		m_phaseDisplayComponent.setWaveformThickness(1.5f);
		m_phaseDisplayComponent.setVisible(false);

		addAndMakeVisible(&m_showPhaseButton);
		m_showPhaseButton.setButtonText("P");
		m_showPhaseButton.setClickingTogglesState(true);
		m_showPhaseButton.setToggleState(false, juce::NotificationType::dontSendNotification);
		m_showPhaseButton.onClick = [this]
		{
			m_phaseDisplayComponent.setVisible(m_showPhaseButton.getToggleState());
		};

	}
	~WaveformEditorComponent() = default;

	void setSampleRate(const int sampleRate)
	{
		m_spectrogramComponent.setSampleRate(sampleRate);
	}

	void setAudioBuffer(const juce::AudioBuffer<float> buffer)
	{
		m_waveformDisplayComponent.setAudioBuffer(buffer);
		m_spectrogramComponent.setAudioBuffer(buffer);
		m_leftSampleIndex = 0;
		m_rightSampleIndex = buffer.getNumSamples();

		// Clear filtered buffer
		m_waveformFilteredComponent.setAudioBuffer(juce::AudioBuffer<float>());

		// Clear phase buffer
		m_phaseDisplayComponent.setAudioBuffer(juce::AudioBuffer<float>());

		// Clear
		m_leftRegionIndex = -1;
		m_rightRegionIndex = -1;

		m_zoomRegionLeftLabel.setText("-1", juce::dontSendNotification);
		m_zoomRegionRightLabel.setText("-1", juce::dontSendNotification);

		m_regions.clear();
		repaint();
	}
	void setFilteredAudioBuffer(const juce::AudioBuffer<float> buffer, const bool shouldDisplay = true)
	{
		if (shouldDisplay && buffer.getNumSamples() > 0)
		{
			// Sanitize buffer: replace NaN values with 0
			auto sanitizedBuffer = buffer;
			for (int channel = 0; channel < sanitizedBuffer.getNumChannels(); ++channel)
			{
				auto* data = sanitizedBuffer.getWritePointer(channel);
				for (int i = 0; i < sanitizedBuffer.getNumSamples(); ++i)
				{
					if (std::isnan(data[i]) || std::isinf(data[i]))
						data[i] = 0.0f;
				}
			}

			m_waveformFilteredComponent.setAudioBuffer(sanitizedBuffer);
		}
		else
		{
			m_waveformFilteredComponent.setAudioBuffer(juce::AudioBuffer<float>());
		}
	}

	void setSpectrogramAudioBuffer(const juce::AudioBuffer<float> buffer)
	{
		m_spectrogramComponent.setAudioBuffer(buffer);
	}
	void setPhaseTrajectory(const juce::AudioBuffer<float>& phaseBuffer)
	{
		if (phaseBuffer.getNumSamples() <= 0)
		{
			m_phaseDisplayComponent.setAudioBuffer(juce::AudioBuffer<float>());
			return;
		}

		// Phase buffer is already normalized [-1, 1] from ZeroCrossingOffline
		// Set it directly to the display component
		m_phaseDisplayComponent.setAudioBuffer(phaseBuffer);
		m_phaseDisplayComponent.setHorizontalZoom(m_leftSampleIndex, m_rightSampleIndex);
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
		m_waveformDisplayComponent.setVerticalZoom(verticalZoom);
		m_waveformFilteredComponent.setVerticalZoom(verticalZoom);
	}

	void setHorizontalZoom(const int leftSampleIndex, const int rightSampleIndex)
	{
		m_leftSampleIndex = leftSampleIndex;
		m_rightSampleIndex = rightSampleIndex;

		m_waveformDisplayComponent.setHorizontalZoom(m_leftSampleIndex, m_rightSampleIndex);
		m_waveformFilteredComponent.setHorizontalZoom(m_leftSampleIndex, m_rightSampleIndex);
		m_phaseDisplayComponent.setHorizontalZoom(m_leftSampleIndex, m_rightSampleIndex);
		m_spectrogramComponent.setHorizontalZoom(m_leftSampleIndex, m_rightSampleIndex);

		repaint();
	}

	void setHorizontalZoomByRegions(const int leftRegionIndex, const int rightRegionIndex)
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

		setHorizontalZoom(m_leftSampleIndex, m_rightSampleIndex);
	}
	void resized() override
	{
		const auto width = getWidth();
		const auto height = getHeight();
		const auto pixelSize = height / 11;
		const auto pixelSize2 = pixelSize + pixelSize;
		const auto pixelSize4 = pixelSize2 + pixelSize2;
		const auto pixelSize8 = pixelSize4 + pixelSize4;

		// Set waveform display components size and position (overlapping)
		const auto waveformHeight = pixelSize8;
		const auto waveformY = pixelSize;
		m_waveformDisplayComponent.setSize(width, waveformHeight);
		m_waveformDisplayComponent.setTopLeftPosition(0, waveformY);
		m_waveformFilteredComponent.setSize(width, waveformHeight);
		m_waveformFilteredComponent.setTopLeftPosition(0, waveformY);
		m_phaseDisplayComponent.setSize(width, waveformHeight);
		m_phaseDisplayComponent.setTopLeftPosition(0, waveformY);

		// Set spectrogram display component size and position (overlapping)
		m_spectrogramComponent.setSize(width, waveformHeight + pixelSize);
		m_spectrogramComponent.setTopLeftPosition(0, waveformY - pixelSize);

		// Set size
		m_nameGroupComponent.setSize(width, pixelSize);
		m_zoomGroupComponent.setSize(pixelSize4, pixelSize);

		m_zoomRegionLeftLabel.setSize(pixelSize, pixelSize);
		m_zoomRegionRightLabel.setSize(pixelSize, pixelSize);
		m_scrollLeftButton.setSize(pixelSize, pixelSize);
		m_scrollRightButton.setSize(pixelSize, pixelSize);
		m_resetZoomButton.setSize(pixelSize, pixelSize);
		m_showDetailsButton.setSize(pixelSize, pixelSize);
		m_showFilteredButton.setSize(pixelSize, pixelSize);
		m_showSpectrogramButton.setSize(pixelSize, pixelSize);
		m_showRegionMarkersButton.setSize(pixelSize, pixelSize);
		m_showPhaseButton.setSize(pixelSize, pixelSize);

		// Set position
		const auto column1 = 0;	
		const auto column4 = width / 2;
		const auto column2 = column4 - pixelSize2;
		const auto column3 = column4 - pixelSize;
		const auto column5 = column4 + pixelSize;
		const auto column6 = column5 + pixelSize2;
		const auto column7 = column6 + pixelSize8;
		const auto column8 = column7 + pixelSize;
		const auto column9 = column8 + pixelSize;

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
		m_showRegionMarkersButton.setTopLeftPosition(column7, row4);
		m_showDetailsButton.setTopLeftPosition		(column8, row4);
		m_showFilteredButton.setTopLeftPosition		(column9, row4);
		m_showSpectrogramButton.setTopLeftPosition	(column9 + pixelSize, row4);
		m_showPhaseButton.setTopLeftPosition		(column9 + pixelSize2, row4);
	}

	void paint(juce::Graphics& g) override
	{
		const auto width = getWidth();
		const auto height = getHeight();
		const auto pixelSize = height / 11;
		const auto pixelSize8 = 8 * pixelSize;
		const auto waveformY = pixelSize;
		const auto waveformHeight = pixelSize8;

		// Draw black background for waveform display area
		g.setColour(juce::Colours::black);
		g.fillRect(0, waveformY, width, waveformHeight);
	}

	void paintOverChildren(juce::Graphics& g) override
	{
		const auto width = getWidth();
		const auto height = getHeight();
		const auto pixelSize = height / 11;
		const auto pixelSize4 = 4 * pixelSize;
		const auto pixelSize5 = 5 * pixelSize;
		const auto pixelSize8 = 8 * pixelSize;
		const auto waveformY = pixelSize;
		const auto waveformHeight = pixelSize8;

		// Draw regions if any exist and if visibility is enabled
		if (m_regions.size() == 0 || m_leftSampleIndex >= m_rightSampleIndex || 
			!m_showRegionMarkersButton.getToggleState())
		{
			return;
		}

		const float factor = (float)width / (float)(m_rightSampleIndex - m_leftSampleIndex);

		for (int region = m_leftRegionIndex; region <= m_rightRegionIndex && region < (int)m_regions.size(); region++)
		{
			const float x = factor * (float)(m_regions[region].m_sampleIndex - m_leftSampleIndex);

			if (m_regions[region].m_isValid)
			{
				g.setColour(juce::Colours::whitesmoke);
				g.drawLine(x, (float)waveformY, x, (float)(waveformY + waveformHeight), 3.0f);

				if (m_showDetailsButton.getToggleState())
				{
					constexpr float recWidth = 50.0f;

					juce::Rectangle<float> rectangle(x + 5.0f, (float)(waveformY + pixelSize4) + 5.0f, recWidth, 20.0f);
					g.setColour(juce::Colours::grey);
					g.fillRect(rectangle);
					g.setColour(juce::Colours::black);
					g.drawText(juce::String((float)region, 0), rectangle, juce::Justification::centred);

					rectangle.setBounds(x + 5.0f, (float)(waveformY + pixelSize4) + 30.0f, recWidth, 20.0f);
					g.setColour(juce::Colours::grey);
					g.fillRect(rectangle);
					g.setColour(juce::Colours::black);
					const int sampleIndex = m_regions[region].m_sampleIndex;
					g.drawText(juce::String((float)sampleIndex, 0), rectangle, juce::Justification::centred);

					const auto& audioBuffer = m_waveformDisplayComponent.getAudioBuffer();
					if (audioBuffer.getNumSamples() >= sampleIndex)
					{
						rectangle.setBounds(x + 5.0f, (float)(waveformY + pixelSize4) + 55.0f, recWidth, 20.0f);
						g.setColour(juce::Colours::grey);
						g.fillRect(rectangle);
						auto* channelData = audioBuffer.getReadPointer(0);
						g.setColour(juce::Colours::black);
						g.drawText(juce::String(juce::Decibels::gainToDecibels(fabsf(channelData[sampleIndex])), 1), rectangle, juce::Justification::centred);
					}

					rectangle.setBounds(x + 5.0f, (float)(waveformY + pixelSize4) + 80.0f, recWidth, 20.0f);
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

			g.drawLine(x, (float)waveformY, x, (float)(waveformY + waveformHeight), 1.0f);
		}

		g.setColour(juce::Colours::whitesmoke);
		g.drawLine((float)width, (float)waveformY, (float)width, (float)(waveformY + waveformHeight), 3.0f);
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

		setHorizontalZoomByRegions(leftRegion, rightRegion);
	}

	std::vector<Region> getExportedRegions() const
	{
		std::vector<Region> exportedRegions;
		
		// If no zoom range is set, export all regions
		if (m_leftRegionIndex < 0 || m_rightRegionIndex < 0 || m_leftRegionIndex > m_rightRegionIndex)
		{
			return m_regions;
		}
		
		// Export only regions within the current zoom range
		for (int i = m_leftRegionIndex; i <= m_rightRegionIndex && i < (int)m_regions.size(); ++i)
		{
			if (i >= 0)
				exportedRegions.push_back(m_regions[i]);
		}
		
		return exportedRegions;
	}

private:
	WaveformDisplayComponent m_waveformDisplayComponent;
	WaveformDisplayComponent m_waveformFilteredComponent;
	WaveformDisplayComponent m_phaseDisplayComponent;
	SpectrogramDisplayComponent m_spectrogramComponent;

	std::vector<Region> m_regions;

	GroupLabelComponent m_nameGroupComponent;
	GroupLabelComponent m_zoomGroupComponent{ "Zoom" };
	juce::Label m_zoomRegionLeftLabel;
	juce::Label m_zoomRegionRightLabel;
	juce::TextButton m_scrollLeftButton;
	juce::TextButton m_scrollRightButton;
	juce::TextButton m_resetZoomButton;
	juce::TextButton m_showDetailsButton;
	juce::TextButton m_showFilteredButton;
	juce::TextButton m_showSpectrogramButton;
	juce::TextButton m_showRegionMarkersButton;
	juce::TextButton m_showPhaseButton;

	int m_leftSampleIndex = 0;
	int m_rightSampleIndex = 0;
	int m_leftRegionIndex = 0;
	int m_rightRegionIndex = 0;
};
