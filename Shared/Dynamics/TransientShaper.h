#pragma once

#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class TransientShaper
{
public:
	TransientShaper() = default;
	~TransientShaper() = default;

	inline void init(const int sampleRate)
	{
		m_envelopeFollowerSlow.init(sampleRate);
		m_envelopeFollowerFast.init(sampleRate);

		m_envelopeFollowerSlow.set(5.0f, 150.0f);
		m_envelopeFollowerFast.set(0.3f, 30.0f);
	}
	inline void set(float attackGain, float sustainGain, float attackTime, float clipGain)
	{
		m_attackGain = attackGain;
		m_sustainGain = sustainGain;
		m_clipGain = clipGain;

		m_envelopeFollowerSlow.set(attackTime, 150.0f);
	}
	inline void process(float& in)
	{
		const float envelopeIn = std::fabsf(in);
		// Add 1e-6 to avoid division by 0. Envelope output is alway > 0
		const float envelopeSlow = 1e-6f + m_envelopeFollowerSlow.process(envelopeIn);
		const float envelopeFast = m_envelopeFollowerFast.process(envelopeIn);

		// Clamp to ~ -12.0db, 12dB
		float difference = std::fminf(4.0f, std::fmaxf(0.25f, envelopeFast / envelopeSlow));

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

		const float clipThreshold = envelopeSlow * m_clipGain;

		in = std::fminf(clipThreshold, std::fmaxf(-clipThreshold, gain * in));
	}

private:
	BranchingEnvelopeFollower<float> m_envelopeFollowerSlow;
	BranchingEnvelopeFollower<float> m_envelopeFollowerFast;
	float m_attackGain = 0.0f;
	float m_sustainGain = 0.0f;
	float m_clipGain = 0.0f;
};

class TransientShaperAdvanced
{
public:
	TransientShaperAdvanced() = default;
	~TransientShaperAdvanced() = default;

	inline void init(const int sampleRate) noexcept
	{
		// Init
		m_envelopeFollowerSlow.init(sampleRate);
		m_envelopeFollowerFast.init(sampleRate);
		m_smoother.init(sampleRate);

		m_envelopeFollowerFast.set(0.3f, 30.0f);
	}
	inline void set(const float attackGain, const float sustainGain, const float attackLenght = 0.0f, const float releaseLength = 0.0f) noexcept
	{
		m_attackGain = 2.0f * attackGain;
		m_sustainGain = -2.0f * sustainGain;

		float attackSlow = 3.0f;
		
		// Handle attack
		if (attackGain > 0.0f)
		{
			const float smootherAttack = attackLenght;
			const float smootherRelease = 10.0f * attackLenght;
			const float smootherHold = 30.0f * attackLenght;
			
			m_smoother.set(smootherAttack, smootherRelease, smootherHold);
		}
		else
		{
			attackSlow = Math::remap(attackLenght, 0.0f, 1.0f, 5.0f, 30.0f);
			
			const float smootherAttack = 30.0f * attackLenght;
			m_smoother.set(smootherAttack, 0.001f, 0.0f);
		}

		// Slow Envelope
		const float releaseSlow = Math::remap(releaseLength, 0.0f, 1.0f, 100.0f, 400.0f);
		m_envelopeFollowerSlow.set(attackSlow, releaseSlow);
	}
	inline float process(const float in) noexcept
	{
		const float envelopeIn = Math::fabsf(in);
		
		// Process Attack
		const float envelopeSlow = m_envelopeFollowerSlow.process(envelopeIn);
		const float envelopeFast = m_envelopeFollowerFast.process(envelopeIn);

		const float envelopeSlowdB = Math::gainTodB(envelopeSlow);
		const float envelopeFastdB = Math::gainTodB(envelopeFast);

		float differencedB = envelopeFastdB - envelopeSlowdB;

		// Limit difference
		differencedB = 12.0f * std::atanf(0.083f * differencedB);

		// Evaluate state
		differencedB = differencedB > 0.0f ? m_attackGain * differencedB : m_sustainGain * differencedB;

		// Store max differencedB
		if (Math::fabsf(differencedB) > Math::fabsf(m_maxGaindB))
		{
			m_maxGaindB = differencedB;
		}
		
		// Smooth
		const float gain = m_smoother.process(Math::dBToGain(differencedB));

		return gain * in;
	}
	inline float getMaxGain()
	{
		float maxGaindB = m_maxGaindB;
		m_maxGaindB = 0.0f;

		return maxGaindB;
	}

private:
	BranchingEnvelopeFollower<float> m_envelopeFollowerSlow;
	BranchingEnvelopeFollower<float> m_envelopeFollowerFast;
	
	HoldEnvelopeFollower<float> m_smoother;

	float m_attackGain = 0.0f;
	float m_sustainGain = 0.0f;

	float m_maxGaindB = 0.0f;
};