#pragma once
#include <JuceHeader.h>
#include <vector>
#include <functional>

//==============================================================================
/**
 * KeyboardController handles all QWERTY keyboard-based slider control
 * Extracted from DebugMidiController to provide clean separation of concerns
 */
class KeyboardController : public juce::Timer
{
public:
    KeyboardController();
    ~KeyboardController();
    
    // Initialization and configuration
    void initialize();
    void setSliderMode(bool isEightSliderMode);
    void setCurrentBank(int bankIndex);
    
    // Key event handling
    bool handleKeyPressed(const juce::KeyPress& key);
    bool handleKeyStateChanged(bool isKeyDown);
    
    // Movement rate control
    void adjustMovementRate(bool increase); // Z/X key handling
    juce::String getSpeedDisplayText() const;
    
    // State queries
    bool isTextInputActive() const;
    
    // Callbacks for parent components
    std::function<void(int sliderIndex, double newValue)> onSliderValueChanged;
    std::function<void(const juce::String&)> onSpeedDisplayChanged;
    std::function<bool(int sliderIndex)> isSliderLocked; // Check if slider is locked
    std::function<double(int sliderIndex)> getSliderValue; // Get current slider value
    std::function<int(int keyboardPosition)> getVisibleSliderIndex; // Map keyboard position to slider index
    
private:
    // Timer callback for continuous movement
    void timerCallback() override;
    
    // Internal state management
    void updateSpeedDisplay();
    void processKeyboardMovement();
    
    // Keyboard mapping structure
    struct KeyboardMapping
    {
        int upKey;
        int downKey;
        bool isPressed = false;
        bool isUpDirection = false;
        double accumulatedMovement = 0.0; // For fractional movement accumulation
        int currentSliderIndex = 0; // Maps to currently visible slider
    };
    
    // Member variables
    std::vector<KeyboardMapping> keyboardMappings;
    std::vector<int> movementRates;
    int currentRateIndex = 2;
    double keyboardMovementRate = 50.0; // MIDI units per second
    
    // Configuration state
    bool isEightSliderMode = false;
    int currentBank = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyboardController)
};