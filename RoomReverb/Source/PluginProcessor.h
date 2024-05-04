#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Filters/CombFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/NoiseGenerator.h"

//==============================================================================
class RoomReverbAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
	, public juce::AudioProcessorARAExtension
#endif
{

public:
	//==============================================================================
	RoomReverbAudioProcessor();
	~RoomReverbAudioProcessor() override;

	static const std::string paramsNames[];
	static const int MAX_COMPLEXITY = 64;
	static const int COMB_FILTER_MAX_TIME_MS = 300;
	static const int ALL_PASS_MAX_TIME_MS = 150;
	static const float m_dampingFrequencyMin;
	static const int PRE_DELAY_MAX_MS = 300;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:
	//==============================================================================

	std::atomic<float>* ERTimeParameter = nullptr;
	std::atomic<float>* ERDampingParameter = nullptr;
	std::atomic<float>* ERLRParameter = nullptr;
	std::atomic<float>* predelayParameter = nullptr;
	std::atomic<float>* timeParameter = nullptr;
	std::atomic<float>* resonanceParameter = nullptr;
	std::atomic<float>* dampingParameter = nullptr;
	std::atomic<float>* widthParameter = nullptr;
	std::atomic<float>* combFilterSeedParameter = nullptr;
	std::atomic<float>* allPassSeedParameter = nullptr;
	std::atomic<float>* timeMinParameter = nullptr;
	std::atomic<float>* complexityParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	juce::AudioParameterBool* earlyReflectionsMuteParameter = nullptr;
	juce::AudioParameterBool* lateReflectionsMuteParameter = nullptr;

	CircularCombFilterAdvanced m_circularCombFilter[2] = {};
	RoomEarlyReflections m_earlyReflaction[2] = {};
	LinearCongruentialNoiseGenerator m_noiseGenerator = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoomReverbAudioProcessor)
};
