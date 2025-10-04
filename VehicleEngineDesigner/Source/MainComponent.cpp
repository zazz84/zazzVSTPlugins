#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	addAndMakeVisible(m_waveformDisplaySource);
	addAndMakeVisible(m_waveformDisplayOutput);

	addAndMakeVisible(m_pluginNameComponent);

	addAndMakeVisible(m_sourceGroupLableComponent);
	addAndMakeVisible(m_regionGroupLableComponent);
	addAndMakeVisible(m_exportGroupLableComponent);
	addAndMakeVisible(m_playbackGroupLableComponent);
	addAndMakeVisible(m_sourceWaveformGroupLableComponent);
	addAndMakeVisible(m_outputWaveformGroupLableComponent);
	addAndMakeVisible(m_zoomGroupLableComponent);
	
	// Labels
	//
	addAndMakeVisible(m_sourceFileNameLabel);
	m_sourceFileNameLabel.setText("", juce::dontSendNotification);
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
	m_detectedFrequencyLabel.setText("Frequency", juce::dontSendNotification);

	m_detectedFrequencyLabel.attachToComponent(&m_detectedFrequencySlider, true);

	//
	//addAndMakeVisible(m_regionOffsetLenghtSlider);
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
	m_regionLenghtExportLabel.setText("Reg Lenght", juce::dontSendNotification);

	m_regionLenghtExportLabel.attachToComponent(&m_regionLenghtExportSlider, true);

	//
	addAndMakeVisible(m_regionsCountLabel);
	m_regionsCountLabel.setJustificationType(juce::Justification::centred);

	//
	//addAndMakeVisible(m_validRegionsCountLabel);
	//m_validRegionsCountLabel.setJustificationType(juce::Justification::centred);

	//
	addAndMakeVisible(m_maxZeroCrossingGainLabel);
	m_maxZeroCrossingGainLabel.setJustificationType(juce::Justification::centred);

	// Zoom	
	addAndMakeVisible(m_zoomRegionLeft);
	m_zoomRegionLeft.setEditable(true, true, false);  // user can click & type
	m_zoomRegionLeft.setJustificationType(juce::Justification::centred);
	m_zoomRegionLeft.setText("-1", juce::dontSendNotification);

	m_zoomRegionLeft.onTextChange = [this]
	{	
		// Reset to invalid when not regions are detected
		if (m_regions.size() == 0)
		{
			m_zoomRegionLeft.setText("-1", juce::dontSendNotification);
			return;
		}
		
		const int leftRegion = m_zoomRegionLeft.getText().getIntValue();
		const int regionsMax = m_regions.size() - 1;

		if (leftRegion < 0 || leftRegion > regionsMax)
		{
			m_zoomRegionLeft.setText("0", juce::dontSendNotification);
		}

		// Limit if large than right region
		const int rightRegion = m_zoomRegionRight.getText().getIntValue();

		if (leftRegion >= rightRegion)
		{
			m_zoomRegionLeft.setText(juce::String((float)(std::max(0, rightRegion - 1)), 0), juce::dontSendNotification);
		}

		setHorizontalZoom();
	};

	addAndMakeVisible(m_zoomRegionRight);
	m_zoomRegionRight.setEditable(true, true, false);  // user can click & type
	m_zoomRegionRight.setJustificationType(juce::Justification::centred);
	m_zoomRegionRight.setText("-1", juce::dontSendNotification);

	m_zoomRegionRight.onTextChange = [this]
	{
		// Reset to invalid when not regions are detected
		if (m_regions.size() == 0)
		{
			m_zoomRegionRight.setText("-1", juce::dontSendNotification);
			return;
		}

		const int rightRegion = m_zoomRegionRight.getText().getIntValue();
		const int regionsMax = m_regions.size() - 1;

		if (rightRegion < 0 || rightRegion > regionsMax)
		{
			m_zoomRegionRight.setText(juce::String((float)(regionsMax)), juce::dontSendNotification);
		}

		// Limit if large than right region
		const int leftRegion = m_zoomRegionLeft.getText().getIntValue();

		if (leftRegion >= rightRegion)
		{
			m_zoomRegionRight.setText(juce::String((float)(std::min(regionsMax, leftRegion + 1)), 0), juce::dontSendNotification);
		}

		setHorizontalZoom();
	};

	// Scroll buttons
	addAndMakeVisible(&m_scrollLeft);
	m_scrollLeft.setButtonText("<");
	m_scrollLeft.onClick = [this]
	{ 
		int zoomLeft = m_zoomRegionLeft.getText().getIntValue();
		int zoomRight = m_zoomRegionRight.getText().getIntValue();
		const bool regionsIsEmpty = m_zoomRegionRight.getText().isEmpty();
		const int regionsMax = m_regions.size() - 1;

		if (zoomLeft > zoomRight || zoomLeft == -1 || zoomRight == -1 || regionsMax == 0 || regionsIsEmpty)
		{
			return;
		}

		// Set left to 0, if not enough regions
		const int size = std::fminf(zoomLeft, zoomRight - zoomLeft);

		zoomLeft -= size;
		zoomRight -= size;

		m_zoomRegionLeft.setText(juce::String((float)zoomLeft, 0), juce::dontSendNotification);
		m_zoomRegionRight.setText(juce::String((float)zoomRight, 0), juce::dontSendNotification);

		setHorizontalZoom();
	};

	addAndMakeVisible(&m_scrollRight);
	m_scrollRight.setButtonText(">");
	m_scrollRight.onClick = [this]
	{
		int zoomLeft = m_zoomRegionLeft.getText().getIntValue();
		int zoomRight = m_zoomRegionRight.getText().getIntValue();
		const bool regionsIsEmpty = m_zoomRegionRight.getText().isEmpty();
		const int regionsMax = m_regions.size() - 1;

		if (zoomLeft > zoomRight || zoomLeft == -1 || zoomRight == -1 || regionsMax == 0 || regionsIsEmpty)
		{
			return;
		}

		// Set right to region max, if not enough regions
		const int size = std::fminf(regionsMax - zoomRight, zoomRight - zoomLeft);

		zoomRight += size;
		zoomLeft += size;

		m_zoomRegionLeft.setText(juce::String((float)zoomLeft, 0), juce::dontSendNotification);
		m_zoomRegionRight.setText(juce::String((float)zoomRight, 0), juce::dontSendNotification);

		setHorizontalZoom();
	};

	// Buttons
	addAndMakeVisible(&m_openSourceButton);
	m_openSourceButton.setButtonText("Open");
	m_openSourceButton.onClick = [this] { openSourceButtonClicked(); };

	//addAndMakeVisible(&m_detectFrequencyButton);
	m_detectFrequencyButton.setButtonText("Detect Frequency");
	m_detectFrequencyButton.onClick = [this] { detectFrequencyButtonClicked(); };

	addAndMakeVisible(&m_detectRegionsButton);
	m_detectRegionsButton.setButtonText("Detect");
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
	
	// Canvas size
	const int canvasWidth = CANVAS_WIDTH * 30;
	const int canvasHeight = CANVAS_HEIGHT * 30;

	setSize(canvasWidth, canvasHeight);

	//
	m_formatManager.registerBasicFormats();

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
			m_playbackIndex += samplesThisTime;
			
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
    g.fillAll (darkColor);
}

void MainComponent::resized()
{	
	// Force fized ratio
	const int width = getWidth();
	const int pixelSize = width / CANVAS_WIDTH;
	const int height = pixelSize * CANVAS_HEIGHT;
	setSize(width, pixelSize * CANVAS_HEIGHT);

	// Calculate helpers
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize + pixelSize2;
	const int pixelSize6 = pixelSize3 + pixelSize3;
	const int pixelSize7 = pixelSize6 + pixelSize;
	const int pixelSize8 = pixelSize6 + pixelSize2;
	const int pixelSize9 = pixelSize3 + pixelSize6;
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
	const int row10 = row9 + pixelSize2;				// Source group label
	const int row11 = row10 + pixelSize;				// Source waveform
	const int row12 = row11 + pixelSize8;				// Output group label
	const int row13 = row12 + pixelSize;				// Output waveform
	const int row14 = row13 + pixelSize8;	// Zoom widgets
	const int row15 = row14 + pixelSize;	// Zoom widgets
	
	// Set size
	m_pluginNameComponent.setSize(width, pixelSize2);

	m_sourceGroupLableComponent.setSize(pixelSize15, pixelSize);
	m_regionGroupLableComponent.setSize(pixelSize15, pixelSize);
	m_exportGroupLableComponent.setSize(pixelSize15, pixelSize);
	m_playbackGroupLableComponent.setSize(pixelSize15, pixelSize);
	m_zoomGroupLableComponent.setSize(pixelSize7, pixelSize);

	m_sourceWaveformGroupLableComponent.setSize(pixelSize31, pixelSize);
	m_outputWaveformGroupLableComponent.setSize(pixelSize31, pixelSize);

	m_openSourceButton.setSize(pixelSize3, pixelSize);
	m_sourceFileNameLabel.setSize(pixelSize12, pixelSize);

	m_detectRegionsButton.setSize(pixelSize3, pixelSize);
	m_detectionTypeComboBox.setSize(pixelSize3, pixelSize);
	m_detectedFrequencySlider.setSize(pixelSize6, pixelSize);
	//m_detectFrequencyButton.setSize(pixelSize3, pixelSize);
	 
	m_regionsCountLabel.setSize(pixelSize15, pixelSize);
	//m_validRegionsCountLabel.setSize(pixelSize15, pixelSize);
	m_regionLenghtMedianLabel.setSize(pixelSize15, pixelSize);
	m_maxZeroCrossingGainLabel.setSize(pixelSize15, pixelSize);

	m_generateButton.setSize(pixelSize3, pixelSize);
	m_regionLenghtExportSlider.setSize(pixelSize6, pixelSize);
	m_saveButton.setSize(pixelSize3, pixelSize);

	m_sourceButton.setSize(pixelSize3, pixelSize);
	m_playButton.setSize(pixelSize3, pixelSize);

	m_waveformDisplaySource.setSize(pixelSize31, pixelSize8);
	m_waveformDisplayOutput.setSize(pixelSize31, pixelSize8);

	m_zoomRegionLeft.setSize(pixelSize3, pixelSize);
	m_zoomRegionRight.setSize(pixelSize3, pixelSize);

	m_scrollLeft.setSize(pixelSize, pixelSize);
	m_scrollRight.setSize(pixelSize, pixelSize);

	// Set position
	m_pluginNameComponent.setTopLeftPosition(column1, row1);

	m_sourceGroupLableComponent.setTopLeftPosition(column2, row2);
	m_regionGroupLableComponent.setTopLeftPosition(column8, row2);
	m_exportGroupLableComponent.setTopLeftPosition(column2, row8);
	m_playbackGroupLableComponent.setTopLeftPosition(column8, row8);
	m_zoomGroupLableComponent.setTopLeftPosition(column6, row14);

	m_sourceWaveformGroupLableComponent.setTopLeftPosition(column2, row10);
	m_outputWaveformGroupLableComponent.setTopLeftPosition(column2, row12);

	m_openSourceButton.setTopLeftPosition(column2, row3);
	m_sourceFileNameLabel.setTopLeftPosition(column3, row3);

	m_detectRegionsButton.setTopLeftPosition(column8, row3);
	m_detectionTypeComboBox.setTopLeftPosition(column9, row3);
	m_detectedFrequencySlider.setTopLeftPosition(column11, row3);
	//m_detectFrequencyButton.setTopLeftPosition(column12, row3);

	m_regionsCountLabel.setTopLeftPosition(column8, row4);
	//m_validRegionsCountLabel.setTopLeftPosition(column8, row5);
	m_regionLenghtMedianLabel.setTopLeftPosition(column8, row5);
	m_maxZeroCrossingGainLabel.setTopLeftPosition(column8, row6);

	m_generateButton.setTopLeftPosition(column2, row9);
	m_regionLenghtExportSlider.setTopLeftPosition(column4, row9);
	m_saveButton.setTopLeftPosition(column6, row9);

	m_sourceButton.setTopLeftPosition(column9, row9);
	m_playButton.setTopLeftPosition(column11, row9);

	m_waveformDisplaySource.setTopLeftPosition(column2, row11);
	m_waveformDisplayOutput.setTopLeftPosition(column2, row13);

	m_zoomRegionLeft.setTopLeftPosition(column6, row15);
	m_zoomRegionRight.setTopLeftPosition(column8, row15);

	m_scrollLeft.setTopLeftPosition(column5 + pixelSize2, row15);
	m_scrollRight.setTopLeftPosition(column9, row15);

	/*const int EXTRA_SPACE = 20;
	const int ROW_HEIGHT = 30;
	const int pixelHeight = 20;
	const int pixelHeight12 = 12 * pixelHeight;

	const int row1 = EXTRA_SPACE;
	const int row2 = row1 + ROW_HEIGHT + EXTRA_SPACE;
	const int row3 = row2 + ROW_HEIGHT + EXTRA_SPACE;
	const int row4 = row3 + ROW_HEIGHT;
	const int row5 = row4 + ROW_HEIGHT + EXTRA_SPACE;
	const int row6 = row5 + ROW_HEIGHT + EXTRA_SPACE;
	const int row7 = row6 + ROW_HEIGHT + EXTRA_SPACE;
	const int row8 = row7 + ROW_HEIGHT + EXTRA_SPACE;
	const int row9 = row8 + ROW_HEIGHT + pixelHeight12;

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
	
	m_regionsCountLabel.setBounds			(column1, row4, pixelWidth, pixelHeight);
	m_validRegionsCountLabel.setBounds		(column2, row4, pixelWidth, pixelHeight);
	m_regionLenghtMedianLabel.setBounds		(column3, row4, pixelWidth, pixelHeight);
	m_maxZeroCrossingGainLabel.setBounds	(column4, row4, pixelWidth, pixelHeight);
	
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
	m_waveformDisplaySource.setBounds			(10, row8, getWidth() - 20, pixelHeight12);

	// Waveform output
	m_waveformDisplayOutput.setBounds			(10, row9, getWidth() - 20, pixelHeight12);*/
}
