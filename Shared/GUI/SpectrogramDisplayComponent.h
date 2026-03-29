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

class SpectrogramDisplayComponent : public juce::Component
{
public:
	SpectrogramDisplayComponent(juce::String name) : m_nameGroupComponent(name)
	{
		addAndMakeVisible(m_nameGroupComponent);
	}

	~SpectrogramDisplayComponent() = default;

	void setAudioBuffer(const juce::AudioBuffer<float> buffer)
	{
		m_audioBuffer = buffer;
		computeSpectrogram();
		repaint();
	}

private:
	static constexpr int FFT_ORDER = 14;  // 4096 samples for good low-frequency resolution
	static constexpr int FFT_SIZE = 1 << FFT_ORDER;
	static constexpr float MIN_FREQUENCY = 20.0f;
	static constexpr float MAX_FREQUENCY = 200.0f;
	static constexpr int NUM_FREQUENCY_BINS = 256;  // Number of pixels on Y axis
	static constexpr int NUM_TIME_BINS = 1024;       // Number of pixels on X axis

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

	void paint(juce::Graphics& g) override
	{
		const auto width = getWidth();
		const auto height = getHeight();
		const auto pixelSize = height / 11;

		const auto spectogramX = 0;
		const auto spectogramY = pixelSize;
		const auto spectogramWidth = width;
		const auto spectogramHeight = height - pixelSize;

		// Draw background
		g.setColour(juce::Colours::black);
		g.fillRect(spectogramX, spectogramY, spectogramWidth, spectogramHeight);

		// Draw spectrogram
		if (m_spectrogram.validTimeSteps > 0)
		{
			const float pixelWidth = (float)spectogramWidth / (float)m_spectrogram.validTimeSteps;
			const float pixelHeight = (float)spectogramHeight / (float)NUM_FREQUENCY_BINS;

			for (int timeIdx = 0; timeIdx < m_spectrogram.validTimeSteps; ++timeIdx)
			{
				for (int freqIdx = 0; freqIdx < NUM_FREQUENCY_BINS; ++freqIdx)
				{
					const float value = m_spectrogram.data[timeIdx][NUM_FREQUENCY_BINS - 1 - freqIdx];
					const juce::Colour colour = getColourForValue(value);

					g.setColour(colour);
					const float x = (float)spectogramX + (float)timeIdx * pixelWidth;
					const float y = (float)spectogramY + (float)freqIdx * pixelHeight;
					g.fillRect(x, y, pixelWidth, pixelHeight);
				}
			}
		}

		// Draw frequency labels on the left side
		g.setColour(juce::Colours::white);
		g.setFont(10.0f);
		g.drawText("200Hz", 0, spectogramY + 15, 45, 15, juce::Justification::topRight);
		g.drawText("20Hz", 0, spectogramY + spectogramHeight - 30, 45, 15, juce::Justification::bottomRight);
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
};
