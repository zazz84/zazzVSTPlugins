#include "MainComponentBase.h"

//==============================================================================
MainComponentBase::MainComponentBase()
{
	m_formatManager.registerBasicFormats();

	setAudioChannels(0, 2);
}

MainComponentBase::~MainComponentBase()
{
	// This shuts down the audio device and clears the audio source.
	shutdownAudio();
}

//==============================================================================
void MainComponentBase::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
}

void MainComponentBase::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	
}

void MainComponentBase::releaseResources()
{
}

//==============================================================================
void MainComponentBase::paint(juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void MainComponentBase::resized()
{
}
