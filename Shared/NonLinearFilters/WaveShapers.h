#pragma once

#include <JuceHeader.h>
#include <math.h>
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

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
		m_hightPassFilter.init(sampleRate);
		m_hightPassFilter.setHighPass(10.0f, 0.707f);
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
		if (offset <= 0.001f)
			return sign * powf(fabsf(in), m_driveExponent);
		else
			return m_hightPassFilter.processDF2T(sign * powf(fabsf(inOffset), m_driveExponent));
	};

private:
	EnvelopeFollower m_envelopeShaper;
	BiquadFilter m_hightPassFilter;
	float m_driveExponent = 1.0f;
	float m_asymetry = 0.0f;
};

class ReciprocalWaveShaper
{
public:
	ReciprocalWaveShaper() {};

	void init(int sampleRate)
	{
		m_envelopeShaper.init(sampleRate);
		m_envelopeShaper.setCoef(5.0f, 15.0f);
		m_hightPassFilter.init(sampleRate);
		m_hightPassFilter.setHighPass(10.0f, 0.707f);
	}
	void set(float drive, float asymetry)
	{
		m_drive = drive;
		m_asymetry = asymetry;
		const float drive3 = drive * drive * drive;
		m_gain = 0.001f + 40.0f * drive3;
		m_volume = (1.0f + 10.0f * drive3) / m_gain;
	};
	float process(float in)
	{
		// https://z2dsp.com/2017/09/04/modelling-fuzz/
		const float envelope = fmaxf(0.0f, fminf(1.0f, m_envelopeShaper.process(in)));
		const float offset = m_asymetry * 0.5f * envelope;
		const float inOffset = in + offset;

		const float inGain = inOffset * m_gain;
		return m_hightPassFilter.processDF2T(m_volume * (inGain / (1.0f + fabsf(inGain))));
	};

private:
	EnvelopeFollower m_envelopeShaper;
	BiquadFilter m_hightPassFilter;
	float m_drive = 0.0f;
	float m_asymetry = 0.0f;
	float m_gain = 0.0f;
	float m_volume = 0.0f;
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

class OddHarmonicsWaveShaper
{
public:
	OddHarmonicsWaveShaper() {};

	inline void set(float saturation)
	{
		m_saturation = 1.0f + 19.0f * saturation;
	};

	/*inline float process(float in)
	{
		const float inGain = in * (1.0f + 10.0f * m_saturation );		
		// Simulate tube saturation by adding even-order harmonics
		const float inSaturated = inGain + m_saturation * inGain * inGain * inGain;
		// Use tanh for soft clipping
		return std::tanh(inSaturated);
	};*/

	inline float process(float in)
	{
		// Adding odd-order harmonics
		const float inSaturated = m_saturation * in + m_saturation * in * in * in;
		// Use tanh for soft clipping
		return std::tanh(inSaturated);
	};

	/*inline float process(float in)
	{
		// Adding odd-order harmonics
		const float inSaturated = m_saturation * in * in;
		// Use tanh for soft clipping
		if (in > 0.0f)
		{
			return std::tanh(inSaturated);
		}
		else
		{
			return -1.0f * std::tanh(inSaturated);
		}
	};*/

private:
	float m_saturation = 1.0f;
};

class  SineWaveShaper
{
public:
	SineWaveShaper() {};

	void set(float shape)
	{
		// Input range
		const float inputMin = -1.0;
		const float  inputMax = 1.0;

		// Output range
		const float  outputMin = 1.8;
		const float  outputMax = 0.8;

		// Interpolation formula
		m_shape = outputMin + (shape - inputMin) * (outputMax - outputMin) / (inputMax - inputMin);
	}
	
	float process(float in)
	{
		const float sign = (in > 0.0f) ? 1.0f : -1.0f;
		const float inAbs = fabsf(in);

		if (inAbs > 2.0f)
		{
			return 0.0f;
		}
		else
		{
			return sign * std::powf(-4.0f * inAbs * (0.25f * inAbs - 0.5f), m_shape);
		}
	};

private:
	float m_shape = 1.0f;
};

class  InflatorWaveShaper
{
public:
	InflatorWaveShaper() {};

	// https://github.com/Kiriki-liszt/JS_Inflator?tab=readme-ov-file
	/*void set(float shape)
	{
		const float temp = shape - 0.5f;
		m_A = 1.5 + temp;
		m_B = -(temp + temp);
		m_C = temp - 0.5f;
		m_D = 0.0625f - temp * 0.25f + (temp * temp) * 0.25f;
	}*/

	//https://github.com/ReaTeam/JSFX/blob/master/Distortion/RCInflator2_Oxford.jsfx
	void set(float shape)
	{
		m_A = shape + 1.5f;
		m_B = -(shape + shape);
		m_C = shape - 0.5f;
		m_D = 0.0625f - shape * 0.25 + shape * shape * 0.0025f;
	}

	float process(float in)
	{
		const float sign = (in > 0.0f) ? 1.0f : -1.0f;

		const float s1 = fabsf(in);
		const float s2 = s1 * s1;
		const float s3 = s2 * s1;
		const float s4 = s2 * s2;

		if (s1 >= 2.0f)
		{
			return 0.0f;
		}
		else if (s1 > 1.0f)
		{
			return sign * ((2.0f * s1) - s2);
		}
		else
		{
			return sign * ((m_A * s1) + (m_B * s2) + (m_C * s3) - (m_D * (s2 - (2.0f * s3) + s4)));
		}
	};

private:
	float m_A = 0.0f;
	float m_B = 0.0f;
	float m_C = 0.0f;
	float m_D = 0.0f;
};