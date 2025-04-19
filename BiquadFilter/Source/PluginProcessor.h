#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

//==============================================================================
class BiquadFilterAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    BiquadFilterAudioProcessor();
    ~BiquadFilterAudioProcessor() override;

	static const int FREQUENCY_MIN = 20;
	static const int FREQUENCY_MAX = 20000;
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

	OnePoleLowPassFilter m_frequencySmoother;
	OnePoleLowPassFilter m_gainSmoother;
	OnePoleLowPassFilter m_QSmoother;
	OnePoleLowPassFilter m_mixSmoother;
	OnePoleLowPassFilter m_volumeSmoother;

	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* gainParameter = nullptr;
	std::atomic<float>* QParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	juce::AudioParameterBool* button1Parameter = nullptr;
	juce::AudioParameterBool* button2Parameter = nullptr;
	juce::AudioParameterBool* button3Parameter = nullptr;
	juce::AudioParameterBool* button4Parameter = nullptr;
	juce::AudioParameterBool* button5Parameter = nullptr;
	juce::AudioParameterBool* button6Parameter = nullptr;
	juce::AudioParameterBool* button7Parameter = nullptr;
	juce::AudioParameterBool* button8Parameter = nullptr;

	juce::AudioParameterBool* button9Parameter = nullptr;
	juce::AudioParameterBool* button10Parameter = nullptr;
	juce::AudioParameterBool* button11Parameter = nullptr;
	juce::AudioParameterBool* button12Parameter = nullptr;

	BiquadFilter m_filter[2] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BiquadFilterAudioProcessor)
};
