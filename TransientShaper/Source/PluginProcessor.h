#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

//==============================================================================
class TransientShaperAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    TransientShaperAudioProcessor();
    ~TransientShaperAudioProcessor() override;

	static const std::string paramsNames[];
	static const std::string paramsUnitNames[];

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
	BranchingSmoothEnvelopeFollower m_envelopeFollowerSlow[2];
	BranchingSmoothEnvelopeFollower m_envelopeFollowerFast[2];

	std::atomic<float>* attackTimeParameter = nullptr;
	std::atomic<float>* attackParameter = nullptr;
	std::atomic<float>* sustainTimeParameter = nullptr;
	std::atomic<float>* sustainParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransientShaperAudioProcessor)
};
