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

	//! Initialize circular buffer to maximum size
	inline void init(const unsigned int circularBufferSize, const unsigned int linearBufferSize = 0u)
	{
		clearBuffer();
			
		// Circular buffer size
		const unsigned int sizePowerOfTwo = GetPowerOfTwo(std::max(circularBufferSize, linearBufferSize)); // Circular buffer needs to have at least the same size as linear buffer
		m_bitMask = m_head = sizePowerOfTwo - 1;
		m_readOffset = sizePowerOfTwo - circularBufferSize;
		m_circularBufferDelay = circularBufferSize;

		// Total buffer size
		const unsigned int bufferSize = sizePowerOfTwo + linearBufferSize;

		// Allocate data
		m_circularBuffer = new float[bufferSize];
		memset(m_circularBuffer, 0, bufferSize * sizeof(float));

		// Set linear buffer poiter
		m_linearBuffer = m_circularBuffer + sizePowerOfTwo;
		m_linearBufferSize = linearBufferSize;
	}
	inline void set(const unsigned int circularBufferSize) noexcept
	{
		m_readOffset = m_bitMask - circularBufferSize + 1;
		m_circularBufferDelay = circularBufferSize;
	};
	inline float* getLinearBuffer(const bool copyData = true)
	{
		if (true)
		{
			const unsigned int circularBufferSize = m_bitMask + 1;
			//const unsigned int readStart = (m_head - m_circularBufferDelay - m_linearBufferSize) & m_bitMask;
			const unsigned int readStart = (m_head + m_readOffset) & m_bitMask;

			const unsigned int firstChunkLength = std::min(m_linearBufferSize, circularBufferSize - readStart);
			const unsigned int secondChunkLength = m_linearBufferSize - firstChunkLength;

			// Copy first chunk
			std::memcpy(m_linearBuffer, m_circularBuffer + readStart, firstChunkLength * sizeof(float));

			// Copy second chunk if wrapped
			if (secondChunkLength > 0)
				std::memcpy(m_linearBuffer + firstChunkLength, m_circularBuffer, secondChunkLength * sizeof(float));
		}

		return m_linearBuffer;
	}
	inline void moveLinearBufferToCircularBuffer()
	{
		const unsigned int circularBufferSize = m_bitMask + 1;

		// Compute start index in circular buffer (wrap with mask)
		const unsigned int firstSampleWrite = (m_head + 1) & m_bitMask;

		// Compute how many samples fit until end of buffer
		const unsigned int firstChunkLength = std::min(m_linearBufferSize, circularBufferSize - firstSampleWrite);
		const unsigned int secondChunkLength = m_linearBufferSize - firstChunkLength;

		// Copy first chunk
		std::memcpy(m_circularBuffer + firstSampleWrite, m_linearBuffer, firstChunkLength * sizeof(float));

		// Copy second chunk if wrapped
		if (secondChunkLength > 0)
			std::memcpy(m_circularBuffer, m_linearBuffer + firstChunkLength, secondChunkLength * sizeof(float));

		// Update head to point to newest sample (wrapped)
		m_head = (firstSampleWrite + m_linearBufferSize - 1) & m_bitMask;
	}
	inline unsigned int getSize() const noexcept
	{
		return m_bitMask - m_readOffset + 1;
	};
	inline void write(const float sample) noexcept
	{
		m_head = (m_head + 1) & m_bitMask;
		m_circularBuffer[m_head] = sample;
	}
	inline float read() const noexcept
	{  
		return m_circularBuffer[(m_head + m_readOffset) & m_bitMask];
	};
	inline void release()
	{
		clearBuffer();
		
		m_head = 0;
		m_bitMask = 0;
		m_readOffset = 0;
		m_linearBufferSize = 0u;
	}
	inline float readDelay(const int sample) const noexcept
	{
		return m_circularBuffer[(m_head - sample) & m_bitMask];
	}
	inline float readDelayLinearInterpolation(const float sample)
	{
		const int sampleTrunc = (int)(sample);

		const int readIdx = m_head - sampleTrunc;
		const float weight = sample - sampleTrunc;

		const int iYounger = readIdx & m_bitMask;
		const int iOlder = (readIdx - 1) & m_bitMask;

		const float younger = m_circularBuffer[iYounger];
		const float older = m_circularBuffer[iOlder];

		return younger + weight * (older - younger);
	}
	// https://yehar.com/blog/wp-content/uploads/2009/08/deip.pdf
	float readDelayTriLinearInterpolation(const float sample)
	{
		const int sampleTrunc = (int)(sample);
		const int readIdx = m_head - sampleTrunc;
		const float weight = sample - sampleTrunc;

		/*const int idx1 = (readIdx - 1) & m_bitMask;
		const int idx2 = readIdx & m_bitMask;
		const int idx3 = (readIdx + 1) & m_bitMask;
		const int idx4 = (readIdx + 2) & m_bitMask;*/

		const int idx4 = (readIdx - 1) & m_bitMask;
		const int idx3 = readIdx & m_bitMask;
		const int idx2 = (readIdx + 1) & m_bitMask;
		const int idx1 = (readIdx + 2) & m_bitMask;

		const float yz1 = m_circularBuffer[idx1];
		const float y0 = m_circularBuffer[idx2];
		const float y1 = m_circularBuffer[idx3];
		const float y2 = m_circularBuffer[idx4];

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
		if (m_circularBuffer != nullptr)
		{
			delete[] m_circularBuffer;
			m_circularBuffer = nullptr;
			m_linearBuffer = nullptr;
		}
	}

	float* m_circularBuffer = nullptr;
	float* m_linearBuffer = nullptr;
	int m_head = 0;					// Index of the youngest sample
	int m_bitMask = 0;
	int m_readOffset = 0;
	unsigned int m_linearBufferSize = 0u;
	unsigned int m_circularBufferDelay = 0u;	// Stores set delay in sample		
};