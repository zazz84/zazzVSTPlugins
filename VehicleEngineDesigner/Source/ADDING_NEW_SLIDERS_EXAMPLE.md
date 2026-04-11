# Example: Adding a New Slider

This document shows practical examples of how to add new sliders using the refactored system.

## Example 1: Simple Slider (No Transformation)

Let's say you want to add a new slider for "Attack Time" (0-500 ms, default 100 ms).

### Step 1: Add the member variable to MainComponent.h

```cpp
// In the Sliders section of MainComponent.h
juce::Slider m_attackTimeSlider;
juce::Label m_attackTimeLabel;
```

### Step 2: Add to m_sliderConfigs in initializeSliderConfigs()

```cpp
// In MainComponent::initializeSliderConfigs() in MainComponent.cpp
SliderConfig(&m_attackTimeSlider, &m_attackTimeLabel, "attackTime",
             0.0, 500.0, 100.0, 1.0)
```

That's it! The slider will automatically:
- ✅ Initialize with range 0-500, default 100, step 1
- ✅ Save/load values to/from JSON
- ✅ Reset to default (100) when "New Project" is clicked

### Step 3 (Optional): UI Setup

In the `resized()` method:
```cpp
m_attackTimeSlider.setSize(pixelSize12, pixelSize);
m_attackTimeSlider.setTopLeftPosition(column2, row5);
m_attackTimeLabel.setText("Attack (ms)", juce::dontSendNotification);
```

---

## Example 2: Slider with Callback

Let's add a "Quality" slider (0-100) that updates a filter when changed.

### Step 1: Member variables

```cpp
juce::Slider m_qualitySlider;
juce::Label m_qualityLabel;
```

### Step 2: Add callback method to MainComponent

```cpp
void qualitySliderChanged()
{
    const double quality = m_qualitySlider.getValue();
    // Update your filter or other component
    updateFilterQuality(quality);
}
```

### Step 3: Add to m_sliderConfigs with callback

```cpp
SliderConfig(&m_qualitySlider, &m_qualityLabel, "quality",
             0.0, 100.0, 50.0, 1.0,
             [this](double value) { qualitySliderChanged(); })  // <- Callback
```

Now when the slider value changes, your callback is invoked automatically!

---

## Example 3: Slider with Value Transformation

Let's add a "Gain" slider that:
- Shows 0-100 to the user (percentage)
- Saves/loads as 0.0-1.0 internally (normalized)

### Step 1: Member variables

```cpp
juce::Slider m_gainSlider;
juce::Label m_gainLabel;
```

### Step 2: Add to m_sliderConfigs with transformation functions

```cpp
SliderConfig(&m_gainSlider, &m_gainLabel, "gain",
             0.0, 100.0, 50.0, 1.0,
             nullptr,  // No callback
             [](double v) { return v / 100.0; },    // Save: 100 -> 1.0
             [](double v) { return v * 100.0; })    // Load: 1.0 -> 100
```

Now:
- UI displays 0-100%
- JSON stores 0.0-1.0
- Automatic conversion happens during save/load

---

## Example 4: Complete Slider with All Features

Let's add a "Frequency" slider with everything:
- Range: 20 Hz to 20000 Hz (logarithmic display)
- Default: 1000 Hz
- Callback: updates a frequency analyzer
- No value transformation (stored as-is)

### Step 1: Member variables

```cpp
juce::Slider m_frequencySlider;
juce::Label m_frequencyLabel;
```

### Step 2: Callback method

```cpp
void frequencySliderChanged()
{
    const double frequency = m_frequencySlider.getValue();
    updateAnalyzerFrequency(frequency);
    m_frequencyLabel.setText("Freq: " + juce::String(frequency, 1) + " Hz",
                            juce::dontSendNotification);
}
```

### Step 3: Add to m_sliderConfigs

```cpp
SliderConfig(&m_frequencySlider, &m_frequencyLabel, "frequency",
             20.0, 20000.0, 1000.0, 1.0,
             [this](double value) { frequencySliderChanged(); })
```

---

## Comparing Before and After

### Before (Old Way - 6 locations to modify)

1. **Declare slider** - MainComponent.h
```cpp
juce::Slider m_gainSlider;
```

2. **Constructor** - MainComponent.cpp
```cpp
MainComponent::MainComponent() {
    addAndMakeVisible(m_gainSlider);
    m_gainSlider.setRange(0.0, 100.0);
    m_gainSlider.setValue(50.0);
}
```

3. **Save (Location 1)** - `saveProjectToFile()`
```cpp
projectObject->setProperty("gain", m_gainSlider.getValue());
```

4. **Save (Location 2)** - `saveProjectButtonClicked()`
```cpp
projectObject->setProperty("gain", m_gainSlider.getValue());
```

5. **Load** - `loadProjectButtonClicked()`
```cpp
if (obj->hasProperty("gain"))
    m_gainSlider.setValue(obj->getProperty("gain"));
```

6. **Reset** - `newProjectButtonClicked()`
```cpp
m_gainSlider.setValue(50.0);
```

### After (New Way - 1 location to modify)

1. **Declare slider** - MainComponent.h
```cpp
juce::Slider m_gainSlider;
```

2. **Single entry** - `initializeSliderConfigs()` in MainComponent.cpp
```cpp
SliderConfig(&m_gainSlider, &m_gainLabel, "gain",
             0.0, 100.0, 50.0, 1.0)
```

Everything else (save, load, reset, initialization) happens **automatically**!

---

## Common Patterns

### Pattern 1: Detection Parameter
```cpp
SliderConfig(&m_thresholdSlider, &m_thresholdLabel, "threshold",
             -120.0, 0.0, -60.0, 0.1)
```

### Pattern 2: Generation Parameter with Callback
```cpp
SliderConfig(&m_crossfadeLengthSlider, &m_crossfadeLengthLabel, 
             "crossfadeLength", 10.0, 1000.0, 100.0, 10.0,
             [this](double v) { onCrossfadeLengthChanged(); })
```

### Pattern 3: Percentage with Transformation
```cpp
SliderConfig(&m_intensitySlider, &m_intensityLabel, "intensity",
             0.0, 100.0, 50.0, 1.0,
             nullptr,
             [](double v) { return v / 100.0; },  // Save as 0-1
             [](double v) { return v * 100.0; })  // Load as 0-100
```

### Pattern 4: Integer-Only Slider
```cpp
SliderConfig(&m_regionCountSlider, &m_regionCountLabel, "regionCount",
             1.0, 1000.0, 100.0, 1.0)  // Step size is 1.0 (integer)
```

### Pattern 5: Frequency Slider
```cpp
SliderConfig(&m_frequencySlider, &m_frequencyLabel, "frequency",
             20.0, 20000.0, 1000.0, 1.0,
             [this](double v) { updateFrequency(v); })
```

---

## Troubleshooting

### Q: The slider isn't appearing in JSON saves
**A:** Make sure the JSON key is unique and matches what you're loading:
```cpp
SliderConfig(..., "mySlider", ...)  // Must be unique!
```

### Q: The slider isn't resetting to default
**A:** Check that the default value is correct in the SliderConfig:
```cpp
SliderConfig(&m_slider, &m_label, "key",
             min, max, 50.0, step)  // 50.0 is default
```

### Q: My callback isn't being called
**A:** Make sure you're passing the lambda/callback as the 8th parameter:
```cpp
SliderConfig(&m_slider, &m_label, "key",
             0.0, 100.0, 50.0, 1.0,
             [this](double v) { yourCallbackHere(); })  // <- Make sure this is included
```

### Q: The slider range seems wrong
**A:** Check that min and max are in the right positions:
```cpp
SliderConfig(&m_slider, &m_label, "key",
             0.0,    // minValue
             100.0,  // maxValue
             50.0,   // defaultValue
             1.0)    // stepSize
```

---

## Summary

The refactored slider system makes it **trivial** to add new sliders:

✅ Just declare the member variables  
✅ Add ONE line to `m_sliderConfigs`  
✅ Everything else is automatic (save, load, reset, initialization)  
✅ Optional: Add callbacks for advanced behavior  
✅ Optional: Add transformations for value scaling  

No more duplicated code in 6 different places!
