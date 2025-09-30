#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	addAndMakeVisible(waveformDisplaySource);
	
	// Labels
	//
	addAndMakeVisible(m_detectedFrequencySlider);
	m_detectedFrequencySlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_detectedFrequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_detectedFrequencySlider.setRange(1.0, 1000.0, 1.0);   // min, max, step
	m_detectedFrequencySlider.setValue(50.0);              // initial value

	addAndMakeVisible(m_detectedFrequencyLabel);
	m_detectedFrequencyLabel.setText("Detected frequency", juce::dontSendNotification);

	m_detectedFrequencyLabel.attachToComponent(&m_detectedFrequencySlider, true);

	//
	addAndMakeVisible(m_regionLenghtSlider);
	m_regionLenghtSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	m_regionLenghtSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
	m_regionLenghtSlider.setRange(0.0, 5000.0, 1.0);        // min, max, step
	m_regionLenghtSlider.setValue(0.0);                     // initial value

	addAndMakeVisible(m_regionLenghtLabel);
	m_regionLenghtLabel.setText("Region Lenght", juce::dontSendNotification);
	
	m_regionLenghtLabel.attachToComponent(&m_regionLenghtSlider, true);

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

	// Buttons
	addAndMakeVisible(&m_openSourceButton);
	m_openSourceButton.setButtonText("Open...");
	m_openSourceButton.onClick = [this] { openSourceButtonClicked(); };

	addAndMakeVisible(&m_detectFrequencyButton);
	m_detectFrequencyButton.setButtonText("Detect Freqeuncy");
	m_detectFrequencyButton.onClick = [this] { detectFrequencyButtonClicked(); };
	m_detectFrequencyButton.setEnabled(false);

	addAndMakeVisible(&m_detectRegionsButton);
	m_detectRegionsButton.setButtonText("Detect Regions");
	m_detectRegionsButton.onClick = [this] { detectRegionsButtonClicked(); };
	m_detectRegionsButton.setEnabled(false);

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
	m_playButton.setEnabled(false);

	addAndMakeVisible(&m_stopButton);
	m_stopButton.setButtonText("Stop");
	m_stopButton.onClick = [this] { stopSourceButtonClicked(); };
	m_stopButton.setEnabled(false);

	setSize(1600, 900);

	formatManager.registerBasicFormats();       // [1]
	m_transportSource.addChangeListener(this);   // [2]

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
	m_transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
	if (readerSource.get() == nullptr)
	{
		bufferToFill.clearActiveBufferRegion();
		return;
	}

	m_transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
	m_transportSource.releaseResources();
	//m_bufferSource.clear();
	//m_bufferOutput.clear();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

	updateValidZeroCrossingIdx();

	drawRegions(g);
}

void MainComponent::resized()
{
	const int row1 = 10;
	const int row2 = row1 + 30 + 10;
	const int row3 = row2 + 30;
	const int row4 = row3 + 30 + 10;
	const int row5 = row4 + 30;
	const int row6 = row5 + 30;
	const int row7 = row6 + 30;
	const int row8 = row7 + 30 + 10;
	const int row9 = row8 + 30;
	const int row10 = row9 + 30 + 10;
	const int row11 = row10 + 30;
	const int row12 = row11 + 30;

	const int width = getWidth();
	
	const int pixelWidth = 300;
	const int pixelWidth2 = pixelWidth + pixelWidth;
	const int pixelWidth4 = pixelWidth2 + pixelWidth2;

	const int pixelHeight = 20;

	const int column1 = (getWidth() / 2) - 600;
	const int column2 = column1 + 300;
	const int column3 = column2 + 300;
	const int column4 = column3 + 300;
	
	//
	m_openSourceButton.setBounds			(column1, row1, pixelWidth4, pixelHeight);
	
	m_detectFrequencyButton.setBounds		(column1, row2, pixelWidth4, pixelHeight);
	m_detectedFrequencySlider.setBounds		(column2, row3, pixelWidth2, pixelHeight);
	
	m_detectRegionsButton.setBounds			(column1, row4, pixelWidth4, pixelHeight);
	m_regionLenghtSlider.setBounds			(column2, row5, pixelWidth2, pixelHeight);
	m_regionOffsetLenghtSlider.setBounds	(column2, row6, pixelWidth2, pixelHeight);
	
	m_regionsCountLabel.setBounds			(column2, row7, pixelWidth, pixelHeight);
	m_validRegionsCountLabel.setBounds		(column3, row7, pixelWidth, pixelHeight);
	
	m_regionLenghtExportSlider.setBounds	(column2, row8, pixelWidth2, pixelHeight);
	m_generateButton.setBounds				(column1, row9, pixelWidth4, pixelHeight);
	m_saveButton.setBounds					(column1, row10, pixelWidth4, pixelHeight);

	// Play buttons
	m_playButton.setBounds					(column2, row11, pixelWidth, pixelHeight);
	m_stopButton.setBounds					(column3, row11, pixelWidth, pixelHeight);

	
	// Waveform
	juce::Rectangle rectangle = getLocalBounds();
	rectangle.removeFromTop(450);
	waveformDisplaySource.setBounds(rectangle);
}
