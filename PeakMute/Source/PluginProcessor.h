#pragma once

#include <array>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/Clippers.h"
#include "../../../zazzVSTPlugins/Shared/Oscillators/SinOscillator.h"

//==============================================================================
class Envelope
{
public:
	Envelope() {};

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
		m_phase = 100 * sampleRate;
	};
	inline void set(const float attackMS, const float sustainMS, const float releaseMS)
	{
		m_attackSamples = (int)((float)m_sampleRate * 0.001f * attackMS);
		m_sustainSamples = m_attackSamples + (int)((float)m_sampleRate * 0.001f * sustainMS);
		m_releaseSamples = m_sustainSamples + (int)((float)m_sampleRate * 0.001f * releaseMS);
	};
	inline float process()
	{
		float out = 0.0f;
		
		if (m_phase < m_attackSamples)
		{
			out = (float)m_phase / m_attackSamples;
		}
		else if (m_phase < m_sustainSamples)
		{
			out = 1.0f;
		}
		else if (m_phase < m_releaseSamples)
		{
			out = 1.0f - (float)(m_phase - m_sustainSamples) / (float)(m_releaseSamples - m_sustainSamples);
		}

		m_phase++;

		return out;
	};
	inline void release()
	{
		m_attackSamples = 0;
		m_sustainSamples = 0;
		m_releaseSamples = 0;
		m_sampleRate = 48000;
	};
	inline void reset()
	{
		m_phase = 0;
	};
	inline void resetSustain()
	{
		m_phase = m_attackSamples;
	};
	inline bool finnished()
	{
		return m_phase >= m_releaseSamples;
	}
	inline bool attackFinnished()
	{
		return m_phase >= m_attackSamples;
	}
	inline bool sustainFinnished()
	{
		return m_phase >= m_sustainSamples;
	}

private:
	int m_phase = 0;
	int m_attackSamples = 0;
	int m_sustainSamples = 0;
	int m_releaseSamples = 0;
	int m_sampleRate = 48000;
};


//==============================================================================
class PeakMuteAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    PeakMuteAudioProcessor();
    ~PeakMuteAudioProcessor() override;

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
	std::array<Envelope, 2> m_envelope;
	std::array<SinOscillator, 2> m_oscillator;

	std::atomic<float>* thresholdParameter = nullptr;
	std::atomic<float>* attenuateParameter = nullptr;
	std::atomic<float>* recoveryParameter = nullptr;
	std::atomic<float>* warningParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PeakMuteAudioProcessor)
};
