#pragma once

#include <array>

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Filters/SmallSpeakerSimulation.h"
#include "../../../zazzVSTPlugins/Shared/Reverbs/RoomEarlyReflection.h"

//==============================================================================
class SmallSpeakerSimulatorAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    SmallSpeakerSimulatorAudioProcessor();
    ~SmallSpeakerSimulatorAudioProcessor() override;

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
	std::array<SmallSpeakerSimulation, 2> m_spekaerSimulation;
	std::array<RoomEarlyReflections, 2> m_earlyReflections;

	std::atomic<float>* typeParameter = nullptr;
	std::atomic<float>* tuneParameter = nullptr;
	std::atomic<float>* resonanceParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmallSpeakerSimulatorAudioProcessor)
};
