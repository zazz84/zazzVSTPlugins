#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : m_waveformDisplaySource("Source"), m_waveformDisplayOutput("Output")
{
	addAndMakeVisible(m_waveformDisplaySource);
	addAndMakeVisible(m_waveformDisplayOutput);

	addAndMakeVisible(m_pluginNameComponent);

	addAndMakeVisible(m_sourceGroupLableComponent);
	addAndMakeVisible(m_regionGroupLableComponent);
	addAndMakeVisible(m_exportGroupLableComponent);
	addAndMakeVisible(m_playbackGroupLableComponent);
	
	// Labels
	//
	addAndMakeVisible(m_sourceFileNameLabel);
	m_sourceFileNameLabel.setText("", juce::dontSendNotification);
	m_sourceFileNameLabel.setJustificationType(juce::Justification::centred);

	addAndMakeVisible(m_regionLenghtMedianLabel);
	m_regionLenghtMedianLabel.setText("", juce::dontSendNotification);
	m_regionLenghtMedianLabel.setJustificationType(juce::Justification::centred);	
	
	addAndMakeVisible(m_regionLengthDiffLabel);
	m_regionLengthDiffLabel.setText("", juce::dontSendNotification);
	m_regionLengthDiffLabel.setJustificationType(juce::Justification::centred);

	addAndMakeVisible(m_validRegionsCountLabel);
	m_validRegionsCountLabel.setText("", juce::dontSendNotification);
	m_validRegionsCountLabel.setJustificationType(juce::Justification::centred);
	//
	addAndMakeVisible(m_detectedFrequencySlider);
	m_detectedFrequencySlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_detectedFrequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_detectedFrequencySlider.setRange(2.0, 200.0, 0.1);   // min, max, step
	m_detectedFrequencySlider.setValue(50.0);              // initial value

	addAndMakeVisible(m_detectedFrequencyLabel);
	m_detectedFrequencyLabel.setText("Filter Freq", juce::dontSendNotification);

	m_detectedFrequencyLabel.attachToComponent(&m_detectedFrequencySlider, true);

	//
	addAndMakeVisible(m_thresholdSlider);
	m_thresholdSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_thresholdSlider.setRange(-60.0, 0.0, 1.0); 
	m_thresholdSlider.setValue(-60.0);

	addAndMakeVisible(m_thresholdLabel);
	m_thresholdLabel.setText("Threshold", juce::dontSendNotification);

	m_thresholdLabel.attachToComponent(&m_thresholdSlider, true);

	//
	addAndMakeVisible(m_SpectrumDifferenceSlider);
	m_SpectrumDifferenceSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_SpectrumDifferenceSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_SpectrumDifferenceSlider.setRange(0.0, 100.0, 1.0);
	m_SpectrumDifferenceSlider.setValue(100.0);

	addAndMakeVisible(m_SpectrumDifferenceLabel);
	m_SpectrumDifferenceLabel.setText("Spectrum", juce::dontSendNotification);

	m_SpectrumDifferenceLabel.attachToComponent(&m_SpectrumDifferenceSlider, true);

	//
	addAndMakeVisible(m_maximumFrequencySlider);
	m_maximumFrequencySlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_maximumFrequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_maximumFrequencySlider.setRange( 2.0, 200.0, 0.1);
	m_maximumFrequencySlider.setValue( 200.0);

	addAndMakeVisible(m_maximumFrequencyLabel);
	m_maximumFrequencyLabel.setText("Max Freq", juce::dontSendNotification);

	m_maximumFrequencyLabel.attachToComponent(&m_maximumFrequencySlider, true);

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
	addAndMakeVisible(m_maxZeroCrossingGainLabel);
	m_maxZeroCrossingGainLabel.setJustificationType(juce::Justification::centred);

	// Export sliders
	addAndMakeVisible(m_exportRegionLeftSlider);
	m_exportRegionLeftSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_exportRegionLeftSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_exportRegionLeftSlider.setRange(0.0, 200.0, 1.0);
	m_exportRegionLeftSlider.setValue(0.0);

	addAndMakeVisible(m_exportRegionLeftLabel);
	m_exportRegionLeftLabel.setText("Reg Left", juce::dontSendNotification);
	m_exportRegionLeftLabel.attachToComponent(&m_exportRegionLeftSlider, true);

	addAndMakeVisible(m_exportRegionRightSlider);
	m_exportRegionRightSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_exportRegionRightSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_exportRegionRightSlider.setRange(0.0, 200.0, 1.0);
	m_exportRegionRightSlider.setValue(0.0);

	addAndMakeVisible(m_exportRegionRightLabel);
	m_exportRegionRightLabel.setText("Reg Right", juce::dontSendNotification);
	m_exportRegionRightLabel.attachToComponent(&m_exportRegionRightSlider, true);

	addAndMakeVisible(m_exportMaxRegionOffsetSlider);
	m_exportMaxRegionOffsetSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_exportMaxRegionOffsetSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_exportMaxRegionOffsetSlider.setRange(0.0, 1000.0, 1.0);
	m_exportMaxRegionOffsetSlider.setValue(1000.0);

	addAndMakeVisible(m_exportMaxRegionOffsetLabel);
	m_exportMaxRegionOffsetLabel.setText("Max Offset", juce::dontSendNotification);
	m_exportMaxRegionOffsetLabel.attachToComponent(&m_exportMaxRegionOffsetSlider, true);

	addAndMakeVisible(m_exportRegionCountSlider);
	m_exportRegionCountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_exportRegionCountSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_exportRegionCountSlider.setRange(0.0, 200.0, 1.0);
	m_exportRegionCountSlider.setValue(200.0);

	addAndMakeVisible(m_exportRegionCountLabel);
	m_exportRegionCountLabel.setText("Reg Count", juce::dontSendNotification);
	m_exportRegionCountLabel.attachToComponent(&m_exportRegionCountSlider, true);

	// Buttons
	addAndMakeVisible(&m_openSourceButton);
	m_openSourceButton.setButtonText("Open");
	m_openSourceButton.onClick = [this] { openSourceButtonClicked(); };

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
	m_detectionTypeComboBox.setSelectedId(1);
	addAndMakeVisible(m_detectionTypeComboBox);

	m_generationTypeComboBox.addItem("Flat", 1);
	m_generationTypeComboBox.addItem("Random Regions", 2);
	m_generationTypeComboBox.setSelectedId(1);
	addAndMakeVisible(m_generationTypeComboBox);
	
	// Canvas size
	const int canvasWidth = CANVAS_WIDTH * 30;
	const int canvasHeight = CANVAS_HEIGHT * 30;

	setSize(canvasWidth, canvasHeight);

	// Force fixes aspect
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
	const int row11 = row10 + pixelSize2;				// Source waveform
	const int row12 = row11 + pixelSize11;				// Output waveform
	
	// Set size
	m_pluginNameComponent.setSize(width, pixelSize2);

	m_sourceGroupLableComponent.setSize(pixelSize15, pixelSize);
	m_regionGroupLableComponent.setSize(pixelSize15, pixelSize);
	m_exportGroupLableComponent.setSize(pixelSize15, pixelSize);
	m_playbackGroupLableComponent.setSize(pixelSize15, pixelSize);

	m_openSourceButton.setSize(pixelSize3, pixelSize);
	m_sourceFileNameLabel.setSize(pixelSize12, pixelSize);

	m_detectRegionsButton.setSize(pixelSize3, pixelSize);
	m_detectionTypeComboBox.setSize(pixelSize3, pixelSize);
	m_detectedFrequencySlider.setSize(pixelSize6, pixelSize);
	m_thresholdSlider.setSize(pixelSize6, pixelSize);
	m_maximumFrequencySlider.setSize(pixelSize6, pixelSize);
	m_exportMaxRegionOffsetSlider.setSize(pixelSize6, pixelSize);
	m_SpectrumDifferenceSlider.setSize(pixelSize6, pixelSize);
	 
	m_regionsCountLabel.setSize(pixelSize6, pixelSize);
	m_validRegionsCountLabel.setSize(pixelSize6, pixelSize);
	m_regionLenghtMedianLabel.setSize(pixelSize6, pixelSize);
	m_regionLengthDiffLabel.setSize(pixelSize6, pixelSize);
	m_maxZeroCrossingGainLabel.setSize(pixelSize6, pixelSize);

	m_generateButton.setSize(pixelSize3, pixelSize);
	m_generationTypeComboBox.setSize(pixelSize9, pixelSize);	
	m_saveButton.setSize(pixelSize3, pixelSize);

	m_regionLenghtExportSlider.setSize(pixelSize12, pixelSize);
	m_exportRegionLeftSlider.setSize(pixelSize12, pixelSize);
	m_exportRegionRightSlider.setSize(pixelSize12, pixelSize);
	m_exportRegionCountSlider.setSize(pixelSize12, pixelSize);

	m_sourceButton.setSize(pixelSize3, pixelSize);
	m_playButton.setSize(pixelSize3, pixelSize);

	m_waveformDisplaySource.setSize(pixelSize31, pixelSize11);
	m_waveformDisplayOutput.setSize(pixelSize31, pixelSize11);

	// Set position
	m_pluginNameComponent.setTopLeftPosition(column1, row1);

	m_sourceGroupLableComponent.setTopLeftPosition(column2, row2);
	m_regionGroupLableComponent.setTopLeftPosition(column8, row2);
	m_exportGroupLableComponent.setTopLeftPosition(column2, row4);
	m_playbackGroupLableComponent.setTopLeftPosition(column8, row9);

	m_openSourceButton.setTopLeftPosition(column2, row3);
	m_sourceFileNameLabel.setTopLeftPosition(column3, row3);

	m_detectRegionsButton.setTopLeftPosition(column8, row3);
	m_detectionTypeComboBox.setTopLeftPosition(column9, row3);

	m_thresholdSlider.setTopLeftPosition(column11, row3);
	m_maximumFrequencySlider.setTopLeftPosition(column11, row4);
	m_detectedFrequencySlider.setTopLeftPosition(column11, row5);
	m_exportMaxRegionOffsetSlider.setTopLeftPosition(column11, row6);
	m_SpectrumDifferenceSlider.setTopLeftPosition(column11, row7);

	m_regionsCountLabel.setTopLeftPosition(column8, row4);
	m_validRegionsCountLabel.setTopLeftPosition(column8, row5);
	m_regionLenghtMedianLabel.setTopLeftPosition(column8, row6);
	m_regionLengthDiffLabel.setTopLeftPosition(column8, row7);
	m_maxZeroCrossingGainLabel.setTopLeftPosition(column8, row8);

	m_generateButton.setTopLeftPosition(column2, row5);
	m_generationTypeComboBox.setTopLeftPosition(column3, row5);
	m_saveButton.setTopLeftPosition(column6, row5);

	m_regionLenghtExportSlider.setTopLeftPosition(column3, row6);
	m_exportRegionLeftSlider.setTopLeftPosition(column3, row7);
	m_exportRegionRightSlider.setTopLeftPosition(column3, row8);
	m_exportRegionCountSlider.setTopLeftPosition(column3, row9);

	m_sourceButton.setTopLeftPosition(column9, row10);
	m_playButton.setTopLeftPosition(column11, row10);

	m_waveformDisplaySource.setTopLeftPosition(column2, row11);
	m_waveformDisplayOutput.setTopLeftPosition(column2, row12);
}
