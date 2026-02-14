#pragma once

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/Clippers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Oversampling.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/AudioBuffer.h"

//==============================================================================
class ClipperAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    ClipperAudioProcessor();
    ~ClipperAudioProcessor() override;

	enum ClipperType
	{
		None,
		Hard,
		Slope,
		Soft,
		FoldBack,
		HalfWay,
		ABS,
		Crisp,
		COUNT
	};

	const static int N_CHANNELS = 2;
	const static int OVERSAMPLING_FACTOR = 4;
	const static int OVERSAMPLING_MULTIPLIER = 1 << OVERSAMPLING_FACTOR;
	static const std::string paramsNames[];
	static const std::string labelNames[];
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
	float getPeakReductiondB()
	{
		const float inputMaxdB = juce::Decibels::gainToDecibels(m_inputMax);
		const float outputMaxdB = juce::Decibels::gainToDecibels(m_outputMax);

		m_inputMax = 0.0f;
		m_outputMax = 0.0f;

		return inputMaxdB - outputMaxdB;
	}
	
	//==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };
	std::atomic<bool> m_guiIsOpen{ false };

private:	
	//==============================================================================
	inline void clip(const Clippers::Params& params, const ClipperType type, const int channel, const int sampleRate)
	{
		if (type == ClipperType::Hard)
		{
			Clippers::HardBlock(params);
		}
		else if (type == ClipperType::Slope)
		{
			auto& slopeClipper = m_slopeClipper[channel];
			slopeClipper.set(sampleRate);

			for (int sample = 0; sample < params.samples; sample++)
			{
				float& in = params.buffer[sample];
				const float clipped = slopeClipper.process(in, params.threshold);
				in = in + params.wet * (clipped - in);
			}
		}
		else if (type == ClipperType::Soft)
		{
			Clippers::SoftBlock(params);
		}
		else if (type == ClipperType::FoldBack)
		{
			for (int sample = 0; sample < params.samples; sample++)
			{
				float& in = params.buffer[sample];
				const float clipped = Clippers::FoldBack(in, params.threshold);
				in = in + params.wet * (clipped - in);
			}
		}
		else if (type == ClipperType::HalfWay)
		{
			Clippers::HalfWaveBlock(params);
		}
		else if (type == ClipperType::ABS)
		{
			Clippers::ABSBlock(params);
		}
		else if (type == ClipperType::Crisp)
		{
			Clippers::CrispBlock(params);
		}
	}

	//==============================================================================	
	std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
	
	SlopeClipper m_slopeClipper[N_CHANNELS];

	float m_lastSample[N_CHANNELS];

	std::atomic<float>* typeParameter = nullptr;
	std::atomic<float>* thresholdParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	juce::AudioParameterBool* button1Parameter = nullptr;
	juce::AudioParameterBool* button2Parameter = nullptr;

	float m_inputMax = 0.0f;
	float m_outputMax = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipperAudioProcessor)
};
