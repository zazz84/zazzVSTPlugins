#pragma once

#include <vector>
#include <complex>
#include <cmath>
#include <cstring>
#include <algorithm>

#include <JuceHeader.h>

class SpectrumMatchFFTApply
{
public:
	/** fftSize must be a power of two (e.g. 512, 1024, 2048, 4096) */
	explicit SpectrumMatchFFTApply(int fftSizeInSamples, int overlapFactor = 4)
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
		m_bucketGain.resize(numBins, 1.0f);

		reset();
	}
	int getLatencyInSamples() const
	{
		return fftSize;
	}
	void init(int sampleRate)
	{
		m_sampleRate = sampleRate;
	}
	void reset()
	{
		count = 0;
		pos = 0;

		std::fill(inputFifo.begin(), inputFifo.end(), 0.0f);
		std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
	}
	void set(const std::vector<float>& spectrum)
	{
		jassert(spectrum.size() == numBins);

		std::copy(spectrum.begin(), spectrum.end(), m_bucketGain.begin());
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

private:
	void processFrame(bool bypassed)
	{
		float* fftPtr = fftData.data();

		// Copy circular buffer into FFT buffer
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

		for (int i = 0; i < numBins; ++i)
		{
			float magnitude = std::abs(cdata[i]);
			float phase = std::arg(cdata[i]);

			cdata[i] = std::polar(magnitude * m_bucketGain[i], phase);
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

	std::vector<float> m_bucketGain;

	int m_sampleRate = 48000;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumMatchFFTApply)
};

