#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WaveshaperAudioProcessorEditor::WaveshaperAudioProcessorEditor (WaveshaperAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_typeSlider	(vts, WaveshaperAudioProcessor::paramsNames[0], WaveshaperAudioProcessor::paramsUnitNames[0], WaveshaperAudioProcessor::labelNames[0], { "Tanh", "Reciprocal", "Exponential" }),
	m_gainSlider	(vts, WaveshaperAudioProcessor::paramsNames[1], WaveshaperAudioProcessor::paramsUnitNames[1], WaveshaperAudioProcessor::labelNames[1]),
	m_colorSlider	(vts, WaveshaperAudioProcessor::paramsNames[2], WaveshaperAudioProcessor::paramsUnitNames[2], WaveshaperAudioProcessor::labelNames[2]),
	m_splitSlider	(vts, WaveshaperAudioProcessor::paramsNames[3], WaveshaperAudioProcessor::paramsUnitNames[3], WaveshaperAudioProcessor::labelNames[3]),
	m_asymetrySlider(vts, WaveshaperAudioProcessor::paramsNames[4], WaveshaperAudioProcessor::paramsUnitNames[4], WaveshaperAudioProcessor::labelNames[4]),
	m_mixSlider		(vts, WaveshaperAudioProcessor::paramsNames[5], WaveshaperAudioProcessor::paramsUnitNames[5], WaveshaperAudioProcessor::labelNames[5]),
	m_volumeSlider	(vts, WaveshaperAudioProcessor::paramsNames[6], WaveshaperAudioProcessor::paramsUnitNames[6], WaveshaperAudioProcessor::labelNames[6]),
	m_pluginLabel("zazz::Waveshaper")
{	
	addAndMakeVisible(m_typeSlider);
	addAndMakeVisible(m_gainSlider);
	addAndMakeVisible(m_colorSlider);
	addAndMakeVisible(m_splitSlider);
	addAndMakeVisible(m_asymetrySlider);
	addAndMakeVisible(m_mixSlider);
	addAndMakeVisible(m_volumeSlider);

	addAndMakeVisible(m_pluginLabel);

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

WaveshaperAudioProcessorEditor::~WaveshaperAudioProcessorEditor()
{
}

void WaveshaperAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void WaveshaperAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize15 = pixelSize + pixelSize / 2;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_typeSlider.setSize(pixelSize3, pixelSize4);
	m_gainSlider.setSize(pixelSize3, pixelSize4);
	m_colorSlider.setSize(pixelSize3, pixelSize4);
	m_splitSlider.setSize(pixelSize3, pixelSize4);
	m_asymetrySlider.setSize(pixelSize3, pixelSize4);
	m_mixSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;
	const int row3 = row2 + pixelSize4;

	const int column1 = 0;
	const int column2 = pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_typeSlider.setTopLeftPosition(column2, row2);
	m_gainSlider.setTopLeftPosition(column3, row2);
	m_colorSlider.setTopLeftPosition(column4, row2);
	m_splitSlider.setTopLeftPosition(column5, row2);
	m_asymetrySlider.setTopLeftPosition(column6, row2);

	m_mixSlider.setTopLeftPosition(column3, row3);
	m_volumeSlider.setTopLeftPosition(column4, row3);
}