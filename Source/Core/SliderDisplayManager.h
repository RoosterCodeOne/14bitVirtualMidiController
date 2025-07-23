#pragma once
#include <JuceHeader.h>
#include <functional>

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
    
    // Value management
    void setMidiValue(double midiValue);
    void setDisplayValue(double displayValue);
    
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
    
    // Callbacks for UI updates
    std::function<void(const juce::String&)> onDisplayTextChanged;
    std::function<void(const juce::String&)> onTargetTextChanged;
    
private:
    // Internal formatting method
    juce::String formatValue(double value) const;
    
    // Member variables
    double displayMin = 0.0;
    double displayMax = 16383.0;
    double currentMidiValue = 0.0;
    double targetDisplayValue = 0.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderDisplayManager)
};