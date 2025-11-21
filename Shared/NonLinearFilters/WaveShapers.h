/*
 * Copyright (C) 2025 Filip Cenzak (filip.c@centrum.cz)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <JuceHeader.h>
#include <math.h>
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

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

	inline static float Tanh(const float in, const float drive, const float asymetry)
	{
		// Branchless remap
		float asymNorm = (in > 0.0f) ? -std::fminf(0.0f, asymetry) : std::fmaxf(0.0f, asymetry);
		float asymGain = 1.0f - 0.5f * asymNorm;

		float driveIn = asymGain * drive * in;

		// Rational tanh approx
		float d2 = driveIn * driveIn;
		float y = driveIn * (27.0f + d2) / (27.0f + 9.0f * d2);

		// Clamp branchlessly
		return std::clamp(y, -1.0f, 1.0f);
	}

	inline static float Reciprocal(float in, float drive)
	{
		const float drive2in = 1.4f * drive * in;
		return drive2in / (1.0f + std::fabsf(drive2in));
	}

	inline static float Reciprocal(const float in, const float drive, const float asymetry)
	{
		const float driveIn = 1.4f * drive * in;
		return driveIn / (1.0f + std::fabsf(driveIn + asymetry));
	}

	inline static float Atan(float in, float drive)
	{
		constexpr float factor = 1.0f / (0.5f * 3.141592f);
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
			return 1.5f * driveAdjustedIn * (1.0f - (driveAdjustedIn * driveAdjustedIn) / 3.0f);
		}
	}

	inline static float ARRY(float in)
	{
		return 1.5f * in * (1.0f - (1.0f / 3.0f) * (in * in));
	};

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
		const float driveAdjusted = 0.4f + std::expf(-0.6f * drive);
		const float inAbs = std::fabsf(in);
		return std::copysign(std::powf(inAbs, driveAdjusted), in);
	}

	inline static float Exponential(float in, float drive, const float asymetry)
	{
		float driveAdjusted = 0.5f + std::expf(-0.6f * drive);

		const float asym = in > 0.0f ? -std::fminf(0.0f, asymetry) : std::fmaxf(0.0f, asymetry);

		driveAdjusted = Math::remap(asym, 0.0f, 1.0f, driveAdjusted, 1.0f);

		const float inAbs = std::fabsf(in);
		const float out = std::copysign(std::powf(inAbs, driveAdjusted), in);

		return Tanh(out, 1.0f, 0.0f);
	}

	// Not gain normalized to the rest of the waveshapers
	// https://www.willpirkle.com/special/Addendum_A19_Pirkle_v1.0.pdf
	// https://patentimages.storage.googleapis.com/26/09/61/06441121cd3b24/US20080049950A1.pdf
	inline static float Poletti(float in, float drive, float positiveLimit, float negativeLimit)
	{
		const float inDrive = in * drive;

		if (in > 0)
		{
			return inDrive / (1.0f + (inDrive / positiveLimit));
		}
		else
		{
			return inDrive / (1.0f - (inDrive / negativeLimit));
		}
	}

	// Slip
	inline static float Split(float in, const float threshold, const float split)
	{
		const float magnitude = std::copysign(split, in);					// Add sign to split
		const float mask = static_cast<float>(std::fabs(in) > threshold);   // 1.0f or 0.0f
		return in + magnitude * mask;
	}

	/*
	#include <immintrin.h>
	#include <cstddef>

	void processBuffer_SSE(const float* in, float* out, std::size_t n)
	{
		const __m128 signMask  = _mm_set1_ps(-0.0f); // sign bit mask
		const __m128 absMask   = _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff));
		const __m128 thresh    = _mm_set1_ps(0.001f);
		const __m128 plusPoint1= _mm_set1_ps(0.1f);

		std::size_t i = 0;
		const std::size_t Vec = 4;

		for (; i + Vec <= n; i += Vec)
		{
			__m128 x   = _mm_loadu_ps(in + i);
			__m128 ax  = _mm_and_ps(x, absMask);                // |x|
			__m128 mask= _mm_cmpgt_ps(ax, thresh);              // compare
			__m128 sign= _mm_and_ps(x, signMask);               // extract sign
			__m128 mag = _mm_or_ps(plusPoint1, sign);           // copysign(0.1, x)
			__m128 addTerm = _mm_and_ps(mag, mask);             // zero where false
			__m128 y = _mm_add_ps(x, addTerm);
			_mm_storeu_ps(out + i, y);
		}

		// tail
		for (; i < n; ++i)
		{
			float mag = std::copysign(0.1f, in[i]);
			out[i] = (std::fabs(in[i]) > 0.001f) ? (in[i] + mag) : in[i];
		}
	}*/

	inline static float Split2(float in, const float threshold, const float split)
	{
		const float inAbs = fabsf(in);

		if (inAbs < threshold)
		{
			return in;
		}
		else if (inAbs < 1.0f)
		{
			return std::copysignf(1.0f + ((1.0f - split) * (inAbs - 1.0f)), in);
		}
		else
		{
			return in;
		}
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
	DecoupeledEnvelopeFollower<float> m_envelopeShaper;
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
	DecoupeledEnvelopeFollower<float> m_envelopeShaper;
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
		const float inputMin = -1.0f;
		const float  inputMax = 1.0f;

		// Output range
		const float  outputMin = 1.8f;
		const float  outputMax = 0.8f;

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
		m_D = 0.0625f - shape * 0.25f + shape * shape * 0.0025f;
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