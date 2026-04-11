#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

//==============================================================================
/**
 * Performs zero-phase (bidirectional) filtering on an audio buffer.
 * Applies a biquad filter in both forward and backward directions to achieve
 * zero phase shift while maintaining the magnitude response.
 * 
 * This is useful for offline processing where phase distortion must be minimized.
 */
class ZeroPhaseFilter
{
public:
	//==========================================================================
	/**
	 * Create a zero-phase filter with specified configuration.
	 * 
	 * @param sampleRate Sample rate in Hz
	 */
	explicit ZeroPhaseFilter(double sampleRate)
		: m_sampleRate(sampleRate)
	{
	}

	//==========================================================================
	/**
	 * Apply zero-phase filtering to an audio buffer using a high-pass filter.
	 * 
	 * @param buffer Audio buffer to filter (modified in-place)
	 * @param frequency Filter frequency in Hz
	 * @param resonance Filter resonance (Q factor)
	 */
	void applyHighPassZeroPhase(juce::AudioBuffer<float>& buffer, float frequency, float resonance)
	{
		BiquadFilter filter;
		filter.init(m_sampleRate);
		filter.setHighPass(frequency, resonance);
		
		applyZeroPhaseFilter(buffer, filter);
	}

	//==========================================================================
	/**
	 * Apply zero-phase filtering to an audio buffer using a low-pass filter.
	 * 
	 * @param buffer Audio buffer to filter (modified in-place)
	 * @param frequency Filter frequency in Hz
	 * @param resonance Filter resonance (Q factor)
	 */
	void applyLowPassZeroPhase(juce::AudioBuffer<float>& buffer, float frequency, float resonance)
	{
		BiquadFilter filter;
		filter.init(m_sampleRate);
		filter.setLowPass(frequency, resonance);
		
		applyZeroPhaseFilter(buffer, filter);
	}

	//==========================================================================
	/**
	 * Apply zero-phase filtering to an audio buffer using a band-pass filter.
	 * This applies high-pass filtering followed by low-pass filtering.
	 * 
	 * @param buffer Audio buffer to filter (modified in-place)
	 * @param lowFrequency Low cut frequency in Hz
	 * @param highFrequency High cut frequency in Hz
	 * @param resonance Filter resonance (Q factor) for both filters
	 */
	void applyBandPassZeroPhase(juce::AudioBuffer<float>& buffer, float lowFrequency, float highFrequency, float resonance)
	{
		// First apply high-pass to remove low frequencies
		applyHighPassZeroPhase(buffer, lowFrequency, resonance);

		// Then apply low-pass to remove high frequencies
		applyLowPassZeroPhase(buffer, highFrequency, resonance);
	}

	//==========================================================================
	/**
	 * Apply zero-phase filtering using a custom configured filter.
	 * The filter should be pre-configured before calling this method.
	 * 
	 * @param buffer Audio buffer to filter (modified in-place)
	 * @param filter Pre-configured BiquadFilter
	 */
	void applyZeroPhaseFilter(juce::AudioBuffer<float>& buffer, BiquadFilter& filter)
	{
		const int numChannels = buffer.getNumChannels();
		const int numSamples = buffer.getNumSamples();

		// Forward pass
		for (int channel = 0; channel < numChannels; ++channel)
		{
			auto* data = buffer.getWritePointer(channel);
			for (int i = 0; i < numSamples; ++i)
			{
				float sample = data[i];
				sample = filter.processDF1(sample);
				data[i] = sample;
			}
			filter.reset();
		}

		// Reverse for backward pass
		for (int channel = 0; channel < numChannels; ++channel)
		{
			auto* data = buffer.getWritePointer(channel);
			std::reverse(data, data + numSamples);
		}

		// Backward pass
		for (int channel = 0; channel < numChannels; ++channel)
		{
			auto* data = buffer.getWritePointer(channel);
			for (int i = 0; i < numSamples; ++i)
			{
				float sample = data[i];
				sample = filter.processDF1(sample);
				data[i] = sample;
			}
			filter.reset();
		}

		// Reverse back to original order
		for (int channel = 0; channel < numChannels; ++channel)
		{
			auto* data = buffer.getWritePointer(channel);
			std::reverse(data, data + numSamples);
		}
	}

private:
	double m_sampleRate;
};
