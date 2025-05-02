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

#include <math.h>
#include <immintrin.h> // for AVX/SSE intrinsics

class CircularBuffer
{
public:
	CircularBuffer() = default;
	~CircularBuffer() { clearBuffer(); }

	/*inline void init(const int size)
	{
		if (m_buffer != nullptr)
		{
			clearBuffer();
		}
		
		m_head = 0;
		const int sizePowerOfTwo = GetPowerOfTwo(size);
		m_bitMask = sizePowerOfTwo - 1;

		m_readOffset = m_bitMask - size;

		m_buffer = new float[sizePowerOfTwo];
		memset(m_buffer, 0, sizePowerOfTwo * sizeof(float));
	}*/
	inline void init(const int size)
	{
		if (m_buffer != nullptr)
		{
			clearBuffer();
		}

		m_head = 0;
		const int sizePowerOfTwo = GetPowerOfTwo(size);
		m_bitMask = sizePowerOfTwo - 1;
		m_readOffset = m_bitMask - size;

		const size_t alignment = 32; // Use 16 for SSE, 32 for AVX
		const size_t allocSize = sizePowerOfTwo * sizeof(float);

#if defined(_MSC_VER) // Windows with MSVC
		m_buffer = static_cast<float*>(_aligned_malloc(allocSize, alignment));
		if (!m_buffer)
		{
			throw std::bad_alloc();
		}
#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__) // POSIX
		if (posix_memalign(reinterpret_cast<void**>(&m_buffer), alignment, allocSize) != 0)
		{
			m_buffer = nullptr;
			throw std::bad_alloc();
		}
#else
#error "Unknown platform: please define aligned allocation"
#endif

		std::memset(m_buffer, 0, allocSize);
	}
	inline void set(const int size) 
	{
		m_readOffset = m_bitMask - size + 1;
	};
	inline int getSize() const
	{
		return m_bitMask - m_readOffset;
	};
	inline void write(const float sample)
	{
		m_buffer[m_head] = sample;
		m_head = (m_head + 1) & m_bitMask;
	}
	inline float read() const
	{ 
		return m_buffer[(m_head + m_readOffset) & m_bitMask];
	};
	inline void release()
	{
		m_head = 0;
		
		clearBuffer();
	}
	inline float readDelay(const int sample) const noexcept
	{
		return m_buffer[(m_head - sample) & m_bitMask];
	}
	inline __m128 readDelaysSIMD128(const int* samples) const noexcept
	{
		alignas(16) float values[4];

		for (int i = 0; i < 4; ++i)
		{
			const int index = (m_head - samples[i]) & m_bitMask;
			values[i] = m_buffer[index];
		}

		return _mm_load_ps(values); // load 4 floats into SIMD register
	}
	// Helper to read 4 samples at once from the delay buffer
	inline __m128 readDelaySIMD128(int d0, int d1, int d2, int d3) const noexcept
	{
		alignas(16) float values[4];
		values[0] = readDelay(d0);
		values[1] = readDelay(d1);
		values[2] = readDelay(d2);
		values[3] = readDelay(d3);

		return _mm_load_ps(values); // Load 4 values into a single __m128 register
	}
	inline __m256 readDelaysSIMD256(const int* samples) const noexcept
	{
		alignas(32) float values[8];

		for (int i = 0; i < 8; ++i)
		{
			const int index = (m_head - samples[i]) & m_bitMask;
			values[i] = m_buffer[index];
		}

		return _mm256_load_ps(values); // load 8 floats into AVX register
	}
	inline float readDelayLinearInterpolation(const float sample)
	{
		const int sampleTrunc = (int)(sample);

		const int readIdx = m_head + m_bitMask - sampleTrunc;
		const float weight = sample - sampleTrunc;

		const int iPrev = readIdx & m_bitMask;
		const int iNext = (readIdx + 1) & m_bitMask;

		const float prev = m_buffer[iPrev];
		const float next = m_buffer[iNext];

		return next + weight * (prev - next);
	}
	float readDelayTriLinearInterpolation(const float sample)
	{
		const int sampleTrunc = (int)(sample);
		const int readIdx = m_head + m_bitMask - sampleTrunc;
		const float x = 1.0f - (sample - sampleTrunc);

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

		return (c2 * x + c1) * x + c0;
	}
	float readDelayHermiteCubicInterpolation(float sample)
	{
		const int sampleTrunc = (int)(sample);
		const int readIdx = m_head + m_bitMask - sampleTrunc;
		const float x = 1.0f - (sample - sampleTrunc);

		const int idx1 = (readIdx - 1) & m_bitMask;
		const int idx2 = readIdx & m_bitMask;
		const int idx3 = (readIdx + 1) & m_bitMask;
		const int idx4 = (readIdx + 2) & m_bitMask;

		const float yz1 = m_buffer[idx1];
		const float y0 = m_buffer[idx2];
		const float y1 = m_buffer[idx3];
		const float y2 = m_buffer[idx4];

		// 4-point, 3rd-order Hermite (x-form)
		float c0 = y0;
		float c1 = 1.0f / 2.0f * (y1 - yz1);
		float c2 = yz1 - 5.0f / 2.0f * y0 + 2.0f * y1 - 1.0f / 2.0f * y2;
		float c3 = 1.0f / 2.0f * (y2 - yz1) + 3.0f / 2.0f * (y0 - y1);

		return ((c3 * x + c2) * x + c1) * x + c0;
	}
	float readDelayOptimalCubicInterpolation(float sample)
	{
		const int sampleTrunc = (int)(sample);
		const int readIdx = m_head + m_bitMask - sampleTrunc;
		const float x = 1.0f - (sample - sampleTrunc);

		const int idx1 = (readIdx - 1) & m_bitMask;
		const int idx2 = readIdx & m_bitMask;
		const int idx3 = (readIdx + 1) & m_bitMask;
		const int idx4 = (readIdx + 2) & m_bitMask;

		const float yz1 = m_buffer[idx1];
		const float y0 = m_buffer[idx2];
		const float y1 = m_buffer[idx3];
		const float y2 = m_buffer[idx4];

		// Optimal 2x (4-point, 3rd-order) (z-form)
		float z = x - 1.0f / 2.0f;
		float even1 = y1 + y0, odd1 = y1 - y0;
		float even2 = y2 + yz1, odd2 = y2 - yz1;
		float c0 = even1 * 0.45868970870461956f + even2 * 0.04131401926395584f;
		float c1 = odd1 * 0.48068024766578432f + odd2 * 0.17577925564495955f;
		float c2 = even1 * -0.246185007019907091f + even2 * 0.24614027139700284f;
		float c3 = odd1 * -0.36030925263849456f + odd2 * 0.10174985775982505f;

		return ((c3 * z + c2) * z + c1) * z + c0;
	}

	inline float* getRawPointer() { return m_buffer; }
	inline const float* getRawPointer() const { return m_buffer; }
	inline int getHead() const { return m_head; }
	inline int getMask() const { return m_bitMask; }

private:
	inline int GetPowerOfTwo(int i)
	{
		int n = i - 1;

		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		n++;

		return n;
	}
	/*inline void clearBuffer()
	{
		delete[] m_buffer;
		m_buffer = nullptr;
	}*/
	inline void clearBuffer()
	{
		if (m_buffer)
		{
#if defined(_MSC_VER)
			_aligned_free(m_buffer);
#else
			free(m_buffer);
#endif
			m_buffer = nullptr;
		}
	}

	float* m_buffer = nullptr;
	int m_head = 0;
	int m_bitMask = 0;
	int m_readOffset = 0;
};