#pragma once

#include <JuceHeader.h>

#include <vector>

#include "MainComponentBase.h"

#include "../../../zazzVSTPlugins/Shared/GUI/WaveformRuntimeComponent.h"

//==============================================================================
/*
	This component lives inside our window, and this is where you should put all
	your controls and content.
*/
class DesignComponent : public MainComponentBase, public juce::Timer
{
public:
	//==============================================================================
	DesignComponent();
	~DesignComponent() override;

	static const int SOURCE_COUNT = 6;

	//==============================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;
	void timerCallback() override;

	//==============================================================================
	void resized() override;
	void paint(juce::Graphics& g) override;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override
	{
	}

	//==============================================================================
	WaveformRuntimeComponent m_waveformDisplay;
	
	// Buttons
	juce::TextButton m_openSourceButton[SOURCE_COUNT];
	juce::TextButton m_playButton;

	// Labels
	juce::Label m_sourceFileNameLabel[SOURCE_COUNT];
	juce::Label m_regionLength[SOURCE_COUNT];
	juce::Label m_regionCrossfade[SOURCE_COUNT];
	juce::Label m_gainLabel[SOURCE_COUNT];
	juce::Label m_regionLengthPlaybackLabel;

	// Sliders
	juce::Slider m_regionLengthPlaybackSlider;

	// Audio buffers
	juce::AudioBuffer<float> m_bufferSource[SOURCE_COUNT];
	int m_sampleRate[SOURCE_COUNT];
	float m_playbackIndex[SOURCE_COUNT][2] = { 0.0f };
	float m_gain[SOURCE_COUNT] = { 0.0f };

	// Misc
	TransportState m_sourceState = TransportState::Stopped;
	int m_usedSources;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DesignComponent)
};