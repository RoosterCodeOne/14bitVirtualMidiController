#pragma once
#include <JuceHeader.h>
#include <functional>

//==============================================================================
/**
 * Defines the display orientation modes for sliders
 */
enum class SliderOrientation
{
    Normal,    // 0 at bottom, max at top (default)
    Inverted,  // 0 at top, max at bottom
    Bipolar    // Custom center point with +/- ranges
};

/**
 * Snap threshold levels for bipolar sliders
 */
enum class SnapThreshold
{
    Small = 0,    // 1% of range
    Medium = 1,   // 2% of range
    Large = 2     // 4% of range
};

/**
 * Settings for bipolar display mode
 * 
 * IMPORTANT: Bipolar mode uses a Display-Centric approach where:
 * - Center is automatically calculated as the middle of the display range
 * - MIDI mapping remains linear across the full display range
 * - Display formatting shows values relative to auto-calculated center (±X format)
 * - Visual center line position is always at the middle of the range
 */
struct BipolarSettings
{
    bool showCenterLine = true;       // Visual center indicator
    bool snapToCenter = true;         // Enable snap-to-center feature
    SnapThreshold snapThreshold = SnapThreshold::Medium;  // Snap sensitivity
    
    BipolarSettings() = default;
    
    // Get snap threshold as percentage of range
    double getSnapThresholdPercent() const 
    {
        switch (snapThreshold)
        {
            case SnapThreshold::Small:  return 0.01;  // 1%
            case SnapThreshold::Medium: return 0.02;  // 2%
            case SnapThreshold::Large:  return 0.04;  // 4%
            default: return 0.02;
        }
    }
};

//==============================================================================
/**
 * SliderDisplayManager handles complex mapping between custom display ranges and internal 14-bit MIDI values
 * Extracted from SimpleSliderControl to provide clean separation of display logic from UI presentation
 */
class SliderDisplayManager
{
public:
    SliderDisplayManager();
    
    // Range configuration
    void setDisplayRange(double minValue, double maxValue);
    void setDisplayRangePreservingCurrentValue(double minValue, double maxValue);
    void setStepIncrement(double increment);
    
    // Orientation configuration
    void setOrientation(SliderOrientation orientation);
    void setBipolarSettings(const BipolarSettings& settings);
    SliderOrientation getOrientation() const;
    BipolarSettings getBipolarSettings() const;
    
    // Value management
    void setMidiValue(double midiValue);
    void setDisplayValue(double displayValue);
    void setDisplayValueWithSnap(double displayValue, bool allowSnap = true, bool isDragUpdate = false);
    void setMidiValueWithSnap(double midiValue, bool allowSnap = true, bool isDragUpdate = false);
    
    // Movement state management
    void setDragState(bool dragging);
    void setKeyboardNavigationMode(bool isKeyboardNav);
    void updateMovementState();
    
    // Value access
    double getMidiValue() const;
    double getDisplayValue() const;
    double getDisplayMin() const;
    double getDisplayMax() const;
    
    // Formatted output
    juce::String getFormattedDisplayValue() const;
    juce::String getFormattedTargetValue() const;
    
    // Validation and clamping
    double clampDisplayValue(double value) const;
    double clampMidiValue(double value) const;
    
    // Target value management (for automation)
    void setTargetDisplayValue(double targetValue);
    double getTargetDisplayValue() const;
    double getTargetMidiValue() const;
    
    // Conversion methods (public for quantization system)
    double midiToDisplay(double midiValue) const;
    double displayToMidi(double displayValue) const;
    
    // Bipolar methods
    double getCenterValue() const;        // Get automatically calculated center (middle of range)
    bool isInSnapZone(double displayValue) const;
    double getSnapThreshold() const;
    bool shouldSnapToCenter(double displayValue) const;
    
    // Auto-step calculation methods
    double calculateOptimalStep(double minVal, double maxVal, bool is14Bit) const;
    double getOptimalStepForCurrentRange(bool is14Bit) const;
    bool isStepCustom() const;
    void setAutoStep(bool is14Bit);
    void setCustomStep(double customStep);
    
    // Callbacks for UI updates
    std::function<void(const juce::String&)> onDisplayTextChanged;
    std::function<void(const juce::String&)> onTargetTextChanged;
    std::function<void(double)> onSnapToCenter;  // Called when snap occurs with snapped MIDI value
    
private:
    // Internal formatting methods
    juce::String formatValue(double value) const;
    int getDecimalPlaces(double value) const;
    int calculateRequiredDecimalPlaces() const;
    
    // Movement tracking helper
    void updateMovementTracking(double newValue, bool isDragUpdate);
    
    // Member variables
    double displayMin = 0.0;
    double displayMax = 16383.0;
    double currentMidiValue = 0.0;
    double targetDisplayValue = 0.0;
    double stepIncrement = 0.0; // 0 = no quantization, >0 = increment value
    bool isCustomStep = false;   // true = user-set custom step, false = auto-calculated step
    
    // Orientation settings
    SliderOrientation orientation = SliderOrientation::Normal;
    BipolarSettings bipolarSettings;
    
    // Movement state tracking
    double lastMovementTime = 0.0;
    double lastValue = 0.0;
    bool isActivelyMoving = false;
    bool isDragging = false;
    bool isKeyboardNavigation = false;
    
    // Movement detection constants
    static constexpr double MOVEMENT_SETTLE_TIME = 300.0;         // milliseconds
    static constexpr double KEYBOARD_SETTLE_TIME = 150.0;        // faster for keyboard
    static constexpr double MOVEMENT_THRESHOLD = 0.001;          // minimum change to count as movement
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderDisplayManager)
};