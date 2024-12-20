#pragma once

#include "../../../zazzVSTPlugins/Shared/GUI/ButtonSlider.h"

class ZazzAudioProcessorEditor
{
public:
	ZazzLookAndFeel zazzLookAndFeel;
	juce::Label m_pluginName;

	static const int SLIDERS[];
	static const int COLUMN_OFFSET[];

	inline void createSliderWithLabel(juce::Slider& slider, juce::Label& label, const std::string text, const std::string unit)
	{
		// Configure the slider
		slider.setLookAndFeel(&zazzLookAndFeel);
		slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
		slider.setTextValueSuffix(unit);

		// Configure the label
		label.setText(text, juce::dontSendNotification);
		label.setFont(juce::Font(ZazzLookAndFeel::LABEL_FONT_SIZE));
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

	inline void createCanvas(juce::AudioProcessorEditor& audioProcessorEditor, const int slidersCount[], int rows)
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
		
		const int canvasWidth = ZazzLookAndFeel::ELEMENT_WIDTH * slidersMax;
		const int canvasHeight = ZazzLookAndFeel::TOP_HEIGHT + ZazzLookAndFeel::NAME_HEIGHT + ZazzLookAndFeel::ELEMENT_HEIGHT * rows;
		
		audioProcessorEditor.setSize(canvasWidth, canvasHeight);

		if (auto* constrainer = audioProcessorEditor.getConstrainer())
		{
			constexpr int minScale = 50; // percentage
			constexpr int maxScale = 200; // percentage

			constrainer->setFixedAspectRatio((double)canvasWidth / (double)canvasHeight);
			constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
		}
	}

	// TODO: Keeping for backwards compatibility. Make one function in the future
	inline void resize(juce::AudioProcessorEditor& audioProcessorEditor, juce::Slider sliders[], juce::Label labels[], const int slidersCount[], const float columnOffset[], int rows, juce::Label& pluginName)
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

		const int canvasHeightUnscaled = ZazzLookAndFeel::TOP_HEIGHT + ZazzLookAndFeel::NAME_HEIGHT + ZazzLookAndFeel::ELEMENT_HEIGHT * rows;
		const int canvasHeightUnscaledOneRow = ZazzLookAndFeel::TOP_HEIGHT + ZazzLookAndFeel::NAME_HEIGHT + ZazzLookAndFeel::ELEMENT_HEIGHT;
		
		const int elementWidth = canvasWidth / slidersMax;
		const float elementHeightRatio = (float)ZazzLookAndFeel::ELEMENT_HEIGHT / (float)canvasHeightUnscaled;
		const int elemenHeight = (int)((float)canvasHeight * elementHeightRatio);
		
		const float topHeightRatio = (float)ZazzLookAndFeel::TOP_HEIGHT / (float)canvasHeightUnscaled;
		const int topHeight = (int)((float)canvasHeight * topHeightRatio);

		const float nameHeightRatio = (float)ZazzLookAndFeel::NAME_HEIGHT / (float)canvasHeightUnscaled;
		const int nameHeight = (int)((float)canvasHeight * nameHeightRatio);
		
		const float headerHeightRatio = (float)ZazzLookAndFeel::NAME_HEIGHT / (float)ZazzLookAndFeel::ELEMENT_HEIGHT;
		const float sliderHeightRatio = (float)ZazzLookAndFeel::SLIDER_HEIGHT / (float)ZazzLookAndFeel::ELEMENT_HEIGHT;
		const float footerHeightRatio = (float)ZazzLookAndFeel::FOOTER_HEIGHT / (float)ZazzLookAndFeel::ELEMENT_HEIGHT;
		const int headerHeight = (int)((float)elemenHeight * headerHeightRatio);
		const int sliderHeight = (int)((float)elemenHeight * sliderHeightRatio);
		const int footerHeight = (int)((float)elemenHeight * footerHeightRatio);
		
		const float nameFontRatio = ZazzLookAndFeel::NAME_FONT_SIZE / (float)canvasHeightUnscaledOneRow;
		const float nameFontSize = (float)elemenHeight * nameFontRatio;		
		
		const float labelFontRatio = ZazzLookAndFeel::LABEL_FONT_SIZE / (float)canvasHeightUnscaledOneRow;
		const float labelFontSize = (float)elemenHeight * labelFontRatio;
		
		const float valueFontRatio = ZazzLookAndFeel::VALUE_FONT_SIZE / (float)canvasHeightUnscaledOneRow;
		const float valueFontSize = (float)elemenHeight * valueFontRatio;
		zazzLookAndFeel.setSliderTextSize(valueFontSize);

		juce::Rectangle<int> pluginNameRectangle;
		pluginNameRectangle.setSize(canvasWidth, nameHeight);
		pluginNameRectangle.setPosition(0, topHeight);

		pluginName.setBounds(pluginNameRectangle);
		pluginName.setFont(juce::Font("Century Gothic", nameFontSize, juce::Font::bold));
		pluginName.setColour(juce::Label::textColourId, ZazzLookAndFeel::DARK_COLOUR);

		int i = 0;

		// Sliders + Labels
		for (int row = 0; row < rows; row++)
		{
			for (int column = 0; column < slidersCount[row]; column++)
			{
				juce::Rectangle<int> rectangle;

				const int xPos = (int)((columnOffset[row] + (float)column) * (float)elementWidth);
				const int yPos = topHeight + nameHeight + row * elemenHeight;

				// Label
				rectangle.setSize(elementWidth, headerHeight);
				rectangle.setPosition(xPos, yPos);

				labels[i].setBounds(rectangle);
				labels[i].setFont(juce::Font("Century Gothic", labelFontSize, juce::Font::bold));

				// Slider
				rectangle.setSize(elementWidth, sliderHeight);
				rectangle.setPosition(xPos, yPos + headerHeight);
				sliders[i].setBounds(rectangle);

				// Resize text box
				sliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, (int)(labelFontSize));

				i++;
			}
		}
	}

	
	inline void resize2(juce::AudioProcessorEditor& audioProcessorEditor, std::unique_ptr<ButtonSlider> sliders[], juce::Label labels[], const int slidersCount[], const float columnOffset[], int rows, juce::Label& pluginName)
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

		const int canvasHeightUnscaled = ZazzLookAndFeel::TOP_HEIGHT + ZazzLookAndFeel::NAME_HEIGHT + ZazzLookAndFeel::ELEMENT_HEIGHT * rows;
		const int canvasHeightUnscaledOneRow = ZazzLookAndFeel::TOP_HEIGHT + ZazzLookAndFeel::NAME_HEIGHT + ZazzLookAndFeel::ELEMENT_HEIGHT;

		const int elementWidth = canvasWidth / slidersMax;
		const float elementHeightRatio = (float)ZazzLookAndFeel::ELEMENT_HEIGHT / (float)canvasHeightUnscaled;
		const int elemenHeight = (int)((float)canvasHeight * elementHeightRatio);

		const float topHeightRatio = (float)ZazzLookAndFeel::TOP_HEIGHT / (float)canvasHeightUnscaled;
		const int topHeight = (int)((float)canvasHeight * topHeightRatio);

		const float nameHeightRatio = (float)ZazzLookAndFeel::NAME_HEIGHT / (float)canvasHeightUnscaled;
		const int nameHeight = (int)((float)canvasHeight * nameHeightRatio);

		const float headerHeightRatio = (float)ZazzLookAndFeel::NAME_HEIGHT / (float)ZazzLookAndFeel::ELEMENT_HEIGHT;
		const float sliderHeightRatio = (float)ZazzLookAndFeel::SLIDER_HEIGHT / (float)ZazzLookAndFeel::ELEMENT_HEIGHT;
		const float footerHeightRatio = (float)ZazzLookAndFeel::FOOTER_HEIGHT / (float)ZazzLookAndFeel::ELEMENT_HEIGHT;
		const int headerHeight = (int)((float)elemenHeight * headerHeightRatio);
		const int sliderHeight = (int)((float)elemenHeight * sliderHeightRatio);
		const int footerHeight = (int)((float)elemenHeight * footerHeightRatio);

		const float nameFontRatio = ZazzLookAndFeel::NAME_FONT_SIZE / (float)canvasHeightUnscaledOneRow;
		const float nameFontSize = (float)elemenHeight * nameFontRatio;

		const float labelFontRatio = ZazzLookAndFeel::LABEL_FONT_SIZE / (float)canvasHeightUnscaledOneRow;
		const float labelFontSize = (float)elemenHeight * labelFontRatio;

		const float valueFontRatio = ZazzLookAndFeel::VALUE_FONT_SIZE / (float)canvasHeightUnscaledOneRow;
		const float valueFontSize = (float)elemenHeight * valueFontRatio;
		zazzLookAndFeel.setSliderTextSize(valueFontSize);

		juce::Rectangle<int> pluginNameRectangle;
		pluginNameRectangle.setSize(canvasWidth, nameHeight);
		pluginNameRectangle.setPosition(0, topHeight);

		pluginName.setBounds(pluginNameRectangle);
		pluginName.setFont(juce::Font("Century Gothic", nameFontSize, juce::Font::bold));
		pluginName.setColour(juce::Label::textColourId, ZazzLookAndFeel::DARK_COLOUR);

		int i = 0;

		// Sliders + Labels
		for (int row = 0; row < rows; row++)
		{
			for (int column = 0; column < slidersCount[row]; column++)
			{
				juce::Rectangle<int> rectangle;

				const int xPos = (int)((columnOffset[row] + (float)column) * (float)elementWidth);
				const int yPos = topHeight + nameHeight + row * elemenHeight;

				// Label
				rectangle.setSize(elementWidth, headerHeight);
				rectangle.setPosition(xPos, yPos);

				labels[i].setBounds(rectangle);
				labels[i].setFont(juce::Font("Century Gothic", labelFontSize, juce::Font::bold));

				// Slider
				rectangle.setSize(elementWidth, sliderHeight);
				rectangle.setPosition(xPos, yPos + headerHeight);
				sliders[i]->setBounds(rectangle);

				// Resize text box
				sliders[i]->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, (int)(labelFontSize));

				i++;
			}
		}
	}
};