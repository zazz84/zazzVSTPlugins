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

	enum Type
	{
		LowPass,
		HighPass,
		BandPassSkirtGain,
		BandPassPeakGain,
		Notch,
		Peak,
		LowShelf,
		HighShelf,
		AllPass
	};

	enum Algorithm
	{
		DF1,
		DF2,
		DF1T,
		DF2T
	};

	inline void init(const int sampleRate) noexcept
	{
		m_samplePeriod = 1.0f / static_cast<float>(sampleRate);
	};
	inline void setLowPass(const float frequency, const float Q, const float gain = 0.0f)
	{
		const float omega = M_PI2 * frequency * m_samplePeriod;
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

		/*const double omega = 3.1415926535897932384626433832795028841971693993751058209749445923078164062 * (double)frequency * (double)m_samplePeriod;
		const double sn = sin(omega);
		const double cs = cos(omega);
		const double alpha = sn / (2.0 * (double)Q);

		const double tmp = 1.0 - cs;
		const double normalize = 1.0 / (1.0 + alpha);
		
		b0 = (float)(tmp * 0.5 * normalize);
		b1 = (float)(tmp * normalize);
		b2 = b0;
		a1 = (float)(-2.0 * cs * normalize);
		a2 = (float)((1.0 - alpha) * normalize);*/
	};
	inline void setHighPass(const float frequency, const float Q, const float gain = 0.0f) noexcept
	{
		const float omega = M_PI2 * frequency * m_samplePeriod;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);

		const float tmp = 1.0f + cs;
		b0 = tmp * 0.5f;
		b1 = -tmp;
		b2 = b0;
		a0 = 1.0f + alpha;
		a1 = -2.0f * cs;
		a2 = 1.0f - alpha;

		normalize();
	};
	inline void setBandPassSkirtGain(const float frequency, const float Q, const float gain = 0.0f) noexcept
	{
		const float omega = M_PI2 * frequency * m_samplePeriod;
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
	inline void setBandPassPeakGain(const float frequency, const float Q, const float gain = 0.0f) noexcept
	{
		const float omega = M_PI2 * frequency * m_samplePeriod;
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
	inline void setNotch(const float frequency, const float Q, const float gain = 0.0f) noexcept
	{
		const float omega = M_PI2 * frequency * m_samplePeriod;
		
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);

		b0 = 1.0f;
		b1 = -2.0f * cs;
		b2 = 1.0f;
		a0 = 1.0f + alpha;
		a1 = b1;
		a2 = 1.0f - alpha;

		normalize();
	};
	inline void setPeak(const float frequency, const float Q, const float gain) noexcept
	{
		const float A = powf(10.0f, gain / 40.0f);
		const float omega = M_PI2 * frequency * m_samplePeriod;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);
		const float beta = sqrt(A) / Q;

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
		const float omega = M_PI2 * frequency * m_samplePeriod;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);
		const float beta = sqrt(A) / Q;

		float tmp1 = A + 1.0f;
		float tmp2 = A - 1.0f;
		b0 = A * (tmp1 - tmp2 * cs + beta * sn);
		b1 = 2.0f * A * (tmp2 - tmp1 * cs);
		b2 = A * (tmp1 - tmp2 * cs - beta * sn);
		a0 = tmp1 + tmp2 * cs + beta * sn;
		a1 = -2.0f * (tmp2 + tmp1 * cs);
		a2 = tmp1 + tmp2 * cs - beta * sn;

		normalize();
	};
	inline void setHighShelf(const float frequency, const float Q, const float gain) noexcept
	{
		const float A = powf(10.0f, gain / 40.0f);
		const float omega = M_PI2 * frequency * m_samplePeriod;
		const float sn = sin(omega);
		const float cs = cos(omega);
		const float alpha = sn / (2.0f * Q);
		const float beta = sqrt(A) / Q;

		float tmp1 = A + 1.0f;
		float tmp2 = A - 1.0f;
		b0 = A * (tmp1 + tmp2 * cs + beta * sn);
		b1 = -2.0f * A * (tmp2 + tmp1 * cs);
		b2 = A * (tmp1 + tmp2 * cs - beta * sn);
		a0 = tmp1 - tmp2 * cs + beta * sn;
		a1 = 2.0f * (tmp2 - tmp1 * cs);
		a2 = tmp1 - tmp2 * cs - beta * sn;

		normalize();
	};
	inline void setAllPass(const float frequency, const float Q, const float gain = 0.0f) noexcept
	{
		const float omega = M_PI2 * frequency * m_samplePeriod;
		const float cs = cos(omega);
		const float sn = sin(omega);
		const float alpha = sn / (2.0f * Q);

		b0 = 1.0f - alpha;
		b1 = -2.0f * cs;
		b2 = 1.0f + alpha;
		a0 = b2;
		a1 = b1;
		a2 = b0;
		
		normalize();
	}

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
		reset();

		a0 = 0.0f;
		a1 = 0.0f;
		a2 = 0.0f;

		b0 = 0.0f;
		b1 = 0.0f;
		b2 = 0.0f;

		m_samplePeriod = 2.08e-5f;
	};
	// Resets samples history
	inline void reset() noexcept
	{
		x1 = 0.0f;
		x2 = 0.0f;

		y1 = 0.0f;
		y2 = 0.0f;
	};

	std::function<void(const float, const float, const float)> set;
	void setType(Type type)
	{
		switch (type)
		{
		case Type::LowPass:
			set = [this](float x, float y, float z) { return setLowPass(x, y, z); };
			break;

		case Type::HighPass:
			set = [this](float x, float y, float z) { return setHighPass(x, y, z); };
			break;

		case Type::BandPassSkirtGain:
			set = [this](float x, float y, float z) { return setBandPassSkirtGain(x, y, z); };
			break;

		case Type::BandPassPeakGain:
			set = [this](float x, float y, float z) { return setBandPassPeakGain(x, y, z); };
			break;

		case Type::Notch:
			set = [this](float x, float y, float z) { return setNotch(x, y, z); };
			break;

		case Type::Peak:
			set = [this](float x, float y, float z) { return setPeak(x, y, z); };
			break;

		case Type::LowShelf:
			set = [this](float x, float y, float z) { return setLowShelf(x, y, z); };
			break;

		case Type::HighShelf:
			set = [this](float x, float y, float z) { return setHighShelf(x, y, z); };
			break;

		case Type::AllPass:
			set = [this](float x, float y, float z) { return setAllPass(x, y, z); };
			break;
		}
	}

	std::function<float(const float)> process;
	void setAlgorithm(Algorithm algorithm)
	{
		switch (algorithm)
		{
		case Algorithm::DF1:
			process = [this](float x) { return processDF1(x); };
			break;

		case Algorithm::DF2:
			process = [this](float x) { return processDF2(x); };
			break;

		case Algorithm::DF1T:
			process = [this](float x) { return processDF1T(x); };
			break;

		case Algorithm::DF2T:
			process = [this](float x) { return processDF2T(x); };
			break;
		}
	}

private:
	inline void normalize() noexcept
	{
		/*b0 = b0 / a0;
		b1 = b1 / a0;
		b2 = b2 / a0;
		a1 = a1 / a0;
		a2 = a2 / a0;*/

		const float normalize = 1.0f / a0;
		b0 = b0 * normalize;
		b1 = b1 * normalize;
		b2 = b2 * normalize;
		a1 = a1 * normalize;
		a2 = a2 * normalize;
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

	float m_samplePeriod = 2.08e-5f;
};

class LowPassBiquadFilter
{
public:
	LowPassBiquadFilter() = default;
	~LowPassBiquadFilter() = default;

	inline void init(const int sampleRate) noexcept
	{
		m_samplePeriod = 1.0f / static_cast<float>(sampleRate);
	};
	inline void set(const float frequency, const float Q) noexcept
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
	};
	inline float process(const float in) noexcept
	{
		const float out = m_b0 * (in + m_x2) + m_b1 * m_x1 + m_a1 * m_y1 + m_a2 * m_y2;

		m_x2 = m_x1;
		m_x1 = in;

		m_y2 = m_y1;
		m_y1 = out;

		return out;
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
	float m_a1 = 0.0f;
	float m_a2 = 0.0f;

	float m_b0 = 0.0f;
	float m_b1 = 0.0f;

	float m_x1 = 0.0f;
	float m_x2 = 0.0f;

	float m_y1 = 0.0f;
	float m_y2 = 0.0f;

	float m_samplePeriod = 2.08e-5f;
};