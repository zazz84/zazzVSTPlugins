#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EnvelopeClonerAudioProcessorEditor::EnvelopeClonerAudioProcessorEditor (EnvelopeClonerAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_dynamicsSlider	(vts, EnvelopeClonerAudioProcessor::paramsNames[0], EnvelopeClonerAudioProcessor::paramsUnitNames[0], EnvelopeClonerAudioProcessor::labelNames[0]),
	m_spectrumSlider	(vts, EnvelopeClonerAudioProcessor::paramsNames[1], EnvelopeClonerAudioProcessor::paramsUnitNames[1], EnvelopeClonerAudioProcessor::labelNames[1]),
	m_attackSlider		(vts, EnvelopeClonerAudioProcessor::paramsNames[2], EnvelopeClonerAudioProcessor::paramsUnitNames[2], EnvelopeClonerAudioProcessor::labelNames[2]),
	m_releaseSlider		(vts, EnvelopeClonerAudioProcessor::paramsNames[3], EnvelopeClonerAudioProcessor::paramsUnitNames[3], EnvelopeClonerAudioProcessor::labelNames[3]),
	m_mixSlider			(vts, EnvelopeClonerAudioProcessor::paramsNames[4], EnvelopeClonerAudioProcessor::paramsUnitNames[4], EnvelopeClonerAudioProcessor::labelNames[4]),
	m_volumeSlider		(vts, EnvelopeClonerAudioProcessor::paramsNames[5], EnvelopeClonerAudioProcessor::paramsUnitNames[5], EnvelopeClonerAudioProcessor::labelNames[5]),
	m_pluginLabel("zazz::EnvelopeCloner"),
	m_gainMeterLow(),
	m_gainMeterMid(),
	m_gainMeterHigh()
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_dynamicsSlider);
	addAndMakeVisible(m_spectrumSlider);
	addAndMakeVisible(m_attackSlider);
	addAndMakeVisible(m_releaseSlider);
	addAndMakeVisible(m_mixSlider);
	addAndMakeVisible(m_volumeSlider);

	addAndMakeVisible(m_gainMeterLow);
	addAndMakeVisible(m_gainMeterMid);
	addAndMakeVisible(m_gainMeterHigh);

	setResizable(true, true);

	const int canvasWidth = CANVAS_WIDTH * 30;
	const int canvasHeight = CANVAS_HEIGHT * 30;

	setSize(canvasWidth, canvasHeight);

	if (auto* constrainer = getConstrainer())
	{
		constexpr int minScale = 50;		// percentage
		constexpr int maxScale = 200;		// percentage

		constrainer->setFixedAspectRatio((double)canvasWidth / (double)canvasHeight);
		constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
	}

	startTimerHz(20);
}

EnvelopeClonerAudioProcessorEditor::~EnvelopeClonerAudioProcessorEditor()
{
	stopTimer();;
}

//==============================================================================
void EnvelopeClonerAudioProcessorEditor::timerCallback()
{
	float low, mid, high;

	audioProcessor.getMaxGain(low, mid, high);
	
	m_gainMeterLow.setLevel(low);
	m_gainMeterMid.setLevel(mid);
	m_gainMeterHigh.setLevel(high);

	m_gainMeterLow.repaint();
	m_gainMeterMid.repaint();
	m_gainMeterHigh.repaint();
}

void EnvelopeClonerAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void EnvelopeClonerAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize12 = pixelSize4 + pixelSize4 + pixelSize4;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_dynamicsSlider.setSize(pixelSize3, pixelSize4);
	m_spectrumSlider.setSize(pixelSize3, pixelSize4);
	m_attackSlider.setSize(pixelSize3, pixelSize4);
	m_releaseSlider.setSize(pixelSize3, pixelSize4);
	m_mixSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	m_gainMeterLow.setSize(pixelSize2, pixelSize12);
	m_gainMeterMid.setSize(pixelSize2, pixelSize12);
	m_gainMeterHigh.setSize(pixelSize2, pixelSize12);

	//Set position
	const int row1 = 0;
	const int row2 = row1 + pixelSize2;
	const int row3 = row2 + pixelSize4;
	const int row4 = row3 + pixelSize4;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3 + pixelSize;
	const int column5 = column4 + pixelSize2;
	const int column6 = column5 + pixelSize2;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_dynamicsSlider.setTopLeftPosition	(column2, row2);
	m_spectrumSlider.setTopLeftPosition	(column3, row2);
	m_attackSlider.setTopLeftPosition	(column2, row3);
	m_releaseSlider.setTopLeftPosition	(column3, row3);
	m_mixSlider.setTopLeftPosition		(column2, row4);
	m_volumeSlider.setTopLeftPosition	(column3, row4);

	m_gainMeterLow.setTopLeftPosition	(column4, row2);
	m_gainMeterMid.setTopLeftPosition	(column5, row2);
	m_gainMeterHigh.setTopLeftPosition	(column6, row2);
}