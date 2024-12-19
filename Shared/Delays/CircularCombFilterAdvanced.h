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

	static const int COMB_FILTER_MAX_TIME_MS = 220;
	static const int ALL_PASS_MAX_TIME_MS = 70;
	static constexpr float m_dampingFrequencyMin = 440.0f;

	inline void init(const int complexity, const int channel, const int sampleRate)
	{	
		m_maxComplexity = complexity;
		m_channel = channel;
		m_sampleRate = sampleRate;

		m_lowPassFilter = new BiquadFilter[complexity];
		m_allPassFilter = new AllPassFilter[complexity];

		const int combFilterSize = (int)(COMB_FILTER_MAX_TIME_MS * 0.001f * sampleRate);
		const int allPassFilterSize = (int)(ALL_PASS_MAX_TIME_MS * 0.001f * sampleRate);

		m_buffer = new CircularBuffer[complexity];
		m_feedback = new float[complexity];
		memset(m_feedback, 0.0f, complexity * sizeof(float));
		m_gain = new float[complexity];
		memset(m_gain, 0.0f, complexity * sizeof(float));
		m_gainAllPass = new float[complexity];
		memset(m_gainAllPass, 0.0f, complexity * sizeof(float));

		for (int i = 0; i < complexity; i++)
		{
			m_buffer[i].init(combFilterSize);		
			m_lowPassFilter[i].init(sampleRate);
			m_allPassFilter[i].init(allPassFilterSize);
		}
	};
	inline void set(const CircularCombFilterAdvancedParams& params)
	{
		float* tmp = new float[params.complexity];
		memset(tmp, 0.0f, params.complexity * sizeof(float));

		// Local constants
		const float combFilterWidth = (m_channel == 0) ? 1.0f - params.width * 0.1f : 1.0f;
		const float allPassWidth = (m_channel == 1) ? 1.0f - params.width * 0.1f : 1.0f;
		// Condition for low sample rates
		const float frequencyMax = fminf(18000.0f, 0.5f * m_sampleRate);

		// Set params
		m_volumeCompensation = 0.4f - (params.complexity * 0.004f) + 0.7f * params.combFilterTime;
		m_complexity = params.complexity;

		// Generate random numbers
		float rndMax = 0.0f;

		auto GenerateRandomNumbers = [&]()
		{
			rndMax = 0.0f;

			for (int i = 0; i < m_complexity; i++)
			{
				const float rnd = m_random.process();
				tmp[i] = rnd;

				if (rnd > rndMax)
				{
					rndMax = rnd;
				}
			}
		};

		// Get random numbers for comb filter
		m_random.set(params.combFilterSeed);
		GenerateRandomNumbers();

		// Set comb filter size and input gain
		const float combFilterSizeMax = combFilterWidth * params.combFilterTime * 0.001f * COMB_FILTER_MAX_TIME_MS * m_sampleRate;
		rndMax = 1.0f / rndMax;

		for (int i = 0; i < m_complexity; i++)
		{
			const float rndNormalized = tmp[i] * rndMax;
			const float rndClamp = params.timeMin + rndNormalized * (1.0f - params.timeMin);
			m_buffer[i].set((int)(rndClamp * combFilterSizeMax));
			
			// Longer buffers have quieter dry input gains
			const float sing = (i % 2 == 0) ? 1.0f : -1.0f;
			m_gain[i] = sing * (1.0f - 0.8f * rndNormalized);
		}

		// Get random numbers for all pass filter
		m_random.set(params.allPassSeed);
		GenerateRandomNumbers();

		// Set all pass filter size
		const float allPassFilterSizeMax = combFilterWidth * params.combFilterTime * 0.001f * ALL_PASS_MAX_TIME_MS * m_sampleRate;
		rndMax = 1.0f / rndMax;

		for (int i = 0; i < m_complexity; i++)
		{
			const float rnd = tmp[i];
			const float rndNormalized = tmp[i] * rndMax;
			const float rndClamp = params.timeMin + rndNormalized * (1.0f - params.timeMin);
			const int size = (int)(rndClamp * allPassFilterSizeMax);
			const float feedback = 0.6f * params.allPassResonance + 0.4f * rnd;

			m_allPassFilter[i].set(feedback, size);

			// Longer buffers have quieter dry input gains
			const float sing = (i % 3 == 0) ? 1.0f : -1.0f;
			m_gainAllPass[i] = sing * (1.0f - 0.8f * rndNormalized);
			
			//m_gainAllPass[i] = 1.0f - size * 0.0001f; //Original calculation
		}

		// Set comb filter feedback
		for (int i = 0; i < m_complexity; i++)
		{
			const float sing = (i % 2 == 0) ? -1.0f : 1.0f;
			m_feedback[i] = sing * params.combFilterResonance;
		}

		// Set damping frequency
		m_random.set((long)params.allPassSeed);
		const float dampingFrequencyBase = m_dampingFrequencyMin + (1.0f - params.damping) * (frequencyMax - m_dampingFrequencyMin);

		for (int i = 0; i < m_complexity; i++)
		{
			const float f = 0.7f * dampingFrequencyBase + 0.3f * m_random.process() * dampingFrequencyBase;
			m_lowPassFilter[i].setLowPass(f, 0.5f);
		}

		// Delete
		delete[] tmp;
	};
	inline float process(const float in)
	{
		float out = 0.0f;

		for (int i = 0; i < m_complexity; i++)
		{
			const auto bufferOut = m_lowPassFilter[i].processDF1(m_allPassFilter[i].process(m_gainAllPass[i] * m_buffer[i].read()));
			out += bufferOut;

			const int writteIdx = (i + 1) % m_complexity;
			m_buffer[writteIdx].write(m_gain[writteIdx] * in + m_feedback[writteIdx] * bufferOut);
		}

		return m_volumeCompensation * out;
	};
	inline void release()
	{
		delete[] m_feedback;
		delete[] m_gain;
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
	float* m_gain;
	float* m_gainAllPass;
	CircularCombFilterAdvancedParams m_paramsLast;
	LinearCongruentialRandom01 m_random;
	float m_volumeCompensation = 1.0f;
	int m_channel = 0;
	int m_complexity = 0;
	int m_maxComplexity = 0;
	int m_sampleRate = 48000;
};