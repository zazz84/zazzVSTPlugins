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

#define PI 3.141592653589793f

 //==============================================================================
class FirstOrderAllPass
{
public:
	FirstOrderAllPass() = default;
	~FirstOrderAllPass() = default;

	inline void init(const int sampleRate)
	{
		m_SampleRate = sampleRate;
	}
	inline void set(const float frequency)
	{
		const float tmp = tanf(PI * frequency / m_SampleRate);
		m_a1 = (tmp - 1.0f) / (tmp + 1.0f);
	}
	inline float process(const float in)
	{
		const float tmp = m_a1 * in + m_d;
		m_d = in - m_a1 * tmp;
		return tmp;
	}

protected:
	float m_a1 = -1.0f; // all pass filter coeficient
	float m_d = 0.0f;   // history d = x[n-1] - a1y[n-1]
	int m_SampleRate;
};
 
//==============================================================================
class SecondOrderAllPass
{
public:
	SecondOrderAllPass() = default;
	~SecondOrderAllPass() = default;

	inline void init(int sampleRate) noexcept
	{
		m_SampleRate = sampleRate;
	}
	inline void setFrequency(float frequency, float Q) noexcept
	{
		const float w = 2.0f * PI * frequency / m_SampleRate;
		const float cosw = cos(w);
		const float alpha = sin(w) * (2.0f * Q);

		const float a2 = 1.0f / (1.0f + alpha);

		m_a0 = (1.0f - alpha) * a2;
		m_a1 = (-2.0f * cosw) * a2;
	}
	inline float process(float in) noexcept
	{
		const float yn = m_a0 * (in - m_ynz2) + m_a1 * (m_xnz1 - m_ynz1) + m_xnz2;

		m_xnz2 = m_xnz1;
		m_xnz1 = in;
		m_ynz2 = m_ynz1;
		m_ynz1 = yn;

		return yn;
	}

protected:
	float m_SampleRate;

	float m_xnz2 = 0.0f;
	float m_xnz1 = 0.0f;
	float m_ynz2 = 0.0f;
	float m_ynz1 = 0.0f;

	float m_a0 = 0.0f;
	float m_a1 = 0.0f;
};

//==============================================================================
class SecondOrderAllPassMulti
{
public:
	SecondOrderAllPassMulti() = default;
	~SecondOrderAllPassMulti() = default;

	struct Data
	{
		float m_xnz2 = 0.0f;
		float m_xnz1 = 0.0f;
		float m_ynz2 = 0.0f;
		float m_ynz1 = 0.0f;
	};

	static const int N_ALL_PASS_SO = 50;

	inline void init(int sampleRate) noexcept
	{
		m_sampleRate = sampleRate;
	}
	inline void set(float frequency, float Q, float count) noexcept
	{
		const float w = 2.0f * PI * frequency / (float)m_sampleRate;
		const float cosw = cos(w);
		const float alpha = sin(w) * (2.0f * Q);

		const float a2 = 1.0f / (1.0f + alpha);

		m_a0 = (1.0f - alpha) * a2;
		m_a1 = -2.0f * cosw * a2;

		m_count = count;
	}
	inline float process(float in) noexcept
	{
		const float a0 = m_a0;
		const float a1 = m_a1;

		float yn = in;

		for (int i = 0; i < m_count; i++)
		{
			const float temp = yn;

			Data& data = m_data[i];
			yn = a0 * (yn - data.m_ynz2) + a1 * (data.m_xnz1 - data.m_ynz1) + data.m_xnz2;

			data.m_xnz2 = data.m_xnz1;
			data.m_xnz1 = temp;
			data.m_ynz2 = data.m_ynz1;
			data.m_ynz1 = yn;
		}
		
		return yn;
	}

protected:
	alignas(64) Data m_data[N_ALL_PASS_SO];

	float m_a0 = 0.0f;
	float m_a1 = 0.0f;

	int m_sampleRate = 48000;
	int m_count = 25;
};

//==============================================================================
// For some reason, phase characteristic is different than the implementation above

/*class SecondOrderAllPassMulti
{
public:
	SecondOrderAllPassMulti() = default;
	~SecondOrderAllPassMulti() = default;

	struct Data
	{
		float x1 = 0.0f;
		float x2 = 0.0f;
	};

	static const int N_ALL_PASS_SO = 50;

	inline void init(int sampleRate) noexcept
	{
		m_samplePeriod = 1.0f / (float)sampleRate;
	}
	inline void set(float frequency, float Q, float count) noexcept
	{
		const float omega = (2.0f * PI) * frequency * m_samplePeriod;
		const float cs = cos(omega);
		const float sn = sin(omega);
		const float alpha = sn / (2.0f * Q);

		b0 = 1.0f - alpha;
		b1 = -2.0f * cs;
		b2 = 1.0f + alpha;
		float a0 = b2;
		//float a1 = b1;
		//float a2 = 1.0f - alpha;

		const float normalize = 1.0f / a0;
		b0 = b0 * normalize;
		b1 = b1 * normalize;
		b2 = b2 * normalize;
		//a1 = b1;
		//a2 = b0;

		m_count = count;
	}
	inline float process(float in) noexcept
	{
		float yn = in;

		for (int i = 0; i < m_count; i++)
		{
			Data& data = m_data[i];
			
			const float out = b0 * yn + data.x2;

			data.x2 = b1 * yn + data.x1 - b1 * out;
			data.x1 = b2 * yn - b0 * out;

			yn = out;
		}

		return yn;
	}

protected:
	Data m_data[N_ALL_PASS_SO];

	float b0, b1, b2;

	float m_samplePeriod = 1.0f;
	int m_count = 25;
};*/