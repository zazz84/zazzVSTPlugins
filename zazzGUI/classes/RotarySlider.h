#pragma once

#include <JuceHeader.h>

#include "../internal/RotarySliderTextBox.h"
#include "../internal/RotarySliderLookAndFeel.h"

namespace zazzGUI
{
	class RotarySlider : public juce::Component, public juce::Slider::Listener
	{
	public:
		struct ParameterDescription
		{
			juce::String paramName;
			juce::String unitName;
			juce::String labelName;
		};

		enum SliderType
		{
			Full,
			Dots,
			NoRing
		};

		RotarySlider(juce::AudioProcessorValueTreeState& vts,
			const juce::String& name,
			const juce::String& unit,
			const juce::String& label)
			: m_valueTreeState(vts),
			m_unit(unit),
			m_label(label)
		{
			// Use custom lookAndFeel
			m_slider.setLookAndFeel(&m_rotarySliderLookAndFeel);

			// Create rotary slider
			m_slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
			m_slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
			m_slider.addListener(this);
			addAndMakeVisible(m_slider);

			// Attach slider to m_valueTreeState
			m_sliderAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, name, m_slider));

			// Create text box
			m_textBox.setJustificationType(juce::Justification::centred);
			m_textBox.setEditable(true, true, false);														// Allow editing

			m_textBox.setText(juce::String(m_slider.getValue()) + unit, juce::dontSendNotification);		// Initialize with slider value
			m_textBox.onTextChange = [this]() { handleTextBoxChange(); };									// Update slider on label change
			addAndMakeVisible(m_textBox);
		}
		RotarySlider(juce::AudioProcessorValueTreeState& vts,
			const ParameterDescription& desc)
			: RotarySlider(vts, desc.paramName.toStdString(), desc.unitName.toStdString(), desc.labelName.toStdString())
		{
		}
		~RotarySlider()
		{
			m_slider.removeListener(this);
		};

		static const int MICRO_LOD_WIDTH_LIMIT = 50;
		static const int SMALL_LOD_WIDTH_LIMIT = 80;

		inline void paint(juce::Graphics& g) override
		{
			//g.fillAll(darkColor);

			// Slider name bounds
			const auto width = getWidth();
			const auto height = getHeight();

			if (width > MICRO_LOD_WIDTH_LIMIT && m_drawLabel)
			{
				juce::Rectangle<int> bounds;
				bounds.setPosition(0, 0 * height / 100);
				bounds.setSize(width, 15 * height / 100);

				// Slider name
				g.setColour(juce::Colours::white);
				g.setFont(0.8f * bounds.getHeight());
				g.drawText(m_label, bounds, juce::Justification::centred, false);
			}
		}
		inline void resized() override
		{
			// Size: 3 x 4

			if (m_drawTextBox)
			{
				if (!m_textBox.isVisible())
				{
					m_textBox.setVisible(true);
				}
			}
			else
			{
				if (m_textBox.isVisible())
				{
					m_textBox.setVisible(false);
				}
			}

			const auto width = getWidth();
			const auto height = getHeight();

			if (width > SMALL_LOD_WIDTH_LIMIT)
			{
				juce::Rectangle<int> bounds;
				bounds.setPosition(0, 20 * height / 100);
				bounds.setSize(width, 60 * height / 100);

				m_slider.setBounds(bounds);

				bounds.setPosition(0, 83 * height / 100);
				bounds.setSize(width, 15 * height / 100);
				
				if (!m_textBox.isVisible() && m_drawTextBox)
				{
					m_textBox.setVisible(true);
				}

				m_textBox.setBounds(bounds);
			}
			else if (width > MICRO_LOD_WIDTH_LIMIT)
			{
				juce::Rectangle<int> bounds;
				bounds.setPosition(0, 20 * height / 100);
				bounds.setSize(width, 70 * height / 100);

				m_slider.setBounds(bounds);

				if (m_textBox.isVisible())
				{
					m_textBox.setVisible(false);
				}
			}
			else
			{
				juce::Rectangle<int> bounds;
				bounds.setPosition(0, 5 * height / 100);
				bounds.setSize(width, 90 * height / 100);

				m_slider.setBounds(bounds);

				if (m_textBox.isVisible())
				{
					m_textBox.setVisible(false);
				}
			}
		}

		void setSliderType(SliderType sliderType)
		{
			m_rotarySliderLookAndFeel.setSliderType(static_cast<zazzGUI::RotarySliderLookAndFeel::SliderType>(sliderType));
		}
		void drawTextBox(bool drawTextBox)
		{
			m_drawTextBox = drawTextBox;
		}
		void drawLabel(bool drawLabel)
		{
			m_drawLabel = drawLabel;
		}

		// Update label when slider changes
		void sliderValueChanged(juce::Slider* sliderThatChanged) override
		{
			if (sliderThatChanged == &m_slider)
			{
				const auto value = m_slider.getValue();
				
				if (juce::ModifierKeys::currentModifiers.isShiftDown())
				{
					m_textBox.setText(juce::String(std::round(value)) + m_unit, juce::dontSendNotification);
				}
				else
				{
					m_textBox.setText(juce::String(value, getNumDecimalsFromInterval(m_slider.getInterval())) + m_unit, juce::dontSendNotification);
				}
			}
		}

		// Update slider when label text changes
		void handleTextBoxChange()
		{
			auto oldValue = m_slider.getValue();
			auto text = m_textBox.getText().trim();

			if (m_unit.isNotEmpty() && text.endsWith(m_unit))
			{
				text = text.dropLastCharacters(m_unit.length()).trim();
			}

			auto newValue = text.getDoubleValue();

			if (text.containsOnly("0123456789.-"))
			{
				m_slider.setValue(newValue, juce::sendNotification);
			}
			else
			{
				// Reset textbox to old value
				m_textBox.setText(juce::String(oldValue, getNumDecimalsFromInterval(m_slider.getInterval())) + m_unit, juce::dontSendNotification);
			}
		}

		float getValue()
		{
			return (float)m_slider.getValue();
		}
		void setValue(double newValue, juce::NotificationType notification = juce::sendNotification)
		{
			m_slider.setValue(newValue, notification);
		}

	protected:
		RotarySliderLookAndFeel m_rotarySliderLookAndFeel;
		juce::Slider m_slider;
		RotarySliderTextBox m_textBox{ "CustomLabel", "Default Text" };

		std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_sliderAttachment;
		juce::AudioProcessorValueTreeState& m_valueTreeState;

	private:
		int getNumDecimalsFromInterval(double interval)
		{
			if (interval <= 0.0)
				return 0;

			return juce::jlimit(0, 10, (int)std::round(std::abs(std::log10(interval))));
		}	

		juce::String m_unit;	// Displayed unit
		juce::String m_label;	// Displayed label

		bool m_drawLabel = true;
		bool m_drawTextBox = true;
	};
}