/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EnvelopeFollower.h"
#include "Filter.h"
#include "NoiseGenerator.h"

//==============================================================================
/**
*/
class NoiseEnhancerAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    NoiseEnhancerAudioProcessor();
    ~NoiseEnhancerAudioProcessor() override;

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

	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* thresholdParameter = nullptr;
	std::atomic<float>* attackParameter = nullptr;
	std::atomic<float>* releaseParameter = nullptr;
	std::atomic<float>* HPParameter = nullptr;
	std::atomic<float>* LPParameter = nullptr;
	std::atomic<float>* noiseParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	juce::AudioParameterBool* buttonSParameter = nullptr;
	juce::AudioParameterBool* button1Parameter = nullptr;
	juce::AudioParameterBool* button2Parameter = nullptr;
	juce::AudioParameterBool* buttonNoiseSParameter = nullptr;
	juce::AudioParameterBool* buttonDParameter = nullptr;

	TwoPoleBandPass m_envelopeFollowerFilter[2] = {};
	EnvelopeFollower m_triggerEnvelopeFollower[2] = {};
	EnvelopeFollower m_noiseEnvelopeFollower[2] = {};
	LowPassFilter12dB m_lowPassFilter[2] = {};
	HighPassFilter m_highPassFilter[2] = {};
	WhiteNoiseGenerator m_whiteNoiseGenerator[2] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseEnhancerAudioProcessor)
};
