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

#pragma

#include <JuceHeader.h>
#include "juce_dsp/juce_dsp.h"

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"
#include "../../../zazzVSTPlugins/Shared/GUI/SpectrumAnalyzerComponent.h"

class FrequencySpectrum
{
public:
	FrequencySpectrum() : m_forwardFFT(fftOrder), m_window(fftSize, juce::dsp::WindowingFunction<float>::hann)
	{
	}
	~FrequencySpectrum() = default;

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;

		// Get indexes
		const int bucketFreq = m_sampleRate / fftSize;

		for (int i = 0; i < scopeSize; i++)
		{
			int index = (frequencies[i] + frequencies[i + 1]) / (2 * bucketFreq);
			if (index < 1)
			{
				index = 1;
			}

			m_fftDataIndexes[i] = index;
		}
	}

	void processBlock(juce::AudioBuffer<float>& buffer)
	{
		const auto channels = buffer.getNumChannels();
		const auto samples = buffer.getNumSamples();
		
		if (channels == 1)
		{
			auto* channelData = buffer.getWritePointer(0);

			for (int sample = 0; sample < samples; sample++)
			{
				process(channelData[sample]);
			}
		}
		else
		{
			auto* channelDataLeft = buffer.getWritePointer(0);
			auto* channelDataRight = buffer.getWritePointer(1);

			for (int sample = 0; sample < samples; sample++)
			{
				const float max = Math::fmaxf(channelDataLeft[sample], channelDataRight[sample]);
				process(max);
			}
		}
	}

	float(&getScopeData())[scopeSize]
	{
		return m_scopeData;
	}

private:
	void process(float sample, int channel) noexcept
	{
		m_fifo[m_fifoIndex] = sample;
		m_fifoIndex++;

		if (m_fifoIndex == fftSize)
		{
			juce::zeromem(m_fftData, sizeof(m_fftData));
			memcpy(m_fftData, m_fifo, sizeof(m_fifo));
			
			getSpectrum();

			m_fifoIndex = 0;
		}
	}

	void getSpectrum()
	{
		// first apply a m_windowing function to our data
		m_window.multiplyWithWindowingTable(m_fftData, fftSize);    

		// then render our FFT data..
		m_forwardFFT.performFrequencyOnlyForwardTransform(m_fftData); 

		auto mindB = -100.0f;
		auto maxdB = 0.0f;

		auto previousIndex = 0;
		for (int i = 0; i < scopeSize; i++) 
		{
			// Find maximum for given frequency range
			auto sumLevel = 0.0f;

			const auto curentIndex = m_fftDataIndexes[i];

			for (int j = previousIndex; j < curentIndex; j++)
			{
				const auto currentLevel = m_fftData[j];
				sumLevel += currentLevel;
			}
			
			sumLevel /= (curentIndex - previousIndex);

			previousIndex = curentIndex;
			
			// Clamped and normalized
			auto level = Math::clamp(sumLevel / static_cast<float>(fftSize), 0.0f, 1.0f);

			m_scopeData[i] = level; 
		}
	}

	juce::dsp::FFT m_forwardFFT;                      
	juce::dsp::WindowingFunction<float> m_window;     

	float m_fifo[2][fftSize];                            
	float m_fftData[2][2 * fftSize];                     
	float m_scopeData[2][scopeSize];

	int m_fftDataIndexes[scopeSize];
	const int frequencies[scopeSize + 1] = { 100, 200, 300, 400, 500, 600, 700, 800, 1000, 2000, 4000, 6000, 8000, 10000, 12000, 16000, 20000 };
	
	int m_fifoIndex = 0;
	int m_sampleRate = 48000;
};