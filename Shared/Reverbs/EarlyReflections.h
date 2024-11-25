#pragma once

#include <immintrin.h>
#include <vector>
#include <math.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math3D.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

#define M_PI  3.14159268f
#define SPEED_OF_SOUND 343.0f

//==============================================================================
class RoomEarlyReflections : public CircularBuffer
{
public:
	RoomEarlyReflections() {};

	static constexpr float delayTimesFactor[] = {	0.3580f,	0.4617f,	0.5753f,	0.5975f,
													0.9555f,	0.7481f,	1.0000f};
	static constexpr float delayGains[] = {			0.5968f,	0.5228f,	0.4540f,	0.4421f,
													0.2996f,	0.3718f,	0.2871f};
	static const int N_DELAY_LINES = 7;

	void init(int size, int sampleRate)
	{
		__super::init(size);

		const int sampleRateHalf = sampleRate / 2;
		m_maximumFilterFrequency = (sampleRateHalf < 18000) ? (float)sampleRateHalf : 18000.0f;

		for (auto filter : m_filter)
		{
			filter.init(sampleRate);
		}
	};
	void setDamping(float damping) 
	{	
		const float frequency2 = m_maximumFilterFrequency - 500.0f;

		for (int i = 0; i < N_DELAY_LINES; i++)
		{
			const float frequency = m_maximumFilterFrequency - damping * delayTimesFactor[i] * frequency2;
			m_filter[i].setLowPass(frequency, 0.7f);
		}
		
		m_damping = damping;
	};
	float process(float in)
	{
		writeSample(in);
	
		const auto size = getSize();
		float out = 0.0f;

		for (int i = 0; i < N_DELAY_LINES; i++)
		{
			const float delayLineOut = delayGains[i] * readDelay((int)(size * delayTimesFactor[i]));
			out += m_filter[i].processDF1(delayLineOut);
		}

		return out;
	};	

private:
	float m_damping = 0.0f;
	float m_maximumFilterFrequency = 18000.0f;
	BiquadFilter m_filter[N_DELAY_LINES];
};

//==============================================================================
class FibonacciSphereEarlyReflections : CircularBuffer
{
public:
	FibonacciSphereEarlyReflections() {};

	void init(float maximumDimension, int sampleRate, int reflectionsCountMax)
	{	
		m_sampleRate = sampleRate;	
		m_maximumDelayTime =  2.0f * maximumDimension * std::sqrtf(3.0f) / SPEED_OF_SOUND;
		
		const int maximumDelayTimeSamples = (int)(m_maximumDelayTime * sampleRate);
		__super::init(maximumDelayTimeSamples);

		m_gains.resize(reflectionsCountMax);
		m_delayTimesSamples.resize(reflectionsCountMax);

		m_dampingFilters.resize(reflectionsCountMax);

		const int sampleRateHalf = sampleRate / 2;
		m_maximumFilterFrequency = (sampleRateHalf < 18000) ? (float)sampleRateHalf : 18000.0f;

		for (int i = 0; i < reflectionsCountMax; i++)
		{
			m_dampingFilters[i].init(sampleRate);
		}

	};
	void set(float roomLenght, float roomWidth, float roomHeightMax, float damping, Point3D listenerPosition, int reflectionsCount)
	{
		m_reflectionsCount = reflectionsCount;
		std::vector<float> distances;
		distances.resize(reflectionsCount);

		Math3D::GetDistances(distances, roomLenght, roomWidth, roomHeightMax, listenerPosition, reflectionsCount);

		const float frequency2 = m_maximumFilterFrequency - 500.0f;

		for (int i = 0; i < reflectionsCount; i++)
		{
			// Set reflections times in samples
			const float distance = 2.0f * distances[i];
			const float time = distance / SPEED_OF_SOUND;
			m_delayTimesSamples[i] = (int)(m_sampleRate * time);

			// Set reflections gains
			constexpr auto a = 10.0f;
			m_gains[i] = a / (distance + a);

			// Set filters
			// TODO: Find better way to calculate filter frequency
			const float distanceFactor = 0.15f * damping;
			const float frequencyFactor = std::fminf(1.0f, distanceFactor * distance);
			const float frequency = m_maximumFilterFrequency - damping * frequencyFactor * frequency2;
			m_dampingFilters[i].set(frequency);
		}
	};
	float process(float sample)
	{
		float out = 0.0f;

		writeSample(sample);

		for (int i = 0; i < m_reflectionsCount; i++)
		{
			out += m_gains[i] * m_dampingFilters[i].process(readDelay(m_delayTimesSamples[i]));
		}
		
		return out;
	};

private:
	std::vector<OnePoleLowPassFilter> m_dampingFilters;
	std::vector<float> m_gains = {};
	std::vector<int> m_delayTimesSamples = {};
	float m_maximumDelayTime = 0.0f;
	float m_maximumFilterFrequency = 0.0f;
	int m_sampleRate = 48000;
	int m_reflectionsCount = 0;
	int m_reflectionsCountMax = 0;
};