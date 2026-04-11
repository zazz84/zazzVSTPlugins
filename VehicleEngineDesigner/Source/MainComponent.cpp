#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : m_waveformDisplaySource("Source"), m_waveformDisplayOutput("Output"),
								  m_spectrogramDisplaySource("Source"), m_spectrogramDisplayOutput("Output")
{
	m_formatManager.registerBasicFormats();

	setAudioChannels(0, 2);

	// Initialize slider configurations
	initializeSliderConfigs();

	addAndMakeVisible(m_waveformDisplaySource);
	addAndMakeVisible(m_waveformDisplayOutput);
	addAndMakeVisible(m_spectrogramDisplaySource);
	addAndMakeVisible(m_spectrogramDisplayOutput);

	// Initially hide spectrograms, show waveforms
	m_spectrogramDisplaySource.setVisible(false);
	m_spectrogramDisplayOutput.setVisible(false);

	addAndMakeVisible(m_fileGroupLableComponent);
	addAndMakeVisible(m_sourceGroupLableComponent);
	addAndMakeVisible(m_regionGroupLableComponent);
	addAndMakeVisible(m_exportGroupLableComponent);
	addAndMakeVisible(m_playbackGroupLableComponent);
	addAndMakeVisible(m_displayGroupLableComponent);

	// Info labels setup
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

	addAndMakeVisible(m_regionsCountLabel);
	m_regionsCountLabel.setJustificationType(juce::Justification::centred);

	addAndMakeVisible(m_maxZeroCrossingGainLabel);
	m_maxZeroCrossingGainLabel.setJustificationType(juce::Justification::centred);

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
	m_saveButton.onClick = [this] { saveWavButtonClicked(); };

	addAndMakeVisible(&m_exportRegionsButton);
	m_exportRegionsButton.setButtonText("Export Regions");
	m_exportRegionsButton.onClick = [this] { exportRegionsButtonClicked(); };

	addAndMakeVisible(&m_saveProjectButton);
	m_saveProjectButton.setButtonText("Save Project");
	m_saveProjectButton.onClick = [this] { saveProjectButtonClicked(); };

	addAndMakeVisible(&m_loadProjectButton);
	m_loadProjectButton.setButtonText("Load Project");
	m_loadProjectButton.onClick = [this] { loadProjectButtonClicked(); };

	addAndMakeVisible(&m_newProjectButton);
	m_newProjectButton.setButtonText("New Project");
	m_newProjectButton.onClick = [this] { newProjectButtonClicked(); };

	// Play buttons
	addAndMakeVisible(&m_playButton);
	m_playButton.setButtonText("Play");
	m_playButton.onClick = [this] { playSourceButtonClicked(); };

	addAndMakeVisible(&m_sourceButton);
	m_sourceButton.setButtonText("Source");
	m_sourceButton.onClick = [this] { sourceButtonClicked(); };

	addAndMakeVisible(&m_displayModeButton);
	m_displayModeButton.setButtonText("Waveform");
	m_displayModeButton.onClick = [this] { displayModeButtonClicked(); };

	// Combo boxes
	m_detectionTypeComboBox.addItem("Time Domain", 1);
	m_detectionTypeComboBox.addItem("Time Domain + Filter", 2);
	m_detectionTypeComboBox.addItem("FFT", 3);
	m_detectionTypeComboBox.addItem("FFT + Filter", 4);
	m_detectionTypeComboBox.setSelectedId(1);
	m_detectionTypeComboBox.onChange = [this] {
		updateDetectionModeUI(m_detectionTypeComboBox.getSelectedId());
	};
	addAndMakeVisible(m_detectionTypeComboBox);

	m_generationTypeComboBox.addItem("Flat", 1);
	m_generationTypeComboBox.addItem("Random Regions", 2);
	m_generationTypeComboBox.setSelectedId(1);
	m_generationTypeComboBox.onChange = [this] {
		const int generationTypeId = m_generationTypeComboBox.getSelectedId();

		// Show crossfade slider only for "Random Regions" mode (id = 2)
		const bool showCrossfade = (generationTypeId == 2);

		m_crossfadeLengthSlider.setVisible(showCrossfade);
		m_crossfadeLengthLabel.setVisible(showCrossfade);
		m_exportRegionCountSlider.setVisible(showCrossfade);
		m_exportRegionCountLabel.setVisible(showCrossfade);

	};
	addAndMakeVisible(m_generationTypeComboBox);

	// Spectrum source combo box
	m_spectrumSourceComboBox.addItem("Average", 1);
	m_spectrumSourceComboBox.addItem("Median", 2);
	m_spectrumSourceComboBox.setSelectedId(1);
	m_spectrumSourceComboBox.onChange = [this] {
		int selectedId = m_spectrumSourceComboBox.getSelectedId();

		if (selectedId == 1)
		{
			// "Average" mode
			m_selectedSpectrumRegionIndex = -1;
			m_useMedianSpectrum = false;
		}
		else if (selectedId == 2)
		{
			// "Median" mode
			m_selectedSpectrumRegionIndex = -2;
			m_useMedianSpectrum = true;
		}
		else
		{
			// Specific region selected (selectedId - 3 because ID 1 is "Average", ID 2 is "Median")
			m_selectedSpectrumRegionIndex = selectedId - 3;
			m_useMedianSpectrum = false;
		}
	};
	addAndMakeVisible(m_spectrumSourceComboBox);

	addAndMakeVisible(m_spectrumSourceLabel);
	m_spectrumSourceLabel.setText("FFT Source", juce::dontSendNotification);
	m_spectrumSourceLabel.attachToComponent(&m_spectrumSourceComboBox, true);

	// Initially hide crossfade slider for Flat mode
	m_crossfadeLengthSlider.setVisible(false);
	m_crossfadeLengthLabel.setVisible(false);

	// Initially hide low-pass frequency slider (show only in Time Domain + Filter mode)
	m_lowPassFrequencySlider.setVisible(false);
	m_lowPassFrequencyLabel.setVisible(false);

	// Initially hide minimum length multiplier slider (show only in FFT + Filter mode)
	m_minimumLengthMultiplierSlider.setVisible(false);
	m_minimumLengthMultiplierLabel.setVisible(false);

	// Canvas size
	const int canvasWidth = CANVAS_WIDTH * PIXEL_SIZE;
	const int canvasHeight = CANVAS_HEIGHT * PIXEL_SIZE;

	setSize(canvasWidth, canvasHeight);
	setSize(getWidth(), getHeight());
}

MainComponent::~MainComponent()
{
	// This shuts down the audio device and clears the audio source.
	shutdownAudio();
}

//==============================================================================
void MainComponent::initializeSliderConfigs()
{
	// Create all slider configurations with metadata
	// Format: Slider*, Label*, JSON key, min, max, default, step, 
	//         label text, style, textbox width, textbox height, attach label

	m_sliderConfigs = {
		// Detection sliders
		SliderConfig(
			&m_thresholdSlider,
			&m_thresholdLabel,
			"threshold",
			-60.0, 0.0, -60.0, 1.0,
			"Threshold"),

		SliderConfig(
			&m_minimumLengthSlider,
			&m_maximumFrequencyLabel,
			"minimumLength",
			10.0, 5000.0, 100.0, 1.0,
			"Min Length"),

		SliderConfig(
			&m_minimumLengthMultiplierSlider,
			&m_minimumLengthMultiplierLabel,
			"minimumLengthMultiplier",
			0.5, 2.0, 0.8, 0.1,
			"Length Mult"),

		SliderConfig(
			&m_SpectrumDifferenceSlider,
			&m_SpectrumDifferenceLabel,
			"spectrumDifference",
			0.0, 200.0, 100.0, 1.0,
			"Spectrum"),

		SliderConfig(
			&m_fftPhaseThresholdSlider,
			&m_fftPhaseThresholdLabel,
			"fftPhaseThreshold",
			-180.0, 180.0, 0.0, 1.0,
			"Phase"),

		SliderConfig(
			&m_zeroCrossingCountSlider,
			&m_zeroCrossingCountLabel,
			"zeroCrossingCount",
			1.0, 12.0, 1.0, 1.0,
			"Multiplier"),

		SliderConfig(
			&m_lowPassFrequencySlider,
			&m_lowPassFrequencyLabel,
			"lowPassFrequency",
			10.0, 2000.0, 500.0, 1.0,
			"Low Pass"),

		// Generation sliders
		SliderConfig(
			&m_crossfadeLengthSlider,
			&m_crossfadeLengthLabel,
			"crossfadeLength",
			0.0, 200.0, 0.0, 1.0,
			"Crossfade"),

		SliderConfig(
			&m_regionLenghtExportSlider,
			&m_regionLenghtExportLabel,
			"regionLenghtExport",
			100.0, 50000.0, 1000.0, 1.0,
			"Reg Lenght"),

		// Export range sliders (ranges are set dynamically)
		SliderConfig(
			&m_exportRegionLeftSlider,
			&m_exportRegionLeftLabel,
			"exportRegionLeft",
			0.0, 1.0, 0.0, 1.0,
			"Reg Left"),

		SliderConfig(
			&m_exportRegionRightSlider,
			&m_exportRegionRightLabel,
			"exportRegionRight",
			0.0, 1.0, 0.0, 1.0,
			"Reg Right"),

		SliderConfig(
			&m_exportMaxRegionOffsetSlider,
			&m_exportMaxRegionOffsetLabel,
			"exportMaxRegionOffset",
			0.0, 5000.0, 1000.0, 1.0,
			"Max Offset"),

		SliderConfig(
			&m_exportRegionCountSlider,
			&m_exportRegionCountLabel,
			"exportRegionCount",
			1.0, 1000.0, 200.0, 1.0,
			"Reg Count"),

		// Spectrum matching slider
		SliderConfig(
			&m_spectrumMatchIntensitySlider,
			&m_spectrumMatchIntensityLabel,
			"spectrumMatchIntensity",
			0.0, 100.0, 0.0, 1.0,
			"FFT Match",
			juce::Slider::LinearHorizontal,
			80, 20, true,
			[this](double value) { spectrumMatchIntensitySliderChanged(); })
	};

	// Setup UI for all sliders (addAndMakeVisible, setSliderStyle, attach labels, etc.)
	SliderManager::setupSliderUI(m_sliderConfigs, this);

	// Initialize all sliders with their value configurations
	SliderManager::initializeSliders(m_sliderConfigs);
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
	if (m_sourceState == TransportState::Playing)
	{
		std::lock_guard<std::mutex> lock(m_bufferMutex);

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
    g.fillAll(zazzGUI::Colors::darkColor);
}

void MainComponent::resized()
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
	const int pizelSize63 = 4 * pixelSize15 + pixelSize3;

	const int column1 = 0;								
	const int column2 = column1 + pixelSize;			// First main column	
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;			// Gap
	const int column8 = column7 + pixelSize;			// Second main column
	const int column9 = column8 + pixelSize3;		
	const int column10 = column9 + pixelSize3;
	const int column11 = column10 + pixelSize3;
	const int column12 = column11 + pixelSize3;
	const int column13 = column12 + pixelSize3;			// Gap
	const int column14 = column13 + pixelSize;			// Third main column
	const int column15 = column14 + pixelSize3;
	const int column16 = column15 + pixelSize3;
	const int column17 = column16 + pixelSize3;
	const int column18 = column17 + pixelSize3;
	const int column19 = column18 + pixelSize3;			// Gap
	const int column20 = column19 + pixelSize;			// Forth main column
	const int column21 = column20 + pixelSize3;
	const int column22 = column21 + pixelSize3;
	const int column23 = column22 + pixelSize3;
	const int column24 = column23 + pixelSize3;


	const int row1 = 0;
	const int row2 = row1 + pixelSize;
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
	
	// Set size
	m_fileGroupLableComponent.setSize(pixelSize15, pixelSize);
	m_sourceGroupLableComponent.setSize(pixelSize15, pixelSize);
	m_regionGroupLableComponent.setSize(pixelSize15, pixelSize);
	m_exportGroupLableComponent.setSize(pixelSize15, pixelSize);
	m_playbackGroupLableComponent.setSize(pixelSize6, pixelSize);
	m_displayGroupLableComponent.setSize(pixelSize3, pixelSize);

	m_openSourceButton.setSize(pixelSize3, pixelSize);
	m_sourceFileNameLabel.setSize(pixelSize12, pixelSize);

	m_detectRegionsButton.setSize(pixelSize3, pixelSize);
	m_detectionTypeComboBox.setSize(pixelSize15, pixelSize);
	m_thresholdSlider.setSize(pixelSize12, pixelSize);
	m_minimumLengthSlider.setSize(pixelSize12, pixelSize);
	m_minimumLengthMultiplierSlider.setSize(pixelSize12, pixelSize);  // NEW: Size for multiplier slider
	m_lowPassFrequencySlider.setSize(pixelSize12, pixelSize);
	m_exportMaxRegionOffsetSlider.setSize(pixelSize12, pixelSize);
	m_SpectrumDifferenceSlider.setSize(pixelSize12, pixelSize);
	m_fftPhaseThresholdSlider.setSize(pixelSize12, pixelSize);
	m_zeroCrossingCountSlider.setSize(pixelSize12, pixelSize);

	m_regionsCountLabel.setSize(pixelSize6, pixelSize);
	m_validRegionsCountLabel.setSize(pixelSize6, pixelSize);
	m_regionLenghtMedianLabel.setSize(pixelSize6, pixelSize);
	m_regionLengthDiffLabel.setSize(pixelSize6, pixelSize);
	m_maxZeroCrossingGainLabel.setSize(pixelSize6, pixelSize);

	m_generateButton.setSize(pixelSize3, pixelSize);
	m_generationTypeComboBox.setSize(pixelSize15, pixelSize);	
	m_saveButton.setSize(pixelSize3, pixelSize);
	m_exportRegionsButton.setSize(pixelSize3, pixelSize);
	m_saveProjectButton.setSize(pixelSize3, pixelSize);
	m_loadProjectButton.setSize(pixelSize3, pixelSize);
	m_newProjectButton.setSize(pixelSize3, pixelSize);

	m_regionLenghtExportSlider.setSize(pixelSize12, pixelSize);
	m_exportRegionLeftSlider.setSize(pixelSize12, pixelSize);
	m_exportRegionRightSlider.setSize(pixelSize12, pixelSize);
	m_exportRegionCountSlider.setSize(pixelSize12, pixelSize);
	m_crossfadeLengthSlider.setSize(pixelSize12, pixelSize);

	m_spectrumSourceComboBox.setSize(pixelSize12, pixelSize);
	m_spectrumMatchIntensitySlider.setSize(pixelSize12, pixelSize);

	m_sourceButton.setSize(pixelSize3, pixelSize);
	m_playButton.setSize(pixelSize3, pixelSize);
	m_displayModeButton.setSize(pixelSize3, pixelSize);

	m_waveformDisplaySource.setSize(pizelSize63, pixelSize11);
	m_waveformDisplayOutput.setSize(pizelSize63, pixelSize11);
	m_spectrogramDisplaySource.setSize(pizelSize63, pixelSize11);
	m_spectrogramDisplayOutput.setSize(pizelSize63, pixelSize11);

	// Set position
	
	// Main column 1
	// File
	m_fileGroupLableComponent.setTopLeftPosition(column2, row1);
	m_newProjectButton.setTopLeftPosition(column3, row2);
	m_loadProjectButton.setTopLeftPosition(column4, row2);
	m_saveProjectButton.setTopLeftPosition(column5, row2);

	// Source
	m_sourceGroupLableComponent.setTopLeftPosition(column2, row3);
	m_openSourceButton.setTopLeftPosition(column2, row4);
	m_sourceFileNameLabel.setTopLeftPosition(column3, row4);
	
	// Main column 2
	// Detect regions
	m_regionGroupLableComponent.setTopLeftPosition(column8, row1);
	m_detectionTypeComboBox.setTopLeftPosition(column8, row2);

	m_minimumLengthSlider.setTopLeftPosition(column9, row3);
	m_minimumLengthMultiplierSlider.setTopLeftPosition(column9, row3);  // Same position as m_minimumLengthSlider
	m_exportMaxRegionOffsetSlider.setTopLeftPosition(column9, row4);
	m_zeroCrossingCountSlider.setTopLeftPosition(column9, row5);
	m_SpectrumDifferenceSlider.setTopLeftPosition(column9, row6);
	m_thresholdSlider.setTopLeftPosition(column9, row7);	
	m_fftPhaseThresholdSlider.setTopLeftPosition(column9, row7);
	m_lowPassFrequencySlider.setTopLeftPosition(column9, row8);
	
	// Info labels
	m_regionsCountLabel.setTopLeftPosition(column2, row8);
	m_validRegionsCountLabel.setTopLeftPosition(column2, row9);
	m_regionLenghtMedianLabel.setTopLeftPosition(column2, row10);
	m_regionLengthDiffLabel.setTopLeftPosition(column5, row8);
	m_maxZeroCrossingGainLabel.setTopLeftPosition(column5, row9);

	m_detectRegionsButton.setTopLeftPosition(column10, row11);

	// Main column 3
	// Generate
	m_exportGroupLableComponent.setTopLeftPosition(column14, row1);
	m_generationTypeComboBox.setTopLeftPosition(column14, row2);

	m_regionLenghtExportSlider.setTopLeftPosition(column15, row3);
	m_exportRegionLeftSlider.setTopLeftPosition(column15, row4);
	m_exportRegionRightSlider.setTopLeftPosition(column15, row5);
	m_exportRegionCountSlider.setTopLeftPosition(column15, row6);
	m_crossfadeLengthSlider.setTopLeftPosition(column15, row7);
	m_spectrumSourceComboBox.setTopLeftPosition(column15, row8);
	m_spectrumMatchIntensitySlider.setTopLeftPosition(column15, row9);

	m_generateButton.setTopLeftPosition(column15, row11);
	m_saveButton.setTopLeftPosition(column16, row11);
	m_exportRegionsButton.setTopLeftPosition(column17, row11);

	// Main column 4
	// Playback
	m_playbackGroupLableComponent.setTopLeftPosition(column20, row1);	
	m_sourceButton.setTopLeftPosition(column20, row2);
	m_playButton.setTopLeftPosition(column21, row2);

	//Display
	m_displayGroupLableComponent.setTopLeftPosition(column20, row3);
	m_displayModeButton.setTopLeftPosition(column20, row4);	
	
	// Waveform and spectrogram displays (overlapping, visibility controlled by display mode button)
	m_waveformDisplaySource.setTopLeftPosition(column2, row12);
	m_waveformDisplayOutput.setTopLeftPosition(column2, row13);
	m_spectrogramDisplaySource.setTopLeftPosition(column2, row12);
	m_spectrogramDisplayOutput.setTopLeftPosition(column2, row13);
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
	if (key.getKeyCode() == juce::KeyPress::spaceKey)
	{
		playSourceButtonClicked();
		return true;
	}
	return false;
}