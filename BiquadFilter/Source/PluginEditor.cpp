#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::LIGHT_COLOUR = juce::Colour::fromHSV(0.82f, 0.5f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel::MEDIUM_COLOUR = juce::Colour::fromHSV(0.82f, 0.5f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::DARK_COLOUR = juce::Colour::fromHSV(0.82f, 0.5f, 0.4f, 1.0f);

//==============================================================================
BiquadFilterAudioProcessorEditor::BiquadFilterAudioProcessorEditor (BiquadFilterAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];
		std::string text = BiquadFilterAudioProcessor::paramsNames[i];

		//Lable
		createLabel(label, text);
		addAndMakeVisible(label);

		//Slider
		createSlider(slider);
		addAndMakeVisible(slider);
		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, text, slider));
	}

	// Buttons
	createButton(type1Button, TYPE_BUTTON_GROUP);
	createButton(type2Button, TYPE_BUTTON_GROUP);	
	createButton(type3Button, TYPE_BUTTON_GROUP);	
	createButton(type4Button, TYPE_BUTTON_GROUP);	
	createButton(type5Button, TYPE_BUTTON_GROUP);	
	createButton(type6Button, TYPE_BUTTON_GROUP);	
	createButton(type7Button, TYPE_BUTTON_GROUP);	
	createButton(type8Button, TYPE_BUTTON_GROUP);

	createButton(algorithmType1Button, ALGORITHM_TYPE_BUTTON_GROUP);
	createButton(algorithmType2Button, ALGORITHM_TYPE_BUTTON_GROUP);
	createButton(algorithmType3Button, ALGORITHM_TYPE_BUTTON_GROUP);
	createButton(algorithmType4Button, ALGORITHM_TYPE_BUTTON_GROUP);

	addAndMakeVisible(type1Button);
	addAndMakeVisible(type2Button);
	addAndMakeVisible(type3Button);
	addAndMakeVisible(type4Button);
	addAndMakeVisible(type5Button);
	addAndMakeVisible(type6Button);
	addAndMakeVisible(type7Button);
	addAndMakeVisible(type8Button);

	addAndMakeVisible(algorithmType1Button);
	addAndMakeVisible(algorithmType2Button);
	addAndMakeVisible(algorithmType3Button);
	addAndMakeVisible(algorithmType4Button);

	button1Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "LP", type1Button));
	button2Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "HP", type2Button));
	button3Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "BP1", type3Button));
	button4Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "BP2", type4Button));
	button5Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "N", type5Button));
	button6Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "P", type6Button));
	button7Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "LS", type7Button));
	button8Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "HS", type8Button));

	button9Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "DF1", algorithmType1Button));
	button10Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "DF2", algorithmType2Button));
	button11Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "DF1T", algorithmType3Button));
	button12Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "DF2T", algorithmType4Button));

	// Canvas
	createCanvas(*this, N_SLIDERS);
}

BiquadFilterAudioProcessorEditor::~BiquadFilterAudioProcessorEditor()
{
}

//==============================================================================
void BiquadFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::LIGHT_COLOUR);
}

void BiquadFilterAudioProcessorEditor::resized()
{
	resize(*this, m_sliders, m_labels, N_SLIDERS);
	
	// Buttons
	const int height = getHeight();
	const float fonthHeight = (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
	const float buttonWidth = 1.5f * (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
	int posY = height - (int)(1.8f * fonthHeight);
	
	const float width = getWidth() / N_SLIDERS;

	type1Button.setBounds((int)(width - buttonWidth), posY, buttonWidth, fonthHeight);
	type2Button.setBounds((int)(width), posY, buttonWidth, fonthHeight);
	type3Button.setBounds((int)(2.0f * width - buttonWidth), posY, buttonWidth, fonthHeight);
	type4Button.setBounds((int)(2.0f * width), posY, buttonWidth, fonthHeight);
	type5Button.setBounds((int)(3.0f * width - buttonWidth), posY, buttonWidth, fonthHeight);
	type6Button.setBounds((int)(3.0f * width), posY, buttonWidth, fonthHeight);
	type7Button.setBounds((int)(4.0f * width - buttonWidth), posY, buttonWidth, fonthHeight);
	type8Button.setBounds((int)(4.0f * width), posY, buttonWidth, fonthHeight);

	posY = (int)(0.8f * fonthHeight);
	algorithmType1Button.setBounds((int)(width - buttonWidth), posY, buttonWidth, fonthHeight);
	algorithmType2Button.setBounds((int)(width), posY, buttonWidth, fonthHeight);

	algorithmType3Button.setBounds((int)(2.0f * width - buttonWidth), posY, buttonWidth, fonthHeight);
	algorithmType4Button.setBounds((int)(2.0f * width), posY, buttonWidth, fonthHeight);
}