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
ManualFlangerAudioProcessorEditor::ManualFlangerAudioProcessorEditor (ManualFlangerAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_frequency			(vts,	ManualFlangerAudioProcessor::paramsNames[0], ManualFlangerAudioProcessor::paramsUnitNames[0], ManualFlangerAudioProcessor::labelNames[0]),
	m_feedback			(vts,	ManualFlangerAudioProcessor::paramsNames[1], ManualFlangerAudioProcessor::paramsUnitNames[1], ManualFlangerAudioProcessor::labelNames[1]),
	m_highPassFrequency	(vts,	ManualFlangerAudioProcessor::paramsNames[2], ManualFlangerAudioProcessor::paramsUnitNames[2], ManualFlangerAudioProcessor::labelNames[2]),
	m_lowPassFrequency	(vts,	ManualFlangerAudioProcessor::paramsNames[3], ManualFlangerAudioProcessor::paramsUnitNames[3], ManualFlangerAudioProcessor::labelNames[3]),
	m_mix				(vts,	ManualFlangerAudioProcessor::paramsNames[4], ManualFlangerAudioProcessor::paramsUnitNames[4], ManualFlangerAudioProcessor::labelNames[4]),
	m_volume			(vts,	ManualFlangerAudioProcessor::paramsNames[5], ManualFlangerAudioProcessor::paramsUnitNames[5], ManualFlangerAudioProcessor::labelNames[5]),
	m_pluginLabel("zazz::ManualFlanger")
{		
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_frequency);
	addAndMakeVisible(m_feedback);
	addAndMakeVisible(m_highPassFrequency);
	addAndMakeVisible(m_lowPassFrequency);
	addAndMakeVisible(m_mix);
	addAndMakeVisible(m_volume);

	m_frequency.setSliderType(ModernRotarySliderLookAndFeel::SliderType::Dots);

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
}

ManualFlangerAudioProcessorEditor::~ManualFlangerAudioProcessorEditor()
{
}

//==============================================================================
void ManualFlangerAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void ManualFlangerAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSizeHalf = pixelSize / 2;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize6 = pixelSize4 + pixelSize2;
	const int pixelSize8 = pixelSize6 + pixelSize2;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_frequency.setSize(pixelSize6, pixelSize8);
	m_feedback.setSize(pixelSize3, pixelSize4);
	m_highPassFrequency.setSize(pixelSize3, pixelSize4);
	m_lowPassFrequency.setSize(pixelSize3, pixelSize4);
	m_mix.setSize(pixelSize3, pixelSize4);
	m_volume.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;
	const int row3 = row2 + pixelSize8;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column35 = column3 + pixelSize + pixelSizeHalf;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_frequency.setTopLeftPosition(column35, row2);

	m_feedback.setTopLeftPosition			(column2, row3);
	m_highPassFrequency.setTopLeftPosition	(column3, row3);
	m_lowPassFrequency.setTopLeftPosition	(column4, row3);
	m_mix.setTopLeftPosition				(column5, row3);
	m_volume.setTopLeftPosition		(column6, row3);
}