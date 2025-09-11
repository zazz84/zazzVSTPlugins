#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MonoToStereoAudioProcessorEditor::MonoToStereoAudioProcessorEditor (MonoToStereoAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_delaySlider		(vts, MonoToStereoAudioProcessor::paramsNames[0], MonoToStereoAudioProcessor::paramsUnitNames[0], MonoToStereoAudioProcessor::labelNames[0]),
	m_widthSlider		(vts, MonoToStereoAudioProcessor::paramsNames[1], MonoToStereoAudioProcessor::paramsUnitNames[1], MonoToStereoAudioProcessor::labelNames[1]),
	m_colorSlider		(vts, MonoToStereoAudioProcessor::paramsNames[2], MonoToStereoAudioProcessor::paramsUnitNames[2], MonoToStereoAudioProcessor::labelNames[2]),
	m_modulationSlider	(vts, MonoToStereoAudioProcessor::paramsNames[3], MonoToStereoAudioProcessor::paramsUnitNames[3], MonoToStereoAudioProcessor::labelNames[3]),
	m_volumeSlider		(vts, MonoToStereoAudioProcessor::paramsNames[4], MonoToStereoAudioProcessor::paramsUnitNames[4], MonoToStereoAudioProcessor::labelNames[4]),
	m_pluginLabel("zazz::MonoToStereo")
{
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_delaySlider);
	addAndMakeVisible(m_widthSlider);
	addAndMakeVisible(m_colorSlider);
	addAndMakeVisible(m_modulationSlider);
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

MonoToStereoAudioProcessorEditor::~MonoToStereoAudioProcessorEditor()
{
}

//==============================================================================
void MonoToStereoAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void MonoToStereoAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_delaySlider.setSize(pixelSize3, pixelSize4);
	m_widthSlider.setSize(pixelSize3, pixelSize4);
	m_colorSlider.setSize(pixelSize3, pixelSize4);
	m_modulationSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_delaySlider.setTopLeftPosition		(column2, row2);
	m_widthSlider.setTopLeftPosition		(column3, row2);
	m_colorSlider.setTopLeftPosition		(column4, row2);
	m_modulationSlider.setTopLeftPosition	(column5, row2);
	m_volumeSlider.setTopLeftPosition		(column6, row2);
}