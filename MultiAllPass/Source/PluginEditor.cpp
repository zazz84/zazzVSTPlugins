#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MultiAllPassAudioProcessorEditor::MultiAllPassAudioProcessorEditor (MultiAllPassAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_frequencySlider	(vts, MultiAllPassAudioProcessor::paramsNames[0], MultiAllPassAudioProcessor::paramsUnitNames[0], MultiAllPassAudioProcessor::labelNames[0]),
	m_styleSlider		(vts, MultiAllPassAudioProcessor::paramsNames[1], MultiAllPassAudioProcessor::paramsUnitNames[1], MultiAllPassAudioProcessor::labelNames[1]),
	m_intensitySlider	(vts, MultiAllPassAudioProcessor::paramsNames[2], MultiAllPassAudioProcessor::paramsUnitNames[2], MultiAllPassAudioProcessor::labelNames[2]),
	m_volumeSlider		(vts, MultiAllPassAudioProcessor::paramsNames[3], MultiAllPassAudioProcessor::paramsUnitNames[3], MultiAllPassAudioProcessor::labelNames[3]),
	m_pluginLabel("zazz::MultiAllPass")
{
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_frequencySlider);
	addAndMakeVisible(m_styleSlider);
	addAndMakeVisible(m_intensitySlider);
	addAndMakeVisible(m_volumeSlider);

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
}

MultiAllPassAudioProcessorEditor::~MultiAllPassAudioProcessorEditor()
{
}

//==============================================================================
void MultiAllPassAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void MultiAllPassAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_frequencySlider.setSize(pixelSize3, pixelSize4);
	m_styleSlider.setSize(pixelSize3, pixelSize4);
	m_intensitySlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_frequencySlider.setTopLeftPosition	(column2, row2);
	m_styleSlider.setTopLeftPosition		(column3, row2);
	m_intensitySlider.setTopLeftPosition	(column4, row2);
	m_volumeSlider.setTopLeftPosition		(column5, row2);
}