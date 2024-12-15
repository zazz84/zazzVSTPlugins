#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

class Limiter2
{
public:
	Limiter2() {};

	inline void init(const int sampleRate, const int size)
	{
		m_sampleRate = sampleRate;
		m_buffer.init(size);
		m_envelopeFollower.init(sampleRate);
	}
	inline void set(const float attackMS, const float releaseMS, const float threshold)
	{
		m_attackSize = (int)(attackMS * 0.001f * (float)m_sampleRate);
		m_thresholdMultiplier = 1.0f / threshold;
		m_envelopeFollower.set(attackMS, releaseMS);
	}
	inline float process(float in)
	{
		// Handle buffer
		const float inDelayed = m_buffer.readDelay(m_attackSize);
		m_buffer.write(in);

		// Get envelope
		const float inAbs = std::fabsf(in);
		const float aboveThresholdNormalized = m_thresholdMultiplier * inAbs;
		const float envelope = std::max(1.0f, m_envelopeFollower.process(aboveThresholdNormalized));

		// apply attenuation for output
		return inDelayed / envelope;
	};
	inline void release()
	{
		m_buffer.release();

		m_thresholdMultiplier = 1.0f;
		m_sampleRate = 48000;
		m_attackSize = 0;
	}

private:
	CircularBuffer m_buffer;
	EnvelopeFollower m_envelopeFollower;

	float m_thresholdMultiplier = 1.0f;
	int m_sampleRate = 48000;
	int m_attackSize = 0;
};