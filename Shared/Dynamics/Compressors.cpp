#pragma once

#include "Compressors.h"

#include <JuceHeader.h>

//==============================================================================
float Compressor::processHardKneeLinPeak(float in)
{	
	// Smooth
	const float smooth = m_envelopeFollowerLin.process(in);

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
	const float rms = RMS_FACTOR * m_RMS.process(in);

	// Smooth
	const float smooth = m_envelopeFollowerLin.process(rms);

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
	const float attenuateSmoothdB = -m_envelopeFollowerLog.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

float Compressor::processHardKneeLogRMS(float in)
{
	// Get RMS
	const float rms = RMS_FACTOR * m_RMS.process(in);

	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(rms);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

	// Smooth
	const float attenuateSmoothdB = -m_envelopeFollowerLog.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

float Compressor::processSoftKneeLinPeak(float in)
{
	// Smooth
	const float smooth = m_envelopeFollowerLin.process(in);

	//Do nothing if below threshold
	if (smooth < m_params.m_T_minus_WHalf)
	{
		return in;
	}

	// Convert input from gain to dB
	const float smoothdB = Math::gainTodB(smooth);

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
	return in * Math::dBToGain(attenuatedB);
}

//==============================================================================
float SlewCompressor::processHardKneeLinPeak(float in)
{	
	// Smooth
	const float smooth = m_envelopeFollowerLin.process(in);

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
	const float rms = RMS_FACTOR * m_RMS.process(in);

	// Smooth
	const float smooth = m_envelopeFollowerLin.process(rms);

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
	const float rms = RMS_FACTOR * m_RMS.process(in);

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
	const float smooth = m_envelopeFollowerLin.process(in);

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
float OptoCompressor::processHardKneeLinPeak(float in)
{
	// Smooth
	const float smooth = m_envelopeFollowerLin.process(24.0f * in) / 24.0f;

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
	const float rms = RMS_FACTOR * m_RMS.process(in);

	// Smooth
	const float smooth = m_envelopeFollowerLin.process(24.0f * rms) / 24.0f;

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
	const float rms = RMS_FACTOR * m_RMS.process(in);

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
	const float smooth = m_envelopeFollowerLin.process(in);

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
float DualCompressor::processHardKneeLinPeak(float in)
{
	// Smooth
	const float smooth = m_envelopeFollowerLin.process(24.0f * in) / 24.0f;

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
	const float rms = RMS_FACTOR * m_RMS.process(in);

	// Smooth
	const float smooth = m_envelopeFollowerLin.process(24.0f * rms) / 24.0f;

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
	const float rms = RMS_FACTOR * m_RMS.process(in);

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
	const float smooth = m_envelopeFollowerLin.process(in);

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