/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/WaveShapers.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include <JuceHeader.h>

//==============================================================================
/**
*/
class DistortionAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    DistortionAudioProcessor();
    ~DistortionAudioProcessor() override;

	static const std::string paramsNames[];

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
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

	std::atomic<float>* driveParameter = nullptr;
	std::atomic<float>* asymetryParameter = nullptr;
	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* resonanceParameter = nullptr;
	std::atomic<float>* dynamicsParameter = nullptr;
	std::atomic<float>* foldbackParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	juce::AudioParameterBool* autoGainReductionParameter = nullptr;

	BiquadFilter m_lowPassFilter[2] = {};
	BiquadFilter m_highPassFilter[2] = {};
	ExponentialWaveShaper m_exponentialWaveShaper[2] = {};
	FoldBackWaveShaper m_foldBackWaveShaper[2] = {};
	SoftClipper m_clipper[2] = {};
	GainCompensation m_gainComponesation[2] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionAudioProcessor)
};
