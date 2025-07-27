#include "SliderDisplayManager.h"

//==============================================================================
SliderDisplayManager::SliderDisplayManager()
{
    DBG("SliderDisplayManager: Created");
}

//==============================================================================
void SliderDisplayManager::setDisplayRange(double minValue, double maxValue)
{
    // Preserve relative position of target value when range changes
    double oldRange = displayMax - displayMin;
    double relativeTargetPosition = 0.5; // Default to middle if no valid old range
    
    if (std::abs(oldRange) > 0.001) // Avoid division by zero
    {
        relativeTargetPosition = (targetDisplayValue - displayMin) / oldRange;
    }
    
    // Update range
    displayMin = minValue;
    displayMax = maxValue;
    
    // Restore target value at same relative position in new range
    double newRange = displayMax - displayMin;
    targetDisplayValue = displayMin + (relativeTargetPosition * newRange);
    
    // Clamp to new range to handle edge cases
    targetDisplayValue = clampDisplayValue(targetDisplayValue);
    
    // Trigger callbacks to update UI
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
    if (onTargetTextChanged)
        onTargetTextChanged(getFormattedTargetValue());
        
    DBG("SliderDisplayManager: Set display range " << minValue << " to " << maxValue << ", preserved relative position " << relativeTargetPosition);
}

void SliderDisplayManager::setDisplayRangePreservingCurrentValue(double minValue, double maxValue)
{
    // Preserve relative position of BOTH current and target values when range changes
    double oldRange = displayMax - displayMin;
    double relativeCurrentPosition = 0.5; // Default to middle if no valid old range
    double relativeTargetPosition = 0.5;
    
    if (std::abs(oldRange) > 0.001) // Avoid division by zero
    {
        double currentDisplayValue = getDisplayValue();
        relativeCurrentPosition = (currentDisplayValue - displayMin) / oldRange;
        relativeTargetPosition = (targetDisplayValue - displayMin) / oldRange;
    }
    
    // Update range
    displayMin = minValue;
    displayMax = maxValue;
    
    // Restore values at same relative positions in new range
    double newRange = displayMax - displayMin;
    
    // Update current value to maintain relative position
    double newCurrentDisplayValue = displayMin + (relativeCurrentPosition * newRange);
    double newCurrentMidiValue = displayToMidi(newCurrentDisplayValue);
    currentMidiValue = clampMidiValue(newCurrentMidiValue);
    
    // Update target value to maintain relative position
    targetDisplayValue = displayMin + (relativeTargetPosition * newRange);
    targetDisplayValue = clampDisplayValue(targetDisplayValue);
    
    // Trigger callbacks to update UI
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
    if (onTargetTextChanged)
        onTargetTextChanged(getFormattedTargetValue());
        
    DBG("SliderDisplayManager: Set display range preserving current value " << minValue << " to " << maxValue << ", preserved positions " << relativeCurrentPosition << ", " << relativeTargetPosition);
}

void SliderDisplayManager::setStepIncrement(double increment)
{
    stepIncrement = juce::jmax(0.0, increment);
    
    // Trigger callbacks to update UI with new formatting
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
    if (onTargetTextChanged)
        onTargetTextChanged(getFormattedTargetValue());
        
    DBG("SliderDisplayManager: Set step increment " << increment);
}

//==============================================================================
void SliderDisplayManager::setOrientation(SliderOrientation newOrientation)
{
    orientation = newOrientation;
    
    // No manual center value setting needed - it's automatically calculated
    
    // Trigger callbacks to update UI
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
    if (onTargetTextChanged)
        onTargetTextChanged(getFormattedTargetValue());
}

void SliderDisplayManager::setBipolarSettings(const BipolarSettings& settings)
{
    bipolarSettings = settings;
    // No center value validation needed - it's automatically calculated
    
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
        // Bipolar display: show values relative to center (±X format)
        // This is purely for display - does NOT affect MIDI output mapping
        double relativeValue = displayValue - getCenterValue();
        juce::String formattedValue = formatValue(std::abs(relativeValue));
        
        if (std::abs(relativeValue) < 0.01)
            return "0";  // At center
        else if (relativeValue > 0)
            return "+" + formattedValue;  // Above center
        else
            return "-" + formattedValue;  // Below center
    }
    
    return formatValue(displayValue);
}

juce::String SliderDisplayManager::getFormattedTargetValue() const
{
    if (orientation == SliderOrientation::Bipolar)
    {
        double relativeValue = targetDisplayValue - getCenterValue();
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
    // NOTE: This conversion is independent of bipolar centerValue - linear mapping across full range
    double normalized = midiValue / 16383.0;
    return displayMin + (normalized * (displayMax - displayMin));
}

double SliderDisplayManager::displayToMidi(double displayValue) const
{
    // Convert display value to MIDI value (0-16383)
    // NOTE: This conversion is independent of bipolar centerValue - linear mapping across full range
    // For bipolar mode: centerValue affects display formatting only, not MIDI output
    double normalized = (displayValue - displayMin) / (displayMax - displayMin);
    return juce::jlimit(0.0, 16383.0, normalized * 16383.0);
}

juce::String SliderDisplayManager::formatValue(double value) const
{
    // Handle zero specially
    if (std::abs(value) < 0.01)
        return "0";
    
    // Use smart decimal formatting based on range and increment
    int decimalPlaces = calculateRequiredDecimalPlaces();
    
    if (decimalPlaces == 0)
    {
        // Show as integer
        return juce::String((int)std::round(value));
    }
    else
    {
        // Show with calculated decimal places
        return juce::String(value, decimalPlaces);
    }
}

int SliderDisplayManager::calculateRequiredDecimalPlaces() const
{
    // Start with default of 0 decimal places (integers)
    int requiredDecimals = 0;
    
    // Check if min/max values require decimals
    bool minIsWhole = (std::abs(displayMin - std::round(displayMin)) < 0.001);
    bool maxIsWhole = (std::abs(displayMax - std::round(displayMax)) < 0.001);
    
    if (!minIsWhole || !maxIsWhole)
    {
        // At least one endpoint has decimals - determine how many
        double minDecimals = displayMin - std::floor(displayMin);
        double maxDecimals = displayMax - std::floor(displayMax);
        
        // Count decimal places needed for min value
        if (!minIsWhole)
        {
            juce::String minStr(displayMin, 10); // High precision string
            int minDecimalCount = 0;
            int dotPos = minStr.indexOfChar('.');
            if (dotPos >= 0)
            {
                // Count significant decimal places (ignore trailing zeros)
                for (int i = minStr.length() - 1; i > dotPos; --i)
                {
                    if (minStr[i] != '0')
                    {
                        minDecimalCount = i - dotPos;
                        break;
                    }
                }
            }
            requiredDecimals = juce::jmax(requiredDecimals, minDecimalCount);
        }
        
        // Count decimal places needed for max value
        if (!maxIsWhole)
        {
            juce::String maxStr(displayMax, 10); // High precision string
            int maxDecimalCount = 0;
            int dotPos = maxStr.indexOfChar('.');
            if (dotPos >= 0)
            {
                // Count significant decimal places (ignore trailing zeros)
                for (int i = maxStr.length() - 1; i > dotPos; --i)
                {
                    if (maxStr[i] != '0')
                    {
                        maxDecimalCount = i - dotPos;
                        break;
                    }
                }
            }
            requiredDecimals = juce::jmax(requiredDecimals, maxDecimalCount);
        }
    }
    
    // Check step increment if it's enabled
    if (stepIncrement > 0.0)
    {
        bool incrementIsWhole = (std::abs(stepIncrement - std::round(stepIncrement)) < 0.001);
        
        if (!incrementIsWhole)
        {
            // Count decimal places needed for step increment
            juce::String incStr(stepIncrement, 10); // High precision string
            int incDecimalCount = 0;
            int dotPos = incStr.indexOfChar('.');
            if (dotPos >= 0)
            {
                // Count significant decimal places (ignore trailing zeros)
                for (int i = incStr.length() - 1; i > dotPos; --i)
                {
                    if (incStr[i] != '0')
                    {
                        incDecimalCount = i - dotPos;
                        break;
                    }
                }
            }
            requiredDecimals = juce::jmax(requiredDecimals, incDecimalCount);
        }
        else if (requiredDecimals == 0)
        {
            // If step is whole number and range suggests integers, stay with integers
            requiredDecimals = 0;
        }
    }
    
    // Cap at reasonable maximum
    return juce::jlimit(0, 3, requiredDecimals);
}

//==============================================================================
bool SliderDisplayManager::isInSnapZone(double displayValue) const
{
    if (orientation != SliderOrientation::Bipolar || !bipolarSettings.snapToCenter)
        return false;
        
    double threshold = getSnapThreshold();
    return std::abs(displayValue - getCenterValue()) <= threshold;
}

double SliderDisplayManager::getSnapThreshold() const
{
    if (orientation != SliderOrientation::Bipolar)
        return 0.0;
        
    double displayRange = std::abs(displayMax - displayMin);
    return displayRange * bipolarSettings.getSnapThresholdPercent();
}

double SliderDisplayManager::getCenterValue() const
{
    // Always return the middle of the display range
    return displayMin + ((displayMax - displayMin) / 2.0);
}