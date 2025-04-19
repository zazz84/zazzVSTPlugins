#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BiquadFilterAudioProcessorEditor::BiquadFilterAudioProcessorEditor(BiquadFilterAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_pluginNameComponent("zazz::BiquadFilter"),
	m_typeGroupLabel("Type"),
	m_algorithmGroupLabel("Algo"),
	m_frequencySlider(vts, BiquadFilterAudioProcessor::paramsNames[0], BiquadFilterAudioProcessor::paramsUnitNames[0], BiquadFilterAudioProcessor::labelNames[0]),
	m_gainSlider(vts, BiquadFilterAudioProcessor::paramsNames[1], BiquadFilterAudioProcessor::paramsUnitNames[1], BiquadFilterAudioProcessor::labelNames[1]),
	m_QSlider(vts, BiquadFilterAudioProcessor::paramsNames[2], BiquadFilterAudioProcessor::paramsUnitNames[2], BiquadFilterAudioProcessor::labelNames[2]),
	m_mixSlider(vts, BiquadFilterAudioProcessor::paramsNames[3], BiquadFilterAudioProcessor::paramsUnitNames[3], BiquadFilterAudioProcessor::labelNames[3]),
	m_volumeSlider(vts, BiquadFilterAudioProcessor::paramsNames[4], BiquadFilterAudioProcessor::paramsUnitNames[4], BiquadFilterAudioProcessor::labelNames[4])
{
	addAndMakeVisible(m_pluginNameComponent);
	addAndMakeVisible(m_typeGroupLabel);
	addAndMakeVisible(m_algorithmGroupLabel);

	addAndMakeVisible(m_frequencySlider);
	addAndMakeVisible(m_gainSlider);
	addAndMakeVisible(m_QSlider);
	addAndMakeVisible(m_mixSlider);
	addAndMakeVisible(m_volumeSlider);

	// Set canvas
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

	// Buttons
	type1Button.setLookAndFeel(&customLook);
	type2Button.setLookAndFeel(&customLook);
	type3Button.setLookAndFeel(&customLook);
	type4Button.setLookAndFeel(&customLook);
	type5Button.setLookAndFeel(&customLook);
	type6Button.setLookAndFeel(&customLook);
	type7Button.setLookAndFeel(&customLook);
	type8Button.setLookAndFeel(&customLook);

	addAndMakeVisible(type1Button);
	addAndMakeVisible(type2Button);
	addAndMakeVisible(type3Button);
	addAndMakeVisible(type4Button);
	addAndMakeVisible(type5Button);
	addAndMakeVisible(type6Button);
	addAndMakeVisible(type7Button);
	addAndMakeVisible(type8Button);

	type1Button.setRadioGroupId(TYPE_BUTTON_GROUP);
	type2Button.setRadioGroupId(TYPE_BUTTON_GROUP);
	type3Button.setRadioGroupId(TYPE_BUTTON_GROUP);
	type4Button.setRadioGroupId(TYPE_BUTTON_GROUP);
	type5Button.setRadioGroupId(TYPE_BUTTON_GROUP);
	type6Button.setRadioGroupId(TYPE_BUTTON_GROUP);
	type7Button.setRadioGroupId(TYPE_BUTTON_GROUP);
	type8Button.setRadioGroupId(TYPE_BUTTON_GROUP);

	type1Button.setColour(juce::TextButton::buttonColourId, lightColor);
	type2Button.setColour(juce::TextButton::buttonColourId, lightColor);
	type3Button.setColour(juce::TextButton::buttonColourId, lightColor);
	type4Button.setColour(juce::TextButton::buttonColourId, lightColor);
	type5Button.setColour(juce::TextButton::buttonColourId, lightColor);
	type6Button.setColour(juce::TextButton::buttonColourId, lightColor);
	type7Button.setColour(juce::TextButton::buttonColourId, lightColor);
	type8Button.setColour(juce::TextButton::buttonColourId, lightColor);

	type1Button.setColour(juce::TextButton::buttonOnColourId, darkColor);
	type2Button.setColour(juce::TextButton::buttonOnColourId, darkColor);
	type3Button.setColour(juce::TextButton::buttonOnColourId, darkColor);
	type4Button.setColour(juce::TextButton::buttonOnColourId, darkColor);
	type5Button.setColour(juce::TextButton::buttonOnColourId, darkColor);
	type6Button.setColour(juce::TextButton::buttonOnColourId, darkColor);
	type7Button.setColour(juce::TextButton::buttonOnColourId, darkColor);
	type8Button.setColour(juce::TextButton::buttonOnColourId, darkColor);

	type1Button.setClickingTogglesState(true);
	type2Button.setClickingTogglesState(true);
	type3Button.setClickingTogglesState(true);
	type4Button.setClickingTogglesState(true);
	type5Button.setClickingTogglesState(true);
	type6Button.setClickingTogglesState(true);
	type7Button.setClickingTogglesState(true);
	type8Button.setClickingTogglesState(true);

	//...

	algorithmType1Button.setLookAndFeel(&customLook);
	algorithmType2Button.setLookAndFeel(&customLook);
	algorithmType3Button.setLookAndFeel(&customLook);
	algorithmType4Button.setLookAndFeel(&customLook);

	addAndMakeVisible(algorithmType1Button);
	addAndMakeVisible(algorithmType2Button);
	addAndMakeVisible(algorithmType3Button);
	addAndMakeVisible(algorithmType4Button);

	algorithmType1Button.setRadioGroupId(ALGORITHM_TYPE_BUTTON_GROUP);
	algorithmType2Button.setRadioGroupId(ALGORITHM_TYPE_BUTTON_GROUP);
	algorithmType3Button.setRadioGroupId(ALGORITHM_TYPE_BUTTON_GROUP);
	algorithmType4Button.setRadioGroupId(ALGORITHM_TYPE_BUTTON_GROUP);

	algorithmType1Button.setColour(juce::TextButton::buttonColourId, lightColor);
	algorithmType2Button.setColour(juce::TextButton::buttonColourId, lightColor);
	algorithmType3Button.setColour(juce::TextButton::buttonColourId, lightColor);
	algorithmType4Button.setColour(juce::TextButton::buttonColourId, lightColor);

	algorithmType1Button.setColour(juce::TextButton::buttonOnColourId, darkColor);
	algorithmType2Button.setColour(juce::TextButton::buttonOnColourId, darkColor);
	algorithmType3Button.setColour(juce::TextButton::buttonOnColourId, darkColor);
	algorithmType4Button.setColour(juce::TextButton::buttonOnColourId, darkColor);

	algorithmType1Button.setClickingTogglesState(true);
	algorithmType2Button.setClickingTogglesState(true);
	algorithmType3Button.setClickingTogglesState(true);
	algorithmType4Button.setClickingTogglesState(true);

	//...

	button1Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "LP", type1Button));
	button2Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "HP", type2Button));
	button3Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "AP", type3Button));
	button4Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "BP", type4Button));
	button5Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "N", type5Button));
	button6Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "P", type6Button));
	button7Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "LS", type7Button));
	button8Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "HS", type8Button));

	button9Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "DF1", algorithmType1Button));
	button10Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "DF2", algorithmType2Button));
	button11Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "DF1T", algorithmType3Button));
	button12Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "DF2T", algorithmType4Button));

	// Hide unused sliders
	type1Button.onClick = [this]()
	{
		if (type1Button.getToggleState())
		{
			m_gainSlider.setVisible(false);
		}
	};

	type2Button.onClick = [this]()
	{
		if (type2Button.getToggleState())
		{
			m_gainSlider.setVisible(false);
		}
	};

	type3Button.onClick = [this]()
	{
		if (type3Button.getToggleState())
		{
			m_gainSlider.setVisible(false);
		}
	};

	type4Button.onClick = [this]()
	{
		if (type4Button.getToggleState())
		{
			m_gainSlider.setVisible(false);
		}
	};

	type5Button.onClick = [this]()
	{
		if (type5Button.getToggleState())
		{
			m_gainSlider.setVisible(false);
		}
	};

	type6Button.onClick = [this]()
	{
		if (type6Button.getToggleState())
		{
			m_gainSlider.setVisible(true);
		}
	};

	type7Button.onClick = [this]()
	{
		if (type7Button.getToggleState())
		{
			m_gainSlider.setVisible(true);
		}
	};

	type8Button.onClick = [this]()
	{
		if (type8Button.getToggleState())
		{
			m_gainSlider.setVisible(true);
		}
	};

	// Handle plugin load
	auto type1 = static_cast<bool>(vts.getRawParameterValue("LP")->load());
	auto type2 = static_cast<bool>(vts.getRawParameterValue("HP")->load());
	auto type3 = static_cast<bool>(vts.getRawParameterValue("AP")->load());
	auto type4 = static_cast<bool>(vts.getRawParameterValue("BP")->load());
	auto type5 = static_cast<bool>(vts.getRawParameterValue("N")->load());
	auto type6 = static_cast<bool>(vts.getRawParameterValue("P")->load());
	auto type7 = static_cast<bool>(vts.getRawParameterValue("LS")->load());
	auto type8 = static_cast<bool>(vts.getRawParameterValue("HS")->load());

	if (type1 || type2 || type3 || type4 || type5)
	{
		m_gainSlider.setVisible(false);
	}
	else
	{
		m_gainSlider.setVisible(true);
	}
}

BiquadFilterAudioProcessorEditor::~BiquadFilterAudioProcessorEditor()
{
}

//==============================================================================
void BiquadFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void BiquadFilterAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize10 = 10 * pixelSize;

	const int smallSliderWidth = 3 * pixelSize2 / 4;

	// Set size
	m_pluginNameComponent.setSize(width, pixelSize2);
	m_algorithmGroupLabel.setSize(pixelSize2, pixelSize);
	m_typeGroupLabel.setSize(pixelSize2, pixelSize);

	m_frequencySlider.setSize(pixelSize3, pixelSize4);
	m_gainSlider.setSize(pixelSize3, pixelSize4);
	m_QSlider.setSize(pixelSize3, pixelSize4);
	m_mixSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	// Set position
	const int row1 = 0;
	const int rowGroupLabel = row1 + pixelSize2;
	const int row2 = rowGroupLabel + pixelSize;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize2;
	const int column4 = column3 + pixelSize;
	const int column5 = column4 + pixelSize2;
	const int column6 = column5 + pixelSize;
	const int column7 = column6 + pixelSize3;
	const int column8 = column7 + pixelSize3;
	const int column9 = column8 + pixelSize3;
	const int column10 = column9 + pixelSize3;

	m_pluginNameComponent.setTopLeftPosition(0, 0);
	m_algorithmGroupLabel.setTopLeftPosition(column2, rowGroupLabel);
	m_typeGroupLabel.setTopLeftPosition(column4, rowGroupLabel);

	m_frequencySlider.setTopLeftPosition(column6, row2);
	m_gainSlider.setTopLeftPosition(column7, row2);
	m_QSlider.setTopLeftPosition(column8, row2);
	m_mixSlider.setTopLeftPosition(column9, row2);
	m_volumeSlider.setTopLeftPosition(column10, row2);

	// Buttons
	const int buttonSize = 90 * pixelSize / 100;

	// Set size
	algorithmType1Button.setSize(buttonSize, buttonSize);
	algorithmType2Button.setSize(buttonSize, buttonSize);
	algorithmType3Button.setSize(buttonSize, buttonSize);
	algorithmType4Button.setSize(buttonSize, buttonSize);

	type1Button.setSize(buttonSize, buttonSize);
	type2Button.setSize(buttonSize, buttonSize);
	type3Button.setSize(buttonSize, buttonSize);
	type4Button.setSize(buttonSize, buttonSize);
	type5Button.setSize(buttonSize, buttonSize);
	type6Button.setSize(buttonSize, buttonSize);
	type7Button.setSize(buttonSize, buttonSize);
	type8Button.setSize(buttonSize, buttonSize);

	const int buttonPixelOffset = 5 * pixelSize / 100;
	
	const int buttonRow1 = row2 + buttonPixelOffset;
	const int buttonRow2 = buttonRow1 + pixelSize;
	const int buttonRow3 = buttonRow2 + pixelSize;
	const int buttonRow4 = buttonRow3 + pixelSize;

	const int buttonColumn1 = column2 + buttonPixelOffset + pixelSize / 2;
	const int buttonColumn2 = column4 + buttonPixelOffset;
	const int buttonColumn3 = buttonColumn2 + pixelSize;

	algorithmType1Button.setTopLeftPosition(buttonColumn1, buttonRow1);
	algorithmType2Button.setTopLeftPosition(buttonColumn1, buttonRow2);
	algorithmType3Button.setTopLeftPosition(buttonColumn1, buttonRow3);
	algorithmType4Button.setTopLeftPosition(buttonColumn1, buttonRow4);

	type1Button.setTopLeftPosition(buttonColumn2, buttonRow1);
	type2Button.setTopLeftPosition(buttonColumn2, buttonRow2);
	type3Button.setTopLeftPosition(buttonColumn2, buttonRow3);
	type4Button.setTopLeftPosition(buttonColumn2, buttonRow4);

	type5Button.setTopLeftPosition(buttonColumn3, buttonRow1);
	type6Button.setTopLeftPosition(buttonColumn3, buttonRow2);
	type7Button.setTopLeftPosition(buttonColumn3, buttonRow3);
	type8Button.setTopLeftPosition(buttonColumn3, buttonRow4);
}