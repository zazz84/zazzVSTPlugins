#pragma once

#include <JuceHeader.h>
#include <vector>
#include <functional>
#include <string>

//==============================================================================
/**
 * Configuration metadata for a single slider.
 * Centralizes ALL slider-related information in one place:
 * - Value configuration (min, max, default, step)
 * - UI configuration (style, text box, label text)
 * - Serialization configuration (callbacks, transformations)
 */
struct SliderConfig
{
	// Value configuration
	juce::Slider* sliderPtr;
	juce::Label* labelPtr;                              // Optional, can be nullptr
	juce::Identifier jsonKey;                           // Key used in JSON serialization
	double minValue;
	double maxValue;
	double defaultValue;
	double stepSize;

	// UI configuration
	juce::String labelText;                             // Text to display in label
	juce::Slider::SliderStyle sliderStyle;              // Visual style of slider
	int textBoxWidth;                                   // Width of text box
	int textBoxHeight;                                  // Height of text box
	bool attachLabelToSlider;                           // Whether to attach label above slider

	// Callback and transformation configuration
	std::function<void(double)> onValueChanged;        // Optional callback when value changes
	std::function<double(double)> serializationFn;     // Optional: transform value for saving
	std::function<double(double)> deserializationFn;   // Optional: transform value when loading

	SliderConfig(
		juce::Slider* slider,
		juce::Label* label,
		const juce::String& key,
		double minVal,
		double maxVal,
		double defaultVal,
		double step = 1.0,
		const juce::String& label_text = "",
		juce::Slider::SliderStyle style = juce::Slider::LinearHorizontal,
		int textbox_width = 60,
		int textbox_height = 20,
		bool attach_label = true,
		std::function<void(double)> onChanged = nullptr,
		std::function<double(double)> serialize = nullptr,
		std::function<double(double)> deserialize = nullptr)
		: sliderPtr(slider),
		labelPtr(label),
		jsonKey(key),
		minValue(minVal),
		maxValue(maxVal),
		defaultValue(defaultVal),
		stepSize(step),
		labelText(label_text),
		sliderStyle(style),
		textBoxWidth(textbox_width),
		textBoxHeight(textbox_height),
		attachLabelToSlider(attach_label),
		onValueChanged(onChanged),
		serializationFn(serialize),
		deserializationFn(deserialize)
	{
	}
};

//==============================================================================
/**
 * Utility class for managing slider configurations and operations.
 * Handles initialization, serialization, deserialization, and reset operations.
 */
class SliderManager
{
public:
	//==========================================================================
	/**
	 * Setup UI for all sliders (addAndMakeVisible, setSliderStyle, etc.)
	 * This should be called in the component constructor.
	 */
	static void setupSliderUI(const std::vector<SliderConfig>& configs, juce::Component* parent)
	{
		for (const auto& config : configs)
		{
			if (config.sliderPtr && parent)
			{
				// Make slider visible
				parent->addAndMakeVisible(config.sliderPtr);

				// Configure slider appearance
				config.sliderPtr->setSliderStyle(config.sliderStyle);
				config.sliderPtr->setTextBoxStyle(juce::Slider::TextBoxRight, false, 
													config.textBoxWidth, config.textBoxHeight);

				// Setup label if provided
				if (config.labelPtr)
				{
					parent->addAndMakeVisible(config.labelPtr);
					config.labelPtr->setText(config.labelText, juce::dontSendNotification);

					// Attach label to slider if requested
					if (config.attachLabelToSlider)
					{
						config.labelPtr->attachToComponent(config.sliderPtr, true);
					}
				}
			}
		}
	}

	//==========================================================================
	/**
	 * Initialize all sliders with their configured ranges and default values.
	 * This should be called during component construction.
	 */
	static void initializeSliders(const std::vector<SliderConfig>& configs)
	{
		for (const auto& config : configs)
		{
			if (config.sliderPtr)
			{
				config.sliderPtr->setRange(config.minValue, config.maxValue, config.stepSize);
				config.sliderPtr->setValue(config.defaultValue);
			}
		}
	}

	//==========================================================================
	/**
	 * Reset all sliders to their default values.
	 */
	static void resetToDefaults(const std::vector<SliderConfig>& configs)
	{
		for (const auto& config : configs)
		{
			if (config.sliderPtr)
			{
				config.sliderPtr->setValue(config.defaultValue);
			}
		}
	}

	//==========================================================================
	/**
	 * Save all slider values to a DynamicObject.
	 */
	static void saveSliders(const std::vector<SliderConfig>& configs, juce::DynamicObject& obj)
	{
		for (const auto& config : configs)
		{
			if (config.sliderPtr)
			{
				double value = config.sliderPtr->getValue();

				// Apply serialization function if provided
				if (config.serializationFn)
				{
					value = config.serializationFn(value);
				}

				obj.setProperty(config.jsonKey, value);
			}
		}
	}

	//==========================================================================
	/**
	 * Load slider values from a DynamicObject.
	 * Only loads values that exist in the object (safe for forward/backward compatibility).
	 */
	static void loadSliders(const std::vector<SliderConfig>& configs, const juce::DynamicObject& obj)
	{
		for (const auto& config : configs)
		{
			if (config.sliderPtr && obj.hasProperty(config.jsonKey))
			{
				double value = obj.getProperty(config.jsonKey);

				// Apply deserialization function if provided
				if (config.deserializationFn)
				{
					value = config.deserializationFn(value);
				}

				config.sliderPtr->setValue(value);
			}
		}
	}

	//==========================================================================
	/**
	 * Get a slider config by its JSON key.
	 * Returns nullptr if not found.
	 */
	static const SliderConfig* findConfigByKey(
		const std::vector<SliderConfig>& configs,
		const juce::Identifier& key)
	{
		for (const auto& config : configs)
		{
			if (config.jsonKey == key)
			{
				return &config;
			}
		}
		return nullptr;
	}

	//==========================================================================
	/**
	 * Get a slider config by its pointer.
	 * Returns nullptr if not found.
	 */
	static const SliderConfig* findConfigBySlider(
		const std::vector<SliderConfig>& configs,
		juce::Slider* slider)
	{
		for (const auto& config : configs)
		{
			if (config.sliderPtr == slider)
			{
				return &config;
			}
		}
		return nullptr;
	}
};
