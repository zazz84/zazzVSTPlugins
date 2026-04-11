# Phase 3: Parameter Separation Completion

## Objective
Disconnect `m_minimumLengthSlider` from zero-phase filter setup and add an independent slider for controlling low-pass frequency in "Time Domain + Filter" detection mode.

## Changes Implemented

### 1. **MainComponent.h - Header Changes**

#### Added Label Member Variable
```cpp
juce::Label m_lowPassFrequencyLabel;  // NEW: Label for low-pass frequency slider
```
- Location: Line 1703 (in Labels section)
- Positioned between `m_zeroCrossingCountLabel` and `m_crossfadeLengthLabel`

#### Updated Filter Logic in detectRegionsButtonClicked()
**Before:**
```cpp
const float lowPassFrequency = maximumFrequencyHz * 1.3f;  // Derived from m_minimumLengthSlider
```

**After:**
```cpp
// Read low-pass frequency from independent slider (in samples), convert to Hz
const double lowPassSamples = m_lowPassFrequencySlider.getValue();
const float lowPassFrequency = (float)(m_sampleRate / lowPassSamples);
```

- **Key Benefits:**
  - Decouples filter frequency from region detection sensitivity
  - Slider values in samples (user-friendly for sample-based thinking)
  - Automatic conversion to Hz using: `Hz = sampleRate / samples`
  - High-pass frequency remains fixed at 10 Hz for stability

### 2. **MainComponent.cpp - Implementation Changes**

#### Added Slider Configuration
```cpp
SliderConfig(
    &m_lowPassFrequencySlider,
    &m_lowPassFrequencyLabel,
    "lowPassFrequency",
    10.0, 10000.0, 1000.0, 1.0,
    "Low Pass (samples)"),
```
- **Range:** 10 to 10,000 samples
- **Default:** 1,000 samples (roughly 22.7 Hz at 44.1 kHz)
- **Step:** 1.0 sample
- **Label:** "Low Pass (samples)" - indicates values are in samples, not Hz

#### Visibility Control in Detection Type Combo Box onChange
```cpp
// Show low-pass frequency slider only in Time Domain + Filter mode (id = 2)
const bool showLowPassFrequency = (detectionTypeId == 2);
m_lowPassFrequencySlider.setVisible(showLowPassFrequency);
m_lowPassFrequencyLabel.setVisible(showLowPassFrequency);
```
- Slider visible ONLY when "Time Domain + Filter" mode (id = 2) is selected
- Automatically hidden for other detection modes
- Provides clear UI indication of which mode uses this slider

#### Initial Visibility Setup in Constructor
```cpp
// Initially hide low-pass frequency slider (show only in Time Domain + Filter mode)
m_lowPassFrequencySlider.setVisible(false);
m_lowPassFrequencyLabel.setVisible(false);
```

#### UI Layout in resized() Method
**Size Setting:**
```cpp
m_lowPassFrequencySlider.setSize(pixelSize12, pixelSize);
```

**Position Setting:**
```cpp
m_lowPassFrequencySlider.setTopLeftPosition(column9, row4);
```
- Positioned at row4, same column as other detection sliders
- Shares row with `m_exportMaxRegionOffsetSlider` (which is hidden in Time Domain + Filter mode)
- Maintains consistent UI layout

## Parameter Separation Architecture

### Before (Phase 2)
```
m_minimumLengthSlider
    ├─> Region Detection Sensitivity
    └─> Filter Frequency Derivation (maximumFrequencyHz = sampleRate / sliderValue)
```
**Problem:** Two independent concerns controlled by one parameter

### After (Phase 3)
```
m_minimumLengthSlider
    └─> Region Detection Sensitivity ONLY

m_lowPassFrequencySlider (NEW)
    └─> Filter Frequency Control ONLY
        └─> Conversion: Hz = sampleRate / samples
```
**Solution:** Each parameter controls exactly one concern

## Time Domain + Filter Mode Configuration

### Detection Settings
- **m_minimumLengthSlider:** Controls minimum region length (1-10,000 samples)
  - Used for region detection sensitivity
  - Determines how small a region can be to be detected

### Filter Settings  
- **m_lowPassFrequencySlider:** Controls low-pass filter frequency (10-10,000 samples)
  - Independent control of filter characteristics
  - Values in samples for intuitive control
  - Typical range: 500-2000 samples (11 Hz - 88 Hz at 44.1 kHz)

### High-Pass Filter
- Fixed at 10 Hz - provides DC offset removal without user configuration

### Band-Pass Chain
```
Input Signal
    ↓
High-Pass Filter (10 Hz)
    ↓
Low-Pass Filter (user-controlled via m_lowPassFrequencySlider)
    ↓
Zero-Phase Bidirectional Processing (via ZeroPhaseFilter)
    ↓
Detection Applied
    ↓
Region Detection Output
```

## Data Persistence

The `SliderConfig` system automatically handles:
- **Save:** `m_lowPassFrequencySlider` value saved to JSON project file under key "lowPassFrequency"
- **Load:** Value restored from project file when loading
- **Reset:** Resets to default value (1000.0 samples)

## Visibility Logic

### Slider Visibility by Detection Mode

| Detection Mode | Min Length | Low Pass Freq | Threshold | FFT Phase | Offset |
|---|---|---|---|---|---|
| Time Domain | ✓ | ✗ | ✓ | ✗ | ✓ |
| **Time Domain + Filter** | **✓** | **✓** | **✓** | **✗** | **✓** |
| FFT | ✓ | ✗ | ✗ | ✓ | ✓ |
| FFT + Filter | ✓(multiplier) | ✗ | ✗ | ✗ | ✗ |

## Build Status
✅ **Successful** - All changes compile without errors or warnings

## Files Modified
1. `MainComponent.h` - Added label variable, updated filter logic
2. `MainComponent.cpp` - Added slider config, visibility control, UI layout

## Testing Checklist
- [x] Build compiles successfully
- [x] Slider configuration registered in SliderManager
- [x] Visibility toggling works in detection mode combo box
- [x] Initial visibility set to hidden
- [x] UI layout positioned correctly
- [x] Values saved/loaded via project system

## Next Steps (Optional Enhancements)
1. Make high-pass frequency user-configurable (currently fixed at 10 Hz)
2. Add preset button combinations for common filter configurations
3. Add frequency response visualization showing actual Hz values being used
4. Add user preference for working in Hz vs. samples

## Summary
Phase 3 successfully separates the region detection sensitivity parameter from the filter frequency parameter, improving code clarity and allowing independent tuning of these two aspects of Time Domain + Filter mode.
