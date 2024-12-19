#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Delays/CircularCombFilterAdvanced.h"
#include "../../../zazzVSTPlugins/Shared/Reverbs/RoomEarlyReflection.h"

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
	static const std::string paramsUnitNames[];
	static const int MIN_ER_PREDELAY_TIME = 1;
	static const int MAX_ER_PREDELAY_TIME = 50;
	static const int MIN_ER_TIME = 10;
	static const int MAX_ER_TIME = 100;
	static const int MAX_COMPLEXITY = 16;

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
	CircularCombFilterAdvanced m_circularCombFilter[2] = {};
	RoomEarlyReflections m_earlyReflaction[2] = {};

	std::atomic<float>* ERPredelayParameter = nullptr;
	std::atomic<float>* ERTimeParameter = nullptr;
	std::atomic<float>* ERAttenuationParameter = nullptr;
	std::atomic<float>* ERDampingParameter = nullptr;
	std::atomic<float>* ERWidthParameter = nullptr;

	std::atomic<float>* predelayParameter = nullptr;
	std::atomic<float>* timeParameter = nullptr;
	std::atomic<float>* resonanceParameter = nullptr;
	std::atomic<float>* dampingParameter = nullptr;
	std::atomic<float>* widthParameter = nullptr;

	std::atomic<float>* combFilterSeedParameter = nullptr;
	std::atomic<float>* allPassSeedParameter = nullptr;
	std::atomic<float>* timeMinParameter = nullptr;
	std::atomic<float>* complexityParameter = nullptr;

	std::atomic<float>* ERVolumeParameter = nullptr;
	std::atomic<float>* LRVolumeParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoomReverbAudioProcessor)
};
