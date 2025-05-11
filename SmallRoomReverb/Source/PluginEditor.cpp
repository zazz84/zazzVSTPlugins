/*
 * Copyright (C) 2025 Filip Cenzak (filip.c@centrum.cz)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::BACKGROUND_COLOR	= juce::Colour::fromRGB(90, 90, 100);
const juce::Colour ZazzLookAndFeel::KNOB_COLOR			= juce::Colour::fromRGB(70, 70, 80);
const juce::Colour ZazzLookAndFeel::KNOB_OUTLINE_COLOR	= juce::Colour::fromRGB(50, 50, 60);
const juce::Colour ZazzLookAndFeel::KNOB_HIGHLIGHT		= juce::Colour::fromRGB(55, 140, 255);
const juce::Colour ZazzLookAndFeel::MAIN_COLOR			= juce::Colour::fromRGB(240, 240, 255);

const int SmallRoomReverbAudioProcessorEditor::SLIDERS[] = { 4, 4, 4, 2 };
const float SmallRoomReverbAudioProcessorEditor::COLUMN_OFFSET[] = { 0.0f, 0.0f, 0.0f, 1.0f };

//==============================================================================
SmallRoomReverbAudioProcessorEditor::SmallRoomReverbAudioProcessorEditor (SmallRoomReverbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Plugin name
	m_pluginName.setText("SmallRoomReverb", juce::dontSendNotification);
	m_pluginName.setFont(juce::Font(ZazzLookAndFeel::NAME_FONT_SIZE));
	m_pluginName.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(m_pluginName);
	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];
		const std::string text = SmallRoomReverbAudioProcessor::paramsNames[i];
		const std::string unit = SmallRoomReverbAudioProcessor::paramsUnitNames[i];

		createSliderWithLabel(slider, label, text, unit);
		addAndMakeVisible(label);
		addAndMakeVisible(slider);

		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, text, slider));
	}

	createCanvas(*this, SLIDERS, N_ROWS);
}

SmallRoomReverbAudioProcessorEditor::~SmallRoomReverbAudioProcessorEditor()
{
}

//==============================================================================
void SmallRoomReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::BACKGROUND_COLOR);
}

void SmallRoomReverbAudioProcessorEditor::resized()
{
	resize(*this, m_sliders, m_labels, SLIDERS, COLUMN_OFFSET, N_ROWS, m_pluginName);
}