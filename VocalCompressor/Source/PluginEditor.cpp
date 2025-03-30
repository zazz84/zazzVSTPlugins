#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VocalCompressorAudioProcessorEditor::VocalCompressorAudioProcessorEditor(VocalCompressorAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_gainSlider   (vts, VocalCompressorAudioProcessor::paramsNames[0], VocalCompressorAudioProcessor::paramsUnitNames[0], VocalCompressorAudioProcessor::labelNames[0]),
	m_mixSlider    (vts, VocalCompressorAudioProcessor::paramsNames[1], VocalCompressorAudioProcessor::paramsUnitNames[1], VocalCompressorAudioProcessor::labelNames[1]),
	m_volumeSlider (vts, VocalCompressorAudioProcessor::paramsNames[2], VocalCompressorAudioProcessor::paramsUnitNames[2], VocalCompressorAudioProcessor::labelNames[2]),
	m_typeSlider   (vts, VocalCompressorAudioProcessor::paramsNames[3], VocalCompressorAudioProcessor::paramsUnitNames[3], VocalCompressorAudioProcessor::labelNames[3], {"Warm", "Clean"}),
	m_pluginLabel("zazz::VocalCompressor"),
	m_gainReductionMeter()
{
	addAndMakeVisible(m_gainSlider);
	addAndMakeVisible(m_mixSlider);
	addAndMakeVisible(m_volumeSlider);
	addAndMakeVisible(m_typeSlider);

	addAndMakeVisible(m_pluginLabel);
	addAndMakeVisible(m_gainReductionMeter);

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

VocalCompressorAudioProcessorEditor::~VocalCompressorAudioProcessorEditor()
{
	stopTimer();
}

void VocalCompressorAudioProcessorEditor::timerCallback()
{
	const float peakReductiondB = audioProcessor.getPeakReductiondB();
	m_gainReductionMeter.setLevel(peakReductiondB);

	m_gainReductionMeter.repaint();
}

void VocalCompressorAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void VocalCompressorAudioProcessorEditor::resized()
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

	m_gainSlider.setSize(pixelSize3, pixelSize4);
	m_mixSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);
	m_typeSlider.setSize(pixelSize3, pixelSize4);

	m_gainReductionMeter.setSize(pixelSize2, pixelSize4 + pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;
	const int row3 = row2 + pixelSize4;
	const int row4 = row3 + pixelSize4;

	const int column1 = 0;
	const int column2 = pixelSize2;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_gainSlider.setTopLeftPosition(column2, row2);
	m_volumeSlider.setTopLeftPosition(column3, row2);

	m_typeSlider.setTopLeftPosition(column2, row3);
	m_mixSlider.setTopLeftPosition(column3, row3);

	m_gainReductionMeter.setTopLeftPosition(column4, row2);
}