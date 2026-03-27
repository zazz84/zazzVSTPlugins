#pragma once

#include <JuceHeader.h>

#include "../classes/Colors.h"

namespace zazzGUI
{
	class RotarySliderLookAndFeel : public juce::LookAndFeel_V4
	{
	public:
		enum SliderType
		{
			Full,
			Dots,
			NoRing
		};

		RotarySliderLookAndFeel() = default;

		void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
		{
			const auto widthHalf = 0.5f * static_cast<float>(width);
			const auto heightHalf = 0.5f * static_cast<float>(height);
			const auto radius = 0.8f * widthHalf;
			const auto centreX = static_cast<float>(x) + widthHalf;
			const auto centreY = static_cast<float>(y) + heightHalf;
			const auto rx = centreX - radius;
			const auto ry = centreY - radius;
			const auto rw = 2.0f * radius;
			const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

			if (m_sliderType == SliderType::Full)
			{
				juce::Rectangle<float> bounds(rx, ry, rw, rw);

				// Light outer circle
				g.setColour(zazzGUI::Colors::lightColor);
				g.fillEllipse(bounds);

				// Dark outer stroke with gradiesnt	
				float expandPixels = 0.006f * rw;
				bounds.expand(-expandPixels, -expandPixels);
				g.setColour(zazzGUI::Colors::darkColor);
				g.drawEllipse(bounds, expandPixels);

				// Light to dark circular gradient		
				bounds.expand(-expandPixels, -expandPixels);
				juce::ColourGradient gradient(zazzGUI::Colors::darkColor, centreX, centreY, zazzGUI::Colors::lightColor, centreX + 0.5f * bounds.getWidth(), centreY + 0.5f * bounds.getWidth(), true);
				gradient.addColour(0.1f, zazzGUI::Colors::darkColor);
				g.setGradientFill(gradient);
				g.fillEllipse(bounds);

				// Dark to light gradient
				expandPixels = bounds.getWidth() / 2.0f / 2.7f;
				bounds.expand(-expandPixels, -expandPixels);
				juce::ColourGradient gradient2(zazzGUI::Colors::lightColor, centreX, centreY, zazzGUI::Colors::darkColor, centreX + 0.5f * bounds.getWidth(), centreY + 0.5f * bounds.getWidth(), true);
				gradient2.addColour(0.8f, zazzGUI::Colors::darkColor);
				g.setGradientFill(gradient2);
				g.fillEllipse(bounds);

				// Light inner ring
				expandPixels = 0.004f * rw;
				bounds.expand(-expandPixels, -expandPixels);
				g.setColour(zazzGUI::Colors::lightColor);
				g.drawEllipse(bounds, expandPixels);

				// Dark center circle
				expandPixels = bounds.getWidth() / 2.0f / 4.3f;
				bounds.expand(-expandPixels, -expandPixels);
				g.setColour(zazzGUI::Colors::darkColor);
				g.fillEllipse(bounds);

				// Light outer ring
				expandPixels = 0.008f * rw;
				g.setColour(zazzGUI::Colors::lightColor);
				g.drawEllipse(bounds, expandPixels);

				// Create background arc path
				const float arcRadius = 0.8f * radius;
				juce::Path arcPath;
				arcPath.addCentredArc(centreX, centreY, arcRadius, arcRadius,
					0.0f,																	// Rotation
					rotaryStartAngle,														// Start angle
					rotaryEndAngle,															// End angle
					true);																	// UseAsSegment

				const float strokeThicknessBeckground = 0.18f * radius;
				juce::PathStrokeType strokeTypeBackground(strokeThicknessBeckground);
				strokeTypeBackground.setEndStyle(juce::PathStrokeType::EndCapStyle::rounded);

				g.setColour(zazzGUI::Colors::darkColor);
				g.strokePath(arcPath, strokeTypeBackground);

				// Create the arc path for set value
				arcPath.clear();
				arcPath.addCentredArc(centreX, centreY, arcRadius, arcRadius,
					0.0f,																	// Rotation
					rotaryStartAngle,														// Start angle																					
					angle,																	// End angle
					true);																	// UseAsSegment

				const float strokeThicknessActive = 0.16f * radius;
				juce::PathStrokeType strokeTypeActive(strokeThicknessActive);
				strokeTypeActive.setEndStyle(juce::PathStrokeType::EndCapStyle::rounded);

				g.setColour(zazzGUI::Colors::highlightColor);
				g.strokePath(arcPath, strokeTypeActive);
				//m_dropShadow.render(g, arcPath);

				// Knob point line
				juce::Path markerPath;
				juce::Line<float> line(0.0f, -0.7f * 0.5f * bounds.getWidth(), 0.0f, -0.4f * 0.5f * bounds.getWidth());

				markerPath.startNewSubPath(line.getStart());
				markerPath.lineTo(line.getEnd());
				markerPath.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

				g.strokePath(markerPath, strokeTypeActive);
			}
			else if (m_sliderType == SliderType::Dots)
			{
				juce::Rectangle<float> bounds(rx, ry, rw, rw);

				// Light outer circle
				g.setColour(zazzGUI::Colors::lightColor);
				g.fillEllipse(bounds);

				// Dark outer stroke with gradiesnt	
				float expandPixels = 0.006f * rw;
				bounds.expand(-expandPixels, -expandPixels);
				g.setColour(zazzGUI::Colors::darkColor);
				g.drawEllipse(bounds, expandPixels);

				// Light to dark circular gradient		
				bounds.expand(-expandPixels, -expandPixels);
				juce::ColourGradient gradient(zazzGUI::Colors::darkColor, centreX, centreY, zazzGUI::Colors::lightColor, centreX + 0.5f * bounds.getWidth(), centreY + 0.5f * bounds.getWidth(), true);
				gradient.addColour(0.1f, zazzGUI::Colors::darkColor);
				g.setGradientFill(gradient);
				g.fillEllipse(bounds);

				// Dark to light gradient
				expandPixels = bounds.getWidth() / 2.0f / 4.0f;
				bounds.expand(-expandPixels, -expandPixels);
				juce::ColourGradient gradient2(zazzGUI::Colors::lightColor, centreX, centreY, zazzGUI::Colors::darkColor, centreX + 0.5f * bounds.getWidth(), centreY + 0.5f * bounds.getWidth(), true);
				gradient2.addColour(0.8f, zazzGUI::Colors::darkColor);
				g.setGradientFill(gradient2);
				g.fillEllipse(bounds);

				// Light inner ring
				expandPixels = 0.004f * rw;
				bounds.expand(-expandPixels, -expandPixels);
				g.setColour(zazzGUI::Colors::lightColor);
				g.drawEllipse(bounds, expandPixels);

				// Dark center circle
				expandPixels = bounds.getWidth() / 2.0f / 5.0f;
				bounds.expand(-expandPixels, -expandPixels);
				g.setColour(zazzGUI::Colors::darkColor);
				g.fillEllipse(bounds);

				// Light outer ring
				expandPixels = 0.008f * rw;
				g.setColour(zazzGUI::Colors::lightColor);
				g.drawEllipse(bounds, expandPixels);

				// Knob point line
				juce::Path markerPath;
				juce::Line<float> line(0.0f, -0.7f * 0.5f * bounds.getWidth(), 0.0f, -0.4f * 0.5f * bounds.getWidth());

				markerPath.startNewSubPath(line.getStart());
				markerPath.lineTo(line.getEnd());
				markerPath.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

				float strokeThicknessActive = 0.16f * radius;
				juce::PathStrokeType strokeTypeActive(strokeThicknessActive);
				strokeTypeActive.setEndStyle(juce::PathStrokeType::EndCapStyle::rounded);

				g.setColour(zazzGUI::Colors::highlightColor);
				g.strokePath(markerPath, strokeTypeActive);

				// Dial dots
				const int dotCount = 18;
				const float angleStep = (rotaryEndAngle - rotaryStartAngle) / static_cast<float>(dotCount);

				float dotAngle = rotaryStartAngle;

				const float dotX = -0.5f * strokeThicknessActive;
				const float dotY = 0.4f * strokeThicknessActive - radius;

				const int dotHighlightCount = static_cast<int>((angle + 0.5f * angleStep - rotaryStartAngle) / angleStep);

				for (int i = 0; i < dotCount + 1; i++)
				{
					juce::Path dotPath;
					dotPath.addEllipse(juce::Rectangle<float>(dotX, dotY, strokeThicknessActive, strokeThicknessActive));
					dotPath.applyTransform(juce::AffineTransform::rotation(dotAngle).translated(centreX, centreY));

					if (i > dotHighlightCount)
					{
						g.setColour(zazzGUI::Colors::darkColor);
					}

					g.fillPath(dotPath);

					dotAngle += angleStep;
				}
			}
			else
			{
				juce::Rectangle<float> bounds(rx, ry, rw, rw);

				// Light outer circle
				g.setColour(zazzGUI::Colors::lightColor);
				g.fillEllipse(bounds);

				// Dark outer stroke with gradiesnt	
				float expandPixels = 0.006f * rw;
				bounds.expand(-expandPixels, -expandPixels);
				g.setColour(zazzGUI::Colors::darkColor);
				g.drawEllipse(bounds, expandPixels);

				// Light to dark circular gradient		
				bounds.expand(-expandPixels, -expandPixels);
				juce::ColourGradient gradient(zazzGUI::Colors::darkColor, centreX, centreY, zazzGUI::Colors::lightColor, centreX + 0.5f * bounds.getWidth(), centreY + 0.5f * bounds.getWidth(), true);
				gradient.addColour(0.1f, zazzGUI::Colors::darkColor);
				g.setGradientFill(gradient);
				g.fillEllipse(bounds);

				// Dark to light gradient
				expandPixels = bounds.getWidth() / 2.0f / 10.0f;
				bounds.expand(-expandPixels, -expandPixels);
				juce::ColourGradient gradient2(zazzGUI::Colors::lightColor, centreX, centreY, zazzGUI::Colors::darkColor, centreX + 0.5f * bounds.getWidth(), centreY + 0.5f * bounds.getWidth(), true);
				gradient2.addColour(0.8f, zazzGUI::Colors::darkColor);
				g.setGradientFill(gradient2);
				g.fillEllipse(bounds);

				// Light inner ring
				expandPixels = 0.004f * rw;
				bounds.expand(-expandPixels, -expandPixels);
				g.setColour(zazzGUI::Colors::lightColor);
				g.drawEllipse(bounds, expandPixels);

				// Dark center circle
				expandPixels = bounds.getWidth() / 2.0f / 6.0f;
				bounds.expand(-expandPixels, -expandPixels);
				g.setColour(zazzGUI::Colors::darkColor);
				g.fillEllipse(bounds);

				// Light outer ring
				expandPixels = 0.008f * rw;
				g.setColour(zazzGUI::Colors::lightColor);
				g.drawEllipse(bounds, expandPixels);

				// Knob point line
				juce::Path markerPath;
				juce::Line<float> line(0.0f, -0.7f * 0.5f * bounds.getWidth(), 0.0f, -0.4f * 0.5f * bounds.getWidth());

				markerPath.startNewSubPath(line.getStart());
				markerPath.lineTo(line.getEnd());
				markerPath.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

				float strokeThicknessActive = 0.16f * radius;
				juce::PathStrokeType strokeTypeActive(strokeThicknessActive);
				strokeTypeActive.setEndStyle(juce::PathStrokeType::EndCapStyle::rounded);

				g.setColour(zazzGUI::Colors::highlightColor);
				g.strokePath(markerPath, strokeTypeActive);
			}
		}

		SliderType m_sliderType = SliderType::Full;

		void setSliderType(SliderType sliderType)
		{
			m_sliderType = sliderType;
		}
	};
}