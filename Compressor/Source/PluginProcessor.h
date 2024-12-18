#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Dynamics/Compressors.h"

//==============================================================================
class CompressorAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    CompressorAudioProcessor();
    ~CompressorAudioProcessor() override;

	static const std::string paramsNames[];

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

	std::atomic<float>* gainParameter = nullptr;
	std::atomic<float>* attackParameter = nullptr;
	std::atomic<float>* releaseParameter = nullptr;
	std::atomic<float>* ratioParameter = nullptr;
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

	Compressor m_compressor[2] = {};
	SlewCompressor m_slewCompressor[2] = {};
	OptoCompressor m_optoCompressor[2] = {};
	DualCompressor m_dualCompressor[2] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorAudioProcessor)
};
