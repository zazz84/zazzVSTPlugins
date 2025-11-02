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
SpectrumMatchAudioProcessorEditor::SpectrumMatchAudioProcessorEditor (SpectrumMatchAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_attackSlider(	vts,	SpectrumMatchAudioProcessor::paramsNames[0], SpectrumMatchAudioProcessor::paramsUnitNames[0], SpectrumMatchAudioProcessor::labelNames[0]),
	m_releaseSlider	(vts,	SpectrumMatchAudioProcessor::paramsNames[1], SpectrumMatchAudioProcessor::paramsUnitNames[1], SpectrumMatchAudioProcessor::labelNames[1]),
	m_gain1Slider	(vts,	SpectrumMatchAudioProcessor::paramsNames[2], SpectrumMatchAudioProcessor::paramsUnitNames[2], SpectrumMatchAudioProcessor::labelNames[2]),
	m_gain2Slider	(vts,	SpectrumMatchAudioProcessor::paramsNames[3], SpectrumMatchAudioProcessor::paramsUnitNames[3], SpectrumMatchAudioProcessor::labelNames[3]),
	m_gain3Slider	(vts,	SpectrumMatchAudioProcessor::paramsNames[4], SpectrumMatchAudioProcessor::paramsUnitNames[4], SpectrumMatchAudioProcessor::labelNames[4]),
	m_gain4Slider	(vts,	SpectrumMatchAudioProcessor::paramsNames[5], SpectrumMatchAudioProcessor::paramsUnitNames[5], SpectrumMatchAudioProcessor::labelNames[5]),
	m_gain5Slider	(vts,	SpectrumMatchAudioProcessor::paramsNames[6], SpectrumMatchAudioProcessor::paramsUnitNames[6], SpectrumMatchAudioProcessor::labelNames[6]),
	m_gain6Slider	(vts,	SpectrumMatchAudioProcessor::paramsNames[7], SpectrumMatchAudioProcessor::paramsUnitNames[7], SpectrumMatchAudioProcessor::labelNames[7]),
	m_volumeSlider	(vts,	SpectrumMatchAudioProcessor::paramsNames[8], SpectrumMatchAudioProcessor::paramsUnitNames[8], SpectrumMatchAudioProcessor::labelNames[8]),
	m_pluginLabel("zazz::SpectrumMatch"),
	m_gain1Meter(),
	m_gain2Meter(),
	m_gain3Meter(),
	m_gain4Meter(),
	m_gain5Meter(),
	m_gain6Meter()
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_attackSlider);
	addAndMakeVisible(m_releaseSlider);
	addAndMakeVisible(m_gain1Slider);
	addAndMakeVisible(m_gain2Slider);
	addAndMakeVisible(m_gain3Slider);
	addAndMakeVisible(m_gain4Slider);
	addAndMakeVisible(m_gain5Slider);
	addAndMakeVisible(m_gain6Slider);
	addAndMakeVisible(m_volumeSlider);

	addAndMakeVisible(m_gain1Meter);
	addAndMakeVisible(m_gain2Meter);
	addAndMakeVisible(m_gain3Meter);
	addAndMakeVisible(m_gain4Meter);
	addAndMakeVisible(m_gain5Meter);
	addAndMakeVisible(m_gain6Meter);

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

SpectrumMatchAudioProcessorEditor::~SpectrumMatchAudioProcessorEditor()
{
	stopTimer();
}

//==============================================================================
void SpectrumMatchAudioProcessorEditor::timerCallback()
{
	auto* gain = audioProcessor.getGains();

	m_gain1Meter.setLevel(gain[0]);
	m_gain2Meter.setLevel(gain[1]);
	m_gain3Meter.setLevel(gain[2]);
	m_gain4Meter.setLevel(gain[3]);
	m_gain5Meter.setLevel(gain[4]);
	m_gain6Meter.setLevel(gain[5]);

	m_gain1Meter.repaint();
	m_gain2Meter.repaint();
	m_gain3Meter.repaint();
	m_gain4Meter.repaint();
	m_gain5Meter.repaint();
	m_gain6Meter.repaint();
}

void SpectrumMatchAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void SpectrumMatchAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelHalf = pixelSize / 2;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize8 = pixelSize4 + pixelSize4;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_gain1Meter.setSize(pixelSize2, pixelSize8);
	m_gain2Meter.setSize(pixelSize2, pixelSize8);
	m_gain3Meter.setSize(pixelSize2, pixelSize8);
	m_gain4Meter.setSize(pixelSize2, pixelSize8);
	m_gain5Meter.setSize(pixelSize2, pixelSize8);
	m_gain6Meter.setSize(pixelSize2, pixelSize8);
	
	m_attackSlider.setSize(pixelSize3, pixelSize4);
	m_releaseSlider.setSize(pixelSize3, pixelSize4);
	
	m_gain1Slider.setSize(pixelSize3, pixelSize4);
	m_gain2Slider.setSize(pixelSize3, pixelSize4);
	m_gain3Slider.setSize(pixelSize3, pixelSize4);
	m_gain4Slider.setSize(pixelSize3, pixelSize4);
	m_gain5Slider.setSize(pixelSize3, pixelSize4);
	m_gain6Slider.setSize(pixelSize3, pixelSize4);
	
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = row1 + pixelSize2;
	const int row3 = row2 + pixelSize8;
	const int row4 = row3 + pixelSize4 + pixelSize;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;

	m_pluginLabel.setTopLeftPosition	(column1, row1);

	m_gain1Meter.setTopLeftPosition		(column2 + pixelHalf, row2);
	m_gain2Meter.setTopLeftPosition		(column3 + pixelHalf, row2);
	m_gain3Meter.setTopLeftPosition		(column4 + pixelHalf, row2);
	m_gain4Meter.setTopLeftPosition		(column5 + pixelHalf, row2);
	m_gain5Meter.setTopLeftPosition		(column6 + pixelHalf, row2);
	m_gain6Meter.setTopLeftPosition		(column7 + pixelHalf, row2);

	m_gain1Slider.setTopLeftPosition	(column2, row3);
	m_gain2Slider.setTopLeftPosition	(column3, row3);
	m_gain3Slider.setTopLeftPosition	(column4, row3);
	m_gain4Slider.setTopLeftPosition	(column5, row3);
	m_gain5Slider.setTopLeftPosition	(column6, row3);
	m_gain6Slider.setTopLeftPosition	(column7, row3);

	m_attackSlider.setTopLeftPosition	(column3, row4);
	m_releaseSlider.setTopLeftPosition	(column4, row4);
	m_volumeSlider.setTopLeftPosition	(column6, row4);
}