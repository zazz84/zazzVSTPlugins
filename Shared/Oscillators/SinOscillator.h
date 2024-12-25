#pragma once

#define M_PI 3.14159265358979f

class SinOscillator
{
public:
	SinOscillator() {};

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
	}
	inline void set(float frequency)
	{
		m_step =  (2.0f * M_PI) * frequency / (float)m_sampleRate;
	}
	inline float process()
	{
		m_phase += m_step;
		
		if (m_phase >= (2.0f * M_PI))
		{
			m_phase -= (2.0f * M_PI);
		}

		return std::sinf(m_phase);
	}
	inline void release()
	{
		m_sampleRate = 48000;
		m_step = 0.0f;
		m_phase = 0.0f;
	};
	
private:
	int m_sampleRate = 48000;
	float m_step = 0.0f;
	float m_phase = 0.0f;
};