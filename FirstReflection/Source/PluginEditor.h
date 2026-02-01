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

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"

//==============================================================================
class SourceListenerVisualizer : public juce::Component
{
public:
	SourceListenerVisualizer()
	{
		// Positions in meters
		sourceX_m = 100.0f;
		sourceY_m = 100.0f;
		listenerX_m = 300.0f;
		listenerY_m = 50.0f;

		// Canvas range in meters
		xMin = -2000.0f;
		xMax = 2000.0f;
		yMin = 0.0f;
		yMax = 500.0f;

		// Speed of sound (m/s)
		speedOfSound = 343.0f;

		// Colours
		sourceColour = juce::Colour::fromRGB(255, 255, 190);
		listenerColour = juce::Colour::fromRGB(68, 68, 68);
		lineColourDirect = juce::Colour::fromRGB(255, 255, 190);
		lineColourReflected = juce::Colour::fromRGB(255, 255, 190);
		textColour = juce::Colours::white;

		sphereRadiusPx = 5.0f;
	}

	//==============================================================================
	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);

		// Map positions to pixels
		float srcX_px = mapX(sourceX_m);
		float srcY_px = mapY(sourceY_m);
		float lstX_px = mapX(listenerX_m);
		float lstY_px = mapY(listenerY_m);

		// Draw ground
		g.setColour(juce::Colours::darkgrey);
		g.drawLine(0.0f, mapY(yMin), (float)getWidth(), mapY(yMin), 1.0f);

		// Draw spheres
		g.setColour(sourceColour);
		g.fillEllipse(srcX_px - sphereRadiusPx, srcY_px - sphereRadiusPx,
			sphereRadiusPx * 2, sphereRadiusPx * 2);

		g.setColour(listenerColour);
		g.fillEllipse(lstX_px - sphereRadiusPx, lstY_px - sphereRadiusPx,
			sphereRadiusPx * 2, sphereRadiusPx * 2);

		// =======================
		// Direct path
		// =======================
		g.setColour(lineColourDirect);
		g.drawLine(srcX_px, srcY_px, lstX_px, lstY_px, 2.0f);

		// =======================
		// Reflected path (image-source method)
		// =======================

		// Mirror source across ground
		float srcMirrorX_m = sourceX_m;
		float srcMirrorY_m = 2.0f * yMin - sourceY_m;

		// Intersection with ground y = yMin
		float tBounce =
			(yMin - srcMirrorY_m) / (listenerY_m - srcMirrorY_m);

		float bounceX_m = srcMirrorX_m +
			tBounce * (listenerX_m - srcMirrorX_m);
		float bounceY_m = yMin;

		float bounceX_px = mapX(bounceX_m);
		float bounceY_px = mapY(bounceY_m);

		// Draw reflected segments
		g.setColour(lineColourReflected);
		g.drawLine(srcX_px, srcY_px, bounceX_px, bounceY_px, 2.0f);
		g.drawLine(bounceX_px, bounceY_px, lstX_px, lstY_px, 2.0f);

		// Bounce marker
		g.fillEllipse(bounceX_px - 4.0f, bounceY_px - 4.0f, 8.0f, 8.0f);

		// =======================
		// Info panel (top-left)
		// =======================
		drawInfoPanel(g);
	}

	//==============================================================================
	void setPositionsMeters(float srcX, float srcY, float lstX, float lstY)
	{
		sourceX_m = srcX;
		sourceY_m = srcY;
		listenerX_m = lstX;
		listenerY_m = lstY;
		repaint();
	}

	void setCanvasRangeMeters(float xmin, float xmax, float ymin, float ymax)
	{
		xMin = xmin;
		xMax = xmax;
		yMin = ymin;
		yMax = ymax;
		repaint();
	}

	void setSpeedOfSound(float c)
	{
		speedOfSound = c;
		repaint();
	}

private:
	//==============================================================================
	// World data
	float sourceX_m, sourceY_m;
	float listenerX_m, listenerY_m;

	float xMin, xMax, yMin, yMax;
	float speedOfSound;

	// Visuals
	float sphereRadiusPx;

	juce::Colour sourceColour;
	juce::Colour listenerColour;
	juce::Colour lineColourDirect;
	juce::Colour lineColourReflected;
	juce::Colour textColour;

	//==============================================================================
	float mapX(float x_m) const
	{
		return ((x_m - xMin) / (xMax - xMin)) * getWidth();
	}

	float mapY(float y_m) const
	{
		return getHeight() -
			((y_m - yMin) / (yMax - yMin)) * getHeight();
	}

	float calculateDirectDistance() const
	{
		float dx = listenerX_m - sourceX_m;
		float dy = listenerY_m - sourceY_m;
		return std::sqrt(dx*dx + dy * dy);
	}

	float calculateReflectedDistance() const
	{
		float srcMirrorY_m = 2.0f * yMin - sourceY_m;
		float dx = listenerX_m - sourceX_m;
		float dy = listenerY_m - srcMirrorY_m;
		return std::sqrt(dx*dx + dy * dy);
	}

	float calculateDelayMs() const
	{
		float deltaD =
			calculateReflectedDistance() - calculateDirectDistance();

		if (deltaD <= 0.0f)
			return 0.0f;

		return (deltaD / speedOfSound) * 1000.0f;
	}

	//==============================================================================
	void drawInfoPanel(juce::Graphics& g)
	{
		float directD = calculateDirectDistance();
		float reflectedD = calculateReflectedDistance();
		float delayMs = calculateDelayMs();

		juce::Rectangle<int> panel(10, 10, 220, 70);

		// Background
		g.setColour(juce::Colours::black.withAlpha(0.6f));
		//g.fillRect(panel);

		g.setColour(textColour);
		g.setFont(14.0f);

		int lineY = panel.getY();

		g.drawText("Direct:      " + juce::String(directD, 1) + " m",
			panel.getX() + 8, lineY, panel.getWidth(), 18,
			juce::Justification::left);

		lineY += 18;
		g.drawText("Reflected: " + juce::String(reflectedD, 1) + " m",
			panel.getX() + 8, lineY, panel.getWidth(), 18,
			juce::Justification::left);

		lineY += 18;
		g.drawText("Delay:      " + juce::String(delayMs, 1) + " ms",
			panel.getX() + 8, lineY, panel.getWidth(), 18,
			juce::Justification::left);
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SourceListenerVisualizer)
};

//==============================================================================
class FirstReflectionAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    FirstReflectionAudioProcessorEditor (FirstReflectionAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~FirstReflectionAudioProcessorEditor() override;

	static const int ROTARY_SLIDERS_COUNT = 6;
	static const int CANVAS_WIDTH = 1 + ROTARY_SLIDERS_COUNT * 3 + 1;
	static const int CANVAS_HEIGHT = 2 + 10 + 4 + 1;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;
	void timerCallback() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    FirstReflectionAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginLabel;

	ModernRotarySlider m_listenerHeightSlider;
	ModernRotarySlider m_emitterHeightSlider;
	ModernRotarySlider m_emitterDistanceSlider;
	ModernRotarySlider m_reflectionVolumeSlider;
	ModernRotarySlider m_reflectionLPCutoffSlider;
	ModernRotarySlider m_volumeSlider;

	SourceListenerVisualizer m_sourceVizualizer;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstReflectionAudioProcessorEditor)
};
