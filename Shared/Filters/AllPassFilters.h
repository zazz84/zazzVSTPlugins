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

	void init(int sampleRate);
	void setFrequency(float frequency, float Q);
	float process(float in);

protected:
	float m_SampleRate;

	float m_xnz2 = 0.0f;
	float m_xnz1 = 0.0f;
	float m_ynz2 = 0.0f;
	float m_ynz1 = 0.0f;

	float m_a0 = 0.0f;
	float m_a1 = 0.0f;
};