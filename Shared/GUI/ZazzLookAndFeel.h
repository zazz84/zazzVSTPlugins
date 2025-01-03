class ZazzLookAndFeel : public juce::LookAndFeel_V4
{
public:
	ZazzLookAndFeel() {};

	static const juce::Colour BACKGROUND_COLOR;
	static const juce::Colour KNOB_COLOR;
	static const juce::Colour KNOB_OUTLINE_COLOR;
	static const juce::Colour KNOB_HIGHLIGHT;
	static const juce::Colour MAIN_COLOR;

	static const int NAME_FONT_SIZE = 35;
	static const int LABEL_FONT_SIZE = 20;
	static const int VALUE_FONT_SIZE = 16;

	static const int ELEMENT_WIDTH = 100;
	static const int TOP_HEIGHT = 10;
	static const int NAME_HEIGHT = 35;
	static const int ELEMENT_HEIGHT = 145;
	
	static const int HEADER_HEIGHT = 35;
	static const int SLIDER_HEIGHT = 105;
	static const int FOOTER_HEIGHT = 0;

	float m_sliderTextSize = 15.0f;

	void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
	{
		const auto widthHalf = (float)width  * 0.5f;
		const auto heightHalf = (float)height  * 0.5f;
		const auto radius = 0.8f * std::fminf(widthHalf, heightHalf);
		const auto centreX = (float)x + widthHalf;
		const auto centreY = (float)y + heightHalf;
		const auto rx = centreX - radius;
		const auto ry = centreY - radius;
		const auto rw = radius * 2.0f;
		const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

		// Draw know
		g.setColour(KNOB_COLOR);
		g.fillEllipse(rx, ry, rw, rw);
		
		// outer shadow
		/*juce::Path shadow;
		shadow.addEllipse(rx, ry, rw, rw); // Example shape

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
		constexpr int kernelSize = 3;
		juce::ImageConvolutionKernel kernel(kernelSize);
		kernel.createGaussianBlur(kernelSize);

		// Apply the kernel to the image
		kernel.applyToImage(image, image, juce::Rectangle<int>(0, 0, imageWidth, imageHeight));

		// Draw the blurred image to the main graphics context
		g.setOpacity(0.8f);
		g.drawImageAt(image, 0, 0);*/

		// Knob outline
		g.setOpacity(1.0f);
		//const float lineThickness = height / 200.0f;
		const float lineThickness = 1.0f;
		g.setColour(KNOB_OUTLINE_COLOR);
		g.drawEllipse(rx, ry, rw, rw, lineThickness);

		// Knob point
		constexpr float knowRadiusFactor = 0.05f;

		juce::Path p;
		auto pointerThickness = lineThickness;
		p.addEllipse(juce::Rectangle<float>(-pointerThickness * 0.5f, -0.85f * radius, knowRadiusFactor * width, knowRadiusFactor * width));
		p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

		g.setColour(KNOB_HIGHLIGHT);
		g.fillPath(p);

		// Create the arc path
		const float arcRadius = 1.08f * radius;
		juce::Path arcPath;
		arcPath.addCentredArc(centreX, centreY, arcRadius, arcRadius,
			0.0f,																	// Rotation
			rotaryStartAngle,														// Start angle
			rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle),		// End angle
			true);																	// UseAsSegment

		juce::PathStrokeType strokeType(height / 25.0f);
		g.setColour(KNOB_HIGHLIGHT);
		g.strokePath(arcPath, strokeType);
																																				
		// Start and stop points
		/*constexpr float radiusOffset = -1.3f;

		juce::Path p2;
		p2.addEllipse(juce::Rectangle<float>(-pointerThickness * 0.5f, radiusOffset * radius, knowRadiusFactor * width, knowRadiusFactor * width));
		p2.applyTransform(juce::AffineTransform::rotation(rotaryStartAngle).translated(centreX, centreY));
		g.setColour(MEDIUM_COLOUR);
		g.fillPath(p2);

		juce::Path p3;
		p3.addEllipse(juce::Rectangle<float>(-pointerThickness * 0.5f, radiusOffset * radius, knowRadiusFactor * width, knowRadiusFactor * width));
		p3.applyTransform(juce::AffineTransform::rotation(rotaryEndAngle).translated(centreX, centreY));
		g.fillPath(p3);*/
	}

	void setSliderTextSize(float size)
	{
		m_sliderTextSize = size;
	}

	void drawLabel(juce::Graphics& g, juce::Label& label) override
	{
		auto font = label.getFont();
		font.setHeight(m_sliderTextSize);
		font.setTypefaceName("Arial");
		g.setFont(font);

		//g.setColour(label.findColour(juce::Label::textColourId));
		g.setColour(MAIN_COLOR);
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