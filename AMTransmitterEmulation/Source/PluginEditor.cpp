#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::BACKGROUND_COLOR = juce::Colour::fromHSV(0.75f, 0.25f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel::KNOB_COLOR = juce::Colour::fromHSV(0.75f, 0.25f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::KNOB_OUTLINE_COLOR = juce::Colour::fromHSV(0.75f, 0.25f, 0.4f, 1.0f);
const juce::Colour ZazzLookAndFeel::KNOB_HIGHLIGHT = juce::Colour::fromHSV(0.75f, 0.25f, 0.4f, 1.0f);
const juce::Colour ZazzLookAndFeel::MAIN_COLOR = juce::Colour::fromHSV(0.75f, 0.25f, 0.4f, 1.0f);

const int AMTransmitterEmulationAudioProcessorEditor::SLIDERS[] = { 3 };
const float AMTransmitterEmulationAudioProcessorEditor::COLUMN_OFFSET[] = { 0.0f };

//==============================================================================
AMTransmitterEmulationAudioProcessorEditor::AMTransmitterEmulationAudioProcessorEditor (AMTransmitterEmulationAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Plugin name
	m_pluginName.setText("AM Transmitter Emulation", juce::dontSendNotification);
	m_pluginName.setFont(juce::Font(ZazzLookAndFeel::NAME_FONT_SIZE));
	m_pluginName.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(m_pluginName);
	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];
		const std::string text = AMTransmitterEmulationAudioProcessor::paramsNames[i];
		const std::string unit = AMTransmitterEmulationAudioProcessor::paramsUnitNames[i];

		createSliderWithLabel(slider, label, text, unit);
		addAndMakeVisible(label);
		addAndMakeVisible(slider);

		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, text, slider));
	}

	createCanvas(*this, SLIDERS, N_ROWS);
}

AMTransmitterEmulationAudioProcessorEditor::~AMTransmitterEmulationAudioProcessorEditor()
{
}

//==============================================================================
void AMTransmitterEmulationAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::BACKGROUND_COLOR);
}

void AMTransmitterEmulationAudioProcessorEditor::resized()
{
	resize(*this, m_sliders, m_labels, SLIDERS, COLUMN_OFFSET, N_ROWS, m_pluginName);
}