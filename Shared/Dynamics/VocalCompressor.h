#pragma once

#include "../../../zazzVSTPlugins/Shared/Dynamics/Compressors.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/SideChainCompressor.h"

class VocalCompressor
{
public:
	VocalCompressor() = default;
	~VocalCompressor() = default;

	inline void init(int sampleRate)
	{
		m_leveler.init(sampleRate);
		m_compressor.init(sampleRate);
		m_tubeEmulation.init(sampleRate);
		set();
	};
	inline void set()
	{
		// Leveler compresor setup
		m_leveler.set(-17.0f, 1.5f, 0.0f, 2000.0f, 3000.0f, 660.0f);

		// Compressor setup
		m_compressor.set(-23.0f, 50.0f, 60.0f, 3.0f, 75.0f);

		// Set tube emilation
		m_tubeEmulation.set(-0.5f);

		m_gainCompensation = juce::Decibels::decibelsToGain(14.0f);
	};
	inline float process(float in)
	{
		return m_gainCompensation * m_compressor.processSoftKneeLinPeak(m_tubeEmulation.process(m_leveler.processHardKnee(in)));
	}

protected:
	SideChainCompressor m_leveler;
	Compressor m_compressor;
	TubeEmulation m_tubeEmulation;
	float m_gainCompensation = 1.0f;
};