#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Filters/AllPassFilters.h"

//==============================================================================
class EnvelopeClonerAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    EnvelopeClonerAudioProcessor();
    ~EnvelopeClonerAudioProcessor() override;

	static const std::string paramsNames[];
	static const float ENVELOPE_MINIMUM;
	static const float RATIO_LIMIT;

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

    std::atomic<float>* dynamicsParameter = nullptr;
    std::atomic<float>* spectrumParameter = nullptr;
    std::atomic<float>* attackParameter = nullptr;
    std::atomic<float>* releaseParameter = nullptr;
    std::atomic<float>* mixParameter = nullptr;
    std::atomic<float>* volumeParameter = nullptr;

    LinkwitzRileySecondOrder m_lowMidFilter[2] = {};
    LinkwitzRileySecondOrder m_midHighFilter[2] = {};
    FirstOrderAllPass m_allPassFilter[2] = {};

    LinkwitzRileySecondOrder m_lowMidFilterSC[2] = {};
    LinkwitzRileySecondOrder m_midHighFilterSC[2] = {};
    FirstOrderAllPass m_allPassFilterSC[2] = {};

    EnvelopeFollower m_envelopeDetectionFullSpectrum[2] = {};
    EnvelopeFollower m_envelopeDetectionFullSpectrumSC[2] = {};

    EnvelopeFollower m_envelopeDetection[2][3] = {};
    EnvelopeFollower m_envelopeDetectionSC[2][3] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeClonerAudioProcessor)
};
