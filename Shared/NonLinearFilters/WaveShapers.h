#pragma once

#include <JuceHeader.h>
#include <math.h>
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

constexpr auto SoftClipperThresholdPositive = 0.9f;
constexpr auto SoftClipperThresholdNegative = -0.9f;
constexpr auto SoftClipperThresholdRatio = 0.3f;		// for 48kHz

class  ARRYWaveShaper
{
public:
	ARRYWaveShaper() {};

	float process(float in)
	{
		return 1.5f * in * (1.0f - in * in / 3.0f);
	};
};

class ExponentialWaveShaper
{
public:
	ExponentialWaveShaper() {};

	void init(int sampleRate)
	{
		m_envelopeShaper.init(sampleRate);
		m_envelopeShaper.setCoef(5.0f, 15.0f);
	}
	inline void set(float drive, float asymetry = 0.0f)
	{
		m_driveExponent = (drive >= 0.0f) ? 1.0f - (0.99f * drive) : 1.0f - 3.0f * drive;
		m_asymetry = asymetry;
	};
	inline float process(float in)
	{
		const float envelope = fmaxf(0.0f, fminf(1.0f, m_envelopeShaper.process(in)));
		const float offset = m_asymetry * envelope;
		const float inOffset = in + offset;

		const float sign = (inOffset >= 0.0f) ? 1.0f : -1.0f;
		return sign * powf(fabsf(inOffset), m_driveExponent);
	};

private:
	EnvelopeFollower m_envelopeShaper;
	float m_driveExponent = 1.0f;
	float m_asymetry = 0.0f;
};

class FoldBackWaveShaper
{
public:
	FoldBackWaveShaper() {};

	void set(float threshold)
	{
		m_threshold = threshold;
	};
	inline float process(float in)
	{
		float inFoldBack = in;

		if (fabsf(inFoldBack) > m_threshold)
		{
			inFoldBack = fabsf(fabsf(fmodf(inFoldBack - m_threshold, m_threshold * 4.0f)) - m_threshold * 2.0f) - m_threshold;
		}

		return inFoldBack;
	}

private:
	float m_threshold = 1.0f;
};

class SoftClipper
{
public:
	SoftClipper() {};

	inline void init(int sampleRate)
	{
		m_softClipperThresholdRatio = SoftClipperThresholdRatio * 48000.0f / (float)sampleRate;
	};

	inline float process(float in)
	{
		float out = 0.0f;

		if (in > SoftClipperThresholdPositive)
		{
			out = SoftClipperThresholdPositive + (in - SoftClipperThresholdPositive) * m_softClipperThresholdRatio;
		}
		else if (in < SoftClipperThresholdNegative)
		{
			out = SoftClipperThresholdNegative + (in - SoftClipperThresholdNegative) * m_softClipperThresholdRatio;
		}
		else
		{
			out = in;
		}

		return fmaxf(-1.0f, fminf(1.0f, out));
	};

private:
	float m_softClipperThresholdRatio = SoftClipperThresholdRatio;
};

class HardClipper
{
public:
	HardClipper() {};

	inline float process(float in)
	{
		if (in > 1.0f)
		{
			return 1.0f;
		}
		else if (in < -1.0f)
		{
			return -1.0f;
		}
		else
		{
			return in;
		}
	}
};

class TubeEmulation
{
public:
	TubeEmulation() {};

	inline void init(int sampleRate)
	{ 
		m_softClipper.init(sampleRate);
	};

	inline void set(float gain)
	{
		m_gain = juce::Decibels::decibelsToGain(gain);
	};

	inline float process(float in)
	{
		float out = m_softClipper.process(in * m_gain);
		out = 0.675f * m_waveShaper.process(out);
		return out;
	};

private:
	float m_gain;
	ARRYWaveShaper m_waveShaper;
	SoftClipper m_softClipper;
};