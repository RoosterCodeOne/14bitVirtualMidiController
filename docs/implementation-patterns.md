# Implementation Patterns - 14-bit Virtual MIDI Controller

## Scale-Aware Development Patterns

### ✅ Correct Scaling Pattern
```cpp
// Use GlobalUIScale for all dimensions
auto& scale = GlobalUIScale::getInstance();
int width = scale.getScaled(490);  // Scales 490px based on current factor

// Use MainControllerLayout Constants for layout values
int panelWidth = MainControllerLayout::Constants::getSettingsPanelWidth(); // Already scaled
int sliderWidth = MainControllerLayout::Constants::getSliderPlateWidth();  // Already scaled
```

### ❌ Incorrect Patterns to Avoid
```cpp
// Never hardcode dimensions
int width = 490;  // Won't scale!
int panelWidth = 350;  // Won't scale!

// Don't double-scale
int width = scale.getScaled(MainControllerLayout::Constants::getSliderPlateWidth()); // Already scaled!
```

### Window Width Calculation Pattern
```cpp
// Reference implementation from scaleFactorChanged() - USE THIS PATTERN
int contentWidth = isEightSliderMode ? scale.getScaled(970) : scale.getScaled(490);
int settingsWidth = (isInSettingsMode || isInLearnMode) ? 
    MainControllerLayout::Constants::getSettingsPanelWidth() : 0;
int targetWidth = contentWidth + settingsWidth;
```

## MIDI Value Patterns

### Internal MIDI (0-16383) to Display Mapping
```cpp
// Use SliderDisplayManager for value conversion
class SliderDisplayManager {
    double midiToDisplay(double midiValue) const;     // 0-16383 → display range
    double displayToMidi(double displayValue) const;  // display range → 0-16383
};

// Example: MIDI 8192 (center) maps to display center regardless of range
// Range 0-100: MIDI 8192 → Display 50
// Range -50 to 50: MIDI 8192 → Display 0 (center)
```

### 14-bit MIDI Output
```cpp
// Automatic MSB/LSB splitting for 14-bit resolution
void send14BitCC(int channel, int ccNumber, int value14Bit) {
    int msb = (value14Bit >> 7) & 0x7F;     // Upper 7 bits
    int lsb = value14Bit & 0x7F;            // Lower 7 bits
    sendMidi(channel, ccNumber, msb);        // CC# for MSB
    sendMidi(channel, ccNumber + 32, lsb);   // CC#+32 for LSB
}
```

## Component Communication Patterns

### Callback-Based Communication (✅ Preferred)
```cpp
// Parent component provides callbacks to child components
slider->setValueChangeCallback([this](double newValue) {
    handleSliderChange(newValue);
});

slider->setAutomationCallback([this](bool isActive) {
    updateAutomationState(isActive);
});
```

### Direct Reference Anti-Pattern (❌ Avoid)
```cpp
// Don't pass direct references between components
slider->setParentController(this);  // Creates tight coupling
```

## Visual-Functional Split Pattern

### JUCE Component Interaction Zones
```cpp
// Invisible JUCE slider handles interaction
juce::Slider functionalSlider;
functionalSlider.setBounds(interactionArea);  // Handles mouse events
functionalSlider.setAlpha(0.0f);              // Invisible

// Custom rendering in parent component
void paint(Graphics& g) override {
    // Custom slider track and thumb rendering
    drawCustomSliderTrack(g, bounds);
    drawCustomSliderThumb(g, thumbPosition);
}
```

## State Management Patterns

### Boolean Flags + Callbacks
```cpp
// State flags
bool isInSettingsMode = false;
bool isInLearnMode = false;

// Mode switching with callbacks
void toggleSettingsWindow() {
    if (isInLearnMode && onLearnModeExit) {
        onLearnModeExit();  // Clean up learn mode first
    }
    isInSettingsMode = !isInSettingsMode;
    // Update UI and constraints...
}
```

### Bank Management Pattern
```cpp
class BankManager {
    int activeBank = 0;
    bool eightSliderMode = false;
    
    std::function<void(int)> onBankChanged;
    std::function<void(bool)> onModeChanged;
    
    void setActiveBank(int bank) {
        activeBank = bank;
        if (onBankChanged) onBankChanged(bank);
    }
};
```

## Automation Patterns

### 3-Phase Automation Structure
```cpp
struct AutomationPhase {
    float duration;      // In seconds or beats
    float targetValue;   // 0.0 - 1.0 normalized
    CurveType curve;     // Linear, Exponential, Logarithmic
};

struct AutomationConfig {
    AutomationPhase delay;    // Hold at current value
    AutomationPhase attack;   // Move to target
    AutomationPhase return_;  // Return to original (optional)
};
```

### Time Mode Handling
```cpp
// Support both time-based and beat-based automation
enum class TimeMode { Seconds, Beats };

float calculatePhaseDuration(const AutomationPhase& phase, TimeMode mode, float bpm) {
    if (mode == TimeMode::Seconds) {
        return phase.duration;
    } else {
        return (phase.duration * 60.0f) / bpm;  // Beats to seconds
    }
}
```

## Error Handling Patterns

### Graceful Degradation
```cpp
// Always provide fallbacks for scaling
float getScaledValue(float baseValue) {
    if (auto* scaleInstance = GlobalUIScale::getInstanceSafe()) {
        return scaleInstance->getScaled(baseValue);
    }
    return baseValue;  // Fallback to unscaled
}
```

### MIDI Device Connection
```cpp
// Handle device disconnection gracefully
bool sendMidiMessage(const MidiMessage& message) {
    if (midiOutput && midiOutput->isValid()) {
        midiOutput->sendMessageNow(message);
        return true;
    }
    // Queue message for when device reconnects
    pendingMessages.push_back(message);
    return false;
}
```