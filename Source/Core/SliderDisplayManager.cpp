#include "SliderDisplayManager.h"

//==============================================================================
SliderDisplayManager::SliderDisplayManager()
{
    DBG("SliderDisplayManager: Created");
}

//==============================================================================
void SliderDisplayManager::setDisplayRange(double minValue, double maxValue)
{
    displayMin = minValue;
    displayMax = maxValue;
    
    // Update target value to maintain display consistency
    targetDisplayValue = clampDisplayValue(targetDisplayValue);
    
    // Trigger callbacks to update UI
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
    if (onTargetTextChanged)
        onTargetTextChanged(getFormattedTargetValue());
        
    DBG("SliderDisplayManager: Set display range " << minValue << " to " << maxValue);
}

//==============================================================================
void SliderDisplayManager::setOrientation(SliderOrientation newOrientation)
{
    orientation = newOrientation;
    
    // If switching to bipolar mode, set default center to middle of range
    if (orientation == SliderOrientation::Bipolar)
    {
        if (bipolarSettings.centerValue < displayMin || bipolarSettings.centerValue > displayMax)
        {
            bipolarSettings.centerValue = displayMin + ((displayMax - displayMin) / 2.0);
        }
    }
    
    // Trigger callbacks to update UI
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
    if (onTargetTextChanged)
        onTargetTextChanged(getFormattedTargetValue());
}

void SliderDisplayManager::setBipolarSettings(const BipolarSettings& settings)
{
    bipolarSettings = settings;
    // Ensure center value is within display range
    bipolarSettings.centerValue = juce::jlimit(displayMin, displayMax, bipolarSettings.centerValue);
    
    // Trigger callbacks to update UI
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
    if (onTargetTextChanged)
        onTargetTextChanged(getFormattedTargetValue());
}

SliderOrientation SliderDisplayManager::getOrientation() const
{
    return orientation;
}

BipolarSettings SliderDisplayManager::getBipolarSettings() const
{
    return bipolarSettings;
}

void SliderDisplayManager::setMidiValue(double midiValue)
{
    currentMidiValue = clampMidiValue(midiValue);
    
    // Trigger callback to update display text
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
}

void SliderDisplayManager::setDisplayValue(double displayValue)
{
    double clampedDisplayValue = clampDisplayValue(displayValue);
    currentMidiValue = displayToMidi(clampedDisplayValue);
    
    // Trigger callback to update display text
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
}

//==============================================================================
double SliderDisplayManager::getMidiValue() const
{
    return currentMidiValue;
}

double SliderDisplayManager::getDisplayValue() const
{
    return midiToDisplay(currentMidiValue);
}

double SliderDisplayManager::getDisplayMin() const
{
    return displayMin;
}

double SliderDisplayManager::getDisplayMax() const
{
    return displayMax;
}

//==============================================================================
juce::String SliderDisplayManager::getFormattedDisplayValue() const
{
    double displayValue = getDisplayValue();
    
    if (orientation == SliderOrientation::Bipolar)
    {
        double relativeValue = displayValue - bipolarSettings.centerValue;
        juce::String formattedValue = formatValue(std::abs(relativeValue));
        
        if (std::abs(relativeValue) < 0.01)
            return "0";
        else if (relativeValue > 0)
            return "+" + formattedValue;
        else
            return "-" + formattedValue;
    }
    
    return formatValue(displayValue);
}

juce::String SliderDisplayManager::getFormattedTargetValue() const
{
    if (orientation == SliderOrientation::Bipolar)
    {
        double relativeValue = targetDisplayValue - bipolarSettings.centerValue;
        juce::String formattedValue = formatValue(std::abs(relativeValue));
        
        if (std::abs(relativeValue) < 0.01)
            return "0";
        else if (relativeValue > 0)
            return "+" + formattedValue;
        else
            return "-" + formattedValue;
    }
    
    return formatValue(targetDisplayValue);
}

//==============================================================================
double SliderDisplayManager::clampDisplayValue(double value) const
{
    return juce::jlimit(displayMin, displayMax, value);
}

double SliderDisplayManager::clampMidiValue(double value) const
{
    return juce::jlimit(0.0, 16383.0, value);
}

//==============================================================================
void SliderDisplayManager::setTargetDisplayValue(double targetValue)
{
    targetDisplayValue = clampDisplayValue(targetValue);
    
    // Trigger callback to update target text
    if (onTargetTextChanged)
        onTargetTextChanged(getFormattedTargetValue());
}

double SliderDisplayManager::getTargetDisplayValue() const
{
    return targetDisplayValue;
}

double SliderDisplayManager::getTargetMidiValue() const
{
    return displayToMidi(targetDisplayValue);
}

//==============================================================================
double SliderDisplayManager::midiToDisplay(double midiValue) const
{
    // Convert MIDI value (0-16383) to display value based on custom range
    double normalized = midiValue / 16383.0;
    return displayMin + (normalized * (displayMax - displayMin));
}

double SliderDisplayManager::displayToMidi(double displayValue) const
{
    // Convert display value to MIDI value (0-16383)
    double normalized = (displayValue - displayMin) / (displayMax - displayMin);
    return juce::jlimit(0.0, 16383.0, normalized * 16383.0);
}

juce::String SliderDisplayManager::formatValue(double value) const
{
    // Format the display value nicely (identical to original SimpleSliderControl formatting)
    juce::String displayText;
    if (std::abs(value) < 0.01)
        displayText = "0";
    else if (std::abs(value - std::round(value)) < 0.01)
        displayText = juce::String((int)std::round(value));
    else
        displayText = juce::String(value, 2);
        
    return displayText;
}