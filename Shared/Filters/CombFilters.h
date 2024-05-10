#pragma once

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/NoiseGenerator.h"

class CombFilter
{
public:
	CombFilter();

	inline void init(int size) { m_buffer.init(size); };
	inline void clear() { m_buffer.clear(); };
	inline void setSize(int size) { m_buffer.setSize(size); };
	inline float getSize() { return m_buffer.getSize(); };
	inline void setFeedback(float feedback) { m_feedback = feedback; }
	float process(float in);

protected:
	CircularBuffer m_buffer;
	float m_feedback = 0.0f;
};

//==============================================================================
class Allpass : public CombFilter
{
public:
	Allpass();

	float process(float in);
};

//==============================================================================
class NestedCombFilter : public CombFilter
{
public:
	NestedCombFilter();

	void init(int combFilterSize, int allPassSize, int sampleRate)
	{ 
		m_buffer.init(combFilterSize);
		m_allPass.init(allPassSize);
		m_filter.init(sampleRate);
	};
	void setSize(int combFilterSize, int allPassSize)
	{ 
		m_buffer.setSize(combFilterSize); 
		m_allPass.setSize(allPassSize);
	};
	void setFeedback(float combFilterFeedback, float allPassFeedback)
	{ 
		m_feedback = combFilterFeedback;
		m_allPass.setFeedback(allPassFeedback);
	};
	void setDamping(float damping)
	{
		m_filter.setHighShelf(2000.0f, 0.7f, damping * -12.0f);
	};

	float process(float in);

protected:
	Allpass m_allPass;
	BiquadFilter m_filter;
};

//==============================================================================
struct CircularCombFilterParams
{
	float combFilterTime;
	float combFilterResonance;
	float allPassTime;
	float allPassResonance;
	float width;
	float damping;
	float combFilterSeed;
	float allPassSeed;
	float timeMin;
	float complexity;
};

class CircularCombFilter
{
public:
	CircularCombFilter();

	void init(int complexity, int* size);
	void setSize(const int* size);
	void setFeedback(const float* feedback);
	void setComplexity(int complexity) { m_complexity = complexity; };

	float process(float in);

protected:
	CircularBuffer* m_buffer;
	float* m_feedback;
	int m_complexity = 0;
};

//==============================================================================
class CircularCombFilterAdvanced : public CircularCombFilter
{
public:
	CircularCombFilterAdvanced();

	static const int MAX_COMPLEXITY = 64;
	static const int COMB_FILTER_MAX_TIME_MS = 300;
	static const int ALL_PASS_MAX_TIME_MS = 150;
	static const float m_dampingFrequencyMin;

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