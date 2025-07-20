#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ClipperAudioProcessorEditor::ClipperAudioProcessorEditor (ClipperAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_typeSlider		(vts, ClipperAudioProcessor::paramsNames[0], ClipperAudioProcessor::paramsUnitNames[0], ClipperAudioProcessor::labelNames[0], { "Hard", "Slope", "Soft", "FoldBack", "Half", "ABS" }),
	m_thresholdSlider	(vts, ClipperAudioProcessor::paramsNames[1], ClipperAudioProcessor::paramsUnitNames[1], ClipperAudioProcessor::labelNames[1]),
	m_mixSlider			(vts, ClipperAudioProcessor::paramsNames[2], ClipperAudioProcessor::paramsUnitNames[2], ClipperAudioProcessor::labelNames[2]),
	m_volumeSlider		(vts, ClipperAudioProcessor::paramsNames[3], ClipperAudioProcessor::paramsUnitNames[3], ClipperAudioProcessor::labelNames[3]),
	m_pluginLabel("zazz::Clipper")
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_typeSlider);
	addAndMakeVisible(m_thresholdSlider);
	addAndMakeVisible(m_mixSlider);
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

ClipperAudioProcessorEditor::~ClipperAudioProcessorEditor()
{
}

//==============================================================================
void ClipperAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void ClipperAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_typeSlider.setSize(pixelSize3, pixelSize4);
	m_thresholdSlider.setSize(pixelSize3, pixelSize4);
	m_mixSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = row1 + pixelSize2;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_typeSlider.setTopLeftPosition		(column2, row2);
	m_thresholdSlider.setTopLeftPosition(column3, row2);
	m_mixSlider.setTopLeftPosition		(column4, row2);
	m_volumeSlider.setTopLeftPosition	(column5, row2);
}