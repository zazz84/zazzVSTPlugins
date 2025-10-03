#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	addAndMakeVisible(waveformDisplaySource);
	addAndMakeVisible(waveformDisplayOutput);
	addAndMakeVisible(m_regionsComponent);
	
	// Labels
	//
	addAndMakeVisible(m_sourceFileNameLabel);
	m_sourceFileNameLabel.setText("--- empty ---", juce::dontSendNotification);
	m_sourceFileNameLabel.setJustificationType(juce::Justification::centred);

	addAndMakeVisible(m_regionLenghtMedianLabel);
	m_regionLenghtMedianLabel.setText("", juce::dontSendNotification);
	m_regionLenghtMedianLabel.setJustificationType(juce::Justification::centred);
	
	//
	addAndMakeVisible(m_detectedFrequencySlider);
	m_detectedFrequencySlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_detectedFrequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_detectedFrequencySlider.setRange(2.0, 200.0, 1.0);   // min, max, step
	m_detectedFrequencySlider.setValue(50.0);              // initial value

	addAndMakeVisible(m_detectedFrequencyLabel);
	m_detectedFrequencyLabel.setText("Detected frequency", juce::dontSendNotification);

	m_detectedFrequencyLabel.attachToComponent(&m_detectedFrequencySlider, true);

	//
	addAndMakeVisible(m_regionOffsetLenghtSlider);
	m_regionOffsetLenghtSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_regionOffsetLenghtSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_regionOffsetLenghtSlider.setRange(0.0, 1000.0, 1.0);   // min, max, step
	m_regionOffsetLenghtSlider.setValue(1000.0);               // initial value

	addAndMakeVisible(m_regionOffsetLenghtLabel);
	m_regionOffsetLenghtLabel.setText("Region Lenght Offset", juce::dontSendNotification);

	m_regionOffsetLenghtLabel.attachToComponent(&m_regionOffsetLenghtSlider, true);

	m_regionOffsetLenghtSlider.onValueChange = [this]()
	{
		updateValidZeroCrossingIdx();
		repaint(); // triggers MyComponent::paint()
	};

	//
	addAndMakeVisible(m_regionLenghtExportSlider);
	m_regionLenghtExportSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_regionLenghtExportSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_regionLenghtExportSlider.setRange(0.0, 5000.0, 1.0);        // min, max, step
	m_regionLenghtExportSlider.setValue(1000.0);                     // initial value

	addAndMakeVisible(m_regionLenghtExportLabel);
	m_regionLenghtExportLabel.setText("Region Lenght Export", juce::dontSendNotification);

	m_regionLenghtExportLabel.attachToComponent(&m_regionLenghtExportSlider, true);

	//
	addAndMakeVisible(m_regionsCountLabel);
	m_regionsCountLabel.setJustificationType(juce::Justification::centred);

	//
	addAndMakeVisible(m_validRegionsCountLabel);
	m_validRegionsCountLabel.setJustificationType(juce::Justification::centred);

	// Zoom	
	addAndMakeVisible(m_zoomRegionLeft);
	m_zoomRegionLeft.setEditable(true, true, false);  // user can click & type
	m_zoomRegionLeft.setJustificationType(juce::Justification::centred);
	m_zoomRegionLeft.setText("-1", juce::dontSendNotification);

	m_zoomRegionLeft.onTextChange = [this]
	{
		setHorizontalZoom();
	};

	addAndMakeVisible(m_zoomRegionRight);
	m_zoomRegionRight.setEditable(true, true, false);  // user can click & type
	m_zoomRegionRight.setJustificationType(juce::Justification::centred);
	m_zoomRegionRight.setText("-1", juce::dontSendNotification);

	m_zoomRegionRight.onTextChange = [this]
	{
		setHorizontalZoom();
	};

	// Buttons
	addAndMakeVisible(&m_openSourceButton);
	m_openSourceButton.setButtonText("Open");
	m_openSourceButton.onClick = [this] { openSourceButtonClicked(); };

	addAndMakeVisible(&m_detectFrequencyButton);
	m_detectFrequencyButton.setButtonText("Detect Frequency");
	m_detectFrequencyButton.onClick = [this] { detectFrequencyButtonClicked(); };

	addAndMakeVisible(&m_detectRegionsButton);
	m_detectRegionsButton.setButtonText("Detect Regions");
	m_detectRegionsButton.onClick = [this] { detectRegionsButtonClicked(); };

	addAndMakeVisible(&m_generateButton);
	m_generateButton.setButtonText("Generate");
	m_generateButton.onClick = [this] { generateButtonClicked(); };

	addAndMakeVisible(&m_saveButton);
	m_saveButton.setButtonText("Save");
	m_saveButton.onClick = [this] { saveButtonClicked(); };

	// Play buttons
	addAndMakeVisible(&m_playButton);
	m_playButton.setButtonText("Play");
	m_playButton.onClick = [this] { playSourceButtonClicked(); };

	addAndMakeVisible(&m_sourceButton);
	m_sourceButton.setButtonText("Source");
	m_sourceButton.onClick = [this] { sourceButtonClicked(); };

	// Combo boxes
	//
	m_detectionTypeComboBox.addItem("Default", 1);
	m_detectionTypeComboBox.addItem("Filter", 2);
	m_detectionTypeComboBox.addItem("Frequency limit", 3);
	m_detectionTypeComboBox.setSelectedId(1);
	addAndMakeVisible(m_detectionTypeComboBox);
	
	setSize(1600, 900);

	m_formatManager.registerBasicFormats();       // [1]

	setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
	if (m_sourceState == TransportState::Playing)
	{
		auto& buffer = m_sourceType == SourceType::Source ? m_bufferSource : m_bufferOutput;

		auto numInputChannels = buffer.getNumChannels();
		auto numOutputChannels = bufferToFill.buffer->getNumChannels();
		auto outputSamplesRemaining = bufferToFill.numSamples;
		auto outputSamplesOffset = bufferToFill.startSample;
		
		while (outputSamplesRemaining > 0)
		{
			auto bufferSamplesRemaining = buffer.getNumSamples() - m_playbackIndex;
			auto samplesThisTime = juce::jmin(outputSamplesRemaining, bufferSamplesRemaining);
			
			for (auto channel = 0; channel < numOutputChannels; ++channel)
			{
				bufferToFill.buffer->copyFrom(channel, outputSamplesOffset, buffer, channel % numInputChannels, m_playbackIndex, samplesThisTime);
			}
			
			outputSamplesRemaining -= samplesThisTime;
			outputSamplesOffset += samplesThisTime;
			m_playbackIndex += samplesThisTime; //
			
			if (m_playbackIndex >= buffer.getNumSamples())
			{
				TransportState::Stopped;
				m_playbackIndex = 0;
			}
		}
	}
}

void MainComponent::releaseResources()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
	const int EXTRA_SPACE = 20;
	const int ROW_HEIGHT = 30;
	const int pixelHeight = 20;
	const int pixelHeight8 = 8 * pixelHeight;

	const int row1 = EXTRA_SPACE;
	const int row2 = row1 + ROW_HEIGHT + EXTRA_SPACE;
	const int row3 = row2 + ROW_HEIGHT + EXTRA_SPACE;
	const int row4 = row3 + ROW_HEIGHT;
	const int row5 = row4 + ROW_HEIGHT + EXTRA_SPACE;
	const int row6 = row5 + ROW_HEIGHT + EXTRA_SPACE;
	const int row7 = row6 + ROW_HEIGHT + EXTRA_SPACE;
	const int row8 = row7 + ROW_HEIGHT + EXTRA_SPACE;
	const int row9 = row8 + ROW_HEIGHT + pixelHeight8;
	const int row10 = row9 + ROW_HEIGHT + pixelHeight8;

	const int width = getWidth();
	
	const int pixelWidth = 300;
	const int pixelWidth2 = pixelWidth + pixelWidth;
	const int pixelWidth3 = pixelWidth2 + pixelWidth;
	const int pixelWidth4 = pixelWidth2 + pixelWidth2;

	const int column1 = (getWidth() / 2) - 600;
	const int column2 = column1 + 300;
	const int column3 = column2 + 300;
	const int column4 = column3 + 300;
	
	//
	m_openSourceButton.setBounds			(column1, row1, pixelWidth, pixelHeight);
	m_sourceFileNameLabel.setBounds			(column2, row1, pixelWidth3, pixelHeight);
	
	m_detectFrequencyButton.setBounds		(column1, row2, pixelWidth, pixelHeight);
	m_detectedFrequencySlider.setBounds		(column4, row2, pixelWidth, pixelHeight);
	
	m_detectRegionsButton.setBounds			(column1, row3, pixelWidth, pixelHeight);	
	m_detectionTypeComboBox.setBounds		(column2, row3, pixelWidth, pixelHeight);
	m_regionOffsetLenghtSlider.setBounds	(column4, row3, pixelWidth, pixelHeight);
	
	m_regionsCountLabel.setBounds			(column2, row4, pixelWidth, pixelHeight);
	m_validRegionsCountLabel.setBounds		(column3, row4, pixelWidth, pixelHeight);
	m_regionLenghtMedianLabel.setBounds		(column4, row4, pixelWidth, pixelHeight);
	
	m_generateButton.setBounds				(column1, row5, pixelWidth, pixelHeight);
	m_regionLenghtExportSlider.setBounds	(column4, row5, pixelWidth, pixelHeight);
	
	m_saveButton.setBounds					(column1, row6, pixelWidth4, pixelHeight);

	// Play buttons
	m_sourceButton.setBounds				(column1, row7, pixelWidth, pixelHeight);
	m_playButton.setBounds					(column2, row7, pixelWidth, pixelHeight);

	// Zoom
	m_zoomRegionLeft.setBounds				(column3, row7, pixelWidth, pixelHeight);
	m_zoomRegionRight.setBounds				(column4, row7, pixelWidth, pixelHeight);
	
	// Waveform source
	waveformDisplaySource.setBounds			(10, row8, getWidth() - 20, pixelHeight8);

	// Regions
	m_regionsComponent.setBounds			(10, row9, getWidth() - 20, pixelHeight8);

	// Waveform output
	waveformDisplayOutput.setBounds			(10, row10, getWidth() - 20, pixelHeight8);
}
