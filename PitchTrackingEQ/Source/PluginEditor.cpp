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
PitchTrackingEQAudioProcessorEditor::PitchTrackingEQAudioProcessorEditor(PitchTrackingEQAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_pluginNameComponent("zazz::PitchTrackingEQ"),
	m_filterGroupLabel("Filter"),
	m_frequencyMeterComponent(),
	m_detectorFrequencyMinSlider     (vts, PitchTrackingEQAudioProcessor::paramsNames[0], PitchTrackingEQAudioProcessor::paramsUnitNames[0], PitchTrackingEQAudioProcessor::labelNames[0]),
	m_detectorFrequencyMaxSlider     (vts, PitchTrackingEQAudioProcessor::paramsNames[1], PitchTrackingEQAudioProcessor::paramsUnitNames[1], PitchTrackingEQAudioProcessor::labelNames[1]),
	m_smootherSpeedSlider            (vts, PitchTrackingEQAudioProcessor::paramsNames[2], PitchTrackingEQAudioProcessor::paramsUnitNames[2], PitchTrackingEQAudioProcessor::labelNames[2]),
	m_filterFrequencyMultiplierSlider(vts, PitchTrackingEQAudioProcessor::paramsNames[3], PitchTrackingEQAudioProcessor::paramsUnitNames[3], PitchTrackingEQAudioProcessor::labelNames[3]),
	m_filterQSlider                  (vts, PitchTrackingEQAudioProcessor::paramsNames[4], PitchTrackingEQAudioProcessor::paramsUnitNames[4], PitchTrackingEQAudioProcessor::labelNames[4]),
	m_filterGainSlider               (vts, PitchTrackingEQAudioProcessor::paramsNames[5], PitchTrackingEQAudioProcessor::paramsUnitNames[5], PitchTrackingEQAudioProcessor::labelNames[5]),
	m_volumeSlider                   (vts, PitchTrackingEQAudioProcessor::paramsNames[6], PitchTrackingEQAudioProcessor::paramsUnitNames[6], PitchTrackingEQAudioProcessor::labelNames[6])
{	

	frequencyMinParameter = valueTreeState.getRawParameterValue(PitchTrackingEQAudioProcessor::paramsNames[0]);
	frequencyMaxParameter = valueTreeState.getRawParameterValue(PitchTrackingEQAudioProcessor::paramsNames[1]);
	
	addAndMakeVisible(m_frequencyMeterComponent);
	addAndMakeVisible(m_pluginNameComponent);
	addAndMakeVisible(m_filterGroupLabel);
	
	addAndMakeVisible(m_detectorFrequencyMinSlider);
	addAndMakeVisible(m_detectorFrequencyMaxSlider);
	addAndMakeVisible(m_smootherSpeedSlider);
	addAndMakeVisible(m_filterFrequencyMultiplierSlider);
	addAndMakeVisible(m_filterQSlider);
	addAndMakeVisible(m_filterGainSlider);
	addAndMakeVisible(m_volumeSlider);

	// Set canvas
	setResizable(true, true);

	const int canvasWidth = (1 + 3 + 1 + 1 + 3 * 3 + 1 + 3 + 1) * 30;
	const int canvasHeight = (2 + 1 + 4 + 2 + 1) * 30;

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

PitchTrackingEQAudioProcessorEditor::~PitchTrackingEQAudioProcessorEditor()
{
	stopTimer();
}

//==============================================================================
void PitchTrackingEQAudioProcessorEditor::timerCallback()
{
	const auto frequency = audioProcessor.getFrequncy();
	m_frequencyMeterComponent.set(frequency, frequencyMinParameter->load(), frequencyMaxParameter->load());
	m_frequencyMeterComponent.repaint();
}

void PitchTrackingEQAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(darkColor);
}
void PitchTrackingEQAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / 20;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize10 = 10 * pixelSize;

	const int smallSliderWidth = 3 * pixelSize2 / 4;

	// Set size
	m_frequencyMeterComponent.setSize(width - pixelSize2 - smallSliderWidth - smallSliderWidth, pixelSize2);
	m_pluginNameComponent.setSize(width, pixelSize2);
	m_filterGroupLabel.setSize(pixelSize10, pixelSize);

	m_detectorFrequencyMinSlider.setSize(smallSliderWidth, pixelSize2);
	m_detectorFrequencyMaxSlider.setSize(smallSliderWidth, pixelSize2);
	m_smootherSpeedSlider.setSize(pixelSize3, pixelSize4);
	m_filterFrequencyMultiplierSlider.setSize(pixelSize3, pixelSize4);
	m_filterQSlider.setSize(pixelSize3, pixelSize4);
	m_filterGainSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);
	
	// Set position
	const int row2 = pixelSize2;
	const int row3 = row2 + pixelSize;
	const int row4 = row3 + pixelSize4;

	const int column1 = pixelSize;
	const int column2 = column1 + pixelSize3;
	const int column3 = column2 + pixelSize;
	const int column4 = column3 + pixelSize;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;
	const int column8 = column7 + pixelSize;

	m_pluginNameComponent.setTopLeftPosition(0, 0);
	m_filterGroupLabel.setTopLeftPosition(column3, row2);
	m_frequencyMeterComponent.setTopLeftPosition(column1 + smallSliderWidth, row4);
	
	m_detectorFrequencyMinSlider.setTopLeftPosition(column1, row4);
	m_detectorFrequencyMaxSlider.setTopLeftPosition(width - pixelSize - smallSliderWidth, row4);

	m_smootherSpeedSlider.setTopLeftPosition(column1, row3);
	m_filterFrequencyMultiplierSlider.setTopLeftPosition(column4, row3);
	m_filterQSlider.setTopLeftPosition(column5, row3);
	m_filterGainSlider.setTopLeftPosition(column6, row3);
	m_volumeSlider.setTopLeftPosition(column8, row3);
}