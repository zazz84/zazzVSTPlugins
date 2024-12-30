#pragma once

#include <type_traits>

#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

#include <JuceHeader.h>

//==============================================================================
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
struct CompressorParams
{
public:
	CompressorParams() = default;
	~CompressorParams() = default;

	inline void set(T thresholddB, T ratio, T kneeWidth)
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
	~Compressor() = default;

	void init(int sampleRate)
	{ 
		m_envelopeFollower.init(sampleRate);
		m_circularBuffer.init(sampleRate);
		const int size = static_cast<int>(0.01f * (float)sampleRate);
		m_circularBuffer.set(size);
	};
	void set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS);
	float processHardKneeLinPeak(float in);
	float processHardKneeLogPeak(float in);
	float processHardKneeLinRMS(float in);
	float processHardKneeLogRMS(float in);
	float processSoftKneeLinPeak(float in);

protected:
	DecoupeledEnvelopeFollower<float> m_envelopeFollower;
	RMSBuffer m_circularBuffer;
	CompressorParams<float> m_params;
};

//==============================================================================
class SlewCompressor
{
public:
	SlewCompressor() = default;
	~SlewCompressor() = default;

	void init(int sampleRate)
	{
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLinear.init(sampleRate);

		m_circularBuffer.init(sampleRate);
		const int size = static_cast<int>(0.01f * sampleRate);
		m_circularBuffer.set(size);
	};
	void set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS);
	float processHardKneeLinPeak(float in);
	float processHardKneeLogPeak(float in);
	float processHardKneeLinRMS(float in);
	float processHardKneeLogRMS(float in);
	float processSoftKnee(float in);

protected:
	SlewEnvelopeFollower<float> m_envelopeFollowerLog;
	SlewEnvelopeFollower<float> m_envelopeFollowerLinear;
	RMSBuffer m_circularBuffer;
	CompressorParams<float> m_params;
};

//==============================================================================
class OptoCompressor
{
public:
	OptoCompressor() = default;
	~OptoCompressor() = default;

	void init(int sampleRate)
	{
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLinear.init(sampleRate);

		m_RMSBuffer.init(sampleRate);
		const int size = static_cast<int>(0.01f * sampleRate);
		m_RMSBuffer.set(size);
	};
	void set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS);
	float processHardKneeLinPeak(float in);
	float processHardKneeLogPeak(float in);
	float processHardKneeLinRMS(float in);
	float processHardKneeLogRMS(float in);
	float processSoftKnee(float in);

protected:
	OptoEnvelopeFollower<float> m_envelopeFollowerLog;
	OptoEnvelopeFollower<float> m_envelopeFollowerLinear;
	RMSBuffer m_RMSBuffer;
	CompressorParams<float> m_params;
};

//==============================================================================
class DualCompressor
{
public:
	DualCompressor() = default;
	~DualCompressor() = default;

	void init(int sampleRate)
	{
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLinear.init(sampleRate);

		m_RMSBuffer.init(sampleRate);
		const int size = static_cast<int>(0.01f * sampleRate);
		m_RMSBuffer.set(size);
	};
	void set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS);
	float processHardKneeLinPeak(float in);
	float processHardKneeLogPeak(float in);
	float processHardKneeLinRMS(float in);
	float processHardKneeLogRMS(float in);
	float processSoftKnee(float in);

protected:
	DualEnvelopeFollower<float> m_envelopeFollowerLog;
	DualEnvelopeFollower<float> m_envelopeFollowerLinear;
	RMSBuffer m_RMSBuffer;
	CompressorParams<float> m_params;
};

//==============================================================================
class AdaptiveCompressor
{
public:
	AdaptiveCompressor() = default;
	~AdaptiveCompressor() = default;

	inline void init(const int sampleRate)
	{
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLinear.init(sampleRate);

		m_RMSBuffer.init(sampleRate);
		const int size = static_cast<int>(0.01f * sampleRate);
		m_RMSBuffer.set(size);
	};
	inline void set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS, float holdTimeMS)
	{
		m_params.set(thresholddB, ratio, kneeWidth);

		// Set envelope followers
		m_envelopeFollowerLinear.set(attackTimeMS, releaseTimeMS, holdTimeMS);
		m_envelopeFollowerLog.set(attackTimeMS, releaseTimeMS, holdTimeMS);
	};
	float processHardKneeLinPeak(float in)
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
	float processHardKneeLogRMS(float in)
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
	float processSoftKnee(float in)
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

protected:
	HoldEnvelopeFollower<float> m_envelopeFollowerLog;
	HoldEnvelopeFollower<float> m_envelopeFollowerLinear;
	RMSBuffer m_RMSBuffer;
	CompressorParams<float> m_params;
};