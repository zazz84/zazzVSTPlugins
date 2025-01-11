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
MidSideAudioProcessorEditor::MidSideAudioProcessorEditor(MidSideAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_midGainSlider(vts, MidSideAudioProcessor::paramsNames[0], MidSideAudioProcessor::paramsUnitNames[0], MidSideAudioProcessor::labelNames[0]),
	m_sideGainSlider(vts, MidSideAudioProcessor::paramsNames[1], MidSideAudioProcessor::paramsUnitNames[1], MidSideAudioProcessor::labelNames[1]),
	m_midPanSlider(vts, MidSideAudioProcessor::paramsNames[2], MidSideAudioProcessor::paramsUnitNames[2], MidSideAudioProcessor::labelNames[2]),
	m_sidePanSlider(vts, MidSideAudioProcessor::paramsNames[3], MidSideAudioProcessor::paramsUnitNames[3], MidSideAudioProcessor::labelNames[3]),
	m_volumeSlider(vts, MidSideAudioProcessor::paramsNames[4], MidSideAudioProcessor::paramsUnitNames[4], MidSideAudioProcessor::labelNames[4]),

	m_gainLabel("Gain"),
	m_panLabel("Pan"),

	m_correlationMeter(),
	m_balanceMeter(),

	m_pluginLabel("Mid Side")
{	
	addAndMakeVisible(m_midGainSlider);
	addAndMakeVisible(m_sideGainSlider);
	addAndMakeVisible(m_midPanSlider);
	addAndMakeVisible(m_sidePanSlider);
	addAndMakeVisible(m_volumeSlider);

	addAndMakeVisible(m_gainLabel);
	addAndMakeVisible(m_panLabel);

	addAndMakeVisible(m_correlationMeter);
	addAndMakeVisible(m_balanceMeter);

	addAndMakeVisible(m_pluginLabel);

	setResizable(true, true);

	const int canvasWidth = (5 * 3 + 2) * 30;
	const int canvasHeight = (2 + 1 + 4 + 2 + 2) * 30;

	setSize(canvasWidth, canvasHeight);

	if (auto* constrainer = getConstrainer())
	{
		constexpr int minScale = 50;		// percentage
		constexpr int maxScale = 200;		// percentage

		constrainer->setFixedAspectRatio((double)canvasWidth / (double)canvasHeight);
		constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
	}

	startTimerHz(30);
}

MidSideAudioProcessorEditor::~MidSideAudioProcessorEditor()
{
	stopTimer();
}

//==============================================================================
void MidSideAudioProcessorEditor::timerCallback()
{
	m_correlationMeter.set(audioProcessor.getCorrelation());
	m_correlationMeter.repaint();

	m_balanceMeter.set(audioProcessor.getBalance());
	m_balanceMeter.repaint();
}

void MidSideAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(juce::Colour::fromRGB(90, 90, 100));
}

void MidSideAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / 17;
	const int pixelSize2 = pixelSize + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	const int sliderWidth = 3 * pixelSize;
	const int sliderHeight = 4 * pixelSize;

	m_gainLabel.setSize(2 * sliderWidth, pixelSize);
	m_panLabel.setSize(2 * sliderWidth, pixelSize);

	m_midGainSlider.setSize(sliderWidth, sliderHeight);
	m_sideGainSlider.setSize(sliderWidth, sliderHeight);
	m_midPanSlider.setSize(sliderWidth, sliderHeight);
	m_sidePanSlider.setSize(sliderWidth, sliderHeight);
	m_volumeSlider.setSize(sliderWidth, sliderHeight);

	m_correlationMeter.setSize(width - pixelSize2, pixelSize2);
	m_balanceMeter.setSize(width - pixelSize2, pixelSize2);

	// Set position
	m_pluginLabel.setTopLeftPosition(0, 0);

	const int sliderColumn1 = pixelSize;
	const int sliderColumn2 = sliderColumn1 + sliderWidth;
	const int sliderColumn3 = sliderColumn2 + sliderWidth;
	const int sliderColumn4 = sliderColumn3 + sliderWidth;
	const int sliderColumn5 = sliderColumn4 + sliderWidth;

	const int lableRow1 = pixelSize2;

	m_gainLabel.setTopLeftPosition(sliderColumn1, lableRow1);
	m_panLabel.setTopLeftPosition(sliderColumn3, lableRow1);

	const int sliderRow1 = pixelSize2 + pixelSize;

	m_midGainSlider.setTopLeftPosition(sliderColumn1, sliderRow1);
	m_sideGainSlider.setTopLeftPosition(sliderColumn2, sliderRow1);
	m_midPanSlider.setTopLeftPosition(sliderColumn3, sliderRow1);
	m_sidePanSlider.setTopLeftPosition(sliderColumn4, sliderRow1);
	m_volumeSlider.setTopLeftPosition(sliderColumn5, sliderRow1);

	m_correlationMeter.setTopLeftPosition(sliderColumn1, sliderRow1 + sliderHeight);
	m_balanceMeter.setTopLeftPosition(sliderColumn1, sliderRow1 + sliderHeight + pixelSize2);
}