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
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor(SpectrumAnalyzerAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_spectrumAnalyzer(),
	m_pluginName("zazz::SpectrumAnalyzer")
{
	addAndMakeVisible(m_spectrumAnalyzer);
	addAndMakeVisible(m_pluginName);
	
	setResizable(true, true);

	const int canvasWidth = 18 * 30;
	const int canvasHeight = 9 * 30;

	setSize(canvasWidth, canvasHeight);

	if (auto* constrainer = getConstrainer())
	{
		constexpr int minScale = 50;		// percentage
		constexpr int maxScale = 200;		// percentage

		constrainer->setFixedAspectRatio(static_cast<double>(canvasWidth) / static_cast<double>(canvasHeight));
		constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
	}

	startTimerHz(30);

	// Buttons
	typeAButton.setLookAndFeel(&customLook);
	typeBButton.setLookAndFeel(&customLook);
	typeCButton.setLookAndFeel(&customLook);

	addAndMakeVisible(typeAButton);
	addAndMakeVisible(typeBButton);
	addAndMakeVisible(typeCButton);

	typeAButton.setRadioGroupId(TYPE_BUTTON_GROUP);
	typeBButton.setRadioGroupId(TYPE_BUTTON_GROUP);
	typeCButton.setRadioGroupId(TYPE_BUTTON_GROUP);

	typeAButton.setClickingTogglesState(true);
	typeBButton.setClickingTogglesState(true);
	typeCButton.setClickingTogglesState(true);

	buttonAAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonA", typeAButton));
	buttonBAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonB", typeBButton));
	buttonCAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonC", typeCButton));

	typeAButton.setColour(juce::TextButton::buttonColourId, lightColor);
	typeBButton.setColour(juce::TextButton::buttonColourId, lightColor);
	typeCButton.setColour(juce::TextButton::buttonColourId, lightColor);

	typeAButton.setColour(juce::TextButton::buttonOnColourId, darkColor);
	typeBButton.setColour(juce::TextButton::buttonOnColourId, darkColor);
	typeCButton.setColour(juce::TextButton::buttonOnColourId, darkColor);
}

SpectrumAnalyzerAudioProcessorEditor::~SpectrumAnalyzerAudioProcessorEditor()
{
	stopTimer();
}

//==============================================================================
void SpectrumAnalyzerAudioProcessorEditor::timerCallback()
{
	juce::ScopedLock lock(scopeLock);
	
	m_spectrumAnalyzer.setScopeDataL(audioProcessor.getFrequencySpectrumL().getScopeData());
	m_spectrumAnalyzer.setScopeDataR(audioProcessor.getFrequencySpectrumR().getScopeData());
	m_spectrumAnalyzer.setType(static_cast<SpectrumAnalyzerComponent::Type>(audioProcessor.getType()));
	
	m_spectrumAnalyzer.repaint();
}

void SpectrumAnalyzerAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void SpectrumAnalyzerAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();
	const int pizelSize = width / 18;

	m_pluginName.setTopLeftPosition(0, 0);
	m_pluginName.setSize(width, pizelSize + pizelSize);

	m_spectrumAnalyzer.setSize(width, 8 * pizelSize);
	m_spectrumAnalyzer.setTopLeftPosition(0, pizelSize);
	
	const int buttonSize = 80 * pizelSize / 100;

	typeAButton.setSize(buttonSize, buttonSize);
	typeBButton.setSize(buttonSize, buttonSize);
	typeCButton.setSize(buttonSize, buttonSize);

	const int posX = (pizelSize - buttonSize) / 2;

	typeAButton.setTopLeftPosition(posX, 3 * pizelSize + posX);
	typeBButton.setTopLeftPosition(posX, 4 * pizelSize + posX);
	typeCButton.setTopLeftPosition(posX, 5 * pizelSize + posX);
}
