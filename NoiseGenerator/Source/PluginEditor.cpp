#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NoiseGeneratorAudioProcessorEditor::NoiseGeneratorAudioProcessorEditor (NoiseGeneratorAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_typeSlider	(vts, NoiseGeneratorAudioProcessor::paramsNames[0], NoiseGeneratorAudioProcessor::paramsUnitNames[0], NoiseGeneratorAudioProcessor::labelNames[0], { "White", "Velvet", "Pink" }),
	m_densitySlider	(vts, NoiseGeneratorAudioProcessor::paramsNames[1], NoiseGeneratorAudioProcessor::paramsUnitNames[1], NoiseGeneratorAudioProcessor::labelNames[1]),
	m_wetSlider		(vts, NoiseGeneratorAudioProcessor::paramsNames[2], NoiseGeneratorAudioProcessor::paramsUnitNames[2], NoiseGeneratorAudioProcessor::labelNames[2]),
	m_drySlider		(vts, NoiseGeneratorAudioProcessor::paramsNames[3], NoiseGeneratorAudioProcessor::paramsUnitNames[3], NoiseGeneratorAudioProcessor::labelNames[3]),
	m_pluginLabel("zazz::NoiseGenerator")
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_typeSlider);
	addAndMakeVisible(m_densitySlider);
	addAndMakeVisible(m_wetSlider);
	addAndMakeVisible(m_drySlider);

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

	// Hide density slider
	m_typeSlider.m_slider.onValueChange = [this]()
	{
		const auto type = m_typeSlider.getValue();
		if (type == 1)
		{
			m_densitySlider.setVisible(false);
		}
		else if(type == 2)
		{
			m_densitySlider.setVisible(true);
		}
		else if (type == 3)
		{
			m_densitySlider.setVisible(false);
		}
	};

	// Handle plugin load
	auto type = static_cast<int>(vts.getRawParameterValue("Type")->load());
	if (type == 1)
	{
		m_densitySlider.setVisible(false);
	}
	else if (type == 2)
	{
		m_densitySlider.setVisible(true);
	}
	else if (type == 3)
	{
		m_densitySlider.setVisible(false);
	}
}

NoiseGeneratorAudioProcessorEditor::~NoiseGeneratorAudioProcessorEditor()
{
}

void NoiseGeneratorAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void NoiseGeneratorAudioProcessorEditor::resized()
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
	m_densitySlider.setSize(pixelSize3, pixelSize4);
	m_wetSlider.setSize(pixelSize3, pixelSize4);
	m_drySlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;

	const int column1 = 0;
	const int column2 = pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_typeSlider.setTopLeftPosition(column2, row2);
	m_densitySlider.setTopLeftPosition(column3, row2);
	m_wetSlider.setTopLeftPosition(column4, row2);
	m_drySlider.setTopLeftPosition(column5, row2);
}