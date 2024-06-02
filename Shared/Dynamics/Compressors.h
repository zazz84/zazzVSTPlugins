#pragma once

#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/WaveShapers.h"
#include <JuceHeader.h>

//==============================================================================
class Compressor
{
public:
	Compressor();

	void init(int sampleRate)
	{ 
		m_envelopeFollower.init(sampleRate);
	};
	void set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS);
	float process(float in);
	float getGainReduction(float in);

protected:
	EnvelopeFollower m_envelopeFollower;
	float m_thresholddB = 0.0f;
	float m_ratio = 2.0f;
	float m_kneeWidth = 0.0f;
	float m_attackTime = 0.0f;
	float m_releaseTime = 0.0f;

	float m_R_Inv_minus_One = 0.0f;
	float m_T_minus_WHalf = 0.0f;
	float m_T_plus_WHalf = 0.0f;
	float m_minus_T_plus_WHalf = 0.0f;
	float m_W2_inv = 0.0f;
};

//==============================================================================
class SideChainCompressor
{
public:
	SideChainCompressor();

	void init(int sampleRate)
	{
		m_envelopeFollower.init(sampleRate);
		m_lowPassFilter.init(sampleRate);
		m_highPassFilter.init(sampleRate);
	};
	void set(float thresholddB, float ratio, float attackTimeMS, float releaseTimeMS, float lowPassFrequency, float highPassFrequency);
	float process(float in);

protected:
	EnvelopeFollower m_envelopeFollower;
	BiquadFilter m_lowPassFilter;
	BiquadFilter m_highPassFilter;
	float m_thresholddB = 0.0f;
	float m_ratio = 2.0f;
	float m_R_Inv_minus_One = 0.0f;
	float m_attackTime = 0.0f;
	float m_releaseTime = 0.0f;
};

//==============================================================================
class VocalCompressor
{
public:
	VocalCompressor();

	void init(int sampleRate)
	{
		m_leveler.init(sampleRate);
		m_compressor.init(sampleRate);
		m_tubeEmulation.init(sampleRate);
		set();
	};
	void set()
	{
		// Leveler compresor setup
		m_leveler.set(-17.0f, 1.5f, 0.0f, 2000.0f, 3000.0f, 660.0f);

		// Compressor setup
		m_compressor.set(-23.0f, 50.0f, 60.0f, 3.0f, 75.0f);

		// Set tube emilation
		m_tubeEmulation.setDrive(-0.5f);

		m_gainCompensation = juce::Decibels::decibelsToGain(14.0f);
	};
	float process(float in);

protected:
	SideChainCompressor m_leveler;
	Compressor m_compressor;
	TubeEmulation m_tubeEmulation;
	float m_gainCompensation = 1.0f;
};