#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::LIGHT_COLOUR  = juce::Colour::fromHSV(0.73f, 0.5f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel::MEDIUM_COLOUR = juce::Colour::fromHSV(0.73f, 0.5f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::DARK_COLOUR   = juce::Colour::fromHSV(0.73f, 0.5f, 0.4f, 1.0f);

//==============================================================================
StateVariableFilterAudioProcessorEditor::StateVariableFilterAudioProcessorEditor (StateVariableFilterAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];
		std::string text = StateVariableFilterAudioProcessor::paramsNames[i];

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

	addAndMakeVisible(type1Button);
	addAndMakeVisible(type2Button);
	addAndMakeVisible(type3Button);

	button1Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "LP", type1Button));
	button2Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "HP", type2Button));
	button3Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "BP1", type3Button));

	// Canvas
	createCanvas(*this, N_SLIDERS);
}

StateVariableFilterAudioProcessorEditor::~StateVariableFilterAudioProcessorEditor()
{
}

//==============================================================================
void StateVariableFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::LIGHT_COLOUR);
}

void StateVariableFilterAudioProcessorEditor::resized()
{
	resize(*this, m_sliders, m_labels, N_SLIDERS);
	
	// Buttons
	const int height = getHeight();
	const float fonthHeight = (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
	const float buttonWidth = 1.2f * (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
	int posY = height - (int)(1.8f * fonthHeight);
	
	const float width = getWidth() / N_SLIDERS;

	type1Button.setBounds((int)(width - 1.5f * buttonWidth), posY, buttonWidth, fonthHeight);
	type2Button.setBounds((int)(width - 0.5f * buttonWidth), posY, buttonWidth, fonthHeight);
	type3Button.setBounds((int)(width + 0.5f * buttonWidth), posY, buttonWidth, fonthHeight);
}