#include "KeyboardController.h"

//==============================================================================
KeyboardController::KeyboardController()
{
    DBG("KeyboardController: Created");
}

KeyboardController::~KeyboardController()
{
    // Stop timer before destruction
    stopTimer();
    DBG("KeyboardController: Destroyed");
}

//==============================================================================
void KeyboardController::initialize()
{
    keyboardMappings.clear();
    
    // Q/A for visible slider 1, W/S for visible slider 2, E/D for visible slider 3, R/F for visible slider 4
    keyboardMappings.push_back({'Q', 'A'});
    keyboardMappings.push_back({'W', 'S'});
    keyboardMappings.push_back({'E', 'D'});
    keyboardMappings.push_back({'R', 'F'});
    
    // Keep the other mappings for future use when 8 sliders are visible
    keyboardMappings.push_back({'U', 'J'});
    keyboardMappings.push_back({'I', 'K'});
    keyboardMappings.push_back({'O', 'L'});
    keyboardMappings.push_back({'P', ';'});
    
    // Initialize discrete movement rates (last one is special: -1 = instant/100%)
    movementRates = {1, 5, 50, 100, 250, 500, 1000, 2500, 5000, 10000, -1};
    currentRateIndex = 2; // Start with 50 units/sec
    keyboardMovementRate = movementRates[currentRateIndex];
    
    updateSpeedDisplay();
}

void KeyboardController::setSliderMode(bool eightSliderMode)
{
    isEightSliderMode = eightSliderMode;
}

void KeyboardController::setCurrentBank(int bankIndex)
{
    currentBank = bankIndex;
}

//==============================================================================
bool KeyboardController::handleKeyPressed(const juce::KeyPress& key)
{
    // Allow system shortcuts when modifier keys are held
    if (key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown() || 
        key.getModifiers().isAltDown())
    {
        return false; // Let the system handle Command+Q, Ctrl+C, etc.
    }
    
    // Don't interfere when any text editor has focus
    if (isTextInputActive())
    {
        return false;
    }
    
    auto keyChar = key.getKeyCode();
    
    // Handle movement rate adjustment (Z/X keys) - discrete rates
    if (keyChar == 'Z' || keyChar == 'z')
    {
        adjustMovementRate(false); // Decrease speed
        return true;
    }
    if (keyChar == 'X' || keyChar == 'x')
    {
        adjustMovementRate(true); // Increase speed
        return true;
    }
    
    // Handle slider control keys - map to currently visible sliders
    int maxMappings = isEightSliderMode ? 8 : 4;
    for (int i = 0; i < keyboardMappings.size() && i < maxMappings; ++i)
    {
        auto& mapping = keyboardMappings[i];
        if (keyChar == mapping.upKey || keyChar == mapping.downKey)
        {
            if (!mapping.isPressed)
            {
                mapping.isPressed = true;
                mapping.isUpDirection = (keyChar == mapping.upKey);
                mapping.accumulatedMovement = 0.0; // Reset accumulator
                
                // Map to visible slider using callback
                if (getVisibleSliderIndex)
                    mapping.currentSliderIndex = getVisibleSliderIndex(i);
                else
                    mapping.currentSliderIndex = i; // Fallback
                
                // Start timer for smooth movement if not already running
                if (!isTimerRunning())
                    startTimer(16); // ~60fps
            }
            return true;
        }
    }
    
    return false;
}

bool KeyboardController::handleKeyStateChanged(bool isKeyDown)
{
    // Don't interfere when modifier keys are held or text editor has focus
    if (juce::ModifierKeys::getCurrentModifiers().isCommandDown() || 
        juce::ModifierKeys::getCurrentModifiers().isCtrlDown() ||
        juce::ModifierKeys::getCurrentModifiers().isAltDown() ||
        isTextInputActive())
    {
        return false;
    }
    
    if (!isKeyDown)
    {
        // Key released - check if any of our keys were released
        for (auto& mapping : keyboardMappings)
        {
            if (mapping.isPressed)
            {
                // Check if this key is still pressed
                bool upStillPressed = juce::KeyPress::isKeyCurrentlyDown(mapping.upKey);
                bool downStillPressed = juce::KeyPress::isKeyCurrentlyDown(mapping.downKey);
                
                if (!upStillPressed && !downStillPressed)
                {
                    mapping.isPressed = false;
                }
            }
        }
        
        // Stop timer if no keys are pressed
        bool anyKeyPressed = false;
        for (const auto& mapping : keyboardMappings)
        {
            if (mapping.isPressed)
            {
                anyKeyPressed = true;
                break;
            }
        }
        
        if (!anyKeyPressed && isTimerRunning())
            stopTimer();
    }
    
    return false;
}

//==============================================================================
void KeyboardController::adjustMovementRate(bool increase)
{
    if (increase)
    {
        if (currentRateIndex < movementRates.size() - 1)
        {
            currentRateIndex++;
            keyboardMovementRate = movementRates[currentRateIndex];
            updateSpeedDisplay();
        }
    }
    else
    {
        if (currentRateIndex > 0)
        {
            currentRateIndex--;
            keyboardMovementRate = movementRates[currentRateIndex];
            updateSpeedDisplay();
        }
    }
}

juce::String KeyboardController::getSpeedDisplayText() const
{
    if (keyboardMovementRate == -1)
        return "Keyboard Speed: 100% (instant) (Z/X to adjust)";
    else
        return "Keyboard Speed: " + juce::String((int)keyboardMovementRate) + " units/sec (Z/X to adjust)";
}

//==============================================================================
bool KeyboardController::isTextInputActive() const
{
    // Check if any text editor in the application currently has focus
    auto* focusedComponent = juce::Component::getCurrentlyFocusedComponent();
    return (focusedComponent != nullptr && 
            dynamic_cast<juce::TextEditor*>(focusedComponent) != nullptr);
}

//==============================================================================
void KeyboardController::timerCallback()
{
    processKeyboardMovement();
}

void KeyboardController::processKeyboardMovement()
{
    // Handle keyboard controls
    for (auto& mapping : keyboardMappings)
    {
        if (mapping.isPressed)
        {
            // Check if slider is locked using callback
            bool sliderIsLocked = false;
            if (isSliderLocked)
                sliderIsLocked = isSliderLocked(mapping.currentSliderIndex);
                
            if (!sliderIsLocked)
            {
                // Get current slider value using callback
                double currentValue = 0.0;
                if (getSliderValue)
                    currentValue = getSliderValue(mapping.currentSliderIndex);
                    
                double newValue = currentValue;
                
                // Handle instant movement (100%)
                if (keyboardMovementRate == -1)
                {
                    if (mapping.isUpDirection)
                    {
                        // Get the effective maximum for this slider mode
                        double stepSize = getSliderStepSize ? getSliderStepSize(mapping.currentSliderIndex) : 1.0;
                        if (stepSize >= 128.0)
                        {
                            // 7-bit mode: effective maximum is 127*128 = 16256
                            newValue = 16256.0;
                            DBG("KeyboardController: Instant mode - 7-bit max: " << newValue);
                        }
                        else
                        {
                            // 14-bit mode: standard maximum
                            newValue = 16383.0;
                        }
                    }
                    else
                    {
                        newValue = 0.0; // Go to min instantly
                    }
                }
                else
                {
                    // Calculate movement delta based on rate (MIDI units per second)
                    double deltaTime = 1.0 / 60.0; // Assuming 60fps timer
                    double movementDelta = keyboardMovementRate * deltaTime;
                    
                    // Get the effective step size for this slider (128 for 7-bit, 1 for 14-bit)
                    double stepSize = 1.0; // Default to 1 unit steps
                    if (getSliderStepSize)
                    {
                        stepSize = getSliderStepSize(mapping.currentSliderIndex);
                        if (stepSize <= 0.0) stepSize = 1.0; // Safety fallback
                    }
                    
                    // Accumulate fractional movement
                    double direction = mapping.isUpDirection ? 1.0 : -1.0;
                    mapping.accumulatedMovement += movementDelta * direction;
                    
                    // Only move when we've accumulated at least one step size
                    if (std::abs(mapping.accumulatedMovement) >= stepSize)
                    {
                        double wholeStepsToMove = std::floor(std::abs(mapping.accumulatedMovement) / stepSize);
                        double unitsToMove = wholeStepsToMove * stepSize;
                        
                        if (mapping.accumulatedMovement > 0)
                        {
                            // Use effective maximum for this slider mode
                            double effectiveMax = (stepSize >= 128.0) ? 16256.0 : 16383.0;
                            newValue = juce::jmin(effectiveMax, currentValue + unitsToMove);
                            mapping.accumulatedMovement -= unitsToMove;
                        }
                        else
                        {
                            newValue = juce::jmax(0.0, currentValue - unitsToMove);
                            mapping.accumulatedMovement += unitsToMove;
                        }
                    }
                }
                
                if (newValue != currentValue)
                {
                    // Debug logging for movement tracking - with range analysis
                    double stepSize = getSliderStepSize ? getSliderStepSize(mapping.currentSliderIndex) : 1.0;
                    juce::String rangeIndicator = (currentValue < 8191.5) ? "Lower" : "Upper";
                    double percentageOfRange = (currentValue / 16383.0) * 100.0;
                    double actualMovement = newValue - currentValue;
                    
                    DBG("KeyboardController: Slider " << mapping.currentSliderIndex 
                        << (mapping.isUpDirection ? " UP" : " DOWN")
                        << ", Current: " << currentValue << " (" << rangeIndicator << " " << (int)percentageOfRange << "%)"
                        << ", New: " << newValue 
                        << ", ActualMove: " << actualMovement
                        << ", Accumulated: " << mapping.accumulatedMovement
                        << ", StepSize: " << stepSize
                        << ", Delta: " << (keyboardMovementRate * (1.0/60.0)));
                    
                    // Update slider value using callback
                    if (onSliderValueChanged)
                        onSliderValueChanged(mapping.currentSliderIndex, newValue);
                }
            }
        }
    }
}

void KeyboardController::updateSpeedDisplay()
{
    if (onSpeedDisplayChanged)
        onSpeedDisplayChanged(getSpeedDisplayText());
}