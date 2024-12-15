#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Delays/AmbientDelay.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Helpers.h"

//==============================================================================
class AmbientDelayAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    AmbientDelayAudioProcessor();
    ~AmbientDelayAudioProcessor() override;

	static const std::string paramsNames[];
	static const std::string paramsUnitNames[];
	static const int N_CHANNELS = 2;
	static const int N_DELAY_LINES = 4;
	static const int MAXIMUM_DISTANCE = 400;

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
	AmbientDelay m_ambientDelay[N_DELAY_LINES];

	std::atomic<float>* distance1Parameter = nullptr;
	std::atomic<float>* pan1Parameter = nullptr;
	std::atomic<float>* distance2Parameter = nullptr;
	std::atomic<float>* pan2Parameter = nullptr;
	std::atomic<float>* distance3Parameter = nullptr;
	std::atomic<float>* pan3Parameter = nullptr;
	std::atomic<float>* distance4Parameter = nullptr;
	std::atomic<float>* pan4Parameter = nullptr;
	std::atomic<float>* attackMinParameter = nullptr;
	std::atomic<float>* attackMaxParameter = nullptr;
	std::atomic<float>* attackTimeMinParameter = nullptr;
	std::atomic<float>* attackTimeMaxParameter = nullptr;
	std::atomic<float>* sustainMinParameter = nullptr;
	std::atomic<float>* sustainMaxParameter = nullptr;
	std::atomic<float>* clipMinParameter = nullptr;
	std::atomic<float>* clipMaxParameter = nullptr;
	std::atomic<float>* LPFreqMinParameter = nullptr;
	std::atomic<float>* LPFreqMaxParameter = nullptr;
	std::atomic<float>* HPFreqMinParameter = nullptr;
	std::atomic<float>* HPFreqMaxParameter = nullptr;
	std::atomic<float>* distanceFactorParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmbientDelayAudioProcessor)
};
