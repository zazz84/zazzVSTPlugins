#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Random.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Delays/AllPassFilter.h"

struct CircularCombFilterAdvancedParams
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

	static const int COMB_FILTER_MAX_TIME_MS = 300;
	static const int ALL_PASS_MAX_TIME_MS = 150;
	static constexpr float m_dampingFrequencyMin = 220.0f;

	inline void init(const int complexity, const int channel, const int sampleRate)
	{	
		m_maxComplexity = complexity;
		m_channel = channel;
		m_sampleRate = sampleRate;

		m_lowPassFilter = new BiquadFilter[complexity];
		m_allPassFilter = new AllPassFilter[complexity];

		const int combFilterSize = (int)(4.0f * COMB_FILTER_MAX_TIME_MS * 0.001f * sampleRate);
		const int allPassFilterSize = (int)(4.0f * ALL_PASS_MAX_TIME_MS * 0.001f * sampleRate);

		m_buffer = new CircularBuffer[complexity];
		m_feedback = new float[complexity];
		memset(m_feedback, 0.0f, complexity * sizeof(float));

		for (int i = 0; i < complexity; i++)
		{
			m_buffer[i].init(combFilterSize);
			
			m_lowPassFilter[i].init(sampleRate);

			m_allPassFilter[i].init(allPassFilterSize);
			m_allPassFilter[i].set(0.2f, allPassFilterSize);
		}
	};
	inline void set(CircularCombFilterAdvancedParams params)
	{
		int* combFilterDelaySamples = new int[m_maxComplexity];
		memset(combFilterDelaySamples, 0, m_maxComplexity * sizeof(int));
		int* allPassDelaySamples = new int[m_maxComplexity];
		memset(allPassDelaySamples, 0, m_maxComplexity * sizeof(int));
		float* combFilterFeedback = new float[m_maxComplexity];
		memset(combFilterFeedback, 0.0f, m_maxComplexity * sizeof(float));
		float* allPassFeedback = new float[m_maxComplexity];
		memset(combFilterFeedback, 0.0f, m_maxComplexity * sizeof(float));
		float* dampingFrequency = new float[m_maxComplexity];
		memset(dampingFrequency, 0.0f, m_maxComplexity * sizeof(float));

		const auto combFilterWidth = (m_channel == 0) ? 1.0f - params.width * 0.1f : 1.0f;
		const auto allPassWidth = (m_channel == 1) ? 1.0f - params.width * 0.1f : 1.0f;
		const float frequencyMax = fminf(20000.0f, 0.48f * m_sampleRate);

		m_random.set((long)params.allPassSeed);

		for (int i = 0; i < m_maxComplexity; i++)
		{
			const float sing = (i % 2 == 0) ? -1.0f : 1.0f;
			combFilterFeedback[i] = sing * params.combFilterResonance;
			allPassFeedback[i] = params.allPassResonance * 0.9f + 0.2 * m_random.process();
			dampingFrequency[i] = fminf(frequencyMax, m_dampingFrequencyMin + (1.0f - params.damping) * (frequencyMax - m_dampingFrequencyMin) * 0.9f + 0.2 * m_random.process());
		}

		m_random.set(params.combFilterSeed);
		const float combFilterTimeFactor = COMB_FILTER_MAX_TIME_MS * 0.001f * m_sampleRate;
		int combFilterSum = 0;

		for (int i = 0; i < m_maxComplexity; i++)
		{
			const float rnd = params.timeMin + m_random.process() * (1.0f - params.timeMin);
			const int samples = (int)(combFilterWidth * params.combFilterTime * rnd * combFilterTimeFactor);
			combFilterDelaySamples[i] = samples;
			combFilterSum += samples;
		}

		m_random.set(params.allPassSeed);
		const float allPassTimeFactor = ALL_PASS_MAX_TIME_MS * 0.001f * m_sampleRate;
		int allPassSum = 0;

		for (int i = 0; i < m_maxComplexity; i++)
		{
			const float rnd = params.timeMin + m_random.process() * (1.0f - params.timeMin);
			const int samples = (int)(allPassWidth * params.allPassTime * rnd * allPassTimeFactor);
			allPassDelaySamples[i] = samples;
			allPassSum += samples;
		}

		//Normalize
		const float combFilterNormalizeFactor = combFilterSum / (m_maxComplexity * params.combFilterTime * combFilterTimeFactor);
		const float allPassNormalizeFactor = allPassSum / (m_maxComplexity * params.allPassTime * allPassTimeFactor);

		for (int i = 0; i < m_maxComplexity; i++)
		{
			combFilterDelaySamples[i] = combFilterDelaySamples[i] * combFilterNormalizeFactor;
			allPassDelaySamples[i] = allPassDelaySamples[i] * allPassNormalizeFactor;
		}

		// Set params
		m_volumeCompensation = 0.4f - (params.complexity * 0.004f);
		m_complexity = params.complexity;
		
		setLowPassFilter(dampingFrequency);
		setAllPassFilter(allPassFeedback, allPassDelaySamples);
		setCombFilter(combFilterFeedback, combFilterDelaySamples);

		// Delete
		delete[] combFilterDelaySamples;
		delete[] allPassDelaySamples;
		delete[] combFilterFeedback;
		delete[] allPassFeedback;
		delete[] dampingFrequency;
	};
	void setLowPassFilter(const float* dampingFrequency)
	{
		for (int i = 0; i < m_complexity; i++)
		{
			m_lowPassFilter[i].setLowPass(dampingFrequency[i], 0.4f);
		}
	};
	void setAllPassFilter(const float* feedback, const int* size)
	{
		for (int i = 0; i < m_complexity; i++)
		{
			m_allPassFilter[i].set(feedback[i], size[i]);
		}
	};
	void setCombFilter(const float* feedback, const int* size)
	{
		for (int i = 0; i < m_complexity; i++)
		{
			m_feedback[i] = feedback[i];
			m_buffer[i].set(size[i]);
		}
	};	
	inline float process(float in)
	{
		float out = 0.0;

		for (int i = 0; i < m_complexity; i++)
		{
			const auto bufferOut = m_lowPassFilter[i].processDF1(m_allPassFilter[i].process(m_buffer[i].read()));
			out += bufferOut;

			int writteIdx = i + 1;
			if (writteIdx >= m_complexity)
				writteIdx = 0;

			m_buffer[writteIdx].write((1.0f - m_buffer[writteIdx].getSize() * 0.0001f) * in + m_feedback[writteIdx] * bufferOut);
		}

		return m_volumeCompensation * out;
	};
	inline void release()
	{

	}

private:
	bool paramsChanged(CircularCombFilterAdvancedParams params)
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

	CircularBuffer* m_buffer;
	BiquadFilter* m_lowPassFilter;
	AllPassFilter* m_allPassFilter;
	float* m_feedback;
	CircularCombFilterAdvancedParams m_paramsLast;
	LinearCongruentialRandom01 m_random;
	float m_volumeCompensation = 1.0f;
	int m_channel = 0;
	int m_complexity = 0;
	int m_maxComplexity = 0;
	int m_sampleRate = 48000;
};