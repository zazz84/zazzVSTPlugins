#pragma once

class TrebleBooster
{
public:
	TrebleBooster() {};

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
	};
	inline void set()	
	{
	};
	inline float process(const float in)
	{	
		const float out = m_x2 - m_x1;
		
		m_x2 = m_x1;
		m_x1 = in;

		return m_dry * in + m_wet * out;
	}

protected:
	float m_x1 = 0.0f;
	float m_x2 = 0.0f;

	float m_dry = 0.1f;
	float m_wet = 0.9f;

	int m_sampleRate = 48000;
};