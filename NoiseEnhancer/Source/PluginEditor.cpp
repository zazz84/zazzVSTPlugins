/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NoiseEnhancerAudioProcessorEditor::NoiseEnhancerAudioProcessorEditor (NoiseEnhancerAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
	juce::Colour light  = juce::Colour::fromHSV(HUE * 0.01f, 0.5f, 0.6f, 1.0f);
	juce::Colour medium = juce::Colour::fromHSV(HUE * 0.01f, 0.5f, 0.5f, 1.0f);
	juce::Colour dark   = juce::Colour::fromHSV(HUE * 0.01f, 0.5f, 0.4f, 1.0f);

	getLookAndFeel().setColour(juce::Slider::thumbColourId, dark);
	getLookAndFeel().setColour(juce::Slider::rotarySliderFillColourId, medium);
	getLookAndFeel().setColour(juce::Slider::rotarySliderOutlineColourId, light);

	for (int i = 0; i < N_SLIDERS_COUNT; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];

		//Lable
		label.setText(NoiseEnhancerAudioProcessor::paramsNames[i], juce::dontSendNotification);
		label.setFont(juce::Font(24.0f * 0.01f * SCALE, juce::Font::bold));
		label.setJustificationType(juce::Justification::centred);
		addAndMakeVisible(label);

		//Slider
		slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
		slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
		addAndMakeVisible(slider);
		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, NoiseEnhancerAudioProcessor::paramsNames[i], slider));
	}

	// Buttons
	addAndMakeVisible(buttonS);
	addAndMakeVisible(button1);
	addAndMakeVisible(button2);
	addAndMakeVisible(buttonNoiseS);
	addAndMakeVisible(buttonD);

	buttonS.setClickingTogglesState(true);
	button1.setClickingTogglesState(true);
	button2.setClickingTogglesState(true);
	buttonNoiseS.setClickingTogglesState(true);
	buttonD.setClickingTogglesState(true);

	button1.setRadioGroupId(TYPE_BUTTON_GROUP);
	button2.setRadioGroupId(TYPE_BUTTON_GROUP);

	buttonSAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonS", buttonS));
	button1Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "Button1", button1));
	button2Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "Button2", button2));
	buttonNoiseSAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonNoiseS", buttonNoiseS));
	buttonDAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonD", buttonD));

	buttonS.setColour(juce::TextButton::buttonColourId, light);
	button1.setColour(juce::TextButton::buttonColourId, light);
	button2.setColour(juce::TextButton::buttonColourId, light);
	buttonNoiseS.setColour(juce::TextButton::buttonColourId, light);
	buttonD.setColour(juce::TextButton::buttonColourId, light);

	buttonS.setColour(juce::TextButton::buttonOnColourId, dark);
	button1.setColour(juce::TextButton::buttonOnColourId, dark);
	button2.setColour(juce::TextButton::buttonOnColourId, dark);
	buttonNoiseS.setColour(juce::TextButton::buttonOnColourId, dark);
	buttonD.setColour(juce::TextButton::buttonOnColourId, dark);

	setSize((int)(SLIDER_WIDTH * 0.01f * SCALE * N_SLIDERS_COUNT), (int)((SLIDER_WIDTH + BOTTOM_MENU_HEIGHT) * 0.01f * SCALE));	
}

NoiseEnhancerAudioProcessorEditor::~NoiseEnhancerAudioProcessorEditor()
{
}

//==============================================================================
void NoiseEnhancerAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(juce::Colour::fromHSV(HUE * 0.01f, 0.5f, 0.7f, 1.0f));
}

void NoiseEnhancerAudioProcessorEditor::resized()
{
	// Sliders + Menus
	int width = getWidth() / N_SLIDERS_COUNT;
	int height = (int)(SLIDER_WIDTH * 0.01f * SCALE);
	juce::Rectangle<int> rectangles[N_SLIDERS_COUNT];

	for (int i = 0; i < N_SLIDERS_COUNT; ++i)
	{
		rectangles[i].setSize(width, height);
		rectangles[i].setPosition(i * width, 0);
		m_sliders[i].setBounds(rectangles[i]);

		rectangles[i].removeFromBottom((int)(LABEL_OFFSET * 0.01f * SCALE));
		m_labels[i].setBounds(rectangles[i]);
	}

	// Buttons
	const int posY = height + (int)(BOTTOM_MENU_HEIGHT * 0.01f * SCALE * 0.25f);
	const int buttonHeight = (int)(BOTTOM_MENU_HEIGHT * 0.01f * SCALE * 0.5f);

	buttonS.setBounds((int)(width * 0.5f - buttonHeight * 0.6f), posY, buttonHeight, buttonHeight);
	
	button1.setBounds((int)(getWidth() * 0.5f - buttonHeight * 1.0f), posY, buttonHeight, buttonHeight);
	button2.setBounds((int)(getWidth() * 0.5f + buttonHeight * 0.2f), posY, buttonHeight, buttonHeight);
	
	buttonD.setBounds((int)(width * 5.5f - buttonHeight * 0.6f), posY, buttonHeight, buttonHeight);

	buttonNoiseS.setBounds((int)(width * 6.5f - buttonHeight * 0.6f), posY, buttonHeight, buttonHeight);
}
