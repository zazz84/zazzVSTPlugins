#pragma once

#include <array>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Random.h"

#include "../../../zazzVSTPlugins/Shared/Dynamics/PeakDetector.h"
#include "../../../zazzVSTPlugins/Shared/Delays/KarplusStrongDelay.h"

//==============================================================================
class SubBassAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
	enum SubBassParams
	{
		Sidechain,
		Threshold,
		Lenght,
		GlideLenght,
		StartFrequency,
		EndFrequency,
		Amount,
		Volume,
		COUNT
	};
	
	//==============================================================================
    SubBassAudioProcessor();
    ~SubBassAudioProcessor() override;

	static const std::string paramsNames[];
	static const std::string paramsUnitNames[];
	static const float FREQUENCY_MIN;
	static const float FREQUENCY_MAX;
	static const int N_SLIDERS = 8;

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
	std::array<std::atomic<bool>, N_SLIDERS> m_buttonState;

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
	int m_samplesFrequencyInterpolate[2];

	std::atomic<float>* sidechainParameter = nullptr;
	std::atomic<float>* thresholdParameter = nullptr;
	std::atomic<float>* lenghtParameter = nullptr;
	std::atomic<float>* glideLenghtParameter = nullptr;
	std::atomic<float>* startFrequencyParameter = nullptr;
	std::atomic<float>* endFrequencyParameter = nullptr;
	std::atomic<float>* amountParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SubBassAudioProcessor)
};
