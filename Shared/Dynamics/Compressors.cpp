#pragma once

#include "Compressors.h"

#include <JuceHeader.h>

//==============================================================================
void Compressor::set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS)
{
	m_params.set(thresholddB, ratio, kneeWidth);
	m_envelopeFollower.set(attackTimeMS, releaseTimeMS);
}

float Compressor::processHardKneeLinPeak(float in)
{	
	// Smooth
	const float smooth = m_envelopeFollower.process(in);

	//Do nothing if below threshold
	if (smooth < m_params.m_threshold)
	{
		return in;
	}

	//Get gain reduction, positive values
	const float attenuatedB = (juce::Decibels::gainToDecibels(smooth) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

float Compressor::processHardKneeLinRMS(float in)
{
	// Get RMS
	m_circularBuffer.write(in);
	const float rms = m_circularBuffer.getRMS();

	// Smooth
	const float smooth = m_envelopeFollower.process(rms);

	//Do nothing if below threshold
	if (smooth < m_params.m_threshold)
	{
		return in;
	}

	//Get gain reduction, positive values
	const float attenuatedB = (juce::Decibels::gainToDecibels(smooth) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

float Compressor::processHardKneeLogPeak(float in)
{
	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(fabsf(in));

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

	// Smooth
	const float attenuateSmoothdB = -m_envelopeFollower.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

float Compressor::processHardKneeLogRMS(float in)
{
	// Get RMS
	m_circularBuffer.write(in);
	const float rms = m_circularBuffer.getRMS();

	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(rms);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

	// Smooth
	const float attenuateSmoothdB = -m_envelopeFollower.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

float Compressor::processSoftKneeLinPeak(float in)
{
	// Smooth
	const float smooth = m_envelopeFollower.process(in);

	//Do nothing if below threshold
	if (smooth < m_params.m_T_minus_WHalf)
	{
		return in;
	}

	// Convert input from gain to dB
	const float smoothdB = juce::Decibels::gainToDecibels(smooth);

	//Get gain reduction, positive values
	float attenuatedB = 0.0f;

	if (smoothdB > m_params.m_T_plus_WHalfdB)
	{
		attenuatedB = (smoothdB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;
	}
	else
	{
		const auto tmp = smoothdB + m_params.m_minus_T_plus_WHalf;
		attenuatedB = m_params.m_R_Inv_minus_One * (tmp * tmp) * m_params.m_W2_inv;
	}

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

//==============================================================================
void SlewCompressor::set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS)
{
	m_params.set(thresholddB, ratio, kneeWidth);

	// Set envelope followers
	m_envelopeFollowerLinear.set(2.2f * attackTimeMS, 3.5f * releaseTimeMS);
	m_envelopeFollowerLog.set(0.1f * attackTimeMS, 0.2f * releaseTimeMS);
}

float SlewCompressor::processHardKneeLinPeak(float in)
{	
	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(in);

	//Do nothing if below threshold
	if (smooth < m_params.m_threshold)
	{
		return in;
	}

	//Get gain reduction, positive values
	const float attenuatedB = (juce::Decibels::gainToDecibels(smooth) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

float SlewCompressor::processHardKneeLinRMS(float in)
{
	// Get RMS
	m_circularBuffer.write(in);
	const float rms = m_circularBuffer.getRMS();

	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(rms);

	//Do nothing if below threshold
	if (smooth < m_params.m_threshold)
	{
		return in;
	}

	//Get gain reduction, positive values
	const float attenuatedB = (juce::Decibels::gainToDecibels(smooth) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

float SlewCompressor::processHardKneeLogPeak(float in)
{
	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(fabsf(in));

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

	// Smooth
	const float attenuateSmoothdB = -m_envelopeFollowerLog.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

float SlewCompressor::processHardKneeLogRMS(float in)
{
	// Get RMS
	m_circularBuffer.write(in);
	const float rms = m_circularBuffer.getRMS();

	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(rms);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

	// Smooth
	const float attenuateSmoothdB = -m_envelopeFollowerLog.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

float SlewCompressor::processSoftKnee(float in)
{
	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(in);

	//Do nothing if below threshold
	if (smooth < m_params.m_T_minus_WHalf)
	{
		return in;
	}

	// Convert input from gain to dB
	const float smoothdB = juce::Decibels::gainToDecibels(smooth);

	//Get gain reduction, positive values
	float attenuatedB = 0.0f;

	if (smoothdB > m_params.m_T_plus_WHalfdB)
	{
		attenuatedB = (smoothdB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;
	}
	else
	{
		const auto tmp = smoothdB + m_params.m_minus_T_plus_WHalf;
		attenuatedB = m_params.m_R_Inv_minus_One * (tmp * tmp) * m_params.m_W2_inv;
	}

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

//==============================================================================
void OptoCompressor::set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS)
{
	m_params.set(thresholddB, ratio, kneeWidth);

	// Set envelope followers
	m_envelopeFollowerLinear.set(attackTimeMS, releaseTimeMS);
	m_envelopeFollowerLog.set(attackTimeMS, releaseTimeMS);
}

float OptoCompressor::processHardKneeLinPeak(float in)
{
	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(24.0f * in) / 24.0f;

	//Do nothing if below threshold
	if (smooth < m_params.m_threshold)
	{
		return in;
	}

	//Get gain reduction, positive values
	const float attenuatedB = (juce::Decibels::gainToDecibels(smooth) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

float OptoCompressor::processHardKneeLinRMS(float in)
{
	// Get RMS
	m_RMSBuffer.write(in);
	const float rms = m_RMSBuffer.getRMS();

	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(24.0f * rms) / 24.0f;

	//Do nothing if below threshold
	if (smooth < m_params.m_threshold)
	{
		return in;
	}

	//Get gain reduction, positive values
	const float attenuatedB = (juce::Decibels::gainToDecibels(smooth) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

float OptoCompressor::processHardKneeLogPeak(float in)
{
	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(fabsf(in));

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

	// Smooth
	const float attenuateSmoothdB = -m_envelopeFollowerLog.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

float OptoCompressor::processHardKneeLogRMS(float in)
{
	// Get RMS
	m_RMSBuffer.write(in);
	const float rms = m_RMSBuffer.getRMS();

	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(rms);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

	// Smooth
	const float attenuateSmoothdB = -m_envelopeFollowerLog.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

float OptoCompressor::processSoftKnee(float in)
{
	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(in);

	//Do nothing if below threshold
	if (smooth < m_params.m_T_minus_WHalf)
	{
		return in;
	}

	// Convert input from gain to dB
	const float smoothdB = juce::Decibels::gainToDecibels(smooth);

	//Get gain reduction, positive values
	float attenuatedB = 0.0f;

	if (smoothdB > m_params.m_T_plus_WHalfdB)
	{
		attenuatedB = (smoothdB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;
	}
	else
	{
		const auto tmp = smoothdB + m_params.m_minus_T_plus_WHalf;
		attenuatedB = m_params.m_R_Inv_minus_One * (tmp * tmp) * m_params.m_W2_inv;
	}

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

//==============================================================================
void DualCompressor::set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS)
{
	m_params.set(thresholddB, ratio, kneeWidth);;

	// Set envelope followers
	m_envelopeFollowerLinear.set(attackTimeMS, releaseTimeMS);
	m_envelopeFollowerLog.set(attackTimeMS, releaseTimeMS);
}

float DualCompressor::processHardKneeLinPeak(float in)
{
	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(24.0f * in) / 24.0f;

	//Do nothing if below threshold
	if (smooth < m_params.m_threshold)
	{
		return in;
	}

	//Get gain reduction, positive values
	const float attenuatedB = (juce::Decibels::gainToDecibels(smooth) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

float DualCompressor::processHardKneeLinRMS(float in)
{
	// Get RMS
	m_RMSBuffer.write(in);
	const float rms = m_RMSBuffer.getRMS();

	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(24.0f * rms) / 24.0f;

	//Do nothing if below threshold
	if (smooth < m_params.m_threshold)
	{
		return in;
	}

	//Get gain reduction, positive values
	const float attenuatedB = (juce::Decibels::gainToDecibels(smooth) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}

float DualCompressor::processHardKneeLogPeak(float in)
{
	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(fabsf(in));

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

	// Smooth
	const float attenuateSmoothdB = -m_envelopeFollowerLog.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

float DualCompressor::processHardKneeLogRMS(float in)
{
	// Get RMS
	m_RMSBuffer.write(in);
	const float rms = m_RMSBuffer.getRMS();

	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(rms);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

	// Smooth
	const float attenuateSmoothdB = -m_envelopeFollowerLog.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

float DualCompressor::processSoftKnee(float in)
{
	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(in);

	//Do nothing if below threshold
	if (smooth < m_params.m_T_minus_WHalf)
	{
		return in;
	}

	// Convert input from gain to dB
	const float smoothdB = juce::Decibels::gainToDecibels(smooth);

	//Get gain reduction, positive values
	float attenuatedB = 0.0f;

	if (smoothdB > m_params.m_T_plus_WHalfdB)
	{
		attenuatedB = (smoothdB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;
	}
	else
	{
		const auto tmp = smoothdB + m_params.m_minus_T_plus_WHalf;
		attenuatedB = m_params.m_R_Inv_minus_One * (tmp * tmp) * m_params.m_W2_inv;
	}

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuatedB);
}