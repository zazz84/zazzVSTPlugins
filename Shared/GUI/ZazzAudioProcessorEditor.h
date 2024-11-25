#pragma once

class ZazzAudioProcessorEditor
{
public:
	ZazzLookAndFeel zazzLookAndFeel;
	ZazzLookAndFeel_V2 zazzLookAndFeel_V2;

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

	inline void createSliderWithLabel(juce::Slider& slider, juce::Label& label, const std::string text, const std::string unit)
	{
		// Configure the slider
		slider.setLookAndFeel(&zazzLookAndFeel_V2);
		slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
		slider.setTextValueSuffix(unit);

		// Configure the label
		label.setText(text, juce::dontSendNotification);
		label.setFont(juce::Font(ZazzLookAndFeel_V2::SLIDER_FONT_SIZE));
		label.setJustificationType(juce::Justification::centred);
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
			constrainer->setSizeLimits((int)((float)width * 0.7f), (int)((float)ZazzLookAndFeel::SLIDER_WIDTH * 0.7f), (int)((float)width * 2.0f), (int)((float)ZazzLookAndFeel::SLIDER_WIDTH * 2.0f));
		}
	}

	inline void createCanvasMultiRow(juce::AudioProcessorEditor& audioProcessorEditor, const int slidersCount[], int rows)
	{
		// TODO: Cash
		int slidersMax = 0;

		for (int i = 0; i < rows; i++)
		{
			if (slidersCount[i] > slidersMax)
			{
				slidersMax = slidersCount[i];
			}
		}
		
		audioProcessorEditor.setResizable(true, true);

		constexpr int widthScale = 65; // percentage

		const int canvasWidth = widthScale * ZazzLookAndFeel_V2::SLIDER_WIDTH * slidersMax / 100;
		const int canvasHeight = ZazzLookAndFeel_V2::SLIDER_WIDTH * rows;
		audioProcessorEditor.setSize(canvasWidth, canvasHeight);

		if (auto* constrainer = audioProcessorEditor.getConstrainer())
		{
			constexpr int minScale = 70; // percentage
			constexpr int maxScale = 200; // percentage

			constrainer->setFixedAspectRatio(0.75 * (double)slidersMax / (double)rows);
			constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
		}
	}

	inline void resize(juce::AudioProcessorEditor& audioProcessorEditor, juce::Slider sliders[], juce::Label labels[], int sliderCount)
	{
		const int width = (int)(audioProcessorEditor.getWidth() / sliderCount);
		const int height = audioProcessorEditor.getHeight();
		const float fonthHeight = (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
		const int labelOffset = zazzLookAndFeel.getLabelOffset();

		// Sliders + Labels
		for (int i = 0; i < sliderCount; i++)
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

	inline void resizeMultiRow(juce::AudioProcessorEditor& audioProcessorEditor, juce::Slider sliders[], juce::Label labels[], const int slidersCount[], const int columnOffset[], int rows)
	{
		int slidersMax = 0;
		for (int i = 0; i < rows; i++)
		{
			if (slidersCount[i] > slidersMax)
			{
				slidersMax = slidersCount[i];
			}
		}

		const int canvasWidth = audioProcessorEditor.getWidth();
		const int canvasHeight = audioProcessorEditor.getHeight();
		
		const int elementWidth = canvasWidth / slidersMax;
		const int elemenHeight = canvasHeight / rows;
		
		const int height5 = elemenHeight / 20;	// 5% height
		const int height10 = elemenHeight / 10;	// 10% height

		const int labelHeight = height10;					// 10%
		const int sliderHeight = 6 * height10 + height5;	// 65%
			
		const float fonthHeight = (float)elemenHeight / (float)ZazzLookAndFeel_V2::FONT_DIVISOR_MULTI_ROW;
		zazzLookAndFeel_V2.setSliderTextSize(fonthHeight);

		int i = 0;

		// Sliders + Labels
		for (int row = 0; row < rows; row++)
		{
			for (int column = 0; column < slidersCount[row]; column++)
			{
				juce::Rectangle<int> rectangle;

				const int xPos = (columnOffset[row] + column) * elementWidth;
				const int yPos = row * elemenHeight + height10;

				// Label
				rectangle.setSize(elementWidth, labelHeight);
				rectangle.setPosition(xPos, yPos);

				labels[i].setBounds(rectangle);
				labels[i].setFont(juce::Font(fonthHeight, juce::Font::bold));

				// Slider
				rectangle.setSize(elementWidth, sliderHeight);
				rectangle.setPosition(xPos, yPos + labelHeight + height5);
				sliders[i].setBounds(rectangle);

				// Resize text box
				// TODO: How to get rid of this?
				sliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, (int)(fonthHeight * 1.2f));

				i++;
			}
		}
	}
};