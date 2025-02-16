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

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

//==============================================================================
NoiseGateAudioProcessorEditor::NoiseGateAudioProcessorEditor (NoiseGateAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_pluginLabel("zazz::NoiseGate"),
	m_thresholdSlider (vts, NoiseGateAudioProcessor::paramsNames[0], NoiseGateAudioProcessor::paramsUnitNames[0], NoiseGateAudioProcessor::labelNames[0]),
	m_hystersisSlider (vts, NoiseGateAudioProcessor::paramsNames[1], NoiseGateAudioProcessor::paramsUnitNames[1], NoiseGateAudioProcessor::labelNames[1]),
	m_attackSlider    (vts, NoiseGateAudioProcessor::paramsNames[2], NoiseGateAudioProcessor::paramsUnitNames[2], NoiseGateAudioProcessor::labelNames[2]),
	m_holdSlider      (vts, NoiseGateAudioProcessor::paramsNames[3], NoiseGateAudioProcessor::paramsUnitNames[3], NoiseGateAudioProcessor::labelNames[3]),
	m_releaseSlider   (vts, NoiseGateAudioProcessor::paramsNames[4], NoiseGateAudioProcessor::paramsUnitNames[4], NoiseGateAudioProcessor::labelNames[4]),
	m_mixPanSlider    (vts, NoiseGateAudioProcessor::paramsNames[5], NoiseGateAudioProcessor::paramsUnitNames[5], NoiseGateAudioProcessor::labelNames[5]),
	m_volumeSlider    (vts, NoiseGateAudioProcessor::paramsNames[6], NoiseGateAudioProcessor::paramsUnitNames[6], NoiseGateAudioProcessor::labelNames[6]),
	m_thresholdMeter()
{	
	addAndMakeVisible(m_pluginLabel);
	
	addAndMakeVisible(m_thresholdSlider);
	addAndMakeVisible(m_hystersisSlider);
	addAndMakeVisible(m_attackSlider);
	addAndMakeVisible(m_holdSlider);
	addAndMakeVisible(m_releaseSlider);
	addAndMakeVisible(m_mixPanSlider);
	addAndMakeVisible(m_volumeSlider);

	addAndMakeVisible(m_thresholdMeter);

	// Set canvas
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

	startTimerHz(30);

	//
	thresholdParameter = valueTreeState.getRawParameterValue(NoiseGateAudioProcessor::paramsNames[0]);
	hystersisParameter = valueTreeState.getRawParameterValue(NoiseGateAudioProcessor::paramsNames[1]);
}

NoiseGateAudioProcessorEditor::~NoiseGateAudioProcessorEditor()
{
	stopTimer();
}

//==============================================================================
void NoiseGateAudioProcessorEditor::timerCallback()
{
	const float amplitudedB = Math::gainTodB(audioProcessor.getPeak());
	const float thresholdbB = thresholdParameter->load();
	const float hystersisHalfdB = 0.5f * hystersisParameter->load();
	const bool isOpen = audioProcessor.isOpen();

	m_thresholdMeter.set(amplitudedB, thresholdbB, thresholdbB + hystersisHalfdB, thresholdbB - hystersisHalfdB, isOpen);
	m_thresholdMeter.repaint();
}

void NoiseGateAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void NoiseGateAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);
	
	m_thresholdSlider.setSize(pixelSize3, pixelSize4);
	m_hystersisSlider.setSize(pixelSize3, pixelSize4);
	m_attackSlider.setSize(pixelSize3, pixelSize4);
	m_holdSlider.setSize(pixelSize3, pixelSize4);
	m_releaseSlider.setSize(pixelSize3, pixelSize4);
	m_mixPanSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	m_thresholdMeter.setSize(width - pixelSize2, pixelSize2);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;
	const int row3 = row2 + pixelSize4 + pixelSize;

	const int column1 = 0;
	const int column2 = pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize;
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;
	const int column8 = column7 + pixelSize3;
	const int column9 = column8 + pixelSize;
	const int column10 = column9 + pixelSize3;
	const int column11 = column10 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_thresholdSlider.setTopLeftPosition(column2, row2);
	m_hystersisSlider.setTopLeftPosition(column3, row2);
	m_attackSlider.setTopLeftPosition(column5, row2);
	m_holdSlider.setTopLeftPosition(column6, row2);
	m_releaseSlider.setTopLeftPosition(column7, row2);
	m_mixPanSlider.setTopLeftPosition(column9, row2);
	m_volumeSlider.setTopLeftPosition(column10, row2);

	m_thresholdMeter.setTopLeftPosition(column2, row3);
}