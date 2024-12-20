#pragma once

#include <JuceHeader.h>
#include <math.h>
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

constexpr auto SoftClipperThresholdPositive = 0.9f;
constexpr auto SoftClipperThresholdNegative = -0.9f;
constexpr auto SoftClipperThresholdRatio = 0.3f;		// for 48kHz


// Waveshapers are gain compensated, so all methods produce more or less the same amount of distortion for the same input
// Note: Tanh, Atan, Sin, SinAproximation, ARRY and ReciprocalQuadratic sound very similar
class Waveshapers
{
public:
	inline static float Tanh(const float in, const float drive)
	{
		//return std::tanhf(drive * in);

		// Tanh aproximation for small values
		const float driveIn = drive * in;
		
		if (driveIn < -3.0f)
		{
			return -1.0f;
		}
		else if (driveIn > 3.0)
		{
			return 1.0f;
		}
		else
		{
			float driveIn2 = driveIn * driveIn;
			return driveIn * (27.0f + driveIn2) / (27.0f + 9.0f * driveIn2);
		}
	}

	inline static float Reciprocal(float in, float drive)
	{
		const float drive2in = 1.35f * drive * in;
		return drive2in / (1.0f + std::fabsf(drive2in));
	}

	inline static float Atan(float in, float drive)
	{
		constexpr float factor = 1.0f / (0.5f * 3.141592);
		return factor * std::atanf(1.7f * drive * in);
	}

	inline static float Sin(float in, float drive)
	{
		const float driveAdjusted = 1.9f * drive;
		const float limit = 3.141592f / driveAdjusted;

		if (in < -limit)
		{
			return -1.0f;
		}
		else if (in > limit)
		{
			return 1.0f;
		}
		else
		{
			return std::sinf(0.5f * driveAdjusted * in);
		}
	}
	inline static float SinAproximation(float in, float drive)
	{
		// Using hyperbolas to aproximate sin function
		const float driveAdjusted = 0.53f * drive;
		const float limit = 1.0f / driveAdjusted;

		if (in < -limit)
		{
			return -1.0f;
		}
		else if (in < 0.0f)
		{
			const float inDriveAdjusted = in * driveAdjusted;
			return inDriveAdjusted * (inDriveAdjusted + 2.0f);
		}
		else if (in < limit)
		{
			const float inDriveAdjusted = in * driveAdjusted;
			return -inDriveAdjusted * (inDriveAdjusted - 2.0f);
		}
		else
		{
			return 1.0f;
		}
	}

	inline static float ARRY(float in, float drive)
	{
		const float driveAdjusted = 0.65f * drive;
		const float limit = 1.0f / driveAdjusted;
		const float driveAdjustedIn = driveAdjusted * in;

		if (in < -limit)
		{
			return -1.0f;
		}
		else if (in > limit)
		{
			return 1.0f;
		}
		else
		{
			return 1.5f * driveAdjustedIn * (1 - (driveAdjustedIn * driveAdjustedIn) / 3.0f);
		}
	}

	inline static float ReciprocalQuadratic(float in, float drive)
	{
		const float driveAdjusted = 0.25f * drive;
		const float limit = 1.0f / (2.0f * driveAdjusted);
		const float driveAdjustedIn = driveAdjusted * in;

		if (in < -limit)
		{
			return -1.0f;
		}
		else if (in > limit)
		{
			return 1.0f;
		}
		else
		{
			return driveAdjustedIn / (0.25f + driveAdjustedIn * driveAdjustedIn);
		}
	}

	inline static float Exponential(float in, float drive)
	{
		const float driveAdjusted = 0.5f + std::expf(-0.5f * drive);
		const float inAbs = std::fabsf(in);
		const float sign = (in > 0.0f) ? 1.0f : -1.0f;

		return sign * std::powf(inAbs, driveAdjusted);
	}
};

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
		m_envelopeShaper.set(5.0f, 15.0f);
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
		m_envelopeShaper.set(5.0f, 15.0f);
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
	float m_gain = 1.0f;
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