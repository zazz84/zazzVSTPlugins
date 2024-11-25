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

//==============================================================================

class ZazzLookAndFeel_V2 : public juce::LookAndFeel_V4
{
public:
	ZazzLookAndFeel_V2() {};

	static const juce::Colour LIGHT_COLOUR;
	static const juce::Colour MEDIUM_COLOUR;
	static const juce::Colour DARK_COLOUR;

	static const int SLIDER_WIDTH = 120;
	static const int SLIDER_FONT_SIZE = 20;
	static const int FONT_DIVISOR = 9;
	static const int FONT_DIVISOR_MULTI_ROW = 12;

	static float getFontHeight() { return (float)SLIDER_WIDTH / (float)FONT_DIVISOR; };
	static int getLabelOffset() { return (int)(SLIDER_WIDTH / FONT_DIVISOR) + 5; };

	float m_sliderTextSize = 16.0f;

	void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
	{
		const auto widthHalf = (float)width  * 0.5f;
		const auto heightHalf = (float)height  * 0.5f;
		const auto radius = 0.9f * std::fminf(widthHalf, heightHalf);
		const auto centreX = (float)x + widthHalf;
		const auto centreY = (float)y + heightHalf;
		const auto rx = centreX - radius;
		const auto ry = centreY - radius;
		const auto rw = radius * 2.0f;
		const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

		// outer shadow
		juce::Path shadow;
		shadow.addEllipse(rx, ry, 1.03f * rw, 1.03f * rw); // Example shape

		// Define the size of the image to contain the path
		int imageWidth = width;
		int imageHeight = height;

		// Create an image to render the path
		juce::Image image(juce::Image::ARGB, imageWidth, imageHeight, true);
		juce::Graphics imageGraphics(image);

		// Draw the path into the image
		imageGraphics.setColour(MEDIUM_COLOUR);
		imageGraphics.fillPath(shadow);

		// Create a convolution kernel for Gaussian blur
		constexpr int kernelSize = 5;
		juce::ImageConvolutionKernel kernel(kernelSize);
		kernel.createGaussianBlur(kernelSize);

		// Apply the kernel to the image
		kernel.applyToImage(image, image, juce::Rectangle<int>(0, 0, imageWidth, imageHeight));

		// Draw the blurred image to the main graphics context
		g.setOpacity(0.8f);
		g.drawImageAt(image, 0, 0);

		// Outline
		g.setOpacity(1.0f);
		const float lineThickness = height / 28.0f;
		g.setColour(DARK_COLOUR);
		g.drawEllipse(rx, ry, rw, rw, lineThickness);

		// Knob point
		constexpr float knowRadiusFactor = 0.05f;

		juce::Path p;
		auto pointerThickness = lineThickness;
		p.addEllipse(juce::Rectangle<float>(-pointerThickness * 0.5f, -0.85f * radius, knowRadiusFactor * width, knowRadiusFactor * width));
		p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

		g.setColour(DARK_COLOUR);
		g.fillPath(p);

		// Start and stop points
		constexpr float radiusOffset = -1.3f;

		juce::Path p2;
		p2.addEllipse(juce::Rectangle<float>(-pointerThickness * 0.5f, radiusOffset * radius, knowRadiusFactor * width, knowRadiusFactor * width));
		p2.applyTransform(juce::AffineTransform::rotation(rotaryStartAngle).translated(centreX, centreY));
		g.setColour(MEDIUM_COLOUR);
		g.fillPath(p2);

		juce::Path p3;
		p3.addEllipse(juce::Rectangle<float>(-pointerThickness * 0.5f, radiusOffset * radius, knowRadiusFactor * width, knowRadiusFactor * width));
		p3.applyTransform(juce::AffineTransform::rotation(rotaryEndAngle).translated(centreX, centreY));
		g.fillPath(p3);
	}

	void setSliderTextSize(float size)
	{
		m_sliderTextSize = size;
	}

	void drawLabel(juce::Graphics& g, juce::Label& label) override
	{
		auto font = label.getFont();
		font.setHeight(m_sliderTextSize);
		g.setFont(font);

		g.setColour(label.findColour(juce::Label::textColourId));
		g.drawFittedText(label.getText(), label.getLocalBounds(), juce::Justification::centred, 1);

		// TODO: Fix lable editing
		label.setEditable(false, false, false);
	}

	void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
	{
		auto buttonArea = button.getLocalBounds();

		g.setColour(backgroundColour);
		g.fillRect(buttonArea);
	}
};