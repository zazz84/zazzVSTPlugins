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

class SpectrogramDisplayComponent : public juce::Component, public juce::TooltipClient
{
public:
	SpectrogramDisplayComponent(juce::String name) : m_nameGroupComponent(name)
	{
		addAndMakeVisible(m_nameGroupComponent);
	}

	~SpectrogramDisplayComponent() = default;
	
	juce::String getTooltip() override
	{
		return "";  // Tooltip is drawn in paint() instead of using the tooltip system
	}

	void setAudioBuffer(const juce::AudioBuffer<float> buffer)
	{
		m_audioBuffer = buffer;
		computeSpectrogram();
		updateSpectrogramImage();
	}
	
private:
	static constexpr int FFT_ORDER = 15;  // 4096 samples for good low-frequency resolution
	static constexpr int FFT_SIZE = 1 << FFT_ORDER;
	static constexpr float MIN_FREQUENCY = 20.0f;
	static constexpr float MAX_FREQUENCY = 200.0f;
	static constexpr int NUM_FREQUENCY_BINS = 128;  // Number of pixels on Y axis
	static constexpr int NUM_TIME_BINS = 2048;      // Number of pixels on X axis

	struct SpectrogramData
	{
		std::vector<std::vector<float>> data;
		int validTimeSteps = 0;
		float maxMagnitude = 0.0f;

		SpectrogramData() : data(NUM_TIME_BINS, std::vector<float>(NUM_FREQUENCY_BINS, 0.0f)) {}
	};

	void computeSpectrogram()
	{
		m_spectrogram.validTimeSteps = 0;
		m_spectrogram.maxMagnitude = 0.0f;

		if (m_audioBuffer.getNumSamples() == 0)
		{
			return;
		}

		juce::dsp::FFT forwardFFT(FFT_ORDER);
		juce::dsp::WindowingFunction<float> window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann);

		auto* channelData = m_audioBuffer.getReadPointer(0);
		const int totalSamples = m_audioBuffer.getNumSamples();
		const int hopSize = (totalSamples - FFT_SIZE) / (NUM_TIME_BINS - 1);

		if (hopSize <= 0)
		{
			return;
		}

		int sampleRate = 48000;  // Default, will be improved by passing it from MainComponent
		const float binFrequencyResolution = (float)sampleRate / FFT_SIZE;

		// First pass: compute all FFT data and find maximum magnitude
		float maxMagnitudeLocal = 0.0f;
		std::vector<std::vector<float>> tempData(NUM_TIME_BINS, std::vector<float>(NUM_FREQUENCY_BINS, 0.0f));

		for (int timeIdx = 0; timeIdx < NUM_TIME_BINS; ++timeIdx)
		{
			int startSample = timeIdx * hopSize;

			if (startSample + FFT_SIZE > totalSamples)
			{
				break;
			}

			float fftData[2 * FFT_SIZE]{};

			// Copy audio data
			for (int i = 0; i < FFT_SIZE; ++i)
			{
				fftData[i] = channelData[startSample + i];
			}

			// Apply windowing
			window.multiplyWithWindowingTable(fftData, FFT_SIZE);

			// Perform FFT
			forwardFFT.performFrequencyOnlyForwardTransform(fftData);

			// Map FFT bins to frequency range [20Hz, 200Hz]
			const int minBin = (int)(MIN_FREQUENCY / binFrequencyResolution);
			const int maxBin = (int)(MAX_FREQUENCY / binFrequencyResolution);
			const int freqBinRange = maxBin - minBin;

			for (int freqIdx = 0; freqIdx < NUM_FREQUENCY_BINS; ++freqIdx)
			{
				const int fftBin = minBin + (freqIdx * freqBinRange) / NUM_FREQUENCY_BINS;

				if (fftBin >= 0 && fftBin < FFT_SIZE)
				{
					const float magnitude = std::sqrt(fftData[fftBin] * fftData[fftBin] + fftData[fftBin + FFT_SIZE] * fftData[fftBin + FFT_SIZE]);
					tempData[timeIdx][freqIdx] = magnitude;
					maxMagnitudeLocal = std::max(maxMagnitudeLocal, magnitude);
				}
			}

			m_spectrogram.validTimeSteps++;
		}

		// Second pass: normalize all values based on maximum magnitude
		m_spectrogram.maxMagnitude = maxMagnitudeLocal;

		if (maxMagnitudeLocal > 0.0f)
		{
			for (int timeIdx = 0; timeIdx < m_spectrogram.validTimeSteps; ++timeIdx)
			{
				for (int freqIdx = 0; freqIdx < NUM_FREQUENCY_BINS; ++freqIdx)
				{
					const float normalized = tempData[timeIdx][freqIdx] / maxMagnitudeLocal;
					m_spectrogram.data[timeIdx][freqIdx] = juce::jlimit(0.0f, 1.0f, normalized);
				}
			}
		}
	}

	void updateSpectrogramImage()
	{
		const int width = getWidth();
		const int height = getHeight();

		if (width <= 0 || height <= 0)
		{
			return;
		}

		// Create image to cache the spectrogram
		m_spectrogramImage = juce::Image(juce::Image::RGB, width, height, true);
		juce::Graphics g(m_spectrogramImage);

		const auto pixelSize = height / 11;

		const int spectrogramX = 0;
		const int spectrogramY = pixelSize;
		const int spectrogramWidth = width;
		const int spectrogramHeight = height - pixelSize;

		// Draw background
		g.setColour(juce::Colours::black);
		g.fillRect(spectrogramX, spectrogramY, spectrogramWidth, spectrogramHeight);

		// Draw spectrogram
		if (m_spectrogram.validTimeSteps > 0)
		{
			const float pixelWidth = (float)spectrogramWidth / (float)m_spectrogram.validTimeSteps;
			const float pixelHeight = (float)spectrogramHeight / (float)NUM_FREQUENCY_BINS;

			for (int timeIdx = 0; timeIdx < m_spectrogram.validTimeSteps; ++timeIdx)
			{
				for (int freqIdx = 0; freqIdx < NUM_FREQUENCY_BINS; ++freqIdx)
				{
					const float value = m_spectrogram.data[timeIdx][NUM_FREQUENCY_BINS - 1 - freqIdx];
					const juce::Colour colour = getColourForValue(value);

					g.setColour(colour);
					const float x = (float)spectrogramX + (float)timeIdx * pixelWidth;
					const float y = (float)spectrogramY + (float)freqIdx * pixelHeight;
					g.fillRect(x, y, pixelWidth, pixelHeight);
				}
			}
		}
	}

	void paint(juce::Graphics& g) override
	{
		// Draw cached spectrogram image
		if (!m_spectrogramImage.isNull())
		{
			g.drawImageAt(m_spectrogramImage, 0, 0);
		}

		// Draw tooltip in the bottom right corner with frequency and samples on separate lines
		if (m_tooltipFrequency >= 0.0f)
		{
			const int sampleRate = 48000;
			const int samplesPerCycle = (int)(sampleRate / m_tooltipFrequency);

			g.setColour(juce::Colours::white);
			g.setFont(12.0f);

			const int tooltipX = getWidth() - 100;
			const int tooltipY = getHeight() - 35;
			const int tooltipWidth = 95;
			const int lineHeight = 15;

			// Draw frequency on first line
			juce::String frequencyLine = juce::String(m_tooltipFrequency, 1) + " Hz";
			g.drawText(frequencyLine, tooltipX, tooltipY, tooltipWidth, lineHeight, juce::Justification::right);

			// Draw samples on second line
			juce::String samplesLine = juce::String(samplesPerCycle) + " samples";
			g.drawText(samplesLine, tooltipX, tooltipY + lineHeight, tooltipWidth, lineHeight, juce::Justification::right);
		}
	}

	void resized() override
	{
		const auto width = getWidth();
		const auto height = getHeight();
		const auto pixelSize = height / 11;

		// Set size
		m_nameGroupComponent.setSize(width, pixelSize);

		// Set position
		const auto column1 = 0;
		const auto row1 = 0;

		m_nameGroupComponent.setTopLeftPosition(column1, row1);

		// Regenerate cached spectrogram image when component is resized
		updateSpectrogramImage();
	}

	void mouseMove(const juce::MouseEvent& event) override
	{
		// Calculate spectrogram display area bounds
		const int pixelSize = getHeight() / 11;
		const int spectrogramX = 0;
		const int spectrogramY = pixelSize;
		const int spectrogramWidth = getWidth();
		const int spectrogramHeight = getHeight() - pixelSize;

		// Check if mouse is over the spectrogram display area
		if (event.x >= spectrogramX && event.x < spectrogramX + spectrogramWidth &&
			event.y >= spectrogramY && event.y < spectrogramY + spectrogramHeight)
		{
			// Calculate which frequency bin this pixel corresponds to
			const float relativY = (float)(event.y - spectrogramY);
			const int freqIdx = (int)((relativY / (float)spectrogramHeight) * NUM_FREQUENCY_BINS);
			const int clampedFreqIdx = juce::jlimit(0, NUM_FREQUENCY_BINS - 1, freqIdx);

			// Convert pixel index back to FFT bin using the same logic as computeSpectrogram
			const int sampleRate = 48000;
			const float binFrequencyResolution = (float)sampleRate / FFT_SIZE;
			const int minBin = (int)(MIN_FREQUENCY / binFrequencyResolution);
			const int maxBin = (int)(MAX_FREQUENCY / binFrequencyResolution);
			const int freqBinRange = maxBin - minBin;

			// Map from pixel index to FFT bin (accounting for display inversion)
			const int displayFreqIdx = NUM_FREQUENCY_BINS - 1 - clampedFreqIdx;
			const int fftBin = minBin + (displayFreqIdx * freqBinRange) / NUM_FREQUENCY_BINS;

			// Convert FFT bin back to frequency
			 q = fftBin * binFrequencyResolution;
		}
		else
		{
			m_tooltipFrequency = -1.0f;  // Invalid frequency to indicate no tooltip
		}

		repaint();  // Only repaint the tooltip overlay, spectrogram is cached
	}

	juce::Colour getColourForValue(float value)
	{
		// Gradient from dark navy blue (value=0) to yellow/orange (value=1)
		// Clamp value to [0, 1] range
		const float clampedValue = juce::jlimit(0.0f, 1.0f, value);

		// Navy blue: H=0.65 (blue), S=0.9, V=0.6
		// Yellow/Orange: H=0.1, S=0.95, V=1.0
		const float hueStart = 0.65f;    // Navy blue
		const float hueEnd = 0.1f;       // Yellow/Orange
		const float satStart = 0.9f;
		const float satEnd = 0.95f;
		const float brightStart = 0.6f;
		const float brightEnd = 1.0f;

		// Interpolate HSV values
		const float hue = hueStart + (hueEnd - hueStart) * clampedValue;
		const float saturation = satStart + (satEnd - satStart) * clampedValue;
		const float brightness = brightStart + (brightEnd - brightStart) * clampedValue;

		return juce::Colour::fromHSV(hue, saturation, brightness, 1.0f);
	}

	juce::AudioBuffer<float> m_audioBuffer;
	SpectrogramData m_spectrogram;
	zazzGUI::GroupLabel m_nameGroupComponent;
	juce::Image m_spectrogramImage;  // Cached spectrogram rendering
	float m_tooltipFrequency = -1.0f;  // Current frequency at mouse position (-1 = no tooltip)

};
