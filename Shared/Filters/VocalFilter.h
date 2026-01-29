#pragma once

#include <vector>
#include <cmath>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

/**
  Based on: https://github.com/hollance/fft-juce
  
  STFT analysis and resynthesis of audio data.

  Each channel should have its own VocalFilter.
 */
class VocalFilter
{
public:
	VocalFilter();

	int getLatencyInSamples() const { return fftSize; }

	void reset();
	float processSample(float sample, bool bypassed);
	void processBlock(float* data, int numSamples, bool bypassed);
	void init(int sampleRate)
	{
		m_sampleRate = sampleRate;
	}

	/*void set(const float gain1, const float gain2, const float gain3, const float gain4, const float gain5)
	{
		// Early exit if nothing changed
		if (m_gain[0] == gain1 &&
			m_gain[1] == gain2 &&
			m_gain[2] == gain3 &&
			m_gain[3] == gain4 &&
			m_gain[4] == gain5)
		{
			return;
		}

		m_gain[0] = gain1;
		m_gain[1] = gain2;
		m_gain[2] = gain3;
		m_gain[3] = gain4;
		m_gain[4] = gain5;

		const int bucketFrequency = m_sampleRate / fftSize;

		for (size_t i = 0; i < 5; i++)
		{
			m_bucketIndex[i] = frequency[i] / bucketFrequency;
		}

		const float gain1Gain = juce::Decibels::decibelsToGain(gain1);
		const float gain5Gain = juce::Decibels::decibelsToGain(gain5);
		int bucketIndex = 0;

		for (size_t i = 0; i < numBins; i++)
		{
			// Move to next bucket if needed
			if (m_bucketIndex[bucketIndex] < i)
			{
				bucketIndex++;
			}

			// Outside bounds
			if (bucketIndex == 0)
			{
				m_binGain[i] = gain1Gain;
			}
			else if (bucketIndex >= 5)
			{
				m_binGain[i] = gain5Gain;
			}
			else
			{
				m_binGain[i] = juce::Decibels::decibelsToGain(Math::remap(i, m_bucketIndex[bucketIndex - 1], m_bucketIndex[bucketIndex], m_gain[bucketIndex - 1], m_gain[bucketIndex]));
			}
		}
	}*/

	void set(const float gain1, const float gain2, const float gain3,
		const float gain4, const float gain5)
	{
		// Early exit if nothing changed
		if (m_gain[0] == gain1 && m_gain[1] == gain2 &&
			m_gain[2] == gain3 && m_gain[3] == gain4 &&
			m_gain[4] == gain5)
			return;

		m_gain[0] = gain1;
		m_gain[1] = gain2;
		m_gain[2] = gain3;
		m_gain[3] = gain4;
		m_gain[4] = gain5;

		// Target frequencies and gains
		constexpr int numPoints = 10;
		const float freqs[numPoints] = { 0.0f,  80.0f, 220.0f, 280.0f, 600.0f, 720.0f, 2800.0f, 3800.0f, 10000.0f, m_sampleRate / 2.0f };
		const float gains[numPoints] = { gain1, gain1, gain2, gain2, gain3, gain3, gain4, gain4, gain5, gain5 };

		// Precompute Mel values of target frequencies
		float mels[numPoints];
		for (int i = 0; i < numPoints; i++)
			mels[i] = 2595.0f * log10f(1.0f + freqs[i] / 700.0f);

		// Fill FFT bin gains using linear interpolation in Mel space
		const float sampleRateHalf = 0.5f * (float)m_sampleRate;
		
		for (int i = 0; i < numBins; i++)
		{
			float freq = (float)i * sampleRateHalf / float(numBins - 1);
			float mel = 2595.0f * log10f(1.0f + freq / 700.0f);

			// Find the interval in Mel table
			int idx = 0;
			while (idx < numPoints - 1 && mel > mels[idx + 1])
				idx++;

			float mel0 = mels[idx];
			float mel1 = mels[idx + 1];
			float g0 = gains[idx];
			float g1 = gains[idx + 1];

			float frac = (mel1 - mel0 > 0.0f) ? (mel - mel0) / (mel1 - mel0) : 0.0f;
			float gain = g0 + frac * (g1 - g0);

			m_binGain[i] = juce::Decibels::decibelsToGain(gain);
		}
	}

private:
    void processFrame(bool bypassed);
    void processSpectrum(float* data, int numBins);

    // The FFT has 2^order points and fftSize/2 + 1 bins.
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;      // 1024 samples
    static constexpr int numBins = fftSize / 2 + 1;    // 513 bins
    static constexpr int overlap = 4;                  // 75% overlap
    static constexpr int hopSize = fftSize / overlap;  // 256 samples
	static const int frequency[];

    // Gain correction for using Hann window with 75% overlap.
    static constexpr float windowCorrection = 2.0f / 3.0f;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    // Counts up until the next hop.
    int count = 0;

    // Write position in input FIFO and read position in output FIFO.
    int pos = 0;

    // Circular buffers for incoming and outgoing audio data.
    std::array<float, fftSize> inputFifo;
    std::array<float, fftSize> outputFifo;

    // The FFT working space. Contains interleaved complex numbers.
    std::array<float, fftSize * 2> fftData;

	// Filter setup
	float m_gain[5];
	float m_binGain[numBins];
	int m_bucketIndex[5];
	int m_sampleRate = 48000;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalFilter)
};
