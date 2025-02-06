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

#define M_LN2 0.69314718f
#define M_PI  3.14159268f
#define M_PI2 6.28318536f

class  BiquadFilter
{
public:
	BiquadFilter() = default;
	~BiquadFilter() = default;

	inline void init(const int sampleRate) noexcept
	{
		m_SampleRate = sampleRate;
	};
	inline void setLowPass(const float frequency, const float Q)
	{
		const float omega = M_PI2 * frequency / m_SampleRate;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);

		const float tmp = 1.0f - cs;
		b0 = tmp * 0.5f;
		b1 = tmp;
		b2 = b0;
		a0 = 1.0f + alpha;
		a1 = -2.0f * cs;
		a2 = 1.0f - alpha;

		normalize();
	};
	inline void setHighPass(const float frequency, const float Q) noexcept
	{
		const float omega = M_PI2 * frequency / m_SampleRate;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);

		const float tmp = 1 + cs;
		b0 = tmp * 0.5f;
		b1 = -tmp;
		b2 = b0;
		a0 = 1.0f + alpha;
		a1 = -2.0f * cs;
		a2 = 1.0f - alpha;

		normalize();
	};
	inline void setBandPassSkirtGain(const float frequency, const float Q) noexcept
	{
		const float omega = M_PI2 * frequency / m_SampleRate;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);

		b0 = Q * alpha;
		b1 = 0.0f;
		b2 = -Q * alpha;
		a0 = 1.0f + alpha;
		a1 = -2.0f * cs;
		a2 = 1.0f - alpha;

		normalize();
	};
	inline void setBandPassPeakGain(const float frequency, const float Q) noexcept
	{
		const float omega = M_PI2 * frequency / m_SampleRate;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);

		b0 = alpha;
		b1 = 0.0f;
		b2 = -alpha;
		a0 = 1.0f + alpha;
		a1 = -2.0f * cs;
		a2 = 1.0f - alpha;

		normalize();
	};
	inline void setNotch(const float frequency, const float Q) noexcept
	{
		const float omega = M_PI2 * frequency / m_SampleRate;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);

		b0 = 1.0f;
		b1 = -2.0f * cs;
		b2 = 1.0f;
		a0 = 1.0f + alpha;
		a1 = -2.0f * cs;
		a2 = 1.0f - alpha;

		normalize();
	};
	inline void setPeak(const float frequency, const float Q, const float gain) noexcept
	{
		const float A = powf(10.0f, gain / 40.0f);
		const float omega = M_PI2 * frequency / m_SampleRate;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);
		const float beta = sqrt(A + A);

		float tmp = alpha * A;
		b0 = 1.0f + tmp;
		b1 = -2.0f * cs;
		b2 = 1.0f - tmp;
		tmp = alpha / A;
		a0 = 1.0f + tmp;
		a1 = -2.0f * cs;
		a2 = 1.0f - tmp;

		normalize();
	};
	inline void setLowShelf(const float frequency, const float Q, const float gain) noexcept
	{
		const float A = powf(10.0f, gain / 40.0f);
		const float omega = M_PI2 * frequency / m_SampleRate;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);
		const float beta = sqrt(A + A);

		float tmp1 = A + 1.0f;
		float tmp2 = A - 1.0f;
		b0 = A * (tmp1 - tmp2 * cs + beta * sn);
		b1 = 2 * A * (tmp2 - tmp1 * cs);
		b2 = A * (tmp1 - tmp2 * cs - beta * sn);
		a0 = tmp1 + tmp2 * cs + beta * sn;
		a1 = -2 * (tmp2 + tmp1 * cs);
		a2 = tmp1 + tmp2 * cs - beta * sn;

		normalize();
	};
	inline void setHighShelf(const float frequency, const float Q, const float gain) noexcept
	{
		const float A = powf(10.0f, gain / 40.0f);
		const float omega = M_PI2 * frequency / m_SampleRate;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);
		const float beta = sqrt(A + A);

		float tmp1 = A + 1.0f;
		float tmp2 = A - 1.0f;
		b0 = A * (tmp1 + tmp2 * cs + beta * sn);
		b1 = -2 * A * (tmp2 + tmp1 * cs);
		b2 = A * (tmp1 + tmp2 * cs - beta * sn);
		a0 = tmp1 - tmp2 * cs + beta * sn;
		a1 = 2 * (tmp2 - tmp1 * cs);
		a2 = tmp1 - tmp2 * cs - beta * sn;

		normalize();
	};
	inline float processDF1(const float in) noexcept
	{
		const float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

		x2 = x1;
		x1 = in;

		y2 = y1;
		y1 = out;

		return out;
	};
	inline float processDF2(const float in)
	{
		const float v = in - a1 * y1 - a2 * y2;
		const float out = b0 * v + b1 * y1 + b2 * y2;

		y2 = y1;
		y1 = v;

		return out;
	};
	inline float processDF1T(const float in) noexcept
	{
		float v = in + y2;
		float out = b0 * v + x2;

		x2 = b1 * v + x1;
		y2 = -a1 * v + y1;
		x1 = b2 * v;
		y1 = -a2 * v;

		return out;
	};
	inline float processDF2T(const float in) noexcept
	{
		const float out = b0 * in + x2;

		/*x2 = b1 * in + x1 + a1 * out;
		x1 = b2 * in + a2 * out;*/

		x2 = b1 * in + x1 - a1 * out;
		x1 = b2 * in - a2 * out;

		return out;
	};
	inline void release() noexcept
	{
		a0 = 0.0f;
		a1 = 0.0f;
		a2 = 0.0f;

		b0 = 0.0f;
		b1 = 0.0f;
		b2 = 0.0f;

		x1 = 0.0f;
		x2 = 0.0f;

		y1 = 0.0f;
		y2 = 0.0f;

		m_SampleRate = 48000;
	};

private:
	inline void normalize() noexcept
	{
		b0 = b0 / a0;
		b1 = b1 / a0;
		b2 = b2 / a0;
		a1 = a1 / a0;
		a2 = a2 / a0;
	};
	
	float a0 = 0.0f;
	float a1 = 0.0f;
	float a2 = 0.0f;
	
	float b0 = 0.0f;
	float b1 = 0.0f;
	float b2 = 0.0f;
	
	float x1 = 0.0f;
	float x2 = 0.0f;

	float y1 = 0.0f;
	float y2 = 0.0f;

	int m_SampleRate = 48000;
};