#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/Compressors.h"

class SideChainCompressor
{
public:
	SideChainCompressor() = default;
	~SideChainCompressor() = default;

	inline void init(const int sampleRate)
	{
		m_envelopeFollower.init(sampleRate);
		m_lowPassFilter.init(sampleRate);
		m_highPassFilter.init(sampleRate);
	};
	inline void set(const float thresholddB, const float ratio, const float attackTimeMS, const float releaseTimeMS, const float lowPassFrequency, const float highPassFrequency)
	{
		m_params.set(thresholddB, ratio, 0.0f);
		m_envelopeFollower.set(attackTimeMS, releaseTimeMS);
	};
	inline float processHardKnee(const float in)
	{
		//Apply filters
		const float sideChainIn = m_lowPassFilter.processDF1(m_highPassFilter.processDF1(in));

		// Smooth
		const float smooth = m_envelopeFollower.process(sideChainIn);

		//Do nothing if below threshold
		if (smooth < m_params.m_threshold)
		{
			return in;
		}

		//Get gain reduction, positive values
		const float attenuatedB = (Math::gainTodB(smooth) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;

		// Apply gain reduction
		return in * Math::dBToGain(attenuatedB);
	};

protected:
	CompressorParams<float> m_params;
	BiquadFilter m_lowPassFilter;
	BiquadFilter m_highPassFilter;
	BranchingEnvelopeFollower<float> m_envelopeFollower;
};