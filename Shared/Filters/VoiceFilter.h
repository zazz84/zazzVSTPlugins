#pragma once

#include <vector>
#include <cmath>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

/**
  Based on: https://github.com/hollance/fft-juce
  
  STFT analysis and resynthesis of audio data.

  Each channel should have its own VoiceFilter.
 */
class VoiceFilter
{
public:
	VoiceFilter();

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
				m_bucketGain[i] = gain1Gain;
			}
			else if (bucketIndex >= 5)
			{
				m_bucketGain[i] = gain5Gain;
			}
			else
			{
				m_bucketGain[i] = juce::Decibels::decibelsToGain(Math::remap(i, m_bucketIndex[bucketIndex - 1], m_bucketIndex[bucketIndex], m_gain[bucketIndex - 1], m_gain[bucketIndex]));
			}
		}
	}*/

	inline float cubicBezier(float p0, float p1, float p2, float p3, float t)
	{
		float u = 1.0f - t;
		return u * u*u*p0 +
			3 * u*u*t*p1 +
			3 * u*t*t*p2 +
			t * t*t*p3;
	}

	/*inline float controlPoint(float prev, float curr, float next, float tension = 0.1f)
	{
		return curr + (next - prev) * tension;
	}*/

	/*inline float computeSlope(float d0, float d1)
	{
		if (d0 * d1 <= 0.0f)
			return 0.0f;

		return (2.0f * d0 * d1) / (d0 + d1);
	}*/

	inline float computeSlope(float d0, float d1, float smoothness = 1.5)
	{
		// Enforce monotonicity
		if (d0 * d1 <= 0.0f)
			return 0.0f;

		// Base Fritsch–Carlson slope
		float m = (2.0f * d0 * d1) / (d0 + d1);

		// Smoothness control
		// 0.0 -> linear
		// 1.0 -> full monotonic cubic
		return m * smoothness;
	}

	inline float hermite(float y0, float y1, float m0, float m1, float t)
	{
		float t2 = t * t;
		float t3 = t2 * t;

		return (2 * t3 - 3 * t2 + 1) * y0 +
			(t3 - 2 * t2 + t) * m0 +
			(-2 * t3 + 3 * t2) * y1 +
			(t3 - t2) * m1;
	}



	void set(float gain1, float gain2, float gain3,
		float gain4, float gain5)
	{
		float freq[7] = {
			0.0f,
			frequency[0],
			frequency[1],
			frequency[2],
			frequency[3],
			frequency[4],
			20000.0f
		};

		float gainDB[7] = {
			gain1, gain1, gain2, gain3, gain4, gain5, gain5
		};

		float gainLin[7];
		for (int i = 0; i < 7; i++)
			gainLin[i] = juce::Decibels::decibelsToGain(gainDB[i]);

		// Precompute slopes
		float slope[7] = {};

		for (int i = 1; i < 6; i++)
		{
			float d0 = gainLin[i] - gainLin[i - 1];
			float d1 = gainLin[i + 1] - gainLin[i];
			slope[i] = computeSlope(d0, d1);
		}

		const float nyquist = m_sampleRate * 0.5f;
		const float binHz = nyquist / (numBins - 1);

		int seg = 0;

		for (int i = 0; i < numBins; i++)
		{
			float hz = i * binHz;

			while (seg < 6 && hz > freq[seg + 1])
				seg++;

			float t = (hz - freq[seg]) / (freq[seg + 1] - freq[seg]);
			t = juce::jlimit(0.0f, 1.0f, t);

			m_bucketGain[i] = hermite(
				gainLin[seg],
				gainLin[seg + 1],
				slope[seg],
				slope[seg + 1],
				t
			);
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceFilter)
};
