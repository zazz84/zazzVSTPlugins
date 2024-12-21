/*
  ==============================================================================

    SmallSpeakerSimulation.h
    Created: 21 Dec 2024 10:11:19pm
    Author:  zazz

  ==============================================================================
*/

#pragma once

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

class SmallSpeakerSimulation
{
public:
	SmallSpeakerSimulation() {};
	inline void init(const int sampleRate)
	{
		m_filter1.init(sampleRate);
		m_filter2.init(sampleRate);
		m_filter3.init(sampleRate);
		m_filter4.init(sampleRate);
		m_filter5.init(sampleRate);
		m_filter6.init(sampleRate);
	};
	inline void set(const int type, const float tune)
	{
		const float tuneFactor = 0.01f * tune;

		if (type == 1)
		{
			m_gain = juce::Decibels::decibelsToGain(6.0f);

			m_filter1.setHighPass(tuneFactor * 330.0f, 0.707f);
			m_filter2.setHighPass(tuneFactor * 1300.0f, 1.0f);
			m_filter3.setPeak(tuneFactor * 330.0f, 0.7f, -4.5f);
			m_filter4.setLowPass(tuneFactor * 3900.0f, 0.52f);
			m_filter5.setLowPass(tuneFactor * 3900.0f, 0.71f);
			m_filter6.setLowPass(tuneFactor * 3900.0f, 1.93f);
		}
		else if (type == 2)
		{
			m_gain = juce::Decibels::decibelsToGain(3.0f);

			m_filter1.setHighPass(tuneFactor * 180.0f, 0.5f);
			m_filter2.setHighPass(tuneFactor * 660.0f, 3.5f);
			m_filter3.setPeak(tuneFactor * 1800.0f, 0.5f, -12.0f);
			m_filter4.setPeak(tuneFactor * 11000.0f, 8.0f, -9.0f);
			m_filter5.setLowPass(tuneFactor * 8000.0f, 1.0f);
			m_filter6.setLowPass(tuneFactor * 8000.0f, 2.0f);
		}
		else if (type == 3)
		{
			m_gain = juce::Decibels::decibelsToGain(3.0f);

			m_filter1.setHighPass(tuneFactor * 1100.0f, 2.1f);
			m_filter2.setHighPass(tuneFactor * 1200.0f, 1.1f);
			m_filter3.setPeak(tuneFactor * 3600.0f, 8.0f, 12.0f);
			m_filter4.setLowPass(tuneFactor * 2200.0f, 0.52f);
			m_filter5.setLowPass(tuneFactor * 2200.0f, 0.71f);
			m_filter6.setLowPass(tuneFactor * 2200.0f, 1.93f);
		}
		else if (type == 4)
		{
			m_gain = 1.0f;

			m_filter1.setHighPass(tuneFactor * 500.0f, 2.0f);
			m_filter2.setPeak(tuneFactor * 1000.0f, 2.0f, -9.0f);
			m_filter3.setPeak(tuneFactor * 3800.0f, 6.0f, -9.0f);
			m_filter4.setHighShelf(tuneFactor * 6000.0f, 0.7f, -12.0f);
			m_filter5.setLowPass(tuneFactor * 7000.0f, 1.5f);
			m_filter6.setLowPass(tuneFactor * 7000.0f, 4.3f);
		}
		else if (type == 5)
		{
			m_gain = juce::Decibels::decibelsToGain(-1.0f);

			m_filter1.setHighPass(tuneFactor * 680.0f, 0.65f);
			m_filter2.setPeak(tuneFactor * 360.0f, 2.8f, 18.0f);
			m_filter3.setPeak(tuneFactor * 680.0f, 1.0f, -6.0f);
			m_filter4.setPeak(tuneFactor * 4600.0f, 6.0f, -9.0f);
			m_filter5.setPeak(tuneFactor * 6500.0f, 6.0f, -12.0f);
			m_filter6.setLowPass(tuneFactor * 3000.0f, 4.3f);
		}
		else if (type == 6)
		{
			m_gain = juce::Decibels::decibelsToGain(-4.0f);

			m_filter1.setHighPass(tuneFactor * 330.0f, 1.0f);
			m_filter2.setHighPass(tuneFactor * 330.0f, 0.7f);
			m_filter3.setPeak(tuneFactor * 380.0f, 1.0f, 18.0f);
			m_filter4.setHighShelf(tuneFactor * 2400.0f, 1.0f, -12.0f);
			m_filter5.setLowPass(tuneFactor * 2400.0f, 3.0f);
			m_filter6.setLowPass(tuneFactor * 2500.0f, 1.0f);
		}
		else if (type == 7)
		{
			m_gain = juce::Decibels::decibelsToGain(11.0f);

			m_filter1.setHighPass(tuneFactor * 600.0f, 3.0f);
			m_filter2.setPeak(tuneFactor * 330.0f, 0.5f, -18.0f);
			m_filter3.setHighShelf(tuneFactor * 2100.0f, 1.0f, -12.0f);
			m_filter4.setLowPass(tuneFactor * 3000.0f, 0.52f);
			m_filter5.setLowPass(tuneFactor * 3000.0f, 0.71f);
			m_filter6.setLowPass(tuneFactor * 3000.0f, 1.93f);
		}
		else if (type == 8)
		{
			m_gain = juce::Decibels::decibelsToGain(-6.0f);

			m_filter1.setHighPass(tuneFactor * 500.0f, 2.0f);
			m_filter2.setPeak(tuneFactor * 560.0f, 1.0f, 9.0f);
			m_filter3.setHighShelf(tuneFactor * 710.0f, 10.0f, -9.0f);
			m_filter4.setLowPass(tuneFactor * 1800.0f, 2.0f);
			m_filter5.setLowPass(tuneFactor * 1800.0f, 2.0f);
			m_filter6.setLowPass(tuneFactor * 1800.0f, 4.0f);
		}
		else if (type == 9)
		{
			m_gain = juce::Decibels::decibelsToGain(-5.0f);

			m_filter1.setHighPass(tuneFactor * 540.0f, 2.5f);
			m_filter2.setHighPass(tuneFactor * 800.0f, 3.0f);
			m_filter3.setHighShelf(tuneFactor * 2000.0f, 10.0f, -18.0f);
			m_filter4.setLowPass(tuneFactor * 2600.0f, 2.0f);
			m_filter5.setLowPass(tuneFactor * 2600.0f, 2.0f);
			m_filter6.setLowPass(tuneFactor * 2600.0f, 4.0f);
		}
	}
	inline float process(const float in)
	{
		float out = m_filter1.processDF2T(in);
		out = m_filter2.processDF2T(out);
		out = m_filter3.processDF2T(out);
		out = m_filter4.processDF2T(out);
		out = m_filter5.processDF2T(out);
		return m_gain * m_filter6.processDF2T(out);
	};
	inline void release()
	{
		m_filter1.release();
		m_filter2.release();
		m_filter3.release();
		m_filter4.release();
		m_filter5.release();
		m_filter6.release();

		m_gain = 1.0f;
	};

private:
	BiquadFilter m_filter1, m_filter2, m_filter3, m_filter4, m_filter5, m_filter6;
	float m_gain = 1.0f;
};
