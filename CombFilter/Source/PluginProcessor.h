#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

//==============================================================================
class CombFilter
{
public:
	CombFilter() {};

	void init(const int size)
	{
		m_buffer.init(size);
	};
	void set(const float delay)
	{
		m_delay = delay;
	}
	float process(const float in)
	{
		m_buffer.writeSample(in);				
		return 0.5f * (in - m_buffer.readDelay((int)m_delay));
	}

private:
	CircularBuffer m_buffer = {};
	float m_delay = 0.0f;;
};

//==============================================================================
class CombFilterAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    CombFilterAudioProcessor();
    ~CombFilterAudioProcessor() override;

	static const std::string paramsNames[];
	static const std::string paramsUnitNames[];
	static const int STAGES_MAX = 24;
	static const int FREQUENY_MIN = 40;
	static const int FREQUENY_MAX = 10000;

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

	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* stagesParameter = nullptr;
	std::atomic<float>* lowCutParameter = nullptr;
	std::atomic<float>* highCutParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	CombFilter m_combFilter[STAGES_MAX][2];
	BiquadFilter m_lowCutFilter[2] = {};
	BiquadFilter m_highCutFilter[2] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CombFilterAudioProcessor)
};
