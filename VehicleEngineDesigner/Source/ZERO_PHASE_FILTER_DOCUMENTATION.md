# ZeroPhaseFilter Class Documentation

## Overview

`ZeroPhaseFilter` is a reusable utility class that performs zero-phase (bidirectional) filtering on audio buffers. It applies a biquad filter in both forward and backward directions to achieve zero phase shift while maintaining the magnitude response.

This is particularly useful for offline audio processing where phase distortion must be minimized, such as in region detection and filtering operations.

## Features

✅ **Zero Phase Shift** - Bidirectional filtering eliminates phase distortion
✅ **Reusable** - Single filter applied twice (forward + backward)
✅ **Multiple Filter Types** - High-pass, low-pass, and band-pass filtering
✅ **Multi-Channel Support** - Works with multi-channel audio buffers
✅ **In-Place Processing** - Modifies buffers directly without extra copies

## API

### Constructor

```cpp
ZeroPhaseFilter(double sampleRate)
```

Creates a zero-phase filter instance.

**Parameters:**
- `sampleRate` - Sample rate in Hz (e.g., 48000.0)

**Example:**
```cpp
ZeroPhaseFilter filter(48000.0);
```

### High-Pass Filtering

```cpp
void applyHighPassZeroPhase(juce::AudioBuffer<float>& buffer, 
                            float frequency, 
                            float resonance)
```

Applies zero-phase high-pass filtering to remove low frequencies.

**Parameters:**
- `buffer` - Audio buffer to filter (modified in-place)
- `frequency` - Filter frequency in Hz (e.g., 100.0)
- `resonance` - Q factor / resonance (e.g., 2.0)

**Example:**
```cpp
filter.applyHighPassZeroPhase(buffer, 100.0f, 2.0f);
```

### Low-Pass Filtering

```cpp
void applyLowPassZeroPhase(juce::AudioBuffer<float>& buffer, 
                           float frequency, 
                           float resonance)
```

Applies zero-phase low-pass filtering to remove high frequencies.

**Parameters:**
- `buffer` - Audio buffer to filter (modified in-place)
- `frequency` - Filter frequency in Hz (e.g., 10000.0)
- `resonance` - Q factor / resonance (e.g., 2.0)

**Example:**
```cpp
filter.applyLowPassZeroPhase(buffer, 10000.0f, 2.0f);
```

### Band-Pass Filtering

```cpp
void applyBandPassZeroPhase(juce::AudioBuffer<float>& buffer, 
                            float lowFrequency, 
                            float highFrequency, 
                            float resonance)
```

Applies zero-phase band-pass filtering by sequentially applying high-pass and low-pass filters.

**Parameters:**
- `buffer` - Audio buffer to filter (modified in-place)
- `lowFrequency` - Low cut frequency in Hz (e.g., 100.0)
- `highFrequency` - High cut frequency in Hz (e.g., 10000.0)
- `resonance` - Q factor for both filters (e.g., 2.0)

**Example:**
```cpp
filter.applyBandPassZeroPhase(buffer, 100.0f, 10000.0f, 2.0f);
```

### Custom Filter Configuration

```cpp
void applyZeroPhaseFilter(juce::AudioBuffer<float>& buffer, 
                          BiquadFilter& filter)
```

Applies zero-phase filtering using a pre-configured `BiquadFilter`.

**Parameters:**
- `buffer` - Audio buffer to filter (modified in-place)
- `filter` - Pre-configured BiquadFilter instance

**Example:**
```cpp
BiquadFilter customFilter;
customFilter.init(48000.0);
customFilter.setHighPass(500.0f, 2.0f);

filter.applyZeroPhaseFilter(buffer, customFilter);
```

## How It Works

The zero-phase filtering process consists of:

1. **Forward Pass** - Applies the filter to the audio buffer from start to end
2. **Reverse** - Reverses the buffer samples
3. **Backward Pass** - Applies the filter to the reversed buffer
4. **Reverse Back** - Reverses the buffer back to original order

This bidirectional processing eliminates phase shift while doubling the filter's magnitude response (which is then squared, resulting in a steeper filter slope).

## Usage Examples

### Example 1: Simple High-Pass Filter

```cpp
// Create a 48kHz sample rate filter
ZeroPhaseFilter filter(48000.0);

// Load audio buffer...
juce::AudioBuffer<float> audioBuffer;

// Remove frequencies below 100 Hz
filter.applyHighPassZeroPhase(audioBuffer, 100.0f, 2.0f);
```

### Example 2: Band-Pass Filtering (Multiple Filters)

```cpp
ZeroPhaseFilter filter(m_sampleRate);

// Extract frequency band between 100 Hz and 10 kHz
filter.applyBandPassZeroPhase(tempBuffer, 100.0f, 10000.0f, 2.0f);
```

### Example 3: Custom Filter with Pre-configuration

```cpp
ZeroPhaseFilter filter(m_sampleRate);
BiquadFilter customFilter;

customFilter.init(m_sampleRate);
customFilter.setLowPass(5000.0f, 1.5f);

// Apply custom low-pass as zero-phase
filter.applyZeroPhaseFilter(audioBuffer, customFilter);
```

## Integration with MainComponent

The `ZeroPhaseFilter` class replaces the manual filtering code in `detectRegionsButtonClicked()`:

### Before (60+ lines of boilerplate):
```cpp
BiquadFilter filterHP, filterLP;
filterHP.init(m_sampleRate);
filterLP.init(m_sampleRate);
filterHP.setHighPass(highPassFrequency, 2.0f);
filterLP.setLowPass(lowPassFrequency, 2.0f);

// Forward pass
for (int channel = 0; channel < tempBuffer.getNumChannels(); ++channel) {
    // ... manual processing ...
}

// ... 50+ more lines of reversing and backward pass ...
```

### After (1 line):
```cpp
ZeroPhaseFilter zeroPhaseFilter(m_sampleRate);
zeroPhaseFilter.applyBandPassZeroPhase(tempBuffer, highPassFrequency, lowPassFrequency, 2.0f);
```

## Performance Considerations

- **Time Complexity**: O(n) where n is the number of samples
  - Two passes through the buffer (forward + backward)
  - Reversal operations are linear
  
- **Space Complexity**: O(1) - In-place processing, no extra buffer allocation

- **Filter State**: Filter state is reset between channels to prevent cross-channel contamination

## Design Benefits

✅ **Reusability** - Use anywhere in the codebase that needs zero-phase filtering
✅ **Maintainability** - Centralized, well-documented filtering logic
✅ **Testability** - Easy to test with different filter parameters
✅ **Extensibility** - Can be extended for other filter types if needed
✅ **Single Responsibility** - Focuses solely on zero-phase filtering

## Integration Steps

1. Include the header:
   ```cpp
   #include "ZeroPhaseFilter.h"
   ```

2. Create an instance with sample rate:
   ```cpp
   ZeroPhaseFilter filter(m_sampleRate);
   ```

3. Apply filtering:
   ```cpp
   filter.applyBandPassZeroPhase(buffer, lowFreq, highFreq, resonance);
   ```

## Related Files

- **ZeroPhaseFilter.h** - Class definition and implementation
- **MainComponent.h** - Updated to use ZeroPhaseFilter in detectRegionsButtonClicked()
- **BiquadFilters.h** - Underlying filter implementation

## Future Enhancements

Potential improvements:
- Support for other filter types (shelf, notch, etc.)
- Batch filter application for multiple parameters
- Filter preset management
- Performance profiling for very long buffers

## Testing

The class has been integrated and tested in the VehicleEngineDesigner application's region detection workflow where it successfully:
- ✅ Filters audio for zero-crossing detection
- ✅ Maintains phase integrity
- ✅ Processes multi-channel audio
- ✅ Compiles and builds successfully

---

**Summary**: `ZeroPhaseFilter` provides a clean, reusable interface for zero-phase filtering, eliminating 60+ lines of repetitive boilerplate code while improving maintainability and code clarity.
