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

#include <cstring>

class AudioBuffer
{
public:
	AudioBuffer() = default;
	~AudioBuffer() { clearBuffer(); }

	inline void init(const unsigned int size)
	{
		clearBuffer();

		m_size = size;

		m_buffer = new float[size];
		std::memset(m_buffer, 0, size * sizeof(float));
	}
	inline int getSize() const noexcept
	{
		return m_size;
	}
	inline void release()
	{
		clearBuffer();
	}
	inline void applyAttack(const int startSample, const int endSample)
	{	
		// Apply silence
		for (int i = 0; i < startSample; i++)
		{
			m_buffer[i] = 0.0f;
		}

		// Apply attack	
		const int lenght = endSample - startSample;
		for (int i = 0; i < lenght; i++)
		{
			const float gain = (float)i / (float)lenght;
			m_buffer[startSample + i] *= gain;
		}
	}

	inline void applyRelease(const int startSample, const int endSample)
	{
		// Apply release	
		const int lenght = endSample - startSample;
		for (int i = 0; i < lenght; i++)
		{
			const float gain = 1.0f - (float)i / (float)lenght;
			m_buffer[startSample + i] *= gain;
		}

		// Apply silence
		for (int i = endSample; i < m_size; i++)
		{
			m_buffer[i] = 0.0f;
		}
	}

	float* m_buffer = nullptr;

private:
	inline void clearBuffer()
	{
		if (m_buffer == nullptr)
		{
			return;
		}

		delete[] m_buffer;
		m_buffer = nullptr;
	}

	int m_size = 0;
};