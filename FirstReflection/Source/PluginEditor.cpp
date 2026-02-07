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
FirstReflectionAudioProcessorEditor::FirstReflectionAudioProcessorEditor (FirstReflectionAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_pluginLabel("zazz::FirstReflection"),
	m_listenerHeightSlider(vts, FirstReflectionAudioProcessor::m_parametersDescritpion[FirstReflectionAudioProcessor::Parameters::ListenerHeight]),
	m_emitterHeightSlider(vts, FirstReflectionAudioProcessor::m_parametersDescritpion[FirstReflectionAudioProcessor::Parameters::EmitterHeight]),
	m_emitterDistanceSlider(vts, FirstReflectionAudioProcessor::m_parametersDescritpion[FirstReflectionAudioProcessor::Parameters::EmitterDistance]),
	m_diffusionSlider(vts, FirstReflectionAudioProcessor::m_parametersDescritpion[FirstReflectionAudioProcessor::Parameters::Diffusion]),
	m_reflectionVolumeSlider(vts, FirstReflectionAudioProcessor::m_parametersDescritpion[FirstReflectionAudioProcessor::Parameters::ReflectionVolume]),
	m_reflectionLPCutoffSlider(vts, FirstReflectionAudioProcessor::m_parametersDescritpion[FirstReflectionAudioProcessor::Parameters::ReflectionLPCutoff]),
	m_volumeSlider(vts, FirstReflectionAudioProcessor::m_parametersDescritpion[FirstReflectionAudioProcessor::Parameters::Volume])
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_sourceVizualizer);

	addAndMakeVisible(m_listenerHeightSlider);
	addAndMakeVisible(m_emitterHeightSlider);
	addAndMakeVisible(m_emitterDistanceSlider);
	addAndMakeVisible(m_diffusionSlider);
	addAndMakeVisible(m_reflectionVolumeSlider);
	addAndMakeVisible(m_reflectionLPCutoffSlider);
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

	startTimerHz(20);
}

FirstReflectionAudioProcessorEditor::~FirstReflectionAudioProcessorEditor()
{
	stopTimer();
}

//==============================================================================
void FirstReflectionAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void FirstReflectionAudioProcessorEditor::timerCallback()
{
	m_sourceVizualizer.setPositionsMeters(m_emitterDistanceSlider.getValue(), m_emitterHeightSlider.getValue(), 0.0f, m_listenerHeightSlider.getValue());
};

void FirstReflectionAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize10 = pixelSize4 + pixelSize4 + pixelSize2;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_sourceVizualizer.setSize(width - pixelSize2, pixelSize10);

	m_listenerHeightSlider.setSize(pixelSize3, pixelSize4);
	m_emitterHeightSlider.setSize(pixelSize3, pixelSize4);
	m_emitterDistanceSlider.setSize(pixelSize3, pixelSize4);
	m_diffusionSlider.setSize(pixelSize3, pixelSize4);
	m_reflectionVolumeSlider.setSize(pixelSize3, pixelSize4);
	m_reflectionLPCutoffSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = row1 + pixelSize2;
	const int row3 = row2 + pixelSize10 + pixelSize / 2;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;
	const int column8 = column7 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_sourceVizualizer.setTopLeftPosition(column2, row2);

	m_listenerHeightSlider.setTopLeftPosition(column2, row3);
	m_emitterHeightSlider.setTopLeftPosition(column3, row3);
	m_emitterDistanceSlider.setTopLeftPosition(column4, row3);
	m_diffusionSlider.setTopLeftPosition(column5, row3);
	m_reflectionVolumeSlider.setTopLeftPosition(column6, row3);
	m_reflectionLPCutoffSlider.setTopLeftPosition(column7, row3);
	m_volumeSlider.setTopLeftPosition(column8, row3);
}