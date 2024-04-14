#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::LIGHT_COLOUR = juce::Colour::fromHSV(0.9f, 0.5f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel::MEDIUM_COLOUR = juce::Colour::fromHSV(0.9f, 0.5f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::DARK_COLOUR = juce::Colour::fromHSV(0.9f, 0.5f, 0.4f, 1.0f);

//==============================================================================
RoomReverbAudioProcessorEditor::RoomReverbAudioProcessorEditor (RoomReverbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];
		std::string text = RoomReverbAudioProcessor::paramsNames[i];

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

	addAndMakeVisible(type1Button);
	addAndMakeVisible(type2Button);

	button1Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "Button1", type1Button));
	button2Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "Button2", type2Button));

	// Canvas
	createCanvas(*this, N_SLIDERS);
}

RoomReverbAudioProcessorEditor::~RoomReverbAudioProcessorEditor()
{
}

//==============================================================================
void RoomReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::LIGHT_COLOUR);
}

void RoomReverbAudioProcessorEditor::resized()
{
	resize(*this, m_sliders, m_labels, N_SLIDERS);
	
	// Buttons
	const int height = getHeight();
	const float fonthHeight = (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
	const int posY = height - (int)(1.8f * fonthHeight);

	type1Button.setBounds((int)(getWidth() * 0.5f - fonthHeight * 1.1f), posY, fonthHeight, fonthHeight);
	type2Button.setBounds((int)(getWidth() * 0.5f + fonthHeight * 0.1f), posY, fonthHeight, fonthHeight);
}