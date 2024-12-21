#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

/*
Room times calculation :

// Room Delay
const float speedOfSound = 343.0f;

// Max dimensions
const float h = pow(EarlyReflectionsAudioProcessor::m_roomSizeMax / 3.65f, 1.0f / 3.0f);
const float w = 1.6f * h;
const float d = 2.66f * h;

// Axial
const float axialHeightTime = h / speedOfSound;
const float axialWidthTime = w / speedOfSound;
const float axialDepthTime = d / speedOfSound;

// Tangential
const float tangentialHorizontal1Time = sqrt(d * d + 4.0f * w * w) / speedOfSound;
const float tangentialHorizontal2Time = sqrt(w * w + 4.0f * d * d) / speedOfSound;

const float tangentialVertical1Time = sqrt(d * d + 4.0f * h * h) / speedOfSound;
const float tangentialVertical2Time = sqrt(w * w + 4.0f * h * h) / speedOfSound;
*/

class RoomEarlyReflections : public CircularBuffer
{
public:
	RoomEarlyReflections() {};

	/*static constexpr float delayTimesFactor[] = {	0.3580f,	0.4617f,	0.5753f,	0.5975f,
													0.9555f,	0.7481f,	1.0000f };*/
	static constexpr float delayTimesFactor[] = {	0.0002f,	0.1615f,	0.3384f,	0.3730f,
													0.9307f,	0.6067f,	1.0000f };

	static constexpr float	widthChannel2[] =  {	0.0200f,	-0.105f,	0.133f,		-0.052f,
													-0.095f,	 0.101f,	0.0000f };

	/*static constexpr float delayGains[] = { 0.5968f,	0.5228f,	0.4540f,	0.4421f,
											0.2996f,	0.3718f,	0.2871f };*/
	static const int N_DELAY_LINES = 7;

	inline void init(const int size, const int sampleRate, const int channel)
	{
		__super::init(size);

		m_channel = channel;

		const int sampleRateHalf = sampleRate / 2;
		m_maximumFilterFrequency = (sampleRateHalf < 18000) ? (float)sampleRateHalf : 18000.0f;

		for (auto filter : m_filter)
		{
			filter.init(sampleRate);
		}
	};
	inline void set(const float damping, const int predelaySize, const int reflectionsSize, const float width, const float attenuationdB)
	{
		//__super::set(size);

		const float frequency2 = m_maximumFilterFrequency - 500.0f;
		const float w = m_channel == 0 ? 0.0f : width;

		for (int i = 0; i < N_DELAY_LINES; i++)
		{
			// Damping
			const float frequency = m_maximumFilterFrequency - damping * delayTimesFactor[i] * frequency2;
			m_filter[i].set(frequency);

			// Delay
			const float delayFactor = delayTimesFactor[i];
			const int s = (int)(predelaySize + reflectionsSize * delayFactor * (1.0f + widthChannel2[i] * w));
			m_delaySize[i] = s;

			// Attenuation
			const float gain = juce::Decibels::decibelsToGain(delayFactor * attenuationdB);
			m_delayGain[i] = gain;
		}

		m_damping = damping;
	};
	inline float process(const float in)
	{
		write(in);

		float out = 0.0f;

		for (int i = 0; i < N_DELAY_LINES; i++)
		{
			const float delayLineOut = m_delayGain[i] * readDelay(m_delaySize[i]);
			out += m_filter[i].process(delayLineOut);
		}

		return out;
	};
	inline void release()
	{
		__super::release();

		for (int i = 0; i < N_DELAY_LINES; i++)
		{
			m_filter[i].release();
			m_delaySize[i] = 0;
			m_delayGain[i] = 0.0f;
		}

		m_damping = 0.0f;
		m_maximumFilterFrequency = 18000.0f;
		m_channel = 0;

	}

private:
	OnePoleLowPassFilter m_filter[N_DELAY_LINES];
	int m_delaySize[N_DELAY_LINES];
	float m_delayGain[N_DELAY_LINES];
	float m_damping = 0.0f;
	float m_maximumFilterFrequency = 18000.0f;
	int m_channel = 0;
};
