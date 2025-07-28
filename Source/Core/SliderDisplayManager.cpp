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

void SliderDisplayManager::setMidiValueWithSnap(double midiValue, bool allowSnap, bool isDragUpdate)
{
    double clampedMidiValue = clampMidiValue(midiValue);
    bool didSnap = false;
    
    // Update movement tracking
    updateMovementTracking(clampedMidiValue, isDragUpdate);
    
    // Convert to display value to check for snap
    if (allowSnap && !isDragUpdate)
    {
        double displayValue = midiToDisplay(clampedMidiValue);
        if (shouldSnapToCenter(displayValue))
        {
            double centerDisplayValue = getCenterValue();
            clampedMidiValue = displayToMidi(centerDisplayValue);
            didSnap = true;
            DBG("SliderDisplayManager: Snapped MIDI value to center " << clampedMidiValue);
        }
    }
    
    currentMidiValue = clampedMidiValue;
    
    // Trigger callback to update display text
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
        
    // Notify about snap event if it occurred
    if (didSnap && onSnapToCenter)
        onSnapToCenter(currentMidiValue);
}

void SliderDisplayManager::setDisplayValue(double displayValue)
{
    double clampedDisplayValue = clampDisplayValue(displayValue);
    currentMidiValue = displayToMidi(clampedDisplayValue);
    
    // Trigger callback to update display text
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
}

void SliderDisplayManager::setDisplayValueWithSnap(double displayValue, bool allowSnap, bool isDragUpdate)
{
    double clampedDisplayValue = clampDisplayValue(displayValue);
    bool didSnap = false;
    
    // Convert to MIDI value for movement tracking
    double midiValue = displayToMidi(clampedDisplayValue);
    updateMovementTracking(midiValue, isDragUpdate);
    
    // Apply bipolar snap if conditions are met
    if (allowSnap && !isDragUpdate && shouldSnapToCenter(clampedDisplayValue))
    {
        clampedDisplayValue = getCenterValue();
        didSnap = true;
        DBG("SliderDisplayManager: Snapped to center value " << clampedDisplayValue);
    }
    
    currentMidiValue = displayToMidi(clampedDisplayValue);
    
    // Trigger callback to update display text
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
        
    // Notify about snap event if it occurred
    if (didSnap && onSnapToCenter)
        onSnapToCenter(currentMidiValue);
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
        
        // Use calculated precision for zero threshold
        int decimalPlaces = calculateRequiredDecimalPlaces();
        double zeroThreshold = decimalPlaces == 0 ? 0.5 : (0.5 * std::pow(10.0, -decimalPlaces));
        
        if (std::abs(relativeValue) < zeroThreshold)
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
        
        // Use calculated precision for zero threshold
        int decimalPlaces = calculateRequiredDecimalPlaces();
        double zeroThreshold = decimalPlaces == 0 ? 0.5 : (0.5 * std::pow(10.0, -decimalPlaces));
        
        if (std::abs(relativeValue) < zeroThreshold)
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
    // Use smart decimal formatting based on range and increment
    int decimalPlaces = calculateRequiredDecimalPlaces();
    
    // Calculate appropriate zero threshold based on precision
    double zeroThreshold = decimalPlaces == 0 ? 0.5 : (0.5 * std::pow(10.0, -decimalPlaces));
    
    // Handle values very close to zero
    if (std::abs(value) < zeroThreshold)
        return "0";
    
    if (decimalPlaces == 0)
    {
        // Show as integer when no decimals are needed
        return juce::String((int)std::round(value));
    }
    else
    {
        // Show with calculated decimal places
        return juce::String(value, decimalPlaces);
    }
}

int SliderDisplayManager::getDecimalPlaces(double value) const
{
    // Handle special cases
    if (std::abs(value) < 0.000001) // Very close to zero
        return 0;
    
    // Convert to string with high precision to count significant decimal places
    juce::String valueStr(value, 10);
    int dotPos = valueStr.indexOfChar('.');
    
    if (dotPos < 0)
        return 0; // No decimal point found
    
    // Count significant decimal places (ignore trailing zeros)
    int significantDecimals = 0;
    for (int i = valueStr.length() - 1; i > dotPos; --i)
    {
        if (valueStr[i] != '0')
        {
            significantDecimals = i - dotPos;
            break;
        }
    }
    
    return significantDecimals;
}

int SliderDisplayManager::calculateRequiredDecimalPlaces() const
{
    // Start with range-based precision requirements
    int rangePrecision = juce::jmax(
        getDecimalPlaces(displayMin),
        getDecimalPlaces(displayMax)
    );
    
    // Check step increment precision
    int stepPrecision = 0;
    if (stepIncrement > 0.0)
    {
        stepPrecision = getDecimalPlaces(stepIncrement);
        
        // For very small auto-calculated steps, provide reasonable precision
        if (!isCustomStep && stepIncrement < 0.01)
        {
            // Calculate precision needed to show meaningful step differences
            // For step like 0.00006, we want at least 4-5 decimal places
            double log10Step = std::log10(stepIncrement);
            if (log10Step < 0)
            {
                stepPrecision = juce::jmax(stepPrecision, (int)std::ceil(-log10Step) + 1);
            }
        }
    }
    
    // Use the higher precision requirement
    int requiredDecimals = juce::jmax(rangePrecision, stepPrecision);
    
    // Special case: if range is like 0.0-1.0, ensure we show enough precision
    double range = std::abs(displayMax - displayMin);
    if (range <= 10.0 && range > 0.0)
    {
        // For small ranges, ensure we have enough precision to show meaningful values
        double log10Range = std::log10(range);
        if (log10Range < 0)
        {
            int rangeBasedDecimals = (int)std::ceil(-log10Range) + 1;
            requiredDecimals = juce::jmax(requiredDecimals, rangeBasedDecimals);
        }
    }
    
    // Cap at reasonable maximum to prevent excessive precision
    return juce::jlimit(0, 4, requiredDecimals);
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

bool SliderDisplayManager::shouldSnapToCenter(double displayValue) const
{
    // Check all snap conditions
    if (orientation != SliderOrientation::Bipolar)
        return false;
        
    if (!bipolarSettings.snapToCenter)
        return false;
        
    // Never snap during active drag
    if (isDragging)
        return false;
    
    // Check if movement has settled
    double currentTime = juce::Time::getMillisecondCounterHiRes();
    double timeSinceMovement = currentTime - lastMovementTime;
    double settleTime = isKeyboardNavigation ? KEYBOARD_SETTLE_TIME : MOVEMENT_SETTLE_TIME;
    
    // Only snap if movement has settled
    if (timeSinceMovement < settleTime)
        return false;
        
    if (isActivelyMoving)
        return false;
    
    // Use existing snap zone logic
    return isInSnapZone(displayValue);
}

void SliderDisplayManager::setDragState(bool dragging)
{
    isDragging = dragging;
    
    if (dragging)
    {
        // Starting drag - reset movement tracking
        lastMovementTime = juce::Time::getMillisecondCounterHiRes();
        isActivelyMoving = true;
    }
    
    DBG("SliderDisplayManager: Drag state set to " << (dragging ? "true" : "false"));
}

void SliderDisplayManager::setKeyboardNavigationMode(bool isKeyboardNav)
{
    isKeyboardNavigation = isKeyboardNav;
    
    if (isKeyboardNav)
    {
        // Starting keyboard navigation - reset movement tracking
        lastMovementTime = juce::Time::getMillisecondCounterHiRes();
        isActivelyMoving = true;
    }
    
    DBG("SliderDisplayManager: Keyboard navigation mode set to " << (isKeyboardNav ? "true" : "false"));
}

void SliderDisplayManager::updateMovementState()
{
    double currentTime = juce::Time::getMillisecondCounterHiRes();
    double timeSinceMovement = currentTime - lastMovementTime;
    double settleTime = isKeyboardNavigation ? KEYBOARD_SETTLE_TIME : MOVEMENT_SETTLE_TIME;
    
    // Update movement state based on time since last movement
    if (timeSinceMovement > settleTime)
    {
        isActivelyMoving = false;
    }
}

void SliderDisplayManager::updateMovementTracking(double newValue, bool isDragUpdate)
{
    double currentTime = juce::Time::getMillisecondCounterHiRes();
    double valueDelta = std::abs(newValue - lastValue);
    
    // Update movement tracking if significant change detected
    if (valueDelta > MOVEMENT_THRESHOLD)
    {
        lastMovementTime = currentTime;
        isActivelyMoving = true;
        lastValue = newValue;
        
        DBG("SliderDisplayManager: Movement detected, delta=" << valueDelta);
    }
    else
    {
        // Check if enough time has passed to consider movement settled
        double timeSinceMovement = currentTime - lastMovementTime;
        double settleTime = isKeyboardNavigation ? KEYBOARD_SETTLE_TIME : MOVEMENT_SETTLE_TIME;
        
        if (timeSinceMovement > settleTime)
        {
            isActivelyMoving = false;
        }
    }
}

//==============================================================================
// Auto-step calculation methods
double SliderDisplayManager::calculateOptimalStep(double minVal, double maxVal, bool is14Bit) const
{
    // Calculate step size to provide full resolution across the range
    int numSteps = is14Bit ? 16384 : 128;
    double range = std::abs(maxVal - minVal);
    
    // Handle edge cases
    if (range < 0.000001) // Essentially zero range
        return 0.0; // No quantization for zero range
    
    // Calculate optimal step to utilize full resolution
    double optimalStep = range / (numSteps - 1); // -1 because we want numSteps discrete positions
    
    // Ensure step is positive and reasonable
    return juce::jmax(0.000001, optimalStep);
}

double SliderDisplayManager::getOptimalStepForCurrentRange(bool is14Bit) const
{
    return calculateOptimalStep(displayMin, displayMax, is14Bit);
}

bool SliderDisplayManager::isStepCustom() const
{
    return isCustomStep;
}

void SliderDisplayManager::setAutoStep(bool is14Bit)
{
    isCustomStep = false;
    stepIncrement = getOptimalStepForCurrentRange(is14Bit);
    
    // Trigger callbacks to update UI with new formatting
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
    if (onTargetTextChanged)
        onTargetTextChanged(getFormattedTargetValue());
        
    DBG("SliderDisplayManager: Set auto step " << stepIncrement << " for " << (is14Bit ? "14-bit" : "7-bit") << " mode, range " << displayMin << "-" << displayMax);
}

void SliderDisplayManager::setCustomStep(double customStep)
{
    isCustomStep = true;
    stepIncrement = juce::jmax(0.0, customStep);
    
    // Trigger callbacks to update UI with new formatting
    if (onDisplayTextChanged)
        onDisplayTextChanged(getFormattedDisplayValue());
    if (onTargetTextChanged)
        onTargetTextChanged(getFormattedTargetValue());
        
    DBG("SliderDisplayManager: Set custom step " << stepIncrement);
}