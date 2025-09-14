#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MonoToStereoAudioProcessorEditor::MonoToStereoAudioProcessorEditor (MonoToStereoAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_delaySlider			(vts, MonoToStereoAudioProcessor::paramsNames[MonoToStereoAudioProcessor::ParamsName::Delay], MonoToStereoAudioProcessor::paramsUnitNames[MonoToStereoAudioProcessor::ParamsName::Delay], MonoToStereoAudioProcessor::labelNames[MonoToStereoAudioProcessor::ParamsName::Delay]),
	m_widthSlider			(vts, MonoToStereoAudioProcessor::paramsNames[MonoToStereoAudioProcessor::ParamsName::Width], MonoToStereoAudioProcessor::paramsUnitNames[MonoToStereoAudioProcessor::ParamsName::Width], MonoToStereoAudioProcessor::labelNames[MonoToStereoAudioProcessor::ParamsName::Width]),
	m_colorSlider			(vts, MonoToStereoAudioProcessor::paramsNames[MonoToStereoAudioProcessor::ParamsName::Color], MonoToStereoAudioProcessor::paramsUnitNames[MonoToStereoAudioProcessor::ParamsName::Color], MonoToStereoAudioProcessor::labelNames[MonoToStereoAudioProcessor::ParamsName::Color]),
	m_HPSlider				(vts, MonoToStereoAudioProcessor::paramsNames[MonoToStereoAudioProcessor::ParamsName::HP], MonoToStereoAudioProcessor::paramsUnitNames[MonoToStereoAudioProcessor::ParamsName::HP], MonoToStereoAudioProcessor::labelNames[MonoToStereoAudioProcessor::ParamsName::HP]),
	m_dynamicSlider			(vts, MonoToStereoAudioProcessor::paramsNames[MonoToStereoAudioProcessor::ParamsName::Dynamic], MonoToStereoAudioProcessor::paramsUnitNames[MonoToStereoAudioProcessor::ParamsName::Dynamic], MonoToStereoAudioProcessor::labelNames[MonoToStereoAudioProcessor::ParamsName::Dynamic]),
	m_dynamicSpeedSlider	(vts, MonoToStereoAudioProcessor::paramsNames[MonoToStereoAudioProcessor::ParamsName::DynamicSpeed], MonoToStereoAudioProcessor::paramsUnitNames[MonoToStereoAudioProcessor::ParamsName::DynamicSpeed], MonoToStereoAudioProcessor::labelNames[MonoToStereoAudioProcessor::ParamsName::DynamicSpeed]),
	m_modulationDepthSlider	(vts, MonoToStereoAudioProcessor::paramsNames[MonoToStereoAudioProcessor::ParamsName::ModulationDepth], MonoToStereoAudioProcessor::paramsUnitNames[MonoToStereoAudioProcessor::ParamsName::ModulationDepth], MonoToStereoAudioProcessor::labelNames[MonoToStereoAudioProcessor::ParamsName::ModulationDepth]),
	m_modulationSpeedSlider	(vts, MonoToStereoAudioProcessor::paramsNames[MonoToStereoAudioProcessor::ParamsName::ModulationSpeed], MonoToStereoAudioProcessor::paramsUnitNames[MonoToStereoAudioProcessor::ParamsName::ModulationSpeed], MonoToStereoAudioProcessor::labelNames[MonoToStereoAudioProcessor::ParamsName::ModulationSpeed]),
	m_volumeSlider			(vts, MonoToStereoAudioProcessor::paramsNames[MonoToStereoAudioProcessor::ParamsName::Volume], MonoToStereoAudioProcessor::paramsUnitNames[MonoToStereoAudioProcessor::ParamsName::Volume], MonoToStereoAudioProcessor::labelNames[MonoToStereoAudioProcessor::ParamsName::Volume]),
	m_pluginLabel("zazz::MonoToStereo"),
	m_stereoWidthMeter(),
	m_colorGroupLabel("COL"),
	m_dynamicGroupLabel("DYN"),
	m_modulationGroupLabel("MOD")
{
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_colorGroupLabel);
	addAndMakeVisible(m_dynamicGroupLabel);
	addAndMakeVisible(m_modulationGroupLabel);

	addAndMakeVisible(m_stereoWidthMeter);

	addAndMakeVisible(m_delaySlider);
	addAndMakeVisible(m_widthSlider);
	addAndMakeVisible(m_colorSlider);
	addAndMakeVisible(m_HPSlider);
	addAndMakeVisible(m_dynamicSlider);
	addAndMakeVisible(m_dynamicSpeedSlider);
	addAndMakeVisible(m_modulationDepthSlider);
	addAndMakeVisible(m_modulationSpeedSlider);
	addAndMakeVisible(m_volumeSlider);

	m_volumeSlider.setSliderType(ModernRotarySliderLookAndFeel::SliderType::Dots);

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

	startTimerHz(60);
}

MonoToStereoAudioProcessorEditor::~MonoToStereoAudioProcessorEditor()
{
	stopTimer();
}

//==============================================================================
void MonoToStereoAudioProcessorEditor::timerCallback()
{
	const float stereoWidth = audioProcessor.getStereoWidth();
	m_stereoWidthMeter.set(stereoWidth);

	m_stereoWidthMeter.repaint();
}

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
	m_stereoWidthMeter.setSize(width - pixelSize2, pixelSize2);

	m_colorGroupLabel.setSize(pixelSize3, pixelSize);
	m_dynamicGroupLabel.setSize(pixelSize3, pixelSize);
	m_modulationGroupLabel.setSize(pixelSize3, pixelSize);

	m_delaySlider.setSize(pixelSize3, pixelSize4);
	m_widthSlider.setSize(pixelSize3, pixelSize4);
	m_colorSlider.setSize(pixelSize3, pixelSize4);
	m_HPSlider.setSize(pixelSize3, pixelSize4);
	m_dynamicSlider.setSize(pixelSize3, pixelSize4);
	m_dynamicSpeedSlider.setSize(pixelSize3, pixelSize4);
	m_modulationDepthSlider.setSize(pixelSize3, pixelSize4);
	m_modulationSpeedSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = row1 + pixelSize2;
	const int row3 = row2 + pixelSize;
	const int row4 = row3 + pixelSize4;
	const int row5 = row4 + pixelSize4 + pixelSize / 2;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);
	m_stereoWidthMeter.setTopLeftPosition(column2, row5);

	m_colorGroupLabel.setTopLeftPosition		(column3, row2);
	m_dynamicGroupLabel.setTopLeftPosition		(column4, row2);
	m_modulationGroupLabel.setTopLeftPosition	(column5, row2);

	m_delaySlider.setTopLeftPosition			(column2, row3);
	m_widthSlider.setTopLeftPosition			(column2, row4);

	m_colorSlider.setTopLeftPosition			(column3, row3);
	m_HPSlider.setTopLeftPosition				(column3, row4);

	m_dynamicSlider.setTopLeftPosition			(column4, row3);
	m_dynamicSpeedSlider.setTopLeftPosition		(column4, row4);

	m_modulationDepthSlider.setTopLeftPosition	(column5, row3);
	m_modulationSpeedSlider.setTopLeftPosition	(column5, row4);

	m_volumeSlider.setTopLeftPosition			(column6, row3 + pixelSize2);
}