#pragma once

#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

class TransientShaper
{
public:
	TransientShaper() {};
	inline void init(const int sampleRate)
	{
		m_envelopeFollowerSlow.init(sampleRate);
		m_envelopeFollowerFast.init(sampleRate);

		m_envelopeFollowerSlow.set(5.0f, 150.0f);
		m_envelopeFollowerFast.set(0.3f, 30.0f);
	}
	inline void set(float attackGain, float sustainGain)
	{
		m_attackGain = attackGain;
		m_sustainGain = sustainGain;
	}
	inline void process(float& in)
	{
		const float envelopeIn = std::fabsf(in);
		// Add 1e-6 to avoid division by 0. Envelope output is alway > 0
		const float envelopeSlow = 1e-6 + m_envelopeFollowerSlow.process(envelopeIn);
		const float envelopeFast = m_envelopeFollowerFast.process(envelopeIn);

		float difference = envelopeFast / envelopeSlow;

		float gain = 1.0f;
		if (difference > 1.0f)
		{
			if (m_attackGain > 0.0f)
			{
				gain = 1.0f + (difference - 1.0f) * m_attackGain;
			}
			else
			{
				gain = 1.0f + (1.0f - 1.0f / difference) * m_attackGain;
			}
		}
		else
		{
			if (m_sustainGain > 0.0f)
			{
				gain = 1.0f + (1.0f / difference - 1.0f) * m_sustainGain;
			}
			else
			{
				gain = 1.0f + (1.0f - difference) * m_sustainGain;
			}
		}

		in = gain * in;
	}

private:
	BranchingSmoothEnvelopeFollower m_envelopeFollowerSlow;
	BranchingSmoothEnvelopeFollower m_envelopeFollowerFast;
	float m_attackGain = 0.0f;
	float m_sustainGain = 0.0f;
};