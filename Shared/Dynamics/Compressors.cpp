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
	m_threshold = juce::Decibels::decibelsToGain(thresholddB);
	m_ratio = ratio;
	m_kneeWidth = kneeWidth;
	m_attackTime = attackTimeMS;
	m_releaseTime = releaseTimeMS;
	m_R_Inv_minus_One = (1.0f / ratio) - 1.0f;

	// Soft knee params
	const float W_Half = kneeWidth * 0.5f;
	m_T_minus_WHalfdB = thresholddB - W_Half;
	m_T_minus_WHalf = juce::Decibels::decibelsToGain(m_T_minus_WHalfdB);
	m_T_plus_WHalfdB = thresholddB + W_Half;
	m_minus_T_plus_WHalf = -1.0f * thresholddB + W_Half;
	m_W2_inv = 1.0f / (kneeWidth * 2.0f);

	m_envelopeFollower.setCoef(attackTimeMS, releaseTimeMS);
}

float Compressor::processHardKnee(float in)
{
	// Smooth
	const float smooth = m_envelopeFollower.process(in);

	//Do nothing if below threshold
	if (smooth < m_threshold)
	{
		return in;
	}

	//Get gain reduction, positive values
	const float attenuatedB = (juce::Decibels::gainToDecibels(smooth) - m_thresholddB) * m_R_Inv_minus_One;

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

float Compressor::processSoftKnee(float in)
{
	// Smooth
	const float smooth = m_envelopeFollower.process(in);

	//Do nothing if below threshold
	if (smooth < m_T_minus_WHalf)
	{
		return in;
	}

	// Convert input from gain to dB
	const float smoothdB = juce::Decibels::gainToDecibels(smooth);

	//Get gain reduction, positive values
	float attenuatedB = 0.0f;

	if (smoothdB > m_T_plus_WHalfdB)
	{
		attenuatedB = (smoothdB - m_thresholddB) * m_R_Inv_minus_One;
	}
	else
	{
		const auto tmp = smoothdB + m_minus_T_plus_WHalf;
		attenuatedB = m_R_Inv_minus_One * (tmp * tmp) * m_W2_inv;
	}

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

//==============================================================================
SideChainCompressor::SideChainCompressor()
{
}

void SideChainCompressor::set(float thresholddB, float ratio, float attackTimeMS, float releaseTimeMS, float lowPassFrequency, float highPassFrequency)
{
	m_thresholddB = thresholddB;
	m_threshold = juce::Decibels::decibelsToGain(thresholddB);
	m_ratio = ratio;
	m_attackTime = attackTimeMS;
	m_releaseTime = releaseTimeMS;
	m_R_Inv_minus_One = (1.0f / ratio) - 1.0f;
	m_lowPassFilter.setLowPass(lowPassFrequency, 1.0f);
	m_highPassFilter.setHighPass(highPassFrequency, 1.0f);

	m_envelopeFollower.setCoef(attackTimeMS, releaseTimeMS);
}

float SideChainCompressor::processHardKnee(float in)
{
	//Apply filters
	const float sideChainIn = m_lowPassFilter.processDF1(m_highPassFilter.processDF1(in));

	// Smooth
	const float smooth = m_envelopeFollower.process(sideChainIn);

	//Do nothing if below threshold
	if (smooth < m_threshold)
	{
		return in;
	}

	//Get gain reduction, positive values
	const float attenuatedB = (juce::Decibels::gainToDecibels(smooth) - m_thresholddB) * m_R_Inv_minus_One;

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

//==============================================================================
VocalCompressor::VocalCompressor()
{
}

float VocalCompressor::process(float in)
{
	return m_gainCompensation * m_compressor.processSoftKnee(m_tubeEmulation.process(m_leveler.processHardKnee(in)));
}