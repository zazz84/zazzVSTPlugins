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
	FrequencySpectrum() : forwardFFT(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann)
	{

	}
	~FrequencySpectrum() = default;

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
		getfftDataIndexes();
	}

	void processBlock(juce::AudioBuffer<float>& buffer)
	{
		const auto channels = buffer.getNumChannels();
		const auto samples = buffer.getNumSamples();
		
		auto* channelData = buffer.getWritePointer(0);
		
		for (int sample = 0; sample < samples; sample++)
		{
			process(channelData[sample]);
		}
	}

	float(&getScopeData())[scopeSize]
	{
		return scopeData;
	}

private:
	void process(float sample) noexcept
	{
		fifo[fifoIndex] = sample;
		fifoIndex++;

		if (fifoIndex == fftSize)
		{
			juce::zeromem(fftData, sizeof(fftData));
			memcpy(fftData, fifo, sizeof(fifo));
			
			getSpectrum();

			fifoIndex = 0;
		}
	}

	void getSpectrum()
	{
		// first apply a windowing function to our data
		window.multiplyWithWindowingTable(fftData, fftSize);       // [1]

		// then render our FFT data..
		forwardFFT.performFrequencyOnlyForwardTransform(fftData);  // [2]

		auto mindB = -100.0f;
		auto maxdB = 0.0f;

		auto previousIndex = 0;
		for (int i = 0; i < scopeSize; i++)                         // [3]
		{
			// Find maximum for given frequency range
			auto sumLevel = 0.0f;

			const auto curentIndex = fftDataIndexes[i];

			for (int j = previousIndex; j < curentIndex; j++)
			{
				const auto currentLevel = fftData[j];
				sumLevel += currentLevel;
			}
			
			sumLevel /= (curentIndex - previousIndex);

			previousIndex = curentIndex;
			
			// Clamped and normalized
			auto level = Math::clamp(sumLevel / static_cast<float>(fftSize), 0.0f, 1.0f);

			scopeData[i] = level;                                   // [4]
		}
	}

	void getfftDataIndexes()
	{
		/*const auto melMax = Math::frequenyToMel(20000.0f);
		const auto melmin = Math::frequenyToMel(40.0f);

		const auto melStep = (melMax - melmin) / static_cast<float>(scopeSize);

		float mel = melmin;
		
		for (int i = 0; i < scopeSize; i++)
		{
			mel += melStep;
			const auto frequency = Math::melToFrequency(mel);

			const auto idx = static_cast<int>(frequency * fftSize / static_cast<float>(m_sampleRate));
			fftDataIndexes[i] = idx;
		}*/


		/*float frequency = 55.0f;

		for (int i = 0; i < scopeSize; i++)
		{
			const auto idx = static_cast<int>(frequency * fftSize / static_cast<float>(m_sampleRate));
			fftDataIndexes[i] = idx;

			frequency *= 2.0f;
		}*/
	}

	juce::dsp::FFT forwardFFT;                      // [4]
	juce::dsp::WindowingFunction<float> window;     // [5]

	float fifo[fftSize];                            // [6]
	float fftData[2 * fftSize];                     // [7]
	int fifoIndex = 0;                              // [8]
	float scopeData[scopeSize];                     // [10]
	int fftDataIndexes[scopeSize] = { 1, 2, 3, 4, 5, 6, 8, 10, 15, 30, 50, 70, 90, 120, 180, 220 };

	int m_sampleRate = 48000;
};