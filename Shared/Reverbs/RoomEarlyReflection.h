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
	RoomEarlyReflections() = default;
	~RoomEarlyReflections() = default;

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
		m_maximumFilterFrequency = sampleRateHalf < 18000 ? static_cast<float>(sampleRateHalf) : 18000.0f;

		for (auto filter : m_filter)
		{
			filter.init(sampleRate);
		}
	};
	inline void set(const float damping, const int predelaySize, const int reflectionsSize, const float width, const float attenuationdB) noexcept
	{
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
	inline float process(const float in) noexcept
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
	float m_delayGain[N_DELAY_LINES];
	int m_delaySize[N_DELAY_LINES];
	float m_damping = 0.0f;
	float m_maximumFilterFrequency = 18000.0f;
	int m_channel = 0;
};

class RoomEarlyReflectionsSimple : public CircularBuffer
{
public:
	RoomEarlyReflectionsSimple() = default;
	~RoomEarlyReflectionsSimple() = default;

	/*static constexpr float delayTimesFactor[] = {	0.1123f,	0.1615f,	0.3384f,	0.3730f,
													0.6067f,	0.9307f,	1.0000f };*/

	static constexpr float delayTimesFactor[] = {   0.0123f,	0.0370f,	0.1963f,	0.2037f,
													0.3070f,	0.6530f,	1.0000f };

	static constexpr float	widthChannel[] =	{	0.0120f,	-0.0253f,	0.0133f,	-0.0520f,
													-0.0950f,	 0.1010f,	0.0000f };

	/*static constexpr float delayGains[] =		{	1.000f,		0.6957f,	0.33118f,	0.3010f,
													0.1850f,	0.1206f,	0.1123f };*/

	static constexpr float delayGains[] =		{	0.5000f,	-0.4500f,	0.4000f,	-0.3500f,
													0.3100f,	-0.2800f,	0.2500f };

	static const int N_DELAY_LINES = 7;

	inline void init(const int size, const int channel) noexcept
	{
		__super::init(size);

		m_channel = channel;
	};
	inline void set(const int predelaySize, const int reflectionsSize, const float width) noexcept
	{
		if (m_channel == 0)
		{
			for (int i = 0; i < N_DELAY_LINES; i++)
			{
				m_delaySize[i] = static_cast<int>(predelaySize + reflectionsSize * delayTimesFactor[i]);
			}
		}
		else if (m_channel == 1)
		{
			for (int i = 0; i < N_DELAY_LINES; i++)
			{
				m_delaySize[i] = static_cast<int>(predelaySize + reflectionsSize * (delayTimesFactor[i] + width * widthChannel[i]));
			}
		}
		else
		{
			for (int i = 0; i < N_DELAY_LINES; i++)
			{
				m_delaySize[i] = static_cast<int>(predelaySize + reflectionsSize * (delayTimesFactor[i] - width * widthChannel[i]));
			}
		}
	};
	inline float process(const float in) noexcept
	{
		write(in);

		float out = 0.0f;

		for (int i = 0; i < N_DELAY_LINES; i++)
		{
			out += delayGains[i] * readDelay(m_delaySize[i]);
		}

		return out;
	};
	inline void release()
	{
		__super::release();

		for (int i = 0; i < N_DELAY_LINES; i++)
		{
			m_delaySize[i] = 0;
		}

		m_channel = 0;
	}

private:
	int m_delaySize[N_DELAY_LINES];
	int m_channel = 0;
};
