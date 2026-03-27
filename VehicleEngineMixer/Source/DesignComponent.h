#pragma once

#include <JuceHeader.h>

#include <vector>

#include "MainComponentBase.h"

#include "../../../zazzVSTPlugins/Shared/GUI/WaveformRuntimeComponent.h"
#include "../../../zazzVSTPlugins/Shared/Filters/SpectrumMatch.h"

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

private:
	int getUsedSourcesCount()
	{
		// Get number of defined sources
		int usedSources = SOURCE_COUNT;
		for (int i = SOURCE_COUNT - 1; i >= 0; i--)
		{
			if (m_bufferSource[i].getNumSamples() == 0)
			{
				usedSources = i;
			}
		}

		return usedSources;
	}
	void resetPlaybackIndex()
	{
		for (size_t i = 0; i < SOURCE_COUNT; i++)
		{
			for (size_t j = 0; j < 2; j++)
			{
				m_playbackIndex[i][j] = 0.0f;
			}
		}
	}

	//==============================================================================
	WaveformRuntimeComponent m_waveformDisplay;
	
	// Buttons
	juce::TextButton m_openSourceButton[SOURCE_COUNT];
	juce::TextButton m_playSourceButton;
	juce::TextButton m_playProcessedButton;
	juce::TextButton m_applySpectrumMatchButton;

	// Labels
	juce::Label m_sourceFileNameLabel[SOURCE_COUNT];
	juce::Label m_regionLength[SOURCE_COUNT];
	juce::Label m_regionCrossfade[SOURCE_COUNT];
	juce::Label m_gainLabel[SOURCE_COUNT];
	juce::Label m_regionLengthPlaybackLabel;

	// Sliders
	juce::Slider m_regionLengthPlaybackSlider;
	juce::Slider m_spectrumMatchSlider[SOURCE_COUNT];

	// Audio buffers
	juce::AudioBuffer<float> m_bufferSource[SOURCE_COUNT];
	juce::AudioBuffer<float> m_bufferProcessed[SOURCE_COUNT];
	int m_sampleRate[SOURCE_COUNT];
	float m_playbackIndex[SOURCE_COUNT][2] = { 0.0f };
	float m_gain[SOURCE_COUNT] = { 0.0f };

	// Misc
	TransportState m_sourceState = TransportState::Stopped;
	int m_usedSources;
	bool m_playSource = true;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DesignComponent)
};