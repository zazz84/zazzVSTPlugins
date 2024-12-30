#pragma once

#include "Compressors.h"
#include <JuceHeader.h>

//==============================================================================
Compressor::Compressor()
{
}

void Compressor::set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS)
{
	// Hard knee + shared params
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

	m_envelopeFollower.set(attackTimeMS, releaseTimeMS);
}

float Compressor::processHardKneeLinPeak(float in)
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

float Compressor::processHardKneeLinRMS(float in)
{
	// Get RMS
	m_circularBuffer.write(in);
	const float rms = m_circularBuffer.getRMS();

	// Smooth
	const float smooth = m_envelopeFollower.process(rms);

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

float Compressor::processHardKneeLogPeak(float in)
{
	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(fabsf(in) + 0.001f);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_thresholddB) ? (indB - m_thresholddB) * m_R_Inv_minus_One : 0.0f;

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
	const float indB = juce::Decibels::gainToDecibels(rms + 0.000001f);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_thresholddB) ? (indB - m_thresholddB) * m_R_Inv_minus_One : 0.0f;

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
/*template <class EnvelopeFollower, class T>
void Compressor1<EnvelopeFollower, T>::set(T thresholddB, T ratio, T kneeWidth, T attackTimeMS, T releaseTimeMS)
{
	// Hard knee + shared params
	m_thresholddB = thresholddB;
	m_threshold = juce::Decibels::decibelsToGain(thresholddB);
	m_ratio = ratio;
	m_kneeWidth = kneeWidth;
	m_attackTime = attackTimeMS;
	m_releaseTime = releaseTimeMS;
	m_R_Inv_minus_One = (1.0 / ratio) - 1.0;

	// Soft knee params
	const T W_Half = kneeWidth * 0.5;
	m_T_minus_WHalfdB = thresholddB - W_Half;
	m_T_minus_WHalf = juce::Decibels::decibelsToGain(m_T_minus_WHalfdB);
	m_T_plus_WHalfdB = thresholddB + W_Half;
	m_minus_T_plus_WHalf = -1.0 * thresholddB + W_Half;
	m_W2_inv = 1.0 / (kneeWidth * 2.0);

	m_envelopeFollower.setCoef(attackTimeMS, releaseTimeMS);
}

template <class EnvelopeFollower, class T>
T Compressor1<EnvelopeFollower, T>::processHardKneeLinPeak(T in)
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

template <class EnvelopeFollower, class T>
T Compressor1<EnvelopeFollower, T>::processHardKneeLinRMS(T in)
{
	// Get RMS
	m_circularBuffer.writeSample(in);
	const float rms = m_circularBuffer.getRMS();

	// Smooth
	const float smooth = m_envelopeFollower.process(rms);

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

template <class EnvelopeFollower, class T>
T Compressor1<EnvelopeFollower, T>::processHardKneeLogPeak(T in)
{
	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(fabsf(in) + 0.001);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_thresholddB) ? (indB - m_thresholddB) * m_R_Inv_minus_One : 0.0;

	// Smooth
	const float attenuateSmoothdB = -m_envelopeFollower.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

template <class EnvelopeFollower, class T>
T Compressor1<EnvelopeFollower, T>::processHardKneeLogRMS(T in)
{
	// Get RMS
	m_circularBuffer.writeSample(in);
	const float rms = m_circularBuffer.getRMS();

	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(rms + 0.000001);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_thresholddB) ? (indB - m_thresholddB) * m_R_Inv_minus_One : 0.0;

	// Smooth
	const float attenuateSmoothdB = -m_envelopeFollower.process(attenuatedB);

	// Apply gain reduction
	return in * juce::Decibels::decibelsToGain(attenuateSmoothdB);
}

template <class EnvelopeFollower, class T>
T Compressor1<EnvelopeFollower, T>::processSoftKneeLinPeak(T in)
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
}*/

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

	m_envelopeFollower.set(attackTimeMS, releaseTimeMS);
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
	return m_gainCompensation * m_compressor.processSoftKneeLinPeak(m_tubeEmulation.process(m_leveler.processHardKnee(in)));
}

//==============================================================================
SlewCompressor::SlewCompressor()
{
}

void SlewCompressor::set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS)
{
	// Hard knee + shared params
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

	// Set envelope followers
	m_envelopeFollowerLinear.set(2.2f * attackTimeMS, 3.5f * releaseTimeMS);
	m_envelopeFollowerLog.set(0.1f * attackTimeMS, 0.2f * releaseTimeMS);
}

float SlewCompressor::processHardKneeLinPeak(float in)
{	
	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(in);

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

float SlewCompressor::processHardKneeLinRMS(float in)
{
	// Get RMS
	m_circularBuffer.write(in);
	const float rms = m_circularBuffer.getRMS();

	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(rms);

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

float SlewCompressor::processHardKneeLogPeak(float in)
{
	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(fabsf(in) + 0.000001f);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_thresholddB) ? (indB - m_thresholddB) * m_R_Inv_minus_One : 0.0f;

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
	const float indB = juce::Decibels::gainToDecibels(rms + 0.000001f);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_thresholddB) ? (indB - m_thresholddB) * m_R_Inv_minus_One : 0.0f;

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
OptoCompressor::OptoCompressor()
{
}

void OptoCompressor::set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS)
{
	// Hard knee + shared params
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

	// Set envelope followers
	m_envelopeFollowerLinear.set(attackTimeMS, releaseTimeMS);
	m_envelopeFollowerLog.set(attackTimeMS, releaseTimeMS);
}

float OptoCompressor::processHardKneeLinPeak(float in)
{
	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(24.0f * in) / 24.0f;

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

float OptoCompressor::processHardKneeLinRMS(float in)
{
	// Get RMS
	m_RMSBuffer.write(in);
	const float rms = m_RMSBuffer.getRMS();

	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(24.0f * rms) / 24.0f;

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

float OptoCompressor::processHardKneeLogPeak(float in)
{
	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(fabsf(in) + 0.000001f);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_thresholddB) ? (indB - m_thresholddB) * m_R_Inv_minus_One : 0.0f;

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
	const float indB = juce::Decibels::gainToDecibels(rms + 0.000001f);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_thresholddB) ? (indB - m_thresholddB) * m_R_Inv_minus_One : 0.0f;

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
DualCompressor::DualCompressor()
{
}

void DualCompressor::set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS)
{
	// Hard knee + shared params
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

	// Set envelope followers
	m_envelopeFollowerLinear.setCoef(attackTimeMS, releaseTimeMS);
	m_envelopeFollowerLog.setCoef(attackTimeMS, releaseTimeMS);
}

float DualCompressor::processHardKneeLinPeak(float in)
{
	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(24.0f * in) / 24.0f;

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

float DualCompressor::processHardKneeLinRMS(float in)
{
	// Get RMS
	m_RMSBuffer.write(in);
	const float rms = m_RMSBuffer.getRMS();

	// Smooth
	const float smooth = m_envelopeFollowerLinear.process(24.0f * rms) / 24.0f;

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

float DualCompressor::processHardKneeLogPeak(float in)
{
	// Convert input from gain to dB
	const float indB = juce::Decibels::gainToDecibels(fabsf(in) + 0.000001f);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_thresholddB) ? (indB - m_thresholddB) * m_R_Inv_minus_One : 0.0f;

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
	const float indB = juce::Decibels::gainToDecibels(rms + 0.000001f);

	//Get gain reduction, positive values
	const float attenuatedB = (indB >= m_thresholddB) ? (indB - m_thresholddB) * m_R_Inv_minus_One : 0.0f;

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