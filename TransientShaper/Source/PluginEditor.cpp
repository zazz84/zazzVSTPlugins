#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TransientShaperAudioProcessorEditor::TransientShaperAudioProcessorEditor(TransientShaperAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_attackLenghtSlider(vts, TransientShaperAudioProcessor::paramsNames[0], TransientShaperAudioProcessor::paramsUnitNames[0], TransientShaperAudioProcessor::labelNames[0]),
	m_attackSlider(vts, TransientShaperAudioProcessor::paramsNames[1], TransientShaperAudioProcessor::paramsUnitNames[1], TransientShaperAudioProcessor::labelNames[1]),
	m_sustainLenghtSlider(vts, TransientShaperAudioProcessor::paramsNames[2], TransientShaperAudioProcessor::paramsUnitNames[2], TransientShaperAudioProcessor::labelNames[2]),
	m_sustainSlider(vts, TransientShaperAudioProcessor::paramsNames[3], TransientShaperAudioProcessor::paramsUnitNames[3], TransientShaperAudioProcessor::labelNames[3]),
	m_volumeSlider(vts, TransientShaperAudioProcessor::paramsNames[4], TransientShaperAudioProcessor::paramsUnitNames[4], TransientShaperAudioProcessor::labelNames[4]),

	m_attackLabel("Attack"),
	m_sustainLabel("Sustain"),

	m_pluginLabel("zazz::TransientShaper"),

	m_gainMeter()
{	
	addAndMakeVisible(m_attackLenghtSlider);
	addAndMakeVisible(m_attackSlider);
	addAndMakeVisible(m_sustainLenghtSlider);
	addAndMakeVisible(m_sustainSlider);
	addAndMakeVisible(m_volumeSlider);

	addAndMakeVisible(m_attackLabel);
	addAndMakeVisible(m_sustainLabel);

	addAndMakeVisible(m_pluginLabel);
	addAndMakeVisible(m_gainMeter);
	
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

	startTimerHz(20);
}

TransientShaperAudioProcessorEditor::~TransientShaperAudioProcessorEditor()
{
	stopTimer();
}

//==============================================================================
void TransientShaperAudioProcessorEditor::timerCallback()
{
	const float maxGain = audioProcessor.getMaxGain();
	m_gainMeter.setLevel(maxGain);

	m_gainMeter.repaint();
}

void TransientShaperAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void TransientShaperAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_attackLabel.setSize(pixelSize3, pixelSize);
	m_sustainLabel.setSize(pixelSize3, pixelSize);

	m_attackLenghtSlider.setSize(pixelSize3, pixelSize4);
	m_attackSlider.setSize(pixelSize3, pixelSize4);
	m_sustainLenghtSlider.setSize(pixelSize3, pixelSize4);
	m_sustainSlider.setSize(pixelSize3, pixelSize4);
	
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	m_gainMeter.setSize(pixelSize2, pixelSize4 + pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;
	const int row3 = row2 + pixelSize;
	const int row4 = row3 + pixelSize4;
	const int rowVolume = row3 + pixelSize2;

	const int column1 = 0;
	const int column2 = pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize2;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_attackLabel.setTopLeftPosition(column2, row2);
	m_sustainLabel.setTopLeftPosition(column4, row2);

	m_attackLenghtSlider.setTopLeftPosition(column2, row3);
	m_attackSlider.setTopLeftPosition(column2, row4);
	
	m_sustainLenghtSlider.setTopLeftPosition(column4, row3);
	m_sustainSlider.setTopLeftPosition(column4, row4);
	
	m_volumeSlider.setTopLeftPosition(column6, rowVolume);

	m_gainMeter.setTopLeftPosition(column3, row3);
}