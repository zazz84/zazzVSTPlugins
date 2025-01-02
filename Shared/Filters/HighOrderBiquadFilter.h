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

// https://www.hxaudiolab.com/uploads/2/5/5/3/25532092/cascading_biquads_to_create_even-order_highlow_pass_filters_2.pdf
// https://henquist.github.io/0.3.2/filterfunctions.html

#pragma once

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

// Using Butterworth q coeficients



//==============================================================================
class SecondOrderLowPassFilter
{
public:
	SecondOrderLowPassFilter() {};
	
	inline void init(const int sampleRate)
	{
		m_filter1.init(sampleRate);
	};
	inline void set(const float frequency)
	{
		m_filter1.setLowPass(frequency, 0.5f);
	};
	inline float process(const float in)
	{
		return m_filter1.processDF1(in);
	};
	inline void release()
	{
		m_filter1.release();
	};

private:
	BiquadFilter m_filter1;
};

//==============================================================================
class ForthOrderHighPassFilter
{
public:
	ForthOrderHighPassFilter() {};

	inline void init(const int sampleRate)
	{
		m_filter1.init(sampleRate);
		m_filter2.init(sampleRate);
	};
	inline void set(const float frequency)
	{
		m_filter1.setHighPass(frequency, 0.540f);
		m_filter2.setHighPass(frequency, 1.310f);
	};
	inline float process(const float in)
	{
		return m_filter1.processDF1(m_filter2.processDF1(in));
	};
	inline void release()
	{
		m_filter1.release();
		m_filter2.release();
	};

private:
	BiquadFilter m_filter1, m_filter2;
};

//==============================================================================
class SixthOrderLowPassFilter
{
public:
	SixthOrderLowPassFilter() {};

	inline void init(const int sampleRate)
	{
		m_filter1.init(sampleRate);
		m_filter2.init(sampleRate);
		m_filter3.init(sampleRate);
	};
	inline void set(const float frequency)
	{
		m_filter1.setLowPass(frequency, 0.520f);
		m_filter2.setLowPass(frequency, 0.710f);
		m_filter3.setLowPass(frequency, 1.930f);
	};
	inline float process(const float in)
	{
		return m_filter1.processDF1(m_filter2.processDF1(m_filter3.processDF1(in)));
	};
	inline void release()
	{
		m_filter1.release();
		m_filter2.release();
		m_filter3.release();
	};

private:
	BiquadFilter m_filter1, m_filter2, m_filter3;
};

//==============================================================================
class SixthOrderHighPassFilter
{
public:
	SixthOrderHighPassFilter() {};

	inline void init(const int sampleRate)
	{
		m_filter1.init(sampleRate);
		m_filter2.init(sampleRate);
		m_filter3.init(sampleRate);
	};
	inline void set(const float frequency)
	{
		m_filter1.setHighPass(frequency, 0.520f);
		m_filter2.setHighPass(frequency, 0.710f);
		m_filter3.setHighPass(frequency, 1.930f);
	};
	inline float process(const float in)
	{
		return m_filter1.processDF1(m_filter2.processDF1(m_filter3.processDF1(in)));
	};
	inline void release()
	{
		m_filter1.release();
		m_filter2.release();
		m_filter3.release();
	};

private:
	BiquadFilter m_filter1, m_filter2, m_filter3;
};

//==============================================================================
class EighthOrderLowPassFilter
{
public:
	EighthOrderLowPassFilter() {};

	inline void init(const int sampleRate)
	{
		m_filter1.init(sampleRate);
		m_filter2.init(sampleRate);
		m_filter3.init(sampleRate);
		m_filter4.init(sampleRate);
	};
	inline void set(const float frequency)
	{
		m_filter1.setLowPass(frequency, 0.510f);
		m_filter2.setLowPass(frequency, 0.600f);
		m_filter3.setLowPass(frequency, 0.900f);
		m_filter4.setLowPass(frequency, 2.560f);
	};
	inline float process(const float in)
	{
		return m_filter1.processDF1(m_filter2.processDF1(m_filter3.processDF1(m_filter4.processDF1(in))));
	};
	inline void release()
	{
		m_filter1.release();
		m_filter2.release();
		m_filter3.release();
		m_filter4.release();
	};

private:
	BiquadFilter m_filter1, m_filter2, m_filter3, m_filter4;
};

//==============================================================================
class EighthOrderHighPassFilter
{
public:
	EighthOrderHighPassFilter() {};

	inline void init(const int sampleRate)
	{
		m_filter1.init(sampleRate);
		m_filter2.init(sampleRate);
		m_filter3.init(sampleRate);
		m_filter4.init(sampleRate);
	};
	inline void set(const float frequency)
	{
		m_filter1.setHighPass(frequency, 0.510f);
		m_filter2.setHighPass(frequency, 0.600f);
		m_filter3.setHighPass(frequency, 0.900f);
		m_filter4.setHighPass(frequency, 2.560f);
	};
	inline float process(const float in)
	{
		return m_filter1.processDF1(m_filter2.processDF1(m_filter3.processDF1(m_filter4.processDF1(in))));
	};
	inline void release()
	{
		m_filter1.release();
		m_filter2.release();
		m_filter3.release();
		m_filter4.release();
	};

private:
	BiquadFilter m_filter1, m_filter2, m_filter3, m_filter4;
};

//==============================================================================
class MultiOrderHighPassFilter
{
public:
	enum FilterType
	{
		LowPass,
		HighPass
	};

	MultiOrderHighPassFilter() {};

	inline void init(const int sampleRate)
	{
		m_filter[0].init(sampleRate);
		m_filter[1].init(sampleRate);
		m_filter[2].init(sampleRate);
		m_filter[3].init(sampleRate);
	};
	inline void set(const float frequency, int order,  FilterType type)
	{
		m_orderHalf = order / 2;

		if (type == FilterType::LowPass)
		{
			if (order == 8)
			{
				m_filter[0].setLowPass(frequency, 0.510f);
				m_filter[1].setLowPass(frequency, 0.600f);
				m_filter[2].setLowPass(frequency, 0.900f);
				m_filter[3].setLowPass(frequency, 2.560f);
			}
			else if (order == 6)
			{
				m_filter[0].setLowPass(frequency, 0.520f);
				m_filter[1].setLowPass(frequency, 0.710f);
				m_filter[2].setLowPass(frequency, 1.930f);
			}
			else if (order == 4)
			{
				m_filter[0].setLowPass(frequency, 0.540f);
				m_filter[1].setLowPass(frequency, 1.310f);
			}
			else
			{
				m_filter[0].setLowPass(frequency, 0.710f);
			}
		}
		else
		{
			if (order == 8)
			{
				m_filter[0].setHighPass(frequency, 0.510f);
				m_filter[1].setHighPass(frequency, 0.600f);
				m_filter[2].setHighPass(frequency, 0.900f);
				m_filter[3].setHighPass(frequency, 2.560f);
			}
			else if (order == 6)
			{
				m_filter[0].setHighPass(frequency, 0.520f);
				m_filter[1].setHighPass(frequency, 0.710f);
				m_filter[2].setHighPass(frequency, 1.930f);
			}
			else if (order == 4)
			{
				m_filter[0].setHighPass(frequency, 0.540f);
				m_filter[1].setHighPass(frequency, 1.310f);
			}
			else
			{
				m_filter[0].setHighPass(frequency, 0.710f);
			}
		}

	};
	inline float process(const float in)
	{
		float output = in;
		
		for (int i = 0; i < m_orderHalf; i++)
		{
			output = m_filter[i].processDF1(output);
		}

		return output;
	};
	inline void release()
	{
		m_filter[0].release();
		m_filter[1].release();
		m_filter[2].release();
		m_filter[3].release();
	};

private:
	BiquadFilter m_filter[4];
	int m_orderHalf = 1;
};