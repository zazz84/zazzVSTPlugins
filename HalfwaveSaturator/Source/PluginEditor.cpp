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
HalfwaveSaturatorAudioProcessorEditor::HalfwaveSaturatorAudioProcessorEditor (HalfwaveSaturatorAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_driveSlider(vts, HalfwaveSaturatorAudioProcessor::m_parametersDescritpion[HalfwaveSaturatorAudioProcessor::Parameters::Drive]),
	m_lowCutSlider(vts, HalfwaveSaturatorAudioProcessor::m_parametersDescritpion[HalfwaveSaturatorAudioProcessor::Parameters::LowCut]),
	m_volumeSlider(vts, HalfwaveSaturatorAudioProcessor::m_parametersDescritpion[HalfwaveSaturatorAudioProcessor::Parameters::Volume]),
	m_pluginLabel("zazz::HalfwaveSaturator")
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_driveSlider);
	addAndMakeVisible(m_lowCutSlider);
	addAndMakeVisible(m_volumeSlider);

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

HalfwaveSaturatorAudioProcessorEditor::~HalfwaveSaturatorAudioProcessorEditor()
{
}

//==============================================================================
void HalfwaveSaturatorAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void HalfwaveSaturatorAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_driveSlider.setSize(pixelSize3, pixelSize4);
	m_lowCutSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_driveSlider.setTopLeftPosition	(column2, row2);
	m_lowCutSlider.setTopLeftPosition	(column3, row2);
	m_volumeSlider.setTopLeftPosition	(column4, row2);
}