#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Random.h"
#include "../../../zazzVSTPlugins/Shared/Delays/AllPassFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

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

struct Comb
{
	CircularBuffer m_buffer;
	OnePoleLowPassFilter m_lowPassFilter;
	AllPassFilter m_allPassFilter;
	float m_gainAllPass = 0.0f;
	float m_gain = 0.0f;				// Stores gain of next comb
	float m_feedback = 0.0f;			// Stores gain of next comb
};

class CircularCombFilterAdvanced
{
public:
	CircularCombFilterAdvanced() {};

	static const int COMB_FILTER_MAX_TIME_MS = 270;			// Time selected so samples size is slightly lower than power of 2
	static const int ALL_PASS_MAX_TIME_MS = 67;				// Time selected so samples size is slightly lower than power of 2
	static constexpr float m_dampingFrequencyMin = 440.0f;

	inline void init(const int complexity, const int channel, const int sampleRate)
	{	
		m_channel = channel;
		m_sampleRate = sampleRate;

		const int combFilterSize = (int)(COMB_FILTER_MAX_TIME_MS * 0.001f * sampleRate);
		const int allPassFilterSize = (int)(ALL_PASS_MAX_TIME_MS * 0.001f * sampleRate);

		m_combFilter = new Comb[complexity];

		for (int i = 0; i < complexity; i++)
		{		
			auto& comb = m_combFilter[i];
			comb.m_buffer.init(combFilterSize);
			comb.m_lowPassFilter.init(sampleRate);
			comb.m_allPassFilter.init(allPassFilterSize);
		}

		m_lowPassFilter.init(sampleRate);
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
	
			m_combFilter[i].m_buffer.set((int)(rndClamp * combFilterSizeMax));

			// Longer buffers have quieter dry input gains
			const float sing = (i % 2 == 0) ? 1.0f : -1.0f;
			const int nextCombIdx = (i + 1) % m_complexity;
			m_combFilter[nextCombIdx].m_gain = sing * (1.0f - 0.8f * rndNormalized);
		}

		// Get random numbers for all pass filter
		m_random.set(params.allPassSeed);
		GenerateRandomNumbers();

		// Set all pass filter size
		const float allPassFilterSizeMax = combFilterWidth * params.combFilterTime * 0.001f * ALL_PASS_MAX_TIME_MS * m_sampleRate;
		rndMax = 1.0f / rndMax;

		for (int i = 0; i < m_complexity; i++)
		{
			auto& comb = m_combFilter[i];
			const float rnd = tmp[i];
			const float rndNormalized = tmp[i] * rndMax;
			const float rndClamp = params.timeMin + rndNormalized * (1.0f - params.timeMin);
			const int size = (int)(rndClamp * allPassFilterSizeMax);
			const float feedback = 0.6f * params.allPassResonance + 0.4f * rnd;

			comb.m_allPassFilter.set(feedback, size);

			// Longer buffers have quieter dry input gains
			const float sing = (i % 3 == 0) ? 1.0f : -1.0f;
			comb.m_gainAllPass = sing * (1.0f - 0.8f * rndNormalized);
			
			//m_gainAllPass[i] = 1.0f - size * 0.0001f; //Original calculation
		}

		// Set comb filter feedback
		for (int i = 0; i < m_complexity; i++)
		{
			const float sing = (i % 2 == 0) ? -1.0f : 1.0f;
			const int nextCombIdx = (i + 1) % m_complexity;
			m_combFilter[nextCombIdx].m_feedback = sing * params.combFilterResonance;
		}

		// Set damping frequency
		m_random.set((long)params.allPassSeed);
		const float dampingFrequencyBase = m_dampingFrequencyMin + (1.0f - params.damping) * (frequencyMax - m_dampingFrequencyMin);

		for (int i = 0; i < m_complexity; i++)
		{
			const float f = 0.7f * dampingFrequencyBase + 0.3f * m_random.process() * dampingFrequencyBase;
			m_combFilter[i].m_lowPassFilter.set(f);
		}

		m_lowPassFilter.set(dampingFrequencyBase);

		// Delete
		delete[] tmp;
	};
	inline float process(const float in)
	{
		const float inLowPass = m_lowPassFilter.process(in);
		float out = 0.0f;

		for (int i = 0; i < m_complexity; i++)
		{
			auto& comb = m_combFilter[i];
			const auto bufferOut = comb.m_lowPassFilter.process(comb.m_allPassFilter.process(comb.m_gainAllPass * comb.m_buffer.read()));
			out += bufferOut;

			const int writteIdx = (i + 1) % m_complexity;
			m_combFilter[writteIdx].m_buffer.write(comb.m_gain * inLowPass + comb.m_feedback * bufferOut);
		}

		return out;
	};
	inline void release()
	{
		delete[] m_combFilter;
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

	Comb* m_combFilter;
	OnePoleLowPassFilter m_lowPassFilter;
	CircularCombFilterAdvancedParams m_paramsLast;
	LinearCongruentialRandom01 m_random;
	int m_channel = 0;
	int m_complexity = 0;
	int m_sampleRate = 48000;
};