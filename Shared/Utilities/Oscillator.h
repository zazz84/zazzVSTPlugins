/*
  ==============================================================================

    Oscillator.h
    Created: 7 Jan 2024 9:16:01am
    Author:  zazz

  ==============================================================================
*/

#pragma once

#include <cassert>
#include <cmath>
#include <iostream>

class SinSquareOscillator
{
public:
	SinSquareOscillator() {};

	void init(int sampleRate);
	float process();
	void setFrequency(float frequency);
	void setShape(float shape);

protected:
	int NUMBER_OF_HARMONICS_MINUS_ONE = 23;
	int m_sampleRate = 48000;
	float m_shape = 0.0f;
	int m_sampleIndex = 0;
	int m_periodLenghtSamples = 48000;
	double m_frequencyCoeficient = 1.0;
};

//==============================================================================

class SinSquareInterpolateOscillator
{
public:
	SinSquareInterpolateOscillator() {};

	void init(int sampleRate);
	float process();
	void setFrequency(float frequency);
	void setShape(float shape);

private:
	int m_sampleRate = 48000;
	float m_shape = 0.0f;
	int m_sampleIndex = 0;
	int m_periodLenghtSamples = 48000;
	int m_periodLenghtSamplesHalf = 24000;
	double m_frequencyCoeficient = 1.0;
	float m_shapeInverse = 0.0f;
};

//==============================================================================
class SinOscillator
{
public:
	SinOscillator() {};

	void init(int sampleRate);
	float process();
	void setFrequency(float frequency);

private:
	int m_sampleRate = 48000;
	float m_step = 0.0f;
	float m_phase = 0.0f;
};

//==============================================================================
class FastSinOscillator
{
public:
	FastSinOscillator() {};

	void init(int sampleRate);
	float process();
	void setFrequency(float frequency);

	float fastSin(int value);

	int m_sampleRate = 48000;
	int m_sampleIndex = 0;
	int m_periodLenghtSamples = 48000;
	int m_step;

	const float m_values[32] = {0,
								0.195090322,
								0.3826834324,
								0.555570233,
								0.7071067812,
								0.8314696123,
								0.9238795325,
								0.9807852804,
								1,
								0.9807852804,
								0.9238795325,
								0.8314696123,
								0.7071067812,
								0.555570233,
								0.3826834324,
								0.195090322,
								0,
								- 0.195090322,
								- 0.3826834324,
								- 0.555570233,
								- 0.7071067812,
								- 0.8314696123,
								- 0.9238795325,
								- 0.9807852804,
								- 1,
								- 0.9807852804,
								- 0.9238795325,
								- 0.8314696123,
								- 0.7071067812,
								- 0.555570233,
								- 0.3826834324,
								- 0.195090322,
	};

	const float m_difference[32] = {	0.195090322,
										0.1875931103,
										0.1728868007,
										0.1515365482,
										0.1243628311,
										0.0924099202,
										0.0569057479,
										0.0192147196,
										- 0.0192147196,
										- 0.0569057479,
										- 0.0924099202,
										- 0.1243628311,
										- 0.1515365482,
										- 0.1728868007,
										- 0.1875931103,
										- 0.195090322,
										- 0.195090322,
										- 0.1875931103,
										- 0.1728868007,
										- 0.1515365482,
										- 0.1243628311,
										- 0.0924099202,
										- 0.0569057479,
										- 0.0192147196,
										0.0192147196,
										0.0569057479,
										0.0924099202,
										0.1243628311,
										0.1515365482,
										0.1728868007,
										0.1875931103,
										0.195090322,
	};
};