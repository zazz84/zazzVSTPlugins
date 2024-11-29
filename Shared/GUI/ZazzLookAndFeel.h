class ZazzLookAndFeel : public juce::LookAndFeel_V4
{
public:
	ZazzLookAndFeel() {};

	static const juce::Colour LIGHT_COLOUR;
	static const juce::Colour MEDIUM_COLOUR;
	static const juce::Colour DARK_COLOUR;

	static const int NAME_FONT_SIZE = 60;
	static const int LABEL_FONT_SIZE = 40;
	static const int VALUE_FONT_SIZE = 30;

	static const int ELEMENT_WIDTH = 160;
	static const int TOP_HEIGHT = 10;
	static const int NAME_HEIGHT = 50;
	static const int ELEMENT_HEIGHT = 240;
	
	static const int HEADER_HEIGHT = 60;
	static const int SLIDER_HEIGHT = 180;
	static const int FOOTER_HEIGHT = 0;

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
		font.setTypefaceName("Century Gothic");
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