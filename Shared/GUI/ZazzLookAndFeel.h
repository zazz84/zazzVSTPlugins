class ZazzLookAndFeel : public juce::LookAndFeel_V4
{
public:
	ZazzLookAndFeel()
	{
		setColour(juce::Slider::thumbColourId, juce::Colours::red);
	}

	static const juce::Colour LIGHT_COLOUR;
	static const juce::Colour MEDIUM_COLOUR;
	static const juce::Colour DARK_COLOUR;

	static const int SLIDER_WIDTH = 120;
	static const int SLIDER_FONT_SIZE = 20;
	static const int FONT_DIVISOR = 9;

	static const int TEXTBOX_FONT_SIZE = 15;

	static float getFontHeight() { return (float)SLIDER_WIDTH / (float)FONT_DIVISOR; };
	static int getLabelOffset() { return (int)(SLIDER_WIDTH / FONT_DIVISOR) + 5; };

	void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
	{
		auto radius = ((float)juce::jmin(width / 2, height / 2) - 4.0f) * 0.9f;
		auto centreX = (float)x + (float)width  * 0.5f;
		auto centreY = (float)y + (float)height * 0.5f;
		auto rx = centreX - radius;
		auto ry = centreY - radius;
		auto rw = radius * 2.0f;
		auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

		// outline
		const float lineThickness = height / 28.0f;
		
		g.setColour(MEDIUM_COLOUR);
		g.drawEllipse(rx, ry, rw, rw, lineThickness);

		juce::Path p;
		auto pointerLength = radius * 0.2f;
		auto pointerThickness = lineThickness;
		p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
		p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

		// pointer
		g.setColour(MEDIUM_COLOUR);
		g.fillPath(p);
	}

	juce::Label *createSliderTextBox(juce::Slider &) override
	{
		auto *l = new juce::Label();
		l->setJustificationType(juce::Justification::centred);
		l->setFont(juce::Font(TEXTBOX_FONT_SIZE));
		return l;
	}

	void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool, bool isButtonDown) override
	{
		auto buttonArea = button.getLocalBounds();

		g.setColour(backgroundColour);
		g.fillRect(buttonArea);
	}
};