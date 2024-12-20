#pragma once

#include <JuceHeader.h>

class ButtonSlider : public juce::Slider
{
public:
    ButtonSlider(std::atomic<bool>& buttonStateRef) : buttonState(buttonStateRef) {}

    void mouseDown(const juce::MouseEvent& event) override
    {
        juce::Slider::mouseDown(event); // Keep normal slider functionality
        buttonState = true;             // Set the button state in the AudioProcessor
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        juce::Slider::mouseUp(event);   // Keep normal slider functionality
        buttonState = false;            // Reset the button state in the AudioProcessor
    }

private:
    std::atomic<bool>& buttonState; // Reference to the variable in the AudioProcessor
};