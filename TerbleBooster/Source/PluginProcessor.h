#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/WaveShapers.h"

//==============================================================================
class MovingAverage
{
public:
	MovingAverage() {};
	void init(int maximumSize)
	{
		m_buffer.init(maximumSize);
	}
	void set(float size)
	{
		//m_buffer.setSize(size);
		m_size = size;
	}
	float process(float in)
	{
		m_buffer.writeSample(in);
		m_sum -= m_buffer.readDelayLinearInterpolation(m_size - 1.0f);
		m_sum += in;
		return m_sum / (float)m_size;
	}

private:
	CircularBuffer m_buffer;
	float m_size = 0;
	float m_sum = 0.0f;
};

//==============================================================================
class TerbleBoosterAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    TerbleBoosterAudioProcessor();
    ~TerbleBoosterAudioProcessor() override;

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

	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* sizeNParameter = nullptr;
	std::atomic<float>* noiseParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	MovingAverage m_filter[2] = {};

	float m_inLast[2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TerbleBoosterAudioProcessor)
};
