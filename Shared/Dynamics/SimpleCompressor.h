#pragma once

#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

//==============================================================================
class SimpleCompressor
{
public:
	SimpleCompressor() {};

	inline void init(int sampleRate)
	{
		m_envelopeFollower.init(sampleRate);
	}
	inline void set(float attackTimeMS, float releaseTimeMS)
	{
		m_envelopeFollower.set(attackTimeMS, releaseTimeMS);
	}
	inline float process(float in)
	{
		const float smooth = m_envelopeFollower.process(std::fabsf(in));
		return GetCompressionMultiplier(smooth) * in;
	}

protected:
	inline float GetCompressionMultiplier(float in) // in has to be > 0
	{
		constexpr float compressionMultiplier[11] = { 1.000f, 1.000f, 0.638f, 0.509f, 0.407f, 0.350f, 0.313f, 0.279f, 0.259f, 0.235f, 0.192f };
		constexpr float minCompressionMultiplier = compressionMultiplier[10];
		constexpr float threshold = 0.1f;

		if (in < threshold)
		{
			return 1.0f;
		}

		if (in > 1.0f)
		{
			return minCompressionMultiplier;
		}

		const float index = 10.0f * in;

		const int leftIndex = (int)index;
		const float leftValue = compressionMultiplier[leftIndex];
		const float rightValue = compressionMultiplier[leftIndex + 1];

		return leftValue + (index - (float)leftIndex) * (rightValue - leftValue);
	}

	BranchingSmoothEnvelopeFollower m_envelopeFollower;
};