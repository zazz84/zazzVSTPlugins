#pragma once

#include <array>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Filters/HighOrderBiquadFilter.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Oversampling.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/PeakDetector.h"
#include "../../../zazzVSTPlugins/Shared/Oscillators/SinOscillator.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Random.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

//==============================================================================
class AMTransmissionEmulationAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    AMTransmissionEmulationAudioProcessor();
    ~AMTransmissionEmulationAudioProcessor() override;

	static const std::string paramsNames[];
	static const std::string paramsUnitNames[];
	static const int OVERSAMPLING_RATIO = 12;

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
	std::array<Oversampling, 2> m_oversampling;
	std::array<SinOscillator, 2> m_oscillator;	
	std::array<DecoupeledEnvelopeFollower<float>, 2> m_diodeDetector;
	std::array<ForthOrderHighPassFilter, 2> m_DCFilter;
	
	std::array<EighthOrderLowPassFilter, 2> m_lowPassFilter;
	std::array<EighthOrderLowPassFilter, 2> m_lowPassPostFilter;

	std::array<BiquadFilter, 2> m_tuningFilter;
	std::array<OnePoleLowPassFilter, 2> m_tuningFrequencySmoother;
	std::array<BiquadFilter, 2> m_noiseFilter;
	
	
	LinearCongruentialRandom01 m_noise;



	
	std::atomic<float>* modulatioDepthParameter = nullptr;
	std::atomic<float>* releaseParameter = nullptr;
	std::atomic<float>* tuneParameter = nullptr;
	std::atomic<float>* noiseParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AMTransmissionEmulationAudioProcessor)
};
