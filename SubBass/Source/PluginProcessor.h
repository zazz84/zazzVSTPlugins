#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Random.h"

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

//==============================================================================
class PeakDetector
{
public:
	PeakDetector() {};

	inline void init(int sampleRate)
	{
		m_sampleRate = sampleRate;
	};
	inline void set(float releaseTimeMS)
	{
		m_decayCoefficient = std::exp(-1.0f / (0.001f * releaseTimeMS * (float)m_sampleRate));
	};
	inline float process(float in)
	{
		// Compute the absolute value of the input
		float absSample = std::fabs(in);

		// Update the peak value
		if (absSample > m_out)
			m_out = absSample; // Instant rise to the new peak value
		else
			m_out *= m_decayCoefficient; // Exponential decay

		return m_out;
	};

private:
	int m_sampleRate = 48000;
	float m_decayCoefficient = 0.0f;
	float m_out = 0.0f;
};

//==============================================================================
class KarplusStrongDelay
{
public:
	KarplusStrongDelay() {};

	inline void init(int sampleRate, float frequencyMin)
	{
		m_delay.init((int)((float)sampleRate / frequencyMin));
		m_filter.init(sampleRate);
		m_postFilter.init(sampleRate);
		m_sampleRate = sampleRate;

	};
	inline void set(float frequency, float lenght)
	{
		m_delay.setSize((int)((float)m_sampleRate / frequency));
		m_filter.setLowPass(2.0f * frequency, 0.7f);
		m_postFilter.setBandPassPeakGain(frequency, 1.0f);
		
		// Not sure this works correctly
		m_decayFactor = std::expf(-6.9078f / (frequency * lenght));
	};
	inline float process(float in)
	{
		const float delay = m_delay.read();
		const float filter = m_filter.processDF1(delay);
		const float mix = (in + filter) * m_decayFactor;
		m_delay.writeSample(mix);
		
		return m_postFilter.processDF1(filter);
	};

private:
	CircularBuffer m_delay;
	BiquadFilter m_filter;
	BiquadFilter m_postFilter;
	float m_decayFactor = 0.85f;
	int m_sampleRate = 48000;
};

//==============================================================================
class SubBassAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    SubBassAudioProcessor();
    ~SubBassAudioProcessor() override;

	static const std::string paramsNames[];
	static const std::string paramsUnitNames[];
	static const float FREQUENCY_MIN;
	static const float FREQUENCY_MAX;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:	
	//==============================================================================
	float remap(float value, float inMin, float inMax, float outMin, float outMax)
	{
		if (value <= inMin)
		{
			return outMin;
		}
		else if (value >= inMax)
		{
			return outMax;
		}
		else
		{
			return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
		}
	}
	
	//==============================================================================
	KarplusStrongDelay m_delay[2];
	BiquadFilter m_triggerFilter[2];
	PeakDetector m_peakDetector[2];

	int m_samplesOpen[2];

	std::atomic<float>* sidechainParameter = nullptr;
	std::atomic<float>* thresholdParameter = nullptr;
	std::atomic<float>* lenghtParameter = nullptr;
	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* amountParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SubBassAudioProcessor)
};
