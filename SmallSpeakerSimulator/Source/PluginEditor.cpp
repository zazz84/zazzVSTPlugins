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
SmallSpeakerSimulatorAudioProcessorEditor::SmallSpeakerSimulatorAudioProcessorEditor (SmallSpeakerSimulatorAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_typeSlider		(vts, SmallSpeakerSimulatorAudioProcessor::paramsNames[0], SmallSpeakerSimulatorAudioProcessor::paramsUnitNames[0], SmallSpeakerSimulatorAudioProcessor::labelNames[0]),
	m_tuneSlider		(vts, SmallSpeakerSimulatorAudioProcessor::paramsNames[1], SmallSpeakerSimulatorAudioProcessor::paramsUnitNames[1], SmallSpeakerSimulatorAudioProcessor::labelNames[1]),
	m_resonanceSlider	(vts, SmallSpeakerSimulatorAudioProcessor::paramsNames[2], SmallSpeakerSimulatorAudioProcessor::paramsUnitNames[2], SmallSpeakerSimulatorAudioProcessor::labelNames[2]),
	m_mixSlider			(vts, SmallSpeakerSimulatorAudioProcessor::paramsNames[3], SmallSpeakerSimulatorAudioProcessor::paramsUnitNames[3], SmallSpeakerSimulatorAudioProcessor::labelNames[3]),
	m_volumeSlider		(vts, SmallSpeakerSimulatorAudioProcessor::paramsNames[4], SmallSpeakerSimulatorAudioProcessor::paramsUnitNames[4], SmallSpeakerSimulatorAudioProcessor::labelNames[4]),
	m_pluginLabel("zazz::SmallSpeakerSimulator")
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_typeSlider);
	addAndMakeVisible(m_tuneSlider);
	addAndMakeVisible(m_resonanceSlider);
	addAndMakeVisible(m_mixSlider);
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

SmallSpeakerSimulatorAudioProcessorEditor::~SmallSpeakerSimulatorAudioProcessorEditor()
{
}

//==============================================================================
void SmallSpeakerSimulatorAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void SmallSpeakerSimulatorAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_typeSlider.setSize(pixelSize3, pixelSize4);
	m_tuneSlider.setSize(pixelSize3, pixelSize4);
	m_resonanceSlider.setSize(pixelSize3, pixelSize4);
	m_mixSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_typeSlider.setTopLeftPosition(column2, row2);
	m_tuneSlider.setTopLeftPosition(column3, row2);
	m_resonanceSlider.setTopLeftPosition(column4, row2);
	m_mixSlider.setTopLeftPosition(column5, row2);
	m_volumeSlider.setTopLeftPosition(column6, row2);
}