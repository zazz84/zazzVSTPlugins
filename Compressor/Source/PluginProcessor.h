#pragma once

#include <array>

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

private:	
	//==============================================================================
	std::array<Compressor, N_CHANNELS> m_compressor;
	std::array<SlewCompressor, N_CHANNELS> m_slewCompressor;
	std::array<OptoCompressor, N_CHANNELS> m_optoCompressor;
	std::array<DualCompressor, N_CHANNELS> m_dualCompressor;
	std::array<AdaptiveCompressor, N_CHANNELS> m_adaptiveCompressor;

	std::atomic<float>* typeParameter = nullptr;
	std::atomic<float>* gainParameter = nullptr;
	std::atomic<float>* attackParameter = nullptr;
	std::atomic<float>* releaseParameter = nullptr;
	std::atomic<float>* ratioParameter = nullptr;
	std::atomic<float>* rmsParameter = nullptr;
	std::atomic<float>* linParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorAudioProcessor)
};