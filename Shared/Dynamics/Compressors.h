#pragma once

#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/WaveShapers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include <JuceHeader.h>

//==============================================================================
class Compressor
{
public:
	Compressor();

	void init(int sampleRate)
	{ 
		m_envelopeFollower.init(sampleRate);
		m_circularBuffer.init(sampleRate);
		const int size = (int)(0.01f * sampleRate);
		m_circularBuffer.setSize(size);
	};
	void set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS);
	float processHardKneeLinPeak(float in);
	float processHardKneeLogPeak(float in);
	float processHardKneeLinRMS(float in);
	float processHardKneeLogRMS(float in);
	float processSoftKneeLinPeak(float in);

protected:
	EnvelopeFollower m_envelopeFollower;
	RMSBuffer m_circularBuffer;
	float m_thresholddB = 0.0f;
	float m_threshold = 1.0f;
	float m_ratio = 2.0f;
	float m_kneeWidth = 0.0f;
	float m_attackTime = 0.0f;
	float m_releaseTime = 0.0f;

	float m_R_Inv_minus_One = 0.0f;
	float m_T_minus_WHalfdB = 0.0f;
	float m_T_minus_WHalf = 1.0f;
	float m_T_plus_WHalfdB = 0.0f;
	float m_minus_T_plus_WHalf = 0.0f;
	float m_W2_inv = 0.0f;
};

//==============================================================================
/*template <class EnvelopeFollower, class T>
class Compressor1
{
public:
	Compressor1() {};

	void init(int sampleRate)
	{
		m_envelopeFollower.init(sampleRate);
		m_circularBuffer.init(sampleRate);
		const int size = (int)(0.01f * sampleRate);
		m_circularBuffer.setSize(size);
	};
	void set(T thresholddB, T ratio, T kneeWidth, T attackTimeMS, T releaseTimeMS);
	T processHardKneeLinPeak(T in);
	T processHardKneeLogPeak(T in);
	T processHardKneeLinRMS(T in);
	T processHardKneeLogRMS(T in);
	T processSoftKneeLinPeak(T in);

protected:
	EnvelopeFollower<T> m_envelopeFollower;
	RMSBuffer m_circularBuffer;
	T m_thresholddB = 0.0;
	T m_threshold = 1.0;
	T m_ratio = 2.0;
	T m_kneeWidth = 0.0;
	T m_attackTime = 0.0;
	T m_releaseTime = 0.0;

	T m_R_Inv_minus_One = 0.0;
	T m_T_minus_WHalfdB = 0.0;
	T m_T_minus_WHalf = 1.0;
	T m_T_plus_WHalfdB = 0.0;
	T m_minus_T_plus_WHalf = 0.0;
	T m_W2_inv = 0.0;
};*/

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
	float processHardKnee(float in);

protected:
	EnvelopeFollower m_envelopeFollower;
	BiquadFilter m_lowPassFilter;
	BiquadFilter m_highPassFilter;
	float m_thresholddB = 0.0f;
	float m_threshold = 1.0f;
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
		m_tubeEmulation.set(-0.5f);

		m_gainCompensation = juce::Decibels::decibelsToGain(14.0f);
	};
	float process(float in);

protected:
	SideChainCompressor m_leveler;
	Compressor m_compressor;
	TubeEmulation m_tubeEmulation;
	float m_gainCompensation = 1.0f;
};

//==============================================================================
class SlewCompressor
{
public:
	SlewCompressor();

	void init(int sampleRate)
	{
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLinear.init(sampleRate);

		m_circularBuffer.init(sampleRate);
		const int size = (int)(0.01f * sampleRate);
		m_circularBuffer.setSize(size);
	};
	void set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS);
	float processHardKneeLinPeak(float in);
	float processHardKneeLogPeak(float in);
	float processHardKneeLinRMS(float in);
	float processHardKneeLogRMS(float in);
	float processSoftKnee(float in);

protected:
	SlewEnvelopeFollower m_envelopeFollowerLog;
	SlewEnvelopeFollower m_envelopeFollowerLinear;
	RMSBuffer m_circularBuffer;
	float m_thresholddB = 0.0f;
	float m_threshold = 1.0f;
	float m_ratio = 2.0f;
	float m_kneeWidth = 0.0f;
	float m_attackTime = 0.0f;
	float m_releaseTime = 0.0f;

	float m_R_Inv_minus_One = 0.0f;
	float m_T_minus_WHalfdB = 0.0f;
	float m_T_minus_WHalf = 1.0f;
	float m_T_plus_WHalfdB = 0.0f;
	float m_minus_T_plus_WHalf = 0.0f;
	float m_W2_inv = 0.0f;
};

//==============================================================================
class OptoCompressor
{
public:
	OptoCompressor();

	void init(int sampleRate)
	{
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLinear.init(sampleRate);

		m_RMSBuffer.init(sampleRate);
		const int size = (int)(0.01f * sampleRate);
		m_RMSBuffer.setSize(size);
	};
	void set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS);
	float processHardKneeLinPeak(float in);
	float processHardKneeLogPeak(float in);
	float processHardKneeLinRMS(float in);
	float processHardKneeLogRMS(float in);
	float processSoftKnee(float in);

protected:
	OptoEnvelopeFollower m_envelopeFollowerLog;
	OptoEnvelopeFollower m_envelopeFollowerLinear;
	RMSBuffer m_RMSBuffer;
	float m_thresholddB = 0.0f;
	float m_threshold = 1.0f;
	float m_ratio = 2.0f;
	float m_kneeWidth = 0.0f;
	float m_attackTime = 0.0f;
	float m_releaseTime = 0.0f;

	float m_R_Inv_minus_One = 0.0f;
	float m_T_minus_WHalfdB = 0.0f;
	float m_T_minus_WHalf = 1.0f;
	float m_T_plus_WHalfdB = 0.0f;
	float m_minus_T_plus_WHalf = 0.0f;
	float m_W2_inv = 0.0f;
};

//==============================================================================
class DualCompressor
{
public:
	DualCompressor();

	void init(int sampleRate)
	{
		m_envelopeFollowerLog.init(sampleRate);
		m_envelopeFollowerLinear.init(sampleRate);

		m_RMSBuffer.init(sampleRate);
		const int size = (int)(0.01f * sampleRate);
		m_RMSBuffer.setSize(size);
	};
	void set(float thresholddB, float ratio, float kneeWidth, float attackTimeMS, float releaseTimeMS);
	float processHardKneeLinPeak(float in);
	float processHardKneeLogPeak(float in);
	float processHardKneeLinRMS(float in);
	float processHardKneeLogRMS(float in);
	float processSoftKnee(float in);

protected:
	DualEnvelopeFollower m_envelopeFollowerLog;
	DualEnvelopeFollower m_envelopeFollowerLinear;
	RMSBuffer m_RMSBuffer;
	float m_thresholddB = 0.0f;
	float m_threshold = 1.0f;
	float m_ratio = 2.0f;
	float m_kneeWidth = 0.0f;
	float m_attackTime = 0.0f;
	float m_releaseTime = 0.0f;

	float m_R_Inv_minus_One = 0.0f;
	float m_T_minus_WHalfdB = 0.0f;
	float m_T_minus_WHalf = 1.0f;
	float m_T_plus_WHalfdB = 0.0f;
	float m_minus_T_plus_WHalf = 0.0f;
	float m_W2_inv = 0.0f;
};