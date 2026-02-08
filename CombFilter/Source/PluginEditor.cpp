#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CombFilterAudioProcessorEditor::CombFilterAudioProcessorEditor (CombFilterAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_pluginLabel("zazz::CombFilter"),
	m_frequencySlider(vts, CombFilterAudioProcessor::m_parametersDescritpion[CombFilterAudioProcessor::Parameters::Frequency]),
	m_stagesSlider(vts, CombFilterAudioProcessor::m_parametersDescritpion[CombFilterAudioProcessor::Parameters::Stages]),
	m_lowCutSlider(vts, CombFilterAudioProcessor::m_parametersDescritpion[CombFilterAudioProcessor::Parameters::LowCut]),
	m_highCutSlider(vts, CombFilterAudioProcessor::m_parametersDescritpion[CombFilterAudioProcessor::Parameters::HighCut]),
	m_mixSlider(vts, CombFilterAudioProcessor::m_parametersDescritpion[CombFilterAudioProcessor::Parameters::Mix]),
	m_volumeSlider(vts, CombFilterAudioProcessor::m_parametersDescritpion[CombFilterAudioProcessor::Parameters::Volume])
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_frequencySlider);
	addAndMakeVisible(m_stagesSlider);
	addAndMakeVisible(m_lowCutSlider);
	addAndMakeVisible(m_highCutSlider);
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

CombFilterAudioProcessorEditor::~CombFilterAudioProcessorEditor()
{
}

//==============================================================================
void CombFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void CombFilterAudioProcessorEditor::resized()
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
	m_stagesSlider.setSize(pixelSize3, pixelSize4);
	m_lowCutSlider.setSize(pixelSize3, pixelSize4);
	m_highCutSlider.setSize(pixelSize3, pixelSize4);
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
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_frequencySlider.setTopLeftPosition(column2, row2);
	m_stagesSlider.setTopLeftPosition(column3, row2);
	m_lowCutSlider.setTopLeftPosition(column4, row2);
	m_highCutSlider.setTopLeftPosition(column5, row2);
	m_mixSlider.setTopLeftPosition(column6, row2);
	m_volumeSlider.setTopLeftPosition(column7, row2);
}