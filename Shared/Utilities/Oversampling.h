#pragma once

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

class Oversampling
{
public:
	Oversampling() {};

	void init(int sampleRate, int oversample, int samples)
	{
		// Set oversample filter
		const int oversampleSampleRate = oversample * sampleRate;
		m_oversampleFilter.init(oversampleSampleRate);
		m_oversampleFilter.setLowPass((0.9f * 0.5f) * (float)sampleRate, 0.7f);

		// Set downsample filter
		m_downsampleFilter.init(oversampleSampleRate);
		m_downsampleFilter.setLowPass((0.9f * 0.5f) * (float)oversampleSampleRate, 0.7f);
		
		m_oversample = oversample;
		m_samples = samples;
	}

	void Oversample(float* inputBuffer, float* oversampeledBuffer)
	{
		for (int i = 0; i < m_samples; i++)
		{
			const int index = 2 * i;
			oversampeledBuffer[index] = m_oversampleFilter.processDF1(2.0f * inputBuffer[i]);
			oversampeledBuffer[index + 1] = m_oversampleFilter.processDF1(0.0f);
		}
	};
	void Downsample(float* oversampeledBuffer, float* downsampeledBuffer)
	{
		for (int i = 0; i < m_samples; i++)
		{
			const int index = 2 * i;
			downsampeledBuffer[i] = m_downsampleFilter.processDF1(oversampeledBuffer[index]);
			m_downsampleFilter.processDF1(oversampeledBuffer[index + 1]);
		}
	};

protected:
	BiquadFilter m_oversampleFilter;
	BiquadFilter m_downsampleFilter;
	int m_oversample = 2;
	int m_samples = 0;
};