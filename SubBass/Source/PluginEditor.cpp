#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::LIGHT_COLOUR  = juce::Colour::fromHSV(0.65f, 0.25f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel::MEDIUM_COLOUR = juce::Colour::fromHSV(0.65f, 0.25f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::DARK_COLOUR   = juce::Colour::fromHSV(0.65f, 0.25f, 0.4f, 1.0f);

const int SubBassAudioProcessorEditor::SLIDERS[] = { 4, 4 };
const float SubBassAudioProcessorEditor::COLUMN_OFFSET[] = { 0.0f, 0.0f };

//==============================================================================
SubBassAudioProcessorEditor::SubBassAudioProcessorEditor (SubBassAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Plugin name
	m_pluginName.setText("Sub Bass", juce::dontSendNotification);
	m_pluginName.setFont(juce::Font(ZazzLookAndFeel::NAME_FONT_SIZE));
	m_pluginName.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(m_pluginName);
	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		m_sliders[i] = std::make_unique<ButtonSlider>(audioProcessor.m_buttonState[i]);
		auto& label = m_labels[i];
		auto& slider = *m_sliders[i];
		const std::string text = SubBassAudioProcessor::paramsNames[i];
		const std::string unit = SubBassAudioProcessor::paramsUnitNames[i];

		createSliderWithLabel(slider, label, text, unit);
		addAndMakeVisible(label);
		addAndMakeVisible(slider);

		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, text, slider));
	}

	createCanvas(*this, SLIDERS, N_ROWS);
}

SubBassAudioProcessorEditor::~SubBassAudioProcessorEditor()
{
}

//==============================================================================
void SubBassAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::LIGHT_COLOUR);
}

void SubBassAudioProcessorEditor::resized()
{
	resize(*this, m_sliders, m_labels, SLIDERS, COLUMN_OFFSET, N_ROWS, m_pluginName);
}