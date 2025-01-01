
#pragma once

#define M_PI 3.14159265f

//==============================================================================
class OnePoleFilter
{
public:
	OnePoleFilter() = default;
	~OnePoleFilter() = default;

	inline void release()
	{
		m_samplePeriod = 0.00002f;
		m_sampleLast = 0.0f;
		m_a0 = 1.0f;
	}

protected:
	float m_samplePeriod = 0.00002f;
	float m_sampleLast = 0.0f;
	float m_a0 = 1.0f;
};

//==============================================================================
class OnePoleLowPassFilter : public OnePoleFilter
{
public:
	OnePoleLowPassFilter() = default;
	~OnePoleLowPassFilter() = default;

	inline void init(const int sampleRate)
	{
		m_samplePeriod = 1.0f / static_cast<float>(sampleRate);
	};
	inline void set(float frequency)
	{
		m_a0 = frequency * M_PI * m_samplePeriod;
	};
	inline float process(float sample)
	{
		return m_sampleLast = m_a0 * (sample - m_sampleLast) + m_sampleLast;
	};
};