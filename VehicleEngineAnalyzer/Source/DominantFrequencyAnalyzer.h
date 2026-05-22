#pragma once

#include <JuceHeader.h>
#include "juce_dsp/juce_dsp.h"
#include <vector>
#include <algorithm>
#include <cmath>

//==============================================================================
/*
	Analyzes audio segments based on dominant frequency ranges and time-based filtering.
	Detects segments where the dominant frequency falls within a specified range,
	with configurable pre/post-segment padding.
*/
class DominantFrequencyAnalyzer
{
public:
	struct AnalysisSegment
	{
		int startSample = 0;
		int endSample = 0;
		float dominantFrequency = 0.0f;
		float magnitude = 0.0f;

		int getDuration() const { return endSample - startSample; }
	};

	DominantFrequencyAnalyzer(int sampleRate = 48000, int fftSize = 4096)
		: m_sampleRate(sampleRate), m_fftSize(fftSize), m_fftOrder(static_cast<int>(std::log2(fftSize)))
	{
		m_fft = std::make_unique<juce::dsp::FFT>(m_fftOrder);
		m_window = std::make_unique<juce::dsp::WindowingFunction<float>>(
			m_fftSize, juce::dsp::WindowingFunction<float>::hann);
	}

	void setSampleRate(int sampleRate)
	{
		m_sampleRate = sampleRate;
	}

	//==========================================================================
	/*
		Analyzes the entire buffer and returns frequency and magnitude at each time window
	*/
	void analyzeBuffer(const juce::AudioBuffer<float>& buffer, 
					   std::vector<float>& dominantFrequencies,
					   std::vector<float>& magnitudes)
	{
		if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
		{
			dominantFrequencies.clear();
			magnitudes.clear();
			return;
		}

		const int numSamples = buffer.getNumSamples();
		const int hopSize = m_fftSize / 2;
		const int numFrames = (numSamples - m_fftSize) / hopSize + 1;

		dominantFrequencies.clear();
		dominantFrequencies.reserve(numFrames);
		magnitudes.clear();
		magnitudes.reserve(numFrames);

		const float* pBuffer = buffer.getReadPointer(0);

		for (int frameIdx = 0; frameIdx < numFrames; ++frameIdx)
		{
			const int startIdx = frameIdx * hopSize;
			if (startIdx + m_fftSize > numSamples)
				break;

			float dominantFreq = 0.0f;
			float magnitude = 0.0f;
			calculateDominantFrequency(pBuffer + startIdx, dominantFreq, magnitude);

			dominantFrequencies.push_back(dominantFreq);
			magnitudes.push_back(magnitude);
		}
	}

	//==========================================================================
	/*
		Finds all segments where the dominant frequency is within the specified range
	*/
	void findSegments(const juce::AudioBuffer<float>& buffer,
					  float minFrequency,
					  float maxFrequency,
					  int padSamples,
					  std::vector<AnalysisSegment>& outSegments)
	{
		outSegments.clear();

		if (buffer.getNumSamples() == 0 || minFrequency >= maxFrequency)
			return;

		// Analyze buffer to get dominant frequencies
		std::vector<float> dominantFreqs;
		std::vector<float> mags;
		analyzeBuffer(buffer, dominantFreqs, mags);

		if (dominantFreqs.empty())
			return;

		const int hopSize = m_fftSize / 2;
		const int numSamples = buffer.getNumSamples();

		// Find continuous regions where frequency is in range
		bool inSegment = false;
		int segmentStart = 0;
		float maxMagnitudeInSegment = 0.0f;
		float dominantFreqInSegment = 0.0f;

		for (size_t i = 0; i < dominantFreqs.size(); ++i)
		{
			const bool freqInRange = dominantFreqs[i] >= minFrequency && dominantFreqs[i] <= maxFrequency;

			if (freqInRange && !inSegment)
			{
				// Start new segment
				inSegment = true;
				segmentStart = static_cast<int>(i * hopSize);
				maxMagnitudeInSegment = mags[i];
				dominantFreqInSegment = dominantFreqs[i];
			}
			else if (!freqInRange && inSegment)
			{
				// End current segment
				inSegment = false;

				int endSample = static_cast<int>(i * hopSize);
				int startSample = segmentStart - padSamples;
				int finalEnd = endSample + padSamples;

				// Clamp to buffer bounds
				startSample = std::max(0, startSample);
				finalEnd = std::min(finalEnd, numSamples);

				if (finalEnd > startSample)
				{
					AnalysisSegment seg;
					seg.startSample = startSample;
					seg.endSample = finalEnd;
					seg.dominantFrequency = dominantFreqInSegment;
					seg.magnitude = maxMagnitudeInSegment;
					outSegments.push_back(seg);
				}
			}
			else if (freqInRange && inSegment)
			{
				// Update max magnitude in current segment
				if (mags[i] > maxMagnitudeInSegment)
				{
					maxMagnitudeInSegment = mags[i];
					dominantFreqInSegment = dominantFreqs[i];
				}
			}
		}

		// Handle case where buffer ends while still in segment
		if (inSegment)
		{
			int startSample = segmentStart - padSamples;
			int finalEnd = numSamples;

			startSample = std::max(0, startSample);

			if (finalEnd > startSample)
			{
				AnalysisSegment seg;
				seg.startSample = startSample;
				seg.endSample = finalEnd;
				seg.dominantFrequency = dominantFreqInSegment;
				seg.magnitude = maxMagnitudeInSegment;
				outSegments.push_back(seg);
			}
		}
	}

	//==========================================================================
	/*
		Extracts all segments and concatenates them into output buffer
	*/
	void extractSegmentsToBuffer(const juce::AudioBuffer<float>& buffer,
								 const std::vector<AnalysisSegment>& segments,
								 juce::AudioBuffer<float>& outBuffer)
	{
		outBuffer.clear();

		if (segments.empty() || buffer.getNumSamples() == 0)
			return;

		// Calculate total size
		int totalSamples = 0;
		for (const auto& seg : segments)
		{
			totalSamples += seg.getDuration();
		}

		if (totalSamples == 0)
			return;

		const int channels = buffer.getNumChannels();
		outBuffer.setSize(channels, totalSamples, false, true);

		// Copy segments
		int writePos = 0;
		for (const auto& seg : segments)
		{
			const int duration = seg.getDuration();

			for (int ch = 0; ch < channels; ++ch)
			{
				const float* srcData = buffer.getReadPointer(ch, seg.startSample);
				float* dstData = outBuffer.getWritePointer(ch, writePos);
				std::copy(srcData, srcData + duration, dstData);
			}

			writePos += duration;
		}
	}

	//==========================================================================
	/*
		Calculates the dominant frequency and magnitude for a single FFT frame
	*/
	void calculateDominantFrequency(const float* frameData, float& outFrequency, float& outMagnitude)
	{
		// Create working buffer
		float fftBuffer[4096 * 2] = {};
		std::copy(frameData, frameData + m_fftSize, fftBuffer);

		// Apply windowing
		m_window->multiplyWithWindowingTable(fftBuffer, m_fftSize);

		// Perform FFT
		m_fft->performFrequencyOnlyForwardTransform(fftBuffer);

		// Find peak in frequency spectrum
		int peakBin = 0;
		float peakMagnitude = 0.0f;

		// Start from bin 1 to skip DC component
		for (int i = 1; i < m_fftSize / 2; ++i)
		{
			if (fftBuffer[i] > peakMagnitude)
			{
				peakMagnitude = fftBuffer[i];
				peakBin = i;
			}
		}

		// Convert bin to frequency
		const float binFrequency = (static_cast<float>(peakBin) * m_sampleRate) / m_fftSize;
		outFrequency = binFrequency;
		outMagnitude = peakMagnitude;
	}

	int getHopSize() const { return m_fftSize / 2; }
	int getFftSize() const { return m_fftSize; }

private:
	std::unique_ptr<juce::dsp::FFT> m_fft;
	std::unique_ptr<juce::dsp::WindowingFunction<float>> m_window;
	int m_sampleRate;
	int m_fftSize;
	int m_fftOrder;
};
