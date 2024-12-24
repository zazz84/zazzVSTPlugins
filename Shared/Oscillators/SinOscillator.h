#pragma once

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
		m_step = (2.0f * 3.141592f) * frequency / (float)m_sampleRate;
	}
	inline float process()
	{
		m_phase += m_step;
		if (m_phase >= 6.283185f)
		{
			m_phase -= 6.283185f;
		}

		return 2.0f * sinf(m_phase) - 1.0f;
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