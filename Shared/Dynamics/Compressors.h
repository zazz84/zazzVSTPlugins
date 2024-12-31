#pragma once

#include <type_traits>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/RMS.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/ZeroCrossingRate.h"

#include <JuceHeader.h>

// Used to match ammount of compression when using RMS compared to peak detection
#define RMS_FACTOR 1.5f

//==============================================================================
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
struct CompressorParams
{
public:
	CompressorParams() = default;
	~CompressorParams() = default;

	inline void set(const T thresholddB, const T ratio, const T kneeWidth)
	{
		// Hard knee + shared params
		m_thresholddB = thresholddB;
		m_threshold = juce::Decibels::decibelsToGain(thresholddB);
		m_ratio = ratio;
		m_kneeWidth = kneeWidth;
		m_R_Inv_minus_One = (T(1.0) / ratio) - T(1.0);

		// Soft knee params
		const T W_Half = T(0.5) * kneeWidth;
		m_T_minus_WHalfdB = thresholddB - W_Half;
		m_T_minus_WHalf = juce::Decibels::decibelsToGain(m_T_minus_WHalfdB);
		m_T_plus_WHalfdB = thresholddB + W_Half;
		m_minus_T_plus_WHalf = T(-1.0) * thresholddB + W_Half;
		m_W2_inv = T(1.0) / (T(2.0) * kneeWidth);
	}

	T m_thresholddB = T(0.0);
	T m_threshold = T(1.0);
	T m_ratio = T(2.0);
	T m_kneeWidth = T(0.0);

	T m_R_Inv_minus_One = T(0.0);
	T m_T_minus_WHalfdB = T(0.0);
	T m_T_minus_WHalf = T(1.0);
	T m_T_plus_WHalfdB = T(0.0);
	T m_minus_T_plus_WHalf = T(0.0);
	T m_W2_inv = T(0.0);
};

//==============================================================================
class Compressor
{
public:
	Compressor() = default;
	~Compressor()
	{
		m_RMS.release();
	};

	inline void init(const int sampleRate)
	{ 
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLin.init(sampleRate);

		const int size = static_cast<int>(0.01f * static_cast<float>(sampleRate));		// RMS calculated from 10ms long buffer
		m_RMS.init(size);
	};
	inline void set(const float thresholddB, const float ratio, const float kneeWidth, const float attackTimeMS, const float releaseTimeMS, const float peakRatio = 1.0f, const float logRatio = 1.0f)
	{
		m_params.set(thresholddB, ratio, kneeWidth);
		
		m_envelopeFollowerLog.set(attackTimeMS, releaseTimeMS);
		m_envelopeFollowerLin.set(attackTimeMS, releaseTimeMS);
		
		m_peakRatio = peakRatio;
		m_logRatio = logRatio;
	}
	inline float processHardKnee(const float in)
	{
		// Get combined peak/rms input
		const float rms = RMS_FACTOR * m_RMS.process(in);
		const float peak = Math::fabsf(in);
		const float inCombined = m_peakRatio * (peak - rms) + rms;

		// Get log attenuation
		const float indB = juce::Decibels::gainToDecibels(inCombined);
		const float envelopeIndB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;
		const float attenuateLogdB = -m_envelopeFollowerLog.process(envelopeIndB);

		// Get lin attenuation
		const float smoothLin = m_envelopeFollowerLin.process(inCombined);
		const float attenuateLindB = smoothLin > m_params.m_threshold ? (juce::Decibels::gainToDecibels(smoothLin) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

		// Get combined lin/log attenuation
		const float attenuatedB = m_logRatio * (attenuateLogdB - attenuateLindB) + attenuateLindB;
		const float attenuateGain = juce::Decibels::decibelsToGain(attenuatedB);
		
		// Apply gain reduction
		return attenuateGain * in;
	};
	float processHardKneeLinPeak(float in);
	float processHardKneeLogPeak(float in);
	float processHardKneeLinRMS(float in);
	float processHardKneeLogRMS(float in);
	float processSoftKneeLinPeak(float in);

protected:
	RMS m_RMS;
	CompressorParams<float> m_params;
	DecoupeledEnvelopeFollower<float> m_envelopeFollowerLog;
	DecoupeledEnvelopeFollower<float> m_envelopeFollowerLin;
	float m_peakRatio = 1.0f;
	float m_logRatio = 1.0f;
};

//==============================================================================
class SlewCompressor
{
public:
	SlewCompressor() = default;
	~SlewCompressor() = default;

	inline void init(const int sampleRate)
	{
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLin.init(sampleRate);

		const int size = static_cast<int>(0.01f * static_cast<float>(sampleRate));		// RMS calculated from 10ms long buffer
		m_RMS.init(size);;
	};
	inline void set(const float thresholddB, const float ratio, const float kneeWidth, const float attackTimeMS, const float releaseTimeMS, const float peakRatio = 1.0f, const float logRatio = 1.0f)
	{
		m_params.set(thresholddB, ratio, kneeWidth);

		m_envelopeFollowerLin.set(2.2f * attackTimeMS, 3.5f * releaseTimeMS);
		m_envelopeFollowerLog.set(0.1f * attackTimeMS, 0.2f * releaseTimeMS);

		m_peakRatio = peakRatio;
		m_logRatio = logRatio;
	}
	inline float processHardKnee(const float in)
	{
		// Get combined peak/rms input
		const float rms = RMS_FACTOR * m_RMS.process(in);
		const float peak = std::fabsf(in);
		const float inCombined = m_peakRatio * (peak - rms) + rms;

		// Get log attenuation
		const float indB = juce::Decibels::gainToDecibels(inCombined);
		const float envelopeIndB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;
		const float attenuateLogdB = -m_envelopeFollowerLog.process(envelopeIndB);

		// Get lin attenuation
		const float smoothLin = m_envelopeFollowerLin.process(inCombined);
		const float attenuateLindB = smoothLin > m_params.m_threshold ? (juce::Decibels::gainToDecibels(smoothLin) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

		// Get combined lin/log attenuation
		const float attenuatedB = m_logRatio * (attenuateLogdB - attenuateLindB) + attenuateLindB;
		const float attenuateGain = juce::Decibels::decibelsToGain(attenuatedB);

		// Apply gain reduction
		return attenuateGain * in;
	};
	inline float processHardKneeLinPeak(const float in);
	inline float processHardKneeLogPeak(const float in);
	inline float processHardKneeLinRMS(const float in);
	inline float processHardKneeLogRMS(const float in);
	inline float processSoftKnee(const float in);

protected:
	SlewEnvelopeFollower<float> m_envelopeFollowerLog;
	SlewEnvelopeFollower<float> m_envelopeFollowerLin;
	RMS m_RMS;
	CompressorParams<float> m_params;
	float m_peakRatio = 1.0f;
	float m_logRatio = 1.0f;;
};

//==============================================================================
class OptoCompressor
{
public:
	OptoCompressor() = default;
	~OptoCompressor() = default;

	inline void init(const int sampleRate)
	{
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLin.init(sampleRate);

		const int size = static_cast<int>(0.01f * static_cast<float>(sampleRate));		// RMS calculated from 10ms long buffer
		m_RMS.init(size);
	};
	inline void set(const float thresholddB, const float ratio, const float kneeWidth, const float attackTimeMS, const float releaseTimeMS, const float peakRatio = 1.0f, const float logRatio = 1.0f)
	{
		m_params.set(thresholddB, ratio, kneeWidth);

		m_envelopeFollowerLin.set(attackTimeMS, releaseTimeMS);
		m_envelopeFollowerLog.set(attackTimeMS, releaseTimeMS);

		m_peakRatio = peakRatio;
		m_logRatio = logRatio;
	};
	inline float processHardKnee(const float in)
	{
		// Get combined peak/rms input
		const float rms = RMS_FACTOR * m_RMS.process(in);
		const float peak = std::fabsf(in);
		const float inCombined = m_peakRatio * (peak - rms) + rms;

		// Get log attenuation
		const float indB = juce::Decibels::gainToDecibels(inCombined);
		const float envelopeIndB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;
		const float attenuateLogdB = -m_envelopeFollowerLog.process(envelopeIndB);

		// Get lin attenuation
		const float smoothLin = (1.0f / 24.0f) * m_envelopeFollowerLin.process(24.0f * inCombined);
		const float attenuateLindB = smoothLin > m_params.m_threshold ? (juce::Decibels::gainToDecibels(smoothLin) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

		// Get combined lin/log attenuation
		const float attenuatedB = m_logRatio * (attenuateLogdB - attenuateLindB) + attenuateLindB;
		const float attenuateGain = juce::Decibels::decibelsToGain(attenuatedB);

		// Apply gain reduction
		return attenuateGain * in;
	};
	float processHardKneeLinPeak(float in);
	float processHardKneeLogPeak(float in);
	float processHardKneeLinRMS(float in);
	float processHardKneeLogRMS(float in);
	float processSoftKnee(float in);

protected:
	OptoEnvelopeFollower<float> m_envelopeFollowerLog;
	OptoEnvelopeFollower<float> m_envelopeFollowerLin;
	RMS m_RMS;
	CompressorParams<float> m_params;
	float m_peakRatio = 1.0f;
	float m_logRatio = 1.0f;
};

//==============================================================================
class DualCompressor
{
public:
	DualCompressor() = default;
	~DualCompressor() = default;

	inline void init(const int sampleRate)
	{
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLin.init(sampleRate);

		const int size = static_cast<int>(0.01f * static_cast<float>(sampleRate));		// RMS calculated from 10ms long buffer
		m_RMS.init(size);
	};
	inline void set(const float thresholddB, const float ratio, const float kneeWidth, const float attackTimeMS, const float releaseTimeMS, const float peakRatio = 1.0f, const float logRatio = 1.0f)
	{
		m_params.set(thresholddB, ratio, kneeWidth);

		m_envelopeFollowerLin.set(attackTimeMS, releaseTimeMS);
		m_envelopeFollowerLog.set(attackTimeMS, releaseTimeMS);

		m_peakRatio = peakRatio;
		m_logRatio = logRatio;
	};
	inline float processHardKnee(const float in)
	{
		// Get combined peak/rms input
		const float rms = RMS_FACTOR * m_RMS.process(in);
		const float peak = std::fabsf(in);
		const float inCombined = m_peakRatio * (peak - rms) + rms;

		// Get log attenuation
		const float indB = juce::Decibels::gainToDecibels(inCombined);
		const float envelopeIndB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;
		const float attenuateLogdB = -m_envelopeFollowerLog.process(envelopeIndB);

		// Get lin attenuation
		const float smoothLin = (1.0f / 24.0f) * m_envelopeFollowerLin.process(24.0f * inCombined);
		const float attenuateLindB = smoothLin > m_params.m_threshold ? (juce::Decibels::gainToDecibels(smoothLin) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

		// Get combined lin/log attenuation
		const float attenuatedB = m_logRatio * (attenuateLogdB - attenuateLindB) + attenuateLindB;
		const float attenuateGain = juce::Decibels::decibelsToGain(attenuatedB);

		// Apply gain reduction
		return attenuateGain * in;
	};
	float processHardKneeLinPeak(float in);
	float processHardKneeLogPeak(float in);
	float processHardKneeLinRMS(float in);
	float processHardKneeLogRMS(float in);
	float processSoftKnee(float in);

protected:
	DualEnvelopeFollower<float> m_envelopeFollowerLog;
	DualEnvelopeFollower<float> m_envelopeFollowerLin;
	RMS m_RMS;
	CompressorParams<float> m_params;
	float m_peakRatio = 1.0f;
	float m_logRatio = 1.0f;
};

//==============================================================================
class AdaptiveCompressor
{
public:
	AdaptiveCompressor() = default;
	~AdaptiveCompressor() = default;

	inline void init(const int sampleRate)
	{
		m_zeroCrossingRate.init(sampleRate);
		
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLin.init(sampleRate);

		const int size = static_cast<int>(0.01f * static_cast<float>(sampleRate));		// RMS calculated from 10ms long buffer
		m_RMS.init(size);
	};
	inline void set(const float thresholddB, const float ratio, const float kneeWidth, const float attackTimeMS, const float releaseTimeMS, const float peakRatio = 1.0f, const float logRatio = 1.0f)
	{
		m_params.set(thresholddB, ratio, kneeWidth);

		m_envelopeFollowerLin.set(attackTimeMS, releaseTimeMS, 0.0f);
		m_envelopeFollowerLog.set(attackTimeMS, releaseTimeMS, 0.0f);

		m_peakRatio = peakRatio;
		m_logRatio = logRatio;
	};
	inline float processHardKnee(const float in)
	{
		// Handle hold time
		const int holdTimeSamples = 2 * m_zeroCrossingRate.process(in);
		m_envelopeFollowerLin.setHoldTimeSamples(holdTimeSamples);
		m_envelopeFollowerLog.setHoldTimeSamples(holdTimeSamples);
		
		// Get combined peak/rms input
		const float rms = RMS_FACTOR * m_RMS.process(in);
		const float peak = std::fabsf(in);
		const float inCombined = m_peakRatio * (peak - rms) + rms;

		// Get log attenuation
		const float indB = juce::Decibels::gainToDecibels(inCombined);
		const float envelopeIndB = (indB >= m_params.m_thresholddB) ? (indB - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;
		const float attenuateLogdB = -m_envelopeFollowerLog.process(envelopeIndB);

		// Get lin attenuation
		const float smoothLin = (1.0f / 24.0f) * m_envelopeFollowerLin.process(24.0f * inCombined);
		const float attenuateLindB = smoothLin > m_params.m_threshold ? (juce::Decibels::gainToDecibels(smoothLin) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One : 0.0f;

		// Get combined lin/log attenuation
		const float attenuatedB = m_logRatio * (attenuateLogdB - attenuateLindB) + attenuateLindB;
		const float attenuateGain = juce::Decibels::decibelsToGain(attenuatedB);

		// Apply gain reduction
		return attenuateGain * in;
	};
	float processHardKneeLinPeak(float in)
	{
		// Smooth
		const float smooth = (1.0f / 24.0f) * m_envelopeFollowerLin.process(24.0f * in);

		//Do nothing if below threshold
		if (smooth < m_params.m_threshold)
		{
			return in;
		}

		//Get gain reduction, positive values
		const float attenuatedB = (juce::Decibels::gainToDecibels(smooth) - m_params.m_thresholddB) * m_params.m_R_Inv_minus_One;

		// Apply gain reduction
		return in * juce::Decibels::decibelsToGain(attenuatedB);
	};
	float processHardKneeLogPeak(float in)
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
	float processHardKneeLinRMS(float in)
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
	float processHardKneeLogRMS(float in)
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
	float processSoftKnee(float in)
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

protected:
	RMS m_RMS;
	CompressorParams<float> m_params;
	ZeroCrossingRate m_zeroCrossingRate;
	HoldEnvelopeFollower<float> m_envelopeFollowerLog;
	HoldEnvelopeFollower<float> m_envelopeFollowerLin;
	float m_peakRatio = 1.0f;
	float m_logRatio = 1.0f;
};