#pragma once

#include "../../../zazzVSTPlugins/Shared/Filters/HighOrderBiquadFilter.h"

class Oversampling
{
public:
	Oversampling() {};

	inline void init(const int sampleRate, const int oversamplingRation, const int samples)
	{
		// Set oversample buffer
		if (m_oversampleBuffer != nullptr)
		{
			clearBuffer();
		}
		
		m_oversampleSamples = oversamplingRation * samples;
		m_oversampleBuffer = new float[m_oversampleSamples];
		memset(m_oversampleBuffer, 0, m_oversampleSamples * sizeof(float));

		// Set oversample filter
		m_oversampleSampleRate = oversamplingRation * sampleRate;
		m_oversampleFilter.init(m_oversampleSampleRate);
		m_oversampleFilter.set((0.8f * 0.5f) * (float)sampleRate);

		// Set downsample filter
		m_downsampleFilter.init(m_oversampleSampleRate);
		m_downsampleFilter.set((0.8f * 0.5f) * (float)sampleRate);
		
		m_oversamplingRatio = oversamplingRation;
		m_samples = samples;
	}
	// oversampeledBuffer has to be filled with zeros
	inline void oversample(float* inputBuffer)
	{
		/*int sampleOversample = 0;
		for (int sample = 0; sample < m_samples; sample++)
		{
			m_oversampleBuffer[sampleOversample] = inputBuffer[sample];
			sampleOversample += m_oversamplingRatio;
		}*/

		for (int sample = 0; sample < m_oversampleSamples; sample++)
		{
			const float in = (sample % m_oversamplingRatio == 0) ? inputBuffer[sample / m_oversamplingRatio] : 0.0f;

			//m_oversampleBuffer[sample] = m_oversampleFilter.process(m_oversampleBuffer[sample]);
			m_oversampleBuffer[sample] = m_oversampleFilter.process(in);
		}
	};
	inline void downsample(float* outputBuffer)
	{
		for (int sample = 0; sample < m_oversampleSamples; sample++)
		{
			m_oversampleBuffer[sample] = m_downsampleFilter.process(m_oversampleBuffer[sample]);
		}

		int sampleOversample = 0;
		for (int sample = 0; sample < m_samples; sample++)
		{
			outputBuffer[sample] = m_oversamplingRatio * m_oversampleBuffer[sampleOversample];
			sampleOversample += m_oversamplingRatio;
		}
	};
	inline void release()
	{
		clearBuffer();
	}
	inline float* getOversampeBuffer()
	{
		return m_oversampleBuffer;
	}

private:
	inline void clearBuffer()
	{
		delete[] m_oversampleBuffer;
		m_oversampleBuffer = nullptr;
	};

	EighthOrderLowPassFilter m_oversampleFilter;
	EighthOrderLowPassFilter m_downsampleFilter;
	float* m_oversampleBuffer = nullptr	;
	int m_oversamplingRatio = 2;
	int m_oversampleSampleRate = 48000;
	int m_samples = 0;
	int m_oversampleSamples = 0;
};