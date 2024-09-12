class ZazzAudioProcessorEditor
{
public:
	ZazzLookAndFeel zazzLookAndFeel;

	inline void createLabel(juce::Label& label, std::string text)
	{
		label.setText(text, juce::dontSendNotification);
		label.setFont(juce::Font(ZazzLookAndFeel::getFontHeight(), juce::Font::bold));
		label.setJustificationType(juce::Justification::centred);
	}

	inline void createSlider(juce::Slider& slider)
	{
		slider.setLookAndFeel(&zazzLookAndFeel);
		slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
		slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, ZazzLookAndFeel::SLIDER_FONT_SIZE);
	}

	inline void createButton(juce::TextButton& button, int groupID = -1)
	{
		if (groupID != -1)
		{
			button.setRadioGroupId(groupID);
		}

		button.setClickingTogglesState(true);
		button.setColour(juce::TextButton::buttonColourId, ZazzLookAndFeel::MEDIUM_COLOUR);
		button.setColour(juce::TextButton::buttonOnColourId, ZazzLookAndFeel::DARK_COLOUR);
		button.setLookAndFeel(&zazzLookAndFeel);
	}

	inline void createCanvas(juce::AudioProcessorEditor& audioProcessorEditor, int sliderCount)
	{
		audioProcessorEditor.setResizable(true, true);
		const int width = ZazzLookAndFeel::SLIDER_WIDTH * sliderCount;
		audioProcessorEditor.setSize(width, ZazzLookAndFeel::SLIDER_WIDTH);

		if (auto* constrainer = audioProcessorEditor.getConstrainer())
		{
			constrainer->setFixedAspectRatio(sliderCount);
			constrainer->setSizeLimits((int)((float)width * 0.7f), (int)((float)ZazzLookAndFeel::SLIDER_WIDTH * 0.7), (int)((float)width * 2.0f), (int)((float)ZazzLookAndFeel::SLIDER_WIDTH * 2.0f));
		}
	}

	inline void resize(juce::AudioProcessorEditor& audioProcessorEditor, juce::Slider sliders[], juce::Label labels[], int sliderCount)
	{
		const int width = (int)(audioProcessorEditor.getWidth() / sliderCount);
		const int height = audioProcessorEditor.getHeight();
		const float fonthHeight = (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
		const int labelOffset = zazzLookAndFeel.getLabelOffset();

		// Sliders + Labels
		for (int i = 0; i < sliderCount; ++i)
		{
			juce::Rectangle<int> rectangle;

			rectangle.setSize(width, height);
			rectangle.setPosition(i * width, 0);
			sliders[i].setBounds(rectangle);

			rectangle.removeFromBottom(labelOffset);
			labels[i].setBounds(rectangle);
			labels[i].setFont(juce::Font(fonthHeight, juce::Font::bold));
		}
	}
};