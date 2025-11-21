#include "DesignComponent.h"

//==============================================================================
DesignComponent::DesignComponent() : MainComponentBase()
{
	for (size_t i = 0; i < SOURCE_COUNT; i++)
	{
		// Open button
		juce::TextButton& openSourceButton = m_openSourceButton[i];

		addAndMakeVisible(openSourceButton);
		openSourceButton.setButtonText("Open");
		openSourceButton.onClick = [this, i]()
		{
			openFile(m_bufferSource[i], m_sampleRate[i], [this, i]()
			{
				m_sourceFileNameLabel[i].setText(m_fileName, juce::dontSendNotification);
			});
		};
		
		// File name label
		juce::Label& sourceFileNameLabel = m_sourceFileNameLabel[i];

		addAndMakeVisible(sourceFileNameLabel);
		sourceFileNameLabel.setText("", juce::dontSendNotification);
		sourceFileNameLabel.setJustificationType(juce::Justification::centred);

		// Region length label
		juce::Label& regionLength = m_regionLength[i];

		addAndMakeVisible(regionLength);
		regionLength.setText("2000", juce::dontSendNotification);
		regionLength.setJustificationType(juce::Justification::centred);

		regionLength.setEditable(true, true, false);

		// Region crossfade label
		juce::Label& regionCrossfade = m_regionCrossfade[i];

		addAndMakeVisible(regionCrossfade);
		regionCrossfade.setText("2000", juce::dontSendNotification);
		regionCrossfade.setJustificationType(juce::Justification::centred);

		regionCrossfade.setEditable(true, true, false);

		// Gain Label
		juce::Label& gainLabel = m_gainLabel[i];

		addAndMakeVisible(gainLabel);
		gainLabel.setText("0.0", juce::dontSendNotification);
		gainLabel.setJustificationType(juce::Justification::centred);

		// Spectrum match slider
		juce::Slider& spectrumMatchSlider = m_spectrumMatchSlider[i];

		addAndMakeVisible(spectrumMatchSlider);
		spectrumMatchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
		spectrumMatchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
		spectrumMatchSlider.setRange(0.0, 100.0, 1.0);
		spectrumMatchSlider.setValue((float)(i * 100 / (SOURCE_COUNT - 1)));
	}

	// Play button
	addAndMakeVisible(&m_playSourceButton);
	m_playSourceButton.setButtonText("Play Source");
	m_playSourceButton.onClick = [this]
	{ 
		if (m_sourceState == TransportState::Stopped)
		{
			m_sourceState = TransportState::Playing;
			m_playSourceButton.setButtonText("Stop Source");

			m_playSource = true;

			m_usedSources = getUsedSourcesCount();
		}
		else if (m_sourceState == TransportState::Playing)
		{
			resetPlaybackIndex();

			m_sourceState = TransportState::Stopped;
			m_playSourceButton.setButtonText("Play Source");
		}
	};

	// Play processed button
	addAndMakeVisible(&m_playProcessedButton);
	m_playProcessedButton.setButtonText("Play Porcessed");
	m_playProcessedButton.onClick = [this]
	{
		if (m_sourceState == TransportState::Stopped)
		{
			m_sourceState = TransportState::Playing;
			m_playProcessedButton.setButtonText("Stop Processed");
		
			m_playSource = false;			
			m_usedSources = getUsedSourcesCount();
		}
		else if (m_sourceState == TransportState::Playing)
		{
			resetPlaybackIndex();

			m_sourceState = TransportState::Stopped;
			m_playSourceButton.setButtonText("Play Processed");
		}
	};

	// Apply spectrum match
	addAndMakeVisible(&m_applySpectrumMatchButton);
	m_applySpectrumMatchButton.setButtonText("Apply spectrum match");
	m_applySpectrumMatchButton.onClick = [this]
	{
		m_usedSources = getUsedSourcesCount();
		if (m_usedSources == 0)
		{
			return;
		}

		float gains[SOURCE_COUNT][SpectrumDetectionTD::BANDS_COUNT] = {};
		
		// Get spectrums
		SpectrumDetectionTD spectrumDetection{};
		spectrumDetection.init(m_sampleRate[0]);
		spectrumDetection.set(400.0f, 800.0f);

		for (int i = 0; i < m_usedSources; i++)
		{
			const int samples = m_bufferSource[i].getNumSamples();
			auto* channelBuffer = m_bufferSource[i].getWritePointer(0);

			for (int sample = 0; sample < samples; sample++)
			{
				spectrumDetection.process(sample);
			}

			// Copy spectrum into local gains buffer
			float* spectrum = spectrumDetection.getSpectrum();
			
			for (int j = 0; j < SpectrumDetectionTD::BANDS_COUNT; j++)
			{
				gains[i][j] = juce::Decibels::gainToDecibels(spectrum[j]);
			}
		}

		// Initialize processed buffers
		for (int i = 0; i < m_usedSources; i++)
		{
			m_bufferProcessed[i].setSize(m_bufferSource[i].getNumChannels(), m_bufferSource[i].getNumSamples());
		}

		// Apply spectrums
		SpectrumApply spectrumApply{};
		spectrumApply.init(m_sampleRate[0]);

		for (int source = 0; source < m_usedSources; source++)
		{
			// Calculate apply gains
			float applyGain[SpectrumApply::BANDS_COUNT]{ 0.0f };
			const float spectrumMatchValue = 0.01f * m_spectrumMatchSlider[source].getValue();

			for (int band = 0; band < SpectrumApply::BANDS_COUNT; band++)
			{
				const float leftdB = gains[0][band];
				const float rightdB = gains[m_usedSources - 1][band];

				const float dB = gains[source][band];
				const float targetdB = leftdB + spectrumMatchValue * (rightdB - leftdB);
				
				applyGain[band] = targetdB - dB;
			}

			// Apply gains
			SpectrumApply::Params params{ applyGain };
			spectrumApply.set(params);

			const float channels = m_bufferProcessed[source].getNumChannels();
			const float samples = m_bufferProcessed[source].getNumSamples();

			for (int channel = 0; channel < channels; channel++)
			{
				auto* bufferSource = m_bufferSource[source].getWritePointer(channel);
				auto* bufferProcessed = m_bufferProcessed[source].getWritePointer(channel);

				for (int sample = 0; sample < samples; sample++)
				{
					const float in = bufferSource[sample];
					const float out = spectrumApply.process(in);
					bufferProcessed[sample] = out;
				}
			}
		}

	};

	// Region length slider
	addAndMakeVisible(m_regionLengthPlaybackLabel);
	m_regionLengthPlaybackLabel.setText("Length", juce::dontSendNotification);
	m_regionLengthPlaybackLabel.setJustificationType(juce::Justification::centred);
	m_regionLengthPlaybackLabel.attachToComponent(&m_regionLengthPlaybackSlider, true);

	addAndMakeVisible(m_regionLengthPlaybackSlider);
	m_regionLengthPlaybackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_regionLengthPlaybackSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_regionLengthPlaybackSlider.setRange(100.0, 2000.0, 1.0);
	m_regionLengthPlaybackSlider.setValue(2000.0);


	addAndMakeVisible(m_waveformDisplay);
	m_waveformDisplay.init(2000 * 10 * 5);
}

DesignComponent::~DesignComponent()
{
	// This shuts down the audio device and clears the audio source.
	shutdownAudio();
}

//==============================================================================
void DesignComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
}

void DesignComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	if (m_sourceState != TransportState::Playing)
	{
		return;
	}
	
	// Get region lengths
	int regionLength[SOURCE_COUNT];
	int regionCrossfade[SOURCE_COUNT];

	for (size_t i = 0; i < SOURCE_COUNT; i++)
	{
		regionLength[i] = m_regionLength[i].getText().getIntValue();
		regionCrossfade[i] = m_regionCrossfade[i].getText().getIntValue();
	}

	int regionLengthPlayback = (int)m_regionLengthPlaybackSlider.getValue();

	// Get iterators
	float m_iterator[SOURCE_COUNT];
	for (size_t i = 0; i < m_usedSources; i++)
	{
		m_iterator[i] = (float)regionLength[i] / (float)regionLengthPlayback;
	}

	// Get buffers length
	int bufferLength[SOURCE_COUNT];
	for (size_t i = 0; i < m_usedSources; i++)
	{
		bufferLength[i] = m_bufferSource[i].getNumSamples();
	}

	// Get gains
	for (size_t i = 0; i < SOURCE_COUNT; i++)
	{
		if (i == 0)
		{
			if (regionLengthPlayback <= regionCrossfade[i])
			{
				m_gain[i] = 1.0f;
			}
			else if (regionLengthPlayback < regionCrossfade[i + 1])
			{
				m_gain[i] = 1.0f - ((float)(regionLengthPlayback - regionCrossfade[i]) / (float)(regionCrossfade[i + 1] - regionCrossfade[i]));
				m_gain[i] = std::sqrtf(m_gain[i]);
			}
			else
			{
				m_gain[i] = 0.0f;
			}
		}
		else if (i == m_usedSources - 1)
		{
			if (regionLengthPlayback >= regionCrossfade[i])
			{
				m_gain[i] = 1.0f;
			}
			else if (regionLengthPlayback > regionCrossfade[i - 1] && regionLengthPlayback < regionCrossfade[i])
			{
				m_gain[i] = (float)(regionLengthPlayback - regionCrossfade[i - 1]) / (float)(regionCrossfade[i] - regionCrossfade[i - 1]);
				m_gain[i] = std::sqrtf(m_gain[i]);
			}
			else
			{
				m_gain[i] = 0.0f;
			}
		}
		else if (i > m_usedSources - 1)
		{
			m_gain[i] = 0.0f;
		}
		else
		{
			if (regionLengthPlayback > regionCrossfade[i - 1] && regionLengthPlayback < regionCrossfade[i])
			{
				m_gain[i] = (float)(regionLengthPlayback - regionCrossfade[i - 1]) / (float)(regionCrossfade[i] - regionCrossfade[i - 1]);
				m_gain[i] = std::sqrtf(m_gain[i]);
			}
			else if (regionLengthPlayback > regionCrossfade[i] && regionLengthPlayback < regionCrossfade[i + 1])
			{
				m_gain[i] = 1.0f - ((float)(regionLengthPlayback - regionCrossfade[i]) / (float)(regionCrossfade[i + 1] - regionCrossfade[i]));
				m_gain[i] = std::sqrtf(m_gain[i]);
			}
			else if (regionLengthPlayback == regionCrossfade[i])
			{
				m_gain[i] = 1.0f;
			}
			else
			{
				m_gain[i] = 0.0f;
			}
		}
	}

	// Writte to outBuffer
	auto channels = bufferToFill.buffer->getNumChannels();
	auto samples = bufferToFill.numSamples;
	auto startSample = bufferToFill.startSample;
	auto& buffer = *bufferToFill.buffer;

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		for (int sample = startSample; sample < startSample + samples; sample++)
		{
			float out = 0.0f;
			for (size_t i = 0; i < m_usedSources; i++)
			{
				float& playbackIndex = m_playbackIndex[i][channel];

				playbackIndex += m_iterator[i];
				if (playbackIndex >= bufferLength[i])
				{
					playbackIndex -= bufferLength[i];
				}

				if (m_playSource)
				{
					out += m_gain[i] * m_bufferSource[i].getWritePointer(0)[(int)playbackIndex];
				}
				else
				{
					out += m_gain[i] * m_bufferProcessed[i].getWritePointer(0)[(int)playbackIndex];
				}
				
			}

			if (channel == 0)
			{
				m_waveformDisplay.process(out);
			}

			channelBuffer[sample] = out;
		}
	}

	startTimerHz(60);
	
}

void DesignComponent::releaseResources()
{
	stopTimer();
}

//==============================================================================
void DesignComponent::paint(juce::Graphics& g)
{
	g.fillAll(darkColor);

	// Update lables
	for (size_t i = 0; i < SOURCE_COUNT; i++)
	{
		m_gainLabel[i].setText(juce::String(m_gain[i], 2), juce::dontSendNotification);
	}
}

void DesignComponent::timerCallback()
{
	// Waveform display
	//if (m_waveformDisplay.canRepaint())
	{
		m_waveformDisplay.set((int)m_regionLengthPlaybackSlider.getValue() * 20);
		m_waveformDisplay.repaint();
	}
}

void DesignComponent::resized()
{	
	// Pixel size
	const int width = getWidth();
	const int pixelSize = width / CANVAS_WIDTH;

	// Calculate helpers
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize + pixelSize2;
	const int pixelSize6 = pixelSize3 + pixelSize3;
	const int pixelSize7 = pixelSize6 + pixelSize;
	const int pixelSize8 = pixelSize6 + pixelSize2;
	const int pixelSize9 = pixelSize3 + pixelSize6;
	const int pixelSize11 = pixelSize9 + pixelSize2;
	const int pixelSize12 = pixelSize6 + pixelSize6;
	const int pixelSize15 = pixelSize9 + pixelSize6;
	const int pixelSize31 = pixelSize15 + pixelSize15 + pixelSize;

	const int column1 = 0;							// Left edge
	const int column2 = column1 + pixelSize;		// First main column
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;		// Gap
	const int column8 = column7 + pixelSize;		// Second main column
	const int column9 = column8 + pixelSize3;
	const int column10 = column9 + pixelSize3;
	const int column11 = column10 + pixelSize3;
	const int column12 = column11 + pixelSize3;

	const int row1 = 0;
	const int row2 = row1 + pixelSize2;
	const int row3 = row2 + pixelSize;
	const int row4 = row3 + pixelSize;
	const int row5 = row4 + pixelSize;
	const int row6 = row5 + pixelSize;
	const int row7 = row6 + pixelSize;
	const int row8 = row7 + pixelSize;
	const int row9 = row8 + pixelSize;
	const int row10 = row9 + pixelSize;
	const int row11 = row10 + pixelSize;
	const int row12 = row11 + pixelSize2;				// Source waveform
	const int row13 = row12 + pixelSize11;				// Output waveform
	const int row14 = row13 + pixelSize;				// Output waveform

	// Set size
	for (size_t i = 0; i < SOURCE_COUNT; i++)
	{
		// Open button
		m_openSourceButton[i].setSize(pixelSize3, pixelSize);
		
		// File name label
		m_sourceFileNameLabel[i].setSize(pixelSize12, pixelSize);

		// Region length
		m_regionLength[i].setSize(pixelSize3, pixelSize);

		// Region crossfade
		m_regionCrossfade[i].setSize(pixelSize3, pixelSize);

		// Gain label
		m_gainLabel[i].setSize(pixelSize3, pixelSize);

		// Spectrum slider
		m_spectrumMatchSlider[i].setSize(pixelSize6, pixelSize);
	}

	m_playSourceButton.setSize(pixelSize31, pixelSize);
	m_playProcessedButton.setSize(pixelSize31, pixelSize);
	m_applySpectrumMatchButton.setSize(pixelSize31, pixelSize);
	m_waveformDisplay.setSize(pixelSize31, pixelSize12);

	m_regionLengthPlaybackSlider.setSize(pixelSize31 - pixelSize3, pixelSize);

	// Set position
	m_openSourceButton[0].setTopLeftPosition(column2, row2);
	m_openSourceButton[1].setTopLeftPosition(column2, row3);
	m_openSourceButton[2].setTopLeftPosition(column2, row4);
	m_openSourceButton[3].setTopLeftPosition(column2, row5);
	m_openSourceButton[4].setTopLeftPosition(column2, row6);
	m_openSourceButton[5].setTopLeftPosition(column2, row7);

	// File name label
	m_sourceFileNameLabel[0].setTopLeftPosition(column3, row2);
	m_sourceFileNameLabel[1].setTopLeftPosition(column3, row3);
	m_sourceFileNameLabel[2].setTopLeftPosition(column3, row4);
	m_sourceFileNameLabel[3].setTopLeftPosition(column3, row5);
	m_sourceFileNameLabel[4].setTopLeftPosition(column3, row6);
	m_sourceFileNameLabel[5].setTopLeftPosition(column3, row7);

	// File length label
	m_regionLength[0].setTopLeftPosition(column8, row2);
	m_regionLength[1].setTopLeftPosition(column8, row3);
	m_regionLength[2].setTopLeftPosition(column8, row4);
	m_regionLength[3].setTopLeftPosition(column8, row5);
	m_regionLength[4].setTopLeftPosition(column8, row6);
	m_regionLength[5].setTopLeftPosition(column8, row7);

	// File crossfade label
	m_regionCrossfade[0].setTopLeftPosition(column9, row2);
	m_regionCrossfade[1].setTopLeftPosition(column9, row3);
	m_regionCrossfade[2].setTopLeftPosition(column9, row4);
	m_regionCrossfade[3].setTopLeftPosition(column9, row5);
	m_regionCrossfade[4].setTopLeftPosition(column9, row6);
	m_regionCrossfade[5].setTopLeftPosition(column9, row7);

	// Gain label label
	m_gainLabel[0].setTopLeftPosition(column10, row2);
	m_gainLabel[1].setTopLeftPosition(column10, row3);
	m_gainLabel[2].setTopLeftPosition(column10, row4);
	m_gainLabel[3].setTopLeftPosition(column10, row5);
	m_gainLabel[4].setTopLeftPosition(column10, row6);
	m_gainLabel[5].setTopLeftPosition(column10, row7);

	// SpectrumMatch slider
	m_spectrumMatchSlider[0].setTopLeftPosition(column11, row2);
	m_spectrumMatchSlider[1].setTopLeftPosition(column11, row3);
	m_spectrumMatchSlider[2].setTopLeftPosition(column11, row4);
	m_spectrumMatchSlider[3].setTopLeftPosition(column11, row5);
	m_spectrumMatchSlider[4].setTopLeftPosition(column11, row6);
	m_spectrumMatchSlider[5].setTopLeftPosition(column11, row7);

	// Other
	m_playSourceButton.setTopLeftPosition(column2, row8);
	m_playProcessedButton.setTopLeftPosition(column2, row9);
	m_applySpectrumMatchButton.setTopLeftPosition(column2, row14);
	m_regionLengthPlaybackSlider.setTopLeftPosition(column3, row10);
	m_waveformDisplay.setTopLeftPosition(column2, row11);
}