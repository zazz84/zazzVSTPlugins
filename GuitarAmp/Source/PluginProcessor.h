#pragma once

#include <array>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Filters/ThreeBandEQ.h"
#include "../../../zazzVSTPlugins/Shared/Filters/TrebleBooster.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Delays/CombFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/SpeakerCabinetSimulation.h"
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/WaveShapers.h"

//==============================================================================
class GuitarAmpAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    GuitarAmpAudioProcessor();
    ~GuitarAmpAudioProcessor() override;

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
	std::array<TrebleBooster, 2> m_preEQ = {};
	std::array<ThreeBandEQ, 2> m_preampEQ = {};

	std::array<BiquadFilter, 2> m_drivePreEQ = {};
	std::array<BiquadFilter, 2> m_drivePostEQ = {};

	std::array<TubeEmulation, 2> m_powerAmp = {};

	std::array<CombFilter, 2> m_combFilter = {};
	std::array<BiquadFilter, 2> m_combHPFilter = {};
	std::array<BiquadFilter, 2> m_combLPFilter = {};

	std::array<SpeakerCabineSimulation, 2> m_speakerCabinetSimulation = {};

	std::atomic<float>* gainParameter = nullptr;
	std::atomic<float>* bassParameter = nullptr;
	std::atomic<float>* midParameter = nullptr;
	std::atomic<float>* trebleParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GuitarAmpAudioProcessor)
};
