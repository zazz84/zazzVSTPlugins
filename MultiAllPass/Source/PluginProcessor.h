#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Filters/AllPassFilters.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

//==============================================================================
class MultiAllPassAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    MultiAllPassAudioProcessor();
    ~MultiAllPassAudioProcessor() override;

	static const int N_ALL_PASS_SO = 50;
	static const int FREQUENCY_MIN = 40;
	static const int FREQUENCY_MAX = 18000;
	static const int N_CHANNELS = 2;
	static const std::string paramsNames[];
	static const std::string labelNames[];
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
	SecondOrderAllPassMulti m_secondOrderAllPassMulti[N_CHANNELS] = {};

	OnePoleLowPassFilter m_frequencySmoother;
	OnePoleLowPassFilter m_styleSmoother;
	
	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* styleParameter = nullptr;
	std::atomic<float>* intensityParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiAllPassAudioProcessor)
};
