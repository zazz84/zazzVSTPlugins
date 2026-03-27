/*
 * Copyright (C) 2026 Filip Cenzak (filip.c@centrum.cz)
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
LimiterAudioProcessorEditor::LimiterAudioProcessorEditor (LimiterAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_gainSlider		(vts, LimiterAudioProcessor::paramsNames[1], LimiterAudioProcessor::paramsUnitNames[1], LimiterAudioProcessor::labelNames[1]),
	m_thresholdSlider	(vts, LimiterAudioProcessor::paramsNames[3], LimiterAudioProcessor::paramsUnitNames[3], LimiterAudioProcessor::labelNames[3]),
	m_volumeSlider		(vts, LimiterAudioProcessor::paramsNames[4], LimiterAudioProcessor::paramsUnitNames[4], LimiterAudioProcessor::labelNames[4]),
	m_typeSlider		(vts, LimiterAudioProcessor::paramsNames[0], LimiterAudioProcessor::paramsUnitNames[0], LimiterAudioProcessor::labelNames[0], { "Dirty", "Agressive", "Clean" }),
	m_releaseSlider		(vts, LimiterAudioProcessor::paramsNames[2], LimiterAudioProcessor::paramsUnitNames[2], LimiterAudioProcessor::labelNames[2]),
	m_pluginLabel("zazz::Limiter"),
	m_ispButton(vts, "ISP"),
	m_clipButton(vts, "Clip"),
	m_adaptiveReleaseButton(vts, "AR"),
	m_gainReductionMeter()
{	
	addAndMakeVisible(m_gainSlider);
	addAndMakeVisible(m_thresholdSlider);
	addAndMakeVisible(m_volumeSlider);
	addAndMakeVisible(m_typeSlider);
	addAndMakeVisible(m_releaseSlider);

	addAndMakeVisible(m_pluginLabel);
	
	addAndMakeVisible(m_gainReductionMeter);

	addAndMakeVisible(m_ispButton);
	addAndMakeVisible(m_clipButton);
	addAndMakeVisible(m_adaptiveReleaseButton);
	m_ispButton.setBorder(10);
	m_clipButton.setBorder(10);
	m_adaptiveReleaseButton.setBorder(10);

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

LimiterAudioProcessorEditor::~LimiterAudioProcessorEditor()
{
	stopTimer();
}

//==============================================================================
void LimiterAudioProcessorEditor::timerCallback()
{
	const float gainReduction = audioProcessor.getPeakReductiondB();
	m_gainReductionMeter.setLevel(gainReduction);
	m_gainReductionMeter.repaint();

	if (valueTreeState.getRawParameterValue("AR")->load() > 0.0f)
	{
		const double adaptiveReleaseTimeMS = (double)audioProcessor.getAdaptiveReleaseTimeMS();
		m_releaseSlider.setValue(adaptiveReleaseTimeMS);
	}
}

void LimiterAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(zazzGUI::Colors::darkColor);
}

void LimiterAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize15 = pixelSize + pixelSize / 2;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_gainReductionMeter.setSize(pixelSize2, pixelSize4 + pixelSize4);

	m_gainSlider.setSize(pixelSize3, pixelSize4);
	m_thresholdSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);
	m_typeSlider.setSize(pixelSize3, pixelSize4);
	m_releaseSlider.setSize(pixelSize3, pixelSize4);

	m_ispButton.setSize(pixelSize, pixelSize);
	m_clipButton.setSize(pixelSize, pixelSize);
	m_adaptiveReleaseButton.setSize(pixelSize, pixelSize);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;
	const int row3 = row2 + pixelSize4;
	const int row4 = row3 + pixelSize;
	const int row5 = row4 + pixelSize;
	const int row6 = row5 + pixelSize;

	const int column1 = 0;
	const int column2 = pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize;
	const int column6 = column5 + pixelSize2;


	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_gainReductionMeter.setTopLeftPosition(column6, row2);

	m_gainSlider.setTopLeftPosition(column2, row2);
	m_thresholdSlider.setTopLeftPosition(column3, row2);
	m_volumeSlider.setTopLeftPosition(column4, row2);

	m_typeSlider.setTopLeftPosition(column2, row3);
	m_releaseSlider.setTopLeftPosition(column3, row3);

	m_ispButton.setTopLeftPosition(column5, row4);
	m_clipButton.setTopLeftPosition(column5, row5);
	m_adaptiveReleaseButton.setTopLeftPosition(column5, row6);

}