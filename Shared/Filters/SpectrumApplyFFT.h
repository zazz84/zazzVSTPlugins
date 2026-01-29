#pragma once

#include <vector>
#include <cmath>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

/**
  STFT analysis and resynthesis of audio data.

  Each channel should have its own SpectrumApplyFFT.
 */
class SpectrumApplyFFT
{
public:
	SpectrumApplyFFT();

	// The FFT has 2^order points and fftSize/2 + 1 bins.
	static constexpr int fftOrder = 12;
	static constexpr int fftSize = 1 << fftOrder;      // 1024 samples
	static constexpr int numBins = fftSize / 2;        // 513 bins
	static constexpr int overlap = 4;                  // 75% overlap
	static constexpr int hopSize = fftSize / overlap;  // 256 samples

	struct Params
	{
		Params()
		{
			std::fill_n(m_gainsdB, numBins, 1.0f);
			std::fill_n(m_bypass, numBins, true);
		}

		explicit Params(const float* gains)
		{
			jassert(gains != nullptr);
			std::copy_n(gains, numBins, m_gainsdB);
			std::fill_n(m_bypass, numBins, true);
		}

		Params(const Params& other) noexcept
		{
			std::copy_n(other.m_gainsdB, numBins, m_gainsdB);
			std::copy_n(other.m_bypass, numBins, m_bypass);
		}

		Params& operator=(const Params& other) noexcept
		{
			if (this == &other)
				return *this;

			std::copy_n(other.m_gainsdB, numBins, m_gainsdB);
			std::copy_n(other.m_bypass, numBins, m_bypass);

			return *this;
		}

		float m_gainsdB[numBins];
		bool  m_bypass[numBins];
	};


	int getLatencyInSamples() const { return fftSize; }

	void reset();
	float processSample(float sample, bool bypassed);
	void processBlock(float* data, int numSamples, bool bypassed);
	void set(Params params)
	{
		m_params = params;
	}

private:
    void processFrame(bool bypassed);
    void processSpectrum(float* data);

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

	Params m_params;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumApplyFFT)
};
