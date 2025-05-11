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

class CircularBuffer
{
public:
	CircularBuffer() = default;
	~CircularBuffer() { clearBuffer(); }

	inline void init(const unsigned int size)
	{
		clearBuffer();
			
		const int sizePowerOfTwo = GetPowerOfTwo(size);
		m_bitMask = m_head = sizePowerOfTwo - 1;
		m_readOffset = sizePowerOfTwo - size;

		m_buffer = new float[sizePowerOfTwo];
		memset(m_buffer, 0, sizePowerOfTwo * sizeof(float));
	}
	inline void set(const unsigned int size) noexcept
	{
		m_readOffset = m_bitMask - size + 1;
	};
	inline int getSize() const noexcept
	{
		return m_bitMask - m_readOffset + 1;
	};
	inline void write(const float sample) noexcept
	{
		m_head = (m_head + 1) & m_bitMask;
		m_buffer[m_head] = sample;
	}
	inline float read() const noexcept
	{  
		return m_buffer[(m_head + m_readOffset) & m_bitMask];
	};
	inline void release()
	{
		clearBuffer();
		
		m_head = 0;
		m_bitMask = 0;
		m_readOffset = 0;
	}
	inline float readDelay(const int sample) const noexcept
	{
		return m_buffer[(m_head - sample) & m_bitMask];
	}
	inline float readDelayLinearInterpolation(const float sample)
	{
		const int sampleTrunc = (int)(sample);

		const int readIdx = m_head - sampleTrunc;
		const float weight = sample - sampleTrunc;

		const int iYounger = readIdx & m_bitMask;
		const int iOlder = (readIdx - 1) & m_bitMask;

		const float younger = m_buffer[iYounger];
		const float older = m_buffer[iOlder];

		return older + weight * (younger - older);
	}
	// https://yehar.com/blog/wp-content/uploads/2009/08/deip.pdf
	float readDelayTriLinearInterpolation(const float sample)
	{
		const int sampleTrunc = (int)(sample);
		const int readIdx = m_head - sampleTrunc;
		const float weight = sample - sampleTrunc;

		const int idx1 = (readIdx - 1) & m_bitMask;
		const int idx2 = readIdx & m_bitMask;
		const int idx3 = (readIdx + 1) & m_bitMask;
		const int idx4 = (readIdx + 2) & m_bitMask;

		const float yz1 = m_buffer[idx1];
		const float y0 = m_buffer[idx2];
		const float y1 = m_buffer[idx3];
		const float y2 = m_buffer[idx4];

		// 4-point, 2nd-order Watte tri-linear (x-form)
		float ym1py2 = yz1 + y2;
		float c0 = y0;
		float c1 = 3.0f / 2.0f * y1 - 1.0f / 2.0f * (y0 + ym1py2);
		float c2 = 1.0f / 2.0f * (ym1py2 - y0 - y1);

		return (c2 * weight + c1) * weight + c0;
	}

private:
	inline int GetPowerOfTwo(int i)
	{
		jassert(i > 0);
		int n = i - 1;

		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		n++;

		return n;
	}
	inline void clearBuffer()
	{
		if (m_buffer == nullptr)
		{
			return;
		}

		delete[] m_buffer;
		m_buffer = nullptr;
	}

	float* m_buffer = nullptr;
	int m_head = 0;					// Index of the youngest sample
	int m_bitMask = 0;
	int m_readOffset = 0;
};