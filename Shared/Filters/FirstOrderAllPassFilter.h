#pragma once

#include <cmath>

class FirstOrderAllPassFilter
{
public:
	FirstOrderAllPassFilter() {};

	inline void init(const int sampleRate)
	{
		m_SampleRate = sampleRate;
	};
	inline void set(const float frequency)
	{
		const float tmp = tanf(3.141592f * frequency / m_SampleRate);
		m_a1 = (tmp - 1.0f) / (tmp + 1.0f);
	}
	inline float process(const float in)
	{
		const float tmp = m_a1 * in + m_d;
		m_d = in - m_a1 * tmp;
		return tmp;
}	;

protected:
	int m_SampleRate;
	float m_a1 = -1.0f; // all pass filter coeficient
	float m_d = 0.0f;   // history d = x[n-1] - a1y[n-1]
};