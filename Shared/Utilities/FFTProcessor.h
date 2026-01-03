#pragma once

#include <vector>
#include <cmath>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

/**
  STFT analysis and resynthesis of audio data.

  Each channel should have its own FFTProcessor.
 */
class FFTProcessor
{
public:
	FFTProcessor();

	int getLatencyInSamples() const { return fftSize; }

	void reset();
	float processSample(float sample, bool bypassed);
	void processBlock(float* data, int numSamples, bool bypassed);
	
	void init(int sampleRate)
	{
		m_sampleRate = sampleRate;
	}

	void set(const float gain1, const float gain2, const float gain3, const float gain4, const float gain5)
	{
		// Settings did not change
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

		int bucketIndex = 0;

		for (size_t i = 0; i < numBins; i++)
		{
			if (m_bucketIndex[bucketIndex] < i)
			{
				bucketIndex++;
			}

			if (bucketIndex == 0)
			{
				m_bucketGain[i] = gain1;
			}
			else if (bucketIndex >= 5)
			{
				m_bucketGain[i] = gain5;
			}
			else
			{
				m_bucketGain[i] = Math::remap(i, m_bucketIndex[bucketIndex - 1], m_bucketIndex[bucketIndex], m_gain[bucketIndex - 1], m_gain[bucketIndex]);
			}
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
	float m_bucketGain[numBins];
	int m_bucketIndex[5];
	int m_sampleRate = 48000;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FFTProcessor)
};
