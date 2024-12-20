#pragma once

#include <cmath>

class PeakDetector
{
public:
	PeakDetector() {};

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
	};
	inline void set(const float releaseTimeMS)
	{
		m_decayCoefficient = std::expf(-1.0f / (0.001f * releaseTimeMS * (float)m_sampleRate));
	};
	inline float process(const float in)
	{
		// Compute the absolute value of the input
		const float inAbs = (in > 0.0f) ? in : -in;

		// Update the peak value
		if (inAbs > m_out)
			m_out = inAbs;						// Instant rise to the new peak value
		else
			m_out *= m_decayCoefficient;		// Exponential decay

		return m_out;
	};

private:
	int m_sampleRate = 48000;
	float m_decayCoefficient = 0.0f;
	float m_out = 0.0f;
};