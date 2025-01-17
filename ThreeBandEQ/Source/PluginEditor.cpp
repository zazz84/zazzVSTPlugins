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
ThreeBandEQAudioProcessorEditor::ThreeBandEQAudioProcessorEditor(ThreeBandEQAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_gainLowSlider(vts, ThreeBandEQAudioProcessor::paramsNames[0], ThreeBandEQAudioProcessor::paramsUnitNames[0], ThreeBandEQAudioProcessor::labelNames[0]),
	m_frequencyLowMidSlider(vts, ThreeBandEQAudioProcessor::paramsNames[1], ThreeBandEQAudioProcessor::paramsUnitNames[1], ThreeBandEQAudioProcessor::labelNames[1]),
	m_gainMidSlider(vts, ThreeBandEQAudioProcessor::paramsNames[2], ThreeBandEQAudioProcessor::paramsUnitNames[2], ThreeBandEQAudioProcessor::labelNames[2]),
	m_frequencyMidHighSlider(vts, ThreeBandEQAudioProcessor::paramsNames[3], ThreeBandEQAudioProcessor::paramsUnitNames[3], ThreeBandEQAudioProcessor::labelNames[3]),
	m_gainHighSlider(vts, ThreeBandEQAudioProcessor::paramsNames[4], ThreeBandEQAudioProcessor::paramsUnitNames[4], ThreeBandEQAudioProcessor::labelNames[4]),
	m_volumeSlider(vts, ThreeBandEQAudioProcessor::paramsNames[5], ThreeBandEQAudioProcessor::paramsUnitNames[5], ThreeBandEQAudioProcessor::labelNames[5]),

	m_pluginName("zazz::ThreeBandEQ"),

	m_lowGroupLabel("Low"),
	m_midGroupLabel("Mid"),
	m_highGroupLabel("High")
{	

	m_frequencyLowMidSlider.setSliderType(ModernRotarySliderLookAndFeel::SliderType::NoRing);
	m_frequencyMidHighSlider.setSliderType(ModernRotarySliderLookAndFeel::SliderType::NoRing);

	addAndMakeVisible(m_gainLowSlider);
	addAndMakeVisible(m_frequencyLowMidSlider);
	addAndMakeVisible(m_gainMidSlider);
	addAndMakeVisible(m_frequencyMidHighSlider);
	addAndMakeVisible(m_gainHighSlider);
	addAndMakeVisible(m_volumeSlider);

	addAndMakeVisible(m_pluginName);

	addAndMakeVisible(m_lowGroupLabel);
	addAndMakeVisible(m_midGroupLabel);
	addAndMakeVisible(m_highGroupLabel);

	setResizable(true, true);

	const int canvasWidth = (2 + 6 * 3) * 30;
	const int canvasHeight = (2 + 1 + 4) * 30;

	setSize(canvasWidth, canvasHeight);

	if (auto* constrainer = getConstrainer())
	{
		constexpr int minScale = 50;		// percentage
		constexpr int maxScale = 200;		// percentage

		constrainer->setFixedAspectRatio((double)canvasWidth / (double)canvasHeight);
		constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
	}
}

ThreeBandEQAudioProcessorEditor::~ThreeBandEQAudioProcessorEditor()
{
}

//==============================================================================
void ThreeBandEQAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void ThreeBandEQAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / 20;
	const int pixelSize2 = pixelSize + pixelSize;

	// Set size
	m_pluginName.setSize(width, pixelSize2);

	const int sliderWidth = 3 * pixelSize;
	const int smallSliderWidth = 3 * pixelSize;
	const int sliderHeight = 4 * pixelSize;
	const int smallSliderWidthHeight = 4 * pixelSize;

	m_lowGroupLabel.setSize(sliderWidth, pixelSize);
	m_midGroupLabel.setSize(sliderWidth, pixelSize);
	m_highGroupLabel.setSize(sliderWidth, pixelSize);

	m_gainLowSlider.setSize(sliderWidth, sliderHeight);
	m_frequencyLowMidSlider.setSize(smallSliderWidth, smallSliderWidthHeight);
	m_gainMidSlider.setSize(sliderWidth, sliderHeight);
	m_frequencyMidHighSlider.setSize(smallSliderWidth, smallSliderWidthHeight);
	m_gainHighSlider.setSize(sliderWidth, sliderHeight);
	m_volumeSlider.setSize(sliderWidth, sliderHeight);

	// Set position
	const int sliderColumn1 = pixelSize;
	const int sliderColumn2 = sliderColumn1 + sliderWidth;
	const int sliderColumn3 = sliderColumn2 + smallSliderWidth;
	const int sliderColumn4 = sliderColumn3 + sliderWidth ;
	const int sliderColumn5 = sliderColumn4 + smallSliderWidth;
	const int sliderColumn6 = sliderColumn5 + sliderWidth;

	const int groupLabelRow1 = pixelSize2;

	m_lowGroupLabel.setTopLeftPosition(sliderColumn1, groupLabelRow1);
	m_midGroupLabel.setTopLeftPosition(sliderColumn3, groupLabelRow1);
	m_highGroupLabel.setTopLeftPosition(sliderColumn5, groupLabelRow1);

	const int sliderRow1 = pixelSize + pixelSize2;

	m_gainLowSlider.setTopLeftPosition(sliderColumn1, sliderRow1);
	m_frequencyLowMidSlider.setTopLeftPosition(sliderColumn2, sliderRow1);
	m_gainMidSlider.setTopLeftPosition(sliderColumn3, sliderRow1);
	m_frequencyMidHighSlider.setTopLeftPosition(sliderColumn4, sliderRow1);
	m_gainHighSlider.setTopLeftPosition(sliderColumn5, sliderRow1);
	m_volumeSlider.setTopLeftPosition(sliderColumn6, sliderRow1);
}