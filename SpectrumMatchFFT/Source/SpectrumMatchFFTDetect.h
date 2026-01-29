#pragma once

#include <vector>
#include <complex>
#include <cmath>
#include <cstring>
#include <atomic>
#include <algorithm>

#include <JuceHeader.h>

/**
  STFT analysis and resynthesis of audio data.

  Each channel should have its own SpectrumMatchFFTDetect.
 */
class SpectrumMatchFFTDetect
{
public:
	/** fftSize must be a power of two */
	explicit SpectrumMatchFFTDetect(int fftSizeInSamples, int overlapFactor = 4)
		: fftSize(fftSizeInSamples),
		overlap(overlapFactor),
		fftOrder(static_cast<int>(std::log2(fftSizeInSamples))),
		hopSize(fftSizeInSamples / overlapFactor),
		numBins(fftSizeInSamples / 2 + 1),
		fft(fftOrder),
		window(fftSizeInSamples + 1, juce::dsp::WindowingFunction<float>::hann, false)
	{
		jassert(juce::isPowerOfTwo(fftSizeInSamples));
		jassert(overlapFactor > 0);

		inputFifo.resize(fftSize);
		outputFifo.resize(fftSize);
		fftData.resize(fftSize * 2);

		m_detectedSpectrumSum.resize(numBins, 0.0f);
		m_averageSpectrum.resize(numBins, 0.0f);

		reset();
	}

	int getLatencyInSamples() const
	{
		return fftSize;
	}

	int getNumBins() const
	{
		return numBins;
	}

	void reset()
	{
		count = 0;
		pos = 0;
		m_processSpectrumCount = 0;

		std::fill(inputFifo.begin(), inputFifo.end(), 0.0f);
		std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
		std::fill(m_detectedSpectrumSum.begin(), m_detectedSpectrumSum.end(), 0.0f);
	}

	void processBlock(float* data, int numSamples, bool bypassed)
	{
		for (int i = 0; i < numSamples; ++i)
			data[i] = processSample(data[i], bypassed);
	}

	float processSample(float sample, bool bypassed)
	{
		inputFifo[pos] = sample;

		float outputSample = outputFifo[pos];
		outputFifo[pos] = 0.0f;

		if (++pos == fftSize)
			pos = 0;

		if (++count == hopSize)
		{
			count = 0;
			processFrame(bypassed);
		}

		return outputSample;
	}

	/** Returns pointer valid until next call */
	float* getSpectrum()
	{
		const float normalisationFactor = (0.5f * 2.0f) / (float)(fftSize * m_processSpectrumCount);

		for (int i = 0; i < numBins; i++)
		{
			m_averageSpectrum[i] = normalisationFactor * m_detectedSpectrumSum[i];
		}

		return m_averageSpectrum.data();
	}

private:
	void processFrame(bool bypassed)
	{
		float* fftPtr = fftData.data();

		std::memcpy(fftPtr,
			inputFifo.data() + pos,
			(fftSize - pos) * sizeof(float));

		if (pos > 0)
		{
			std::memcpy(fftPtr + fftSize - pos,
				inputFifo.data(),
				pos * sizeof(float));
		}

		window.multiplyWithWindowingTable(fftPtr, fftSize);

		if (!bypassed)
		{
			fft.performRealOnlyForwardTransform(fftPtr, true);
			processSpectrum(fftPtr);
			fft.performRealOnlyInverseTransform(fftPtr);
		}

		window.multiplyWithWindowingTable(fftPtr, fftSize);

		constexpr float windowCorrection = 2.0f / 3.0f;
		for (int i = 0; i < fftSize; ++i)
			fftPtr[i] *= windowCorrection;

		// Overlap-add
		for (int i = 0; i < pos; ++i)
			outputFifo[i] += fftData[i + fftSize - pos];

		for (int i = 0; i < fftSize - pos; ++i)
			outputFifo[i + pos] += fftData[i];
	}

	void processSpectrum(float* data)
	{
		auto* cdata = reinterpret_cast<std::complex<float>*>(data);

		++m_processSpectrumCount;

		for (int i = 0; i < numBins; ++i)
		{
			float magnitude = std::abs(cdata[i]);
			float phase = std::arg(cdata[i]);

			m_detectedSpectrumSum[i] += magnitude;
			cdata[i] = std::polar(magnitude, phase);
		}
	}

private:
	// Runtime configuration
	const int fftSize;
	const int overlap;
	const int fftOrder;
	const int hopSize;
	const int numBins;

	juce::dsp::FFT fft;
	juce::dsp::WindowingFunction<float> window;

	int count = 0;
	int pos = 0;

	std::vector<float> inputFifo;
	std::vector<float> outputFifo;
	std::vector<float> fftData;

	std::vector<float> m_detectedSpectrumSum;
	std::vector<float> m_averageSpectrum;

	int m_processSpectrumCount = 0;

	std::atomic<bool> newDataReady{ false };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumMatchFFTDetect)
};

