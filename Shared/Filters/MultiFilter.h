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

#define M_PI2 6.28318536f

class SecondOrderMultiFilter
{
public:
	SecondOrderMultiFilter() = default;
	~SecondOrderMultiFilter() = default;

	inline void init(const int sampleRate) noexcept
	{
		m_samplePeriod = 1.0f / static_cast<float>(sampleRate);
	};
	__forceinline void set(const float frequency, const float Q = 0.5f) noexcept
	{
		const float omega = M_PI2 * frequency * m_samplePeriod;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);

		m_b1 = 1.0f - cs;
		const float a0 = 1.0f + alpha;
		m_a1 = 2.0f * cs;
		m_a2 = alpha - 1.0f;

		const float normalize = 1.0f / a0;
		
		m_b1 *= normalize;
		m_b0 = 0.5f * m_b1;
		m_a1 *= normalize;
		m_a2 *= normalize;
	}
	__forceinline void process(const float in, float& __restrict lpOut, float& __restrict hpOut) noexcept
	{
		lpOut = m_b0 * (in + m_x2) + m_b1 * m_x1 + m_a1 * m_y1 + m_a2 * m_y2;

		m_x2 = m_x1;
		m_x1 = in;

		m_y2 = m_y1;
		m_y1 = lpOut;

		hpOut = in - lpOut;
	};
	inline void release() noexcept
	{
		m_a1 = 0.0f;
		m_a2 = 0.0f;

		m_b0 = 0.0f;
		m_b1 = 0.0f;

		m_x1 = 0.0f;
		m_x2 = 0.0f;

		m_y1 = 0.0f;
		m_y2 = 0.0f;

		m_samplePeriod = 2.08e-5f;
	};

private:
	// Coefficients (rarely changed after set())
	float m_b0 = 0.0f;
	float m_b1 = 0.0f;
	float m_a1 = 0.0f;
	float m_a2 = 0.0f;

	// State (updated every sample)
	float m_x1 = 0.0f;
	float m_x2 = 0.0f;
	float m_y1 = 0.0f;
	float m_y2 = 0.0f;

	float m_samplePeriod = 2.08e-5f;
};
