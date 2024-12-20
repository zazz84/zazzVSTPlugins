#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/MovingAverage.h"

class TrebleBooster
{
public:
	TrebleBooster() {};

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
		
		const int size = (int)(sampleRate / 48000.0f);
		m_movingAverage.init(size);

		m_movingAverage.set(0.1f * (float)size);
	};
	// TODO: Does not work
	// Set to fixed hight frequency boost now
	// Frequency and gain control does not work
	inline void set(const float frequencyNormalized, float gainNormalized)	
	{
		m_dry = 1.0f - gainNormalized;
		m_wet = gainNormalized;

		const float size = 0.1f * frequencyNormalized * m_sampleRate / 48000.0f;
		m_movingAverage.set(size);
	};
	inline float process(const float in)
	{	
		const float diff = in - m_last;
		const float out = m_movingAverage.process(diff);
		m_last = in;

		return m_dry * in + m_wet * out;
	}

protected:
	MovingAverage m_movingAverage;
	float m_last = 0.0f;
	float m_dry = 0.25f;
	float m_wet = 0.75f;
	int m_sampleRate = 48000;
};