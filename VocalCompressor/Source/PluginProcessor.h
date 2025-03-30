#pragma once

#include <array>

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Dynamics/VocalCompressor.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/VocalCompressorClean.h"

//==============================================================================
class VocalCompressorAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    VocalCompressorAudioProcessor();
    ~VocalCompressorAudioProcessor() override;

	static const std::string paramsNames[];
	static const std::string labelNames[];
	static const std::string paramsUnitNames[];
	static const int N_CHANNELS = 2;

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

	float getPeakReductiondB()
	{
		const float inputMaxdB = juce::Decibels::gainToDecibels(m_inputMax);
		const float outputMaxdB = juce::Decibels::gainToDecibels(m_outputMax);

		m_inputMax = 0.0f;
		m_outputMax = 0.0f;

		return inputMaxdB - outputMaxdB;
	}

private:	
	//==============================================================================
	std::array<VocalCompressor, N_CHANNELS> m_vocalCompressor;
	std::array<VocalCompressorClean, N_CHANNELS> m_vocalCompressorClean;

	std::atomic<float>* gainParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;
	std::atomic<float>* typeParameter = nullptr;

	float m_inputMax = 0.0f;
	float m_outputMax = 0.0f;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalCompressorAudioProcessor)
};
