// https://www.hxaudiolab.com/uploads/2/5/5/3/25532092/cascading_biquads_to_create_even-order_highlow_pass_filters_2.pdf
// https://henquist.github.io/0.3.2/filterfunctions.html

#pragma once

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

// Using Butterworth q coeficients

//==============================================================================
class ForthOrderLowPassFilter
{
public:
	ForthOrderLowPassFilter() {};
	
	inline void init(const int sampleRate)
	{
		m_filter1.init(sampleRate);
		m_filter2.init(sampleRate);
	};
	inline void set(const float frequency)
	{
		m_filter1.setLowPass(frequency, 0.540f);
		m_filter2.setLowPass(frequency, 1.310f);
	};
	inline float process(const float in)
	{
		return m_filter1.processDF1(m_filter2.processDF1(in));
	};

private:
	BiquadFilter m_filter1, m_filter2;
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

private:
	BiquadFilter m_filter1, m_filter2, m_filter3, m_filter4;
};