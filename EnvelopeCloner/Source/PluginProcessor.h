#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/RMS.h"
#include "../../../zazzVSTPlugins/Shared/Filters/LinkwitzRileyFilter.h"
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
	static const std::string labelNames[];
	static const std::string paramsUnitNames[];
	static const int N_CHANNLES = 2;
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
	inline void getMaxGain(float& low, float& mid, float& high)
	{
		low = m_maxGainLow;
		mid =  m_maxGainMid;
		high = m_maxGainHigh;

		m_maxGainLow = 0.0f;
		m_maxGainMid = 0.0f;
		m_maxGainHigh = 0.0f;
	}
	
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

	LinkwitzRileyFilter m_lowMidFilter[N_CHANNLES] = {};
	LinkwitzRileyFilter m_midHighFilter[N_CHANNLES] = {};
    FirstOrderAllPass m_allPassFilter[N_CHANNLES] = {};

	LinkwitzRileyFilter m_lowMidFilterSC[N_CHANNLES] = {};
	LinkwitzRileyFilter m_midHighFilterSC[N_CHANNLES] = {};
    FirstOrderAllPass m_allPassFilterSC[N_CHANNLES] = {};

	RMS m_rmsFullSpectrum[N_CHANNLES];
	RMS m_rmsFullSpectrumSC[N_CHANNLES];

	RMS m_rmsBands[N_CHANNLES][3] = {};
	RMS m_rmsBandsSC[N_CHANNLES][3] = {};

	BranchingEnvelopeFollower<float> m_envelopeDetection[N_CHANNLES][3] = {};

	float m_maxGainLow = 0.0f;
	float m_maxGainMid = 0.0f;
	float m_maxGainHigh = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeClonerAudioProcessor)
};
