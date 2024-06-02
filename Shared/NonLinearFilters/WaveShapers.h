#pragma once

constexpr auto SoftClipperThresholdPositive = 0.9f;
constexpr auto SoftClipperThresholdNegative = -0.9f;
constexpr auto SoftClipperThresholdRatio = 0.3f;		// for 48kHz

class  WaveShaper
{
public:
	WaveShaper() {};

	inline void setSaturation(float saturation) { m_saturation = saturation; };

	inline float processARRY(float in)
	{
		return 1.5f * in * (1.0f - in * in / 3.0f);
	};

private:
	float m_saturation = 1.0f;
};

class SoftClipper
{
public:
	SoftClipper() {};

	inline void init(int sampleRate)
	{
		m_softClipperThresholdRatio = SoftClipperThresholdRatio * 48000.0f / (float)sampleRate;
	};

	inline float process(float in)
	{
		float out = 0.0f;

		if (in > SoftClipperThresholdPositive)
		{
			out = SoftClipperThresholdPositive + (in - SoftClipperThresholdPositive) * m_softClipperThresholdRatio;
		}
		else if (in < SoftClipperThresholdNegative)
		{
			out = SoftClipperThresholdNegative + (in - SoftClipperThresholdNegative) * m_softClipperThresholdRatio;
		}
		else
		{
			out = in;
		}

		return fmaxf(-1.0f, fminf(1.0f, out));
	};

private:
	float m_softClipperThresholdRatio = SoftClipperThresholdRatio;
};


class TubeEmulation
{
public:
	TubeEmulation() {};

	inline void init(int sampleRate)
	{ 
		m_softClipper.init(sampleRate);
	};

	inline void setDrive(float drive) { m_drive = juce::Decibels::decibelsToGain(drive); };

	inline float process(float in)
	{
		float out = m_softClipper.process(in * m_drive);
		out = 0.675f * m_waveShaper.processARRY(out);
		return out;
	};

private:
	float m_drive;
	WaveShaper m_waveShaper;
	SoftClipper m_softClipper;
};