#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

#define M_PI  3.14159268f

//==============================================================================
class OnePoleFilter
{
public:
	void init(const float sampleRate)
	{
		m_sampleRate = sampleRate;
	};
	void set(const float frequency)
	{
		m_a0 = frequency * M_PI / m_sampleRate;
		m_b1 = 1.0f - m_a0;
	};
	float process(const float sample)
	{
		return m_sampleLast = m_a0 * sample + m_b1 * m_sampleLast;
	};

private:
	float m_sampleRate = 48000.0f;
	float m_sampleLast = 0.0f;
	float m_a0 = 1.0f;
	float m_b1 = 0.0f;
};

//==============================================================================
class FDNReverbAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    FDNReverbAudioProcessor();
    ~FDNReverbAudioProcessor() override;

	static const std::string paramsNames[];
	static const float MAXIMUM_DELAY_TIME;
	static const int DELAY_LINES_COUNT = 16;

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
	// Input size must be a power of 2.
	void FWHT(float (&data)[DELAY_LINES_COUNT])
	{
		int h = 1;

		// Iterative Fast Walsh-Hadamard Transform
		while (h < DELAY_LINES_COUNT)
		{
			for (int i = 0; i < DELAY_LINES_COUNT; i += h * 2)
			{
				for (int j = i; j < i + h; ++j)
				{
					float x = data[j];
					float y = data[j + h];
					data[j] = x + y;
					data[j + h] = x - y;
				}
			}

			// Normalize by sqrt(2) at each step
			for (int i = 0; i < DELAY_LINES_COUNT; ++i)
			{
				data[i] *= 0.707f;
			}

			h *= 2;
		}
	}
	
	//==============================================================================

	std::atomic<float>* sizeParameter = nullptr;
	std::atomic<float>* resonanceParameter = nullptr;
	std::atomic<float>* dampingParameter = nullptr;
	std::atomic<float>* colorParameter = nullptr;
	std::atomic<float>* widthParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	CircularBuffer m_buffer[2][DELAY_LINES_COUNT];
	OnePoleFilter m_filter[2][DELAY_LINES_COUNT];
	BiquadFilter m_lowShelf[2];
	BiquadFilter m_highShelf[2];
	BiquadFilter m_lowPass[2];
	float m_tmp[DELAY_LINES_COUNT];
	const int m_primeNumbers[DELAY_LINES_COUNT] = { 631, 7, 839, 83, 29, 757, 23, 887, 31, 941, 211, 719, 857, 569, 47, 991 };
	float m_delayTimes[DELAY_LINES_COUNT];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FDNReverbAudioProcessor)
};
