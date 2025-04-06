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
MultiPeakFilterAudioProcessorEditor::MultiPeakFilterAudioProcessorEditor (MultiPeakFilterAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_pluginNameComponent("zazz::MultiPeakFilter"),
	m_frequencySlider	(vts, MultiPeakFilterAudioProcessor::paramsNames[0], MultiPeakFilterAudioProcessor::paramsUnitNames[0], MultiPeakFilterAudioProcessor::labelNames[0]),
	m_qSlider			(vts, MultiPeakFilterAudioProcessor::paramsNames[1], MultiPeakFilterAudioProcessor::paramsUnitNames[1], MultiPeakFilterAudioProcessor::labelNames[1]),
	m_gainSlider		(vts, MultiPeakFilterAudioProcessor::paramsNames[2], MultiPeakFilterAudioProcessor::paramsUnitNames[2], MultiPeakFilterAudioProcessor::labelNames[2]),
	m_stepSlider		(vts, MultiPeakFilterAudioProcessor::paramsNames[3], MultiPeakFilterAudioProcessor::paramsUnitNames[3], MultiPeakFilterAudioProcessor::labelNames[3]),
	m_coutSlider		(vts, MultiPeakFilterAudioProcessor::paramsNames[4], MultiPeakFilterAudioProcessor::paramsUnitNames[4], MultiPeakFilterAudioProcessor::labelNames[4]),
	m_slopeSlider		(vts, MultiPeakFilterAudioProcessor::paramsNames[5], MultiPeakFilterAudioProcessor::paramsUnitNames[5], MultiPeakFilterAudioProcessor::labelNames[5]),
	m_volumeSlider		(vts, MultiPeakFilterAudioProcessor::paramsNames[6], MultiPeakFilterAudioProcessor::paramsUnitNames[6], MultiPeakFilterAudioProcessor::labelNames[6]),
	m_noteSlider		(vts, MultiPeakFilterAudioProcessor::paramsNames[7], MultiPeakFilterAudioProcessor::paramsUnitNames[7], MultiPeakFilterAudioProcessor::labelNames[7])
{	
	addAndMakeVisible(m_pluginNameComponent);

	addAndMakeVisible(m_frequencySlider);
	addAndMakeVisible(m_noteSlider);
	addAndMakeVisible(m_qSlider);
	addAndMakeVisible(m_gainSlider);
	addAndMakeVisible(m_stepSlider);
	addAndMakeVisible(m_coutSlider);
	addAndMakeVisible(m_slopeSlider);
	addAndMakeVisible(m_volumeSlider);
	
	m_volumeSlider.setSliderType(ModernRotarySliderLookAndFeel::SliderType::Dots);

	// Set canvas
	setResizable(true, true);

	const int canvasWidth = (1 + 3 + 3 + 3 + 1 + 3 + 1) * 30;
	const int canvasHeight = (2 + 4 + 2 + 4 + 1) * 30;

	setSize(canvasWidth, canvasHeight);

	if (auto* constrainer = getConstrainer())
	{
		constexpr int minScale = 50;		// percentage
		constexpr int maxScale = 200;		// percentage

		constrainer->setFixedAspectRatio((double)canvasWidth / (double)canvasHeight);
		constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
	}

	// Buttons
	typeAButton.setLookAndFeel(&customLook);
	typeBButton.setLookAndFeel(&customLook);

	addAndMakeVisible(typeAButton);
	addAndMakeVisible(typeBButton);

	typeAButton.setClickingTogglesState(true);
	typeBButton.setClickingTogglesState(true);

	buttonAAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonA", typeAButton));
	buttonAAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonB", typeBButton));

	typeAButton.setColour(juce::TextButton::buttonColourId, lightColor);
	typeBButton.setColour(juce::TextButton::buttonColourId, lightColor);

	typeAButton.setColour(juce::TextButton::buttonOnColourId, darkColor);
	typeBButton.setColour(juce::TextButton::buttonOnColourId, darkColor);

	// Handle switching between frequency and note sliders
	typeBButton.onClick = [this]()
	{
		if (typeBButton.getToggleState())
		{
			m_frequencySlider.setVisible(false);
			m_noteSlider.setVisible(true);
		}
		else
		{
			m_frequencySlider.setVisible(true);
			m_noteSlider.setVisible(false);
		}
	};

	// Handle load
	auto note = static_cast<bool>(vts.getRawParameterValue("ButtonB")->load());
	
	if (note)
	{
		m_frequencySlider.setVisible(false);
		m_noteSlider.setVisible(true);
	}
	else
	{
		m_frequencySlider.setVisible(true);
		m_noteSlider.setVisible(false);
	}
}

MultiPeakFilterAudioProcessorEditor::~MultiPeakFilterAudioProcessorEditor()
{
}

//==============================================================================
void MultiPeakFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void MultiPeakFilterAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / 15;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize5 = pixelSize4 + pixelSize;
	const int pixelSize15 = pixelSize3 / 2;

	// Set size
	m_pluginNameComponent.setSize(width, pixelSize2);

	m_frequencySlider.setSize(pixelSize3, pixelSize4);
	m_noteSlider.setSize(pixelSize3, pixelSize4);
	m_qSlider.setSize(pixelSize3, pixelSize4);
	m_gainSlider.setSize(pixelSize3, pixelSize4);
	m_stepSlider.setSize(pixelSize3, pixelSize4);
	m_coutSlider.setSize(pixelSize3, pixelSize4);
	m_slopeSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	// Set position
	const int row1 = 0;
	const int row2 = pixelSize2;
	const int row3 = row2 + pixelSize4;
	const int row4 = row3 + pixelSize2;

	const int column1 = pixelSize;
	const int column2 = column1 + pixelSize3;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize;

	m_pluginNameComponent.setTopLeftPosition(0, row1);

	m_frequencySlider.setTopLeftPosition(column1, row2);
	m_noteSlider.setTopLeftPosition(column1, row2);
	m_qSlider.setTopLeftPosition(column2, row2);
	m_gainSlider.setTopLeftPosition(column3, row2);

	m_stepSlider.setTopLeftPosition(column1, row4);
	m_coutSlider.setTopLeftPosition(column2, row4);
	m_slopeSlider.setTopLeftPosition(column3, row4);

	const int volumePosY = row2 + pixelSize3;
	m_volumeSlider.setTopLeftPosition(column5, volumePosY);

	// Buttons
	const int buttonSize = 70 * pixelSize / 100;

	typeAButton.setSize(buttonSize, buttonSize);
	typeBButton.setSize(buttonSize, buttonSize);

	const int buttonOffsetX = pixelSize15 - buttonSize / 2;
	const int buttonOffsetY = pixelSize4 + buttonSize / 2;

	typeAButton.setTopLeftPosition(column5 + buttonOffsetX, volumePosY + buttonOffsetY);
	typeBButton.setTopLeftPosition(column1 + buttonOffsetX, row2 + buttonOffsetY);
}