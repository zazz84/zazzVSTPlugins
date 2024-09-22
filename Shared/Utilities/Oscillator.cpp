/*
  ==============================================================================

    Oscillator.cpp
    Created: 7 Jan 2024 9:16:01am
    Author:  zazz

  ==============================================================================
*/

#include "Oscillator.h"
#include <math.h>

#define M_PI  3.14159268f
#define M_PI2 6.28318536f

//==============================================================================
void SinSquareOscillator::init(int sampleRate)
{
	m_sampleRate = sampleRate;
}

float SinSquareOscillator::process()
{
	m_sampleIndex++;
	m_sampleIndex %= m_periodLenghtSamples;

	float fNumberOfHarmonics = 1.0f + NUMBER_OF_HARMONICS_MINUS_ONE * m_shape;
	int numberOfHarmonics = (int)(fNumberOfHarmonics);

	float out = 0.0f;
	for (int i = 1; i <= numberOfHarmonics; i++)
	{
		const double temp = (double)(2 * i - 1);
		out += sin(m_frequencyCoeficient * (double)m_sampleIndex * temp) / temp;
	}

	// Partial harmonics
	const double temp = (double)(2 * numberOfHarmonics + 1);
	const float fraction = fNumberOfHarmonics - numberOfHarmonics;
	out += fraction * sin(m_frequencyCoeficient * (double)m_sampleIndex * temp) / temp;

	return (float)out;
}

void SinSquareOscillator::setFrequency(float frequency)
{
	m_periodLenghtSamples = m_sampleRate / (int)(frequency);
	// Frequency is rounded to period lenght in samples
	const int frequencyRounded = m_sampleRate / m_periodLenghtSamples;
	m_frequencyCoeficient = 2.0f * M_PI * (double)frequencyRounded / (double)m_sampleRate;
}

void SinSquareOscillator::setShape(float shape)
{
	m_shape = powf(shape, 3);
}

//==============================================================================
void SinSquareInterpolateOscillator::init(int sampleRate)
{
	m_sampleRate = sampleRate;
}


float SinSquareInterpolateOscillator::process()
{
	m_sampleIndex++;
	m_sampleIndex %= m_periodLenghtSamples;

	const float sin = sinf(m_frequencyCoeficient * (double)m_sampleIndex);
	const float square = m_sampleIndex > m_periodLenghtSamplesHalf ? 1.0f : -1.0f;
	
	return m_shapeInverse * sin + m_shape * square;
}

void SinSquareInterpolateOscillator::setFrequency(float frequency)
{
	m_periodLenghtSamples = m_sampleRate / (int)(frequency);
	m_periodLenghtSamplesHalf = m_periodLenghtSamples / 2;

	// Frequency is rounded to period lenght in samples
	const int frequencyRounded = m_sampleRate / m_periodLenghtSamples;
	m_frequencyCoeficient = 2.0f * M_PI * (double)frequencyRounded / (double)m_sampleRate;
}

void SinSquareInterpolateOscillator::setShape(float shape)
{
	m_shape = powf(shape, 2);
	m_shapeInverse = 1.0f - m_shape;
}

//==============================================================================
void SinOscillator::init(int sampleRate)
{
	m_sampleRate = sampleRate;
}

float SinOscillator::process()
{
	m_phase += m_step;
	if (m_phase >= M_PI2)
		m_phase -= M_PI2;

	return sinf(m_phase);
}

void SinOscillator::setFrequency(float frequency)
{
	m_step = 2.0f * M_PI * frequency / (float)m_sampleRate;
}

//==============================================================================
void FastSinOscillator::init(int sampleRate)
{
	m_sampleRate = sampleRate;
}

float FastSinOscillator::process()
{
	m_sampleIndex++;
	m_sampleIndex %= m_periodLenghtSamples;

	const float out = fastSin(m_sampleIndex);

	return out;
}

void FastSinOscillator::setFrequency(float frequency)
{
	m_step = ((m_sampleRate / (int)(frequency)) >> 5);
	m_periodLenghtSamples = m_step << 5;
}

float FastSinOscillator::fastSin(int value)
{
	const int index = value / m_step;;
	const float time = ((float)value / m_step) - index;

	const float out = m_values[index] + time * m_difference[index];

	return out;
}