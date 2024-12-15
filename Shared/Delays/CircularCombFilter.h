#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

//==============================================================================
class CircularCombFilter
{
public:
	CircularCombFilter() {};

	inline void init(int complexity, int* size)
	{
		m_buffer = new CircularBuffer[complexity];
		m_feedback = new float[complexity];
		m_complexity = complexity;

		for (int i = 0; i < m_complexity; i++)
		{
			m_buffer[i].init(size[i]);
		}
	};
	void setSize(const float* feedback, const int* size, const int complexity)
	{
		m_complexity = complexity
		
		for (int i = 0; i < complexity; i++)
		{
			m_feedback[i] = feedback[i];
			m_buffer[i].setSize(size[i]);
		}
	};
	inline float process(float in)
	{
		float out = 0.0;

		for (int i = 0; i < m_complexity; i++)
		{
			const float bufferOut = m_buffer[i].read();
			out += bufferOut;

			int writteIdx = i + 1;
			if (writteIdx >= m_complexity)
				writteIdx = 0;

			m_buffer[writteIdx].writeSample(in + m_feedback[writteIdx] * bufferOut);
		}

		return out;
	}

protected:
	CircularBuffer* m_buffer;
	float* m_feedback;
	int m_complexity = 0;
};

//==============================================================================
struct CircularCombFilterParams
{
	float combFilterTime = 0.0f;
	float combFilterResonance = 0.0f;
	float allPassTime = 0.0f;
	float allPassResonance = 0.0f;
	float width = 0.0f;
	float damping = 0.0f;
	float combFilterSeed = 0.0f;
	float allPassSeed = 0.0f;
	float timeMin = 0.0f;
	float complexity = 0.0f;
};

class CircularCombFilterAdvanced
{
public:
	CircularCombFilterAdvanced() {};

	static const int MAX_COMPLEXITY = 64;
	static const int COMB_FILTER_MAX_TIME_MS = 300;
	static const int ALL_PASS_MAX_TIME_MS = 150;
	static constexpr float m_dampingFrequencyMin = 220.0f;

	void set(CircularCombFilterParams params);
	void setDampingFrequency(const float* dampingFrequency);
	void setAllPassSize(const int* size);
	void setAllPassFeedback(const float* feedback);

	void init(int channel = 0, int sampleRate = 48000);
	float process(float in);

private:
	bool paramsChanged(CircularCombFilterParams params)
	{
		if (params.combFilterTime != m_paramsLast.combFilterTime ||
			params.combFilterResonance != m_paramsLast.combFilterResonance ||
			params.allPassTime != m_paramsLast.allPassTime ||
			params.allPassResonance != m_paramsLast.allPassResonance ||
			params.width != m_paramsLast.width ||
			params.damping != m_paramsLast.damping ||
			params.combFilterSeed != m_paramsLast.combFilterSeed ||
			params.allPassSeed != m_paramsLast.allPassSeed ||
			params.timeMin != m_paramsLast.timeMin ||
			params.complexity != m_paramsLast.complexity)
		{
			m_paramsLast = params;

			return true;
		}
		else
		{
			return false;
		}
	}

	CircularCombFilterParams m_paramsLast;
	BiquadFilter* m_filter;
	Allpass* m_allPass;
	LinearCongruentialNoiseGenerator m_noiseGenerator;
	int m_channel = 0;
	int m_sampleRate = 48000;
	float m_volumeCompensation = 1.0f;
};

//==============================================================================
class NEDCombFilter
{
public:
	NEDCombFilter() {};

	inline void init(int size)
	{
		m_buffer1.init(size);
		m_buffer2.init(size);
	};
	inline void set(float feedback, int size)
	{
		m_feedback = feedback;
		m_buffer1.setSize(size);
		m_buffer2.setSize(size);
	};
	inline float process(float sample)
	{
		float tmp1 = m_buffer1.read();
		m_buffer1.writeSample(sample + tmp1 * m_feedback);
		float tmp2 = m_buffer2.read();
		m_buffer2.writeSample(tmp1 + tmp2 * m_feedback);

		return (1.0f + m_feedback) * tmp1 + tmp2;
	}

private:
	CircularBuffer m_buffer1, m_buffer2;
	float m_feedback = 0.0f;
};




-----------------------------
TODO:



void CircularCombFilterAdvanced::init(int channel, int sampleRate)
{
	int delayCombFilterSamples[MAX_COMPLEXITY];
	int delayAllPassSamples[MAX_COMPLEXITY];

	for (int i = 0; i < MAX_COMPLEXITY; i++)
	{
		delayCombFilterSamples[i] = (int)(4.0f * COMB_FILTER_MAX_TIME_MS * 0.001f * sampleRate);
		delayAllPassSamples[i] = (int)(4.0f * ALL_PASS_MAX_TIME_MS * 0.001f * sampleRate);
	}

	__super::init(MAX_COMPLEXITY, delayCombFilterSamples);

	m_filter = new BiquadFilter[MAX_COMPLEXITY];
	m_allPass = new Allpass[MAX_COMPLEXITY];

	for (int i = 0; i < m_complexity; i++)
	{
		m_filter[i].init(sampleRate);
		m_allPass[i].init(delayAllPassSamples[i]);
		m_allPass[i].setFeedback(0.2f);
	}

	m_channel = channel;
	m_sampleRate = sampleRate;
}

void CircularCombFilterAdvanced::set(CircularCombFilterParams params)
{
	//if (!paramsChanged(params))
	//	return;

	int combFilterDelaySamples[MAX_COMPLEXITY];
	int allPassDelaySamples[MAX_COMPLEXITY];
	float combFilterFeedback[MAX_COMPLEXITY];
	float allPassFeedback[MAX_COMPLEXITY];
	float dampingFrequency[MAX_COMPLEXITY];

	const auto combFilterWidth = (m_channel == 0) ? 1.0f - params.width * 0.1f : 1.0f;
	const auto allPassWidth = (m_channel == 1) ? 1.0f - params.width * 0.1f : 1.0f;
	const float frequencyMax = fminf(20000.0f, 0.48f * m_sampleRate);

	m_noiseGenerator.setSeed((long)params.allPassSeed);

	for (int i = 0; i < MAX_COMPLEXITY; i++)
	{
		const float sing = (i % 2 == 0) ? -1.0f : 1.0f;
		combFilterFeedback[i] = sing * params.combFilterResonance;
		allPassFeedback[i] = params.allPassResonance * 0.9f + 0.2 * m_noiseGenerator.process();
		dampingFrequency[i] = fminf(frequencyMax, m_dampingFrequencyMin + (1.0f - params.damping) * (frequencyMax - m_dampingFrequencyMin) * 0.9f + 0.2 * m_noiseGenerator.process());
	}

	m_noiseGenerator.setSeed(params.combFilterSeed);
	const float combFilterTimeFactor = COMB_FILTER_MAX_TIME_MS * 0.001f * m_sampleRate;
	int combFilterSum = 0;

	for (int i = 0; i < MAX_COMPLEXITY; i++)
	{
		const float rnd = params.timeMin + m_noiseGenerator.process() * (1.0f - params.timeMin);
		const int samples = (int)(combFilterWidth * params.combFilterTime * rnd * combFilterTimeFactor);
		combFilterDelaySamples[i] = samples;
		combFilterSum += samples;
	}

	m_noiseGenerator.setSeed(params.allPassSeed);
	const float allPassTimeFactor = ALL_PASS_MAX_TIME_MS * 0.001f * m_sampleRate;
	int allPassSum = 0;

	for (int i = 0; i < MAX_COMPLEXITY; i++)
	{
		const float rnd = params.timeMin + m_noiseGenerator.process() * (1.0f - params.timeMin);
		const int samples = (int)(allPassWidth * params.allPassTime * rnd * allPassTimeFactor);
		allPassDelaySamples[i] = samples;
		allPassSum += samples;
	}

	//Normalize
	const float combFilterNormalizeFactor = combFilterSum / (MAX_COMPLEXITY * params.combFilterTime * combFilterTimeFactor);
	const float allPassNormalizeFactor = allPassSum / (MAX_COMPLEXITY * params.allPassTime * allPassTimeFactor);

	for (int i = 0; i < MAX_COMPLEXITY; i++)
	{
		combFilterDelaySamples[i] = combFilterDelaySamples[i] * combFilterNormalizeFactor;
		allPassDelaySamples[i] = allPassDelaySamples[i] * allPassNormalizeFactor;
	}

	// Set params
	m_volumeCompensation = 0.4f - (params.complexity * 0.004f);
	setComplexity(params.complexity);
	setSize(combFilterDelaySamples);
	setFeedback(combFilterFeedback);
	setAllPassSize(allPassDelaySamples);
	setAllPassFeedback(allPassFeedback);
	setDampingFrequency(dampingFrequency);
}

void CircularCombFilterAdvanced::setDampingFrequency(const float* dampingFrequency)
{
	for (int i = 0; i < m_complexity; i++)
	{
		m_filter[i].setLowPass(dampingFrequency[i], 0.4f);
	}
}

void CircularCombFilterAdvanced::setAllPassSize(const int* size)
{
	for (int i = 0; i < m_complexity; i++)
	{
		m_allPass[i].setSize(size[i]);
	}
}

void CircularCombFilterAdvanced::setAllPassFeedback(const float* feedback)
{
	for (int i = 0; i < m_complexity; i++)
	{
		m_allPass[i].setFeedback(feedback[i]);
	}
}

float CircularCombFilterAdvanced::process(float in)
{
	float out = 0.0;

	for (int i = 0; i < m_complexity; i++)
	{
		const auto bufferOut = m_filter[i].processDF1(m_allPass[i].process(m_buffer[i].read()));
		//const auto bufferOut = m_allPass[i].process(m_buffer[i].read());
		out += bufferOut;

		int writteIdx = i + 1;
		if (writteIdx >= m_complexity)
			writteIdx = 0;

		m_buffer[writteIdx].write((1.0f - m_buffer[writteIdx].getSize() * 0.0001f) * in + m_feedback[writteIdx] * bufferOut);
	}

	return m_volumeCompensation * out;
}