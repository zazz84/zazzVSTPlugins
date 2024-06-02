#pragma once

#include "Compressors.h"
#include <JuceHeader.h>

//==============================================================================
Compressor::Compressor()
{
}

void Compressor::set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS)
{
	m_thresholddB = thresholddB;
	m_ratio = ratio;
	m_kneeWidth = kneeWidth;
	m_attackTime = attackTimeMS;
	m_releaseTime = releaseTimeMS;
	m_R_Inv_minus_One = (1.0f / ratio) - 1.0f;

	// Soft knee params
	const float W_Half = kneeWidth * 0.5f;
	m_T_minus_WHalf = thresholddB - W_Half;
	m_T_plus_WHalf = thresholddB + W_Half;
	m_minus_T_plus_WHalf = -1.0f * thresholddB + W_Half;
	m_W2_inv = 1.0f / (kneeWidth * 2.0f);

	m_envelopeFollower.setCoef(attackTimeMS, releaseTimeMS);
}

float Compressor::process(float in)
{
	// Smooth
	const float smooth = m_envelopeFollower.process(in);

	// Convert input from gain to dB
	const float smoothdB = juce::Decibels::gainToDecibels(smooth + 0.000001f);

	//Get gain reduction, positive values
	//const float attenuatedB = (smoothdB >= m_thresholddB) ? (smoothdB - m_thresholddB) * m_R_Inv_minus_One : 0.0f;
	const float attenuatedB = getGainReduction(smoothdB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

float Compressor::getGainReduction(float indB)
{
	if (m_kneeWidth < 0.001f)
	{
		return (indB >= m_thresholddB) ? (indB - m_thresholddB) * m_R_Inv_minus_One : 0.0f;
	}
	else
	{
		if (indB < m_T_minus_WHalf)
		{
			return 0.0f;
		}
		else if (indB > m_T_plus_WHalf)
		{
			return (indB - m_thresholddB) * m_R_Inv_minus_One;
		}
		else
		{
			const auto tmp = indB + m_minus_T_plus_WHalf;
			return m_R_Inv_minus_One * (tmp * tmp) * m_W2_inv;
		}
	}
}

//==============================================================================
SideChainCompressor::SideChainCompressor()
{
}

void SideChainCompressor::set(float thresholddB, float ratio, float attackTimeMS, float releaseTimeMS, float lowPassFrequency, float highPassFrequency)
{
	m_thresholddB = thresholddB;
	m_ratio = ratio;
	m_attackTime = attackTimeMS;
	m_releaseTime = releaseTimeMS;
	m_R_Inv_minus_One = (1.0f / ratio) - 1.0f;
	m_lowPassFilter.setLowPass(lowPassFrequency, 1.0f);
	m_highPassFilter.setHighPass(highPassFrequency, 1.0f);

	m_envelopeFollower.setCoef(attackTimeMS, releaseTimeMS);
}

float SideChainCompressor::process(float in)
{
	//Apply filters
	const float sideChainIn = m_lowPassFilter.processDF1(m_highPassFilter.processDF1(in));

	// Smooth
	const float smooth = m_envelopeFollower.process(sideChainIn);

	// Convert input from gain to dB
	const float smoothdB = juce::Decibels::gainToDecibels(smooth + 0.000001f);

	//Get gain reduction, positive values
	const float attenuatedB = (smoothdB >= m_thresholddB) ? (smoothdB - m_thresholddB) * m_R_Inv_minus_One : 0.0f;

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

//==============================================================================
VocalCompressor::VocalCompressor()
{
}

float VocalCompressor::process(float in)
{
	return m_gainCompensation * m_compressor.process(m_tubeEmulation.process(m_leveler.process(in)));
}