#include "Midi7BitController.h"

//==============================================================================
Midi7BitController::Midi7BitController()
{
    // Initialize CC mappings to -1 (not mapped)
    ccMappings.fill(-1);
    
    DBG("Midi7BitController: Created");
}

Midi7BitController::~Midi7BitController()
{
    // Stop timer before destruction
    stopTimer();
    
    DBG("Midi7BitController: Destroyed");
}

//==============================================================================
void Midi7BitController::processIncomingCC(int ccNumber, int ccValue, int channel)
{
    DBG("Midi7BitController::processIncomingCC: CC=" << ccNumber << " Val=" << ccValue 
        << " Ch=" << channel << " (learn mode: " << (int)learningMode 
        << " target=" << learnTargetSlider << ")");
    
    // LEARN MODE: Process on ANY channel (hardware controllers use different channels)
    if (learningMode && learnTargetSlider != -1)
    {
        DBG("LEARN MODE: Processing CC " << ccNumber << " from channel " << channel);
        handleLearnMode(ccNumber, ccValue, channel);
        return; // Early return - learn mode takes priority
    }
    
    // Find which slider this CC number maps to
    int sliderIndex = findSliderIndexForCC(ccNumber);
    if (sliderIndex == -1) 
    {
        DBG("No slider mapped to CC " << ccNumber);
        return;
    }
    
    DBG("Found slider " << sliderIndex << " for CC " << ccNumber);
    
    // Move all UI-related processing to the message thread
    juce::MessageManager::callAsync([this, sliderIndex, ccNumber, ccValue, channel]() {
        // Update MIDI tracking display
        if (onMidiTooltipUpdate)
            onMidiTooltipUpdate(sliderIndex, channel, ccNumber, ccValue);
        
        // Check if slider is locked
        bool sliderIsLocked = false;
        if (isSliderLocked)
            sliderIsLocked = isSliderLocked(sliderIndex);
            
        if (sliderIsLocked)
            return;
        
        auto& controlState = controlStates[sliderIndex];
        controlState.lastCCValue = ccValue;
        controlState.lastUpdateTime = juce::Time::getMillisecondCounterHiRes();
        controlState.isActive = true;
        
        // Check if we're in the deadzone (58-68)
        if (ccValue >= DEADZONE_MIN && ccValue <= DEADZONE_MAX)
        {
            // Stop continuous movement
            controlState.isMoving = false;
            controlState.movementSpeed = 0.0;
        }
        else
        {
            // Calculate movement speed based on distance from center
            double distanceFromCenter = calculateDistanceFromCenter(ccValue);
            controlState.movementSpeed = calculateExponentialSpeed(distanceFromCenter);
            controlState.movementDirection = (ccValue > DEADZONE_MAX) ? 1.0 : -1.0;
            controlState.isMoving = true;
            
            // Start continuous movement timer if not already running
            if (!isTimerRunning())
                startTimer(TIMER_INTERVAL); // ~60fps
        }
        
        // Trigger activity indicator
        if (onSliderActivityTrigger)
            onSliderActivityTrigger(sliderIndex);
    });
}

//==============================================================================
void Midi7BitController::startLearnMode()
{
    learningMode = true;
    
    if (onLearnModeChanged)
        onLearnModeChanged();
        
    DBG("Midi7BitController: Learn mode started");
}

void Midi7BitController::stopLearnMode()
{
    learningMode = false;
    learnTargetSlider = -1;
    
    if (onLearnModeChanged)
        onLearnModeChanged();
        
    DBG("Midi7BitController: Learn mode stopped");
}

void Midi7BitController::setLearnTarget(int sliderIndex)
{
    learnTargetSlider = sliderIndex;
    DBG("Midi7BitController: Learn target set to slider " << sliderIndex);
}

void Midi7BitController::clearMapping(int sliderIndex)
{
    if (sliderIndex >= 0 && sliderIndex < 16)
    {
        ccMappings[sliderIndex] = -1;
        
        if (onMappingCleared)
            onMappingCleared(sliderIndex);
            
        DBG("Midi7BitController: Cleared mapping for slider " << sliderIndex);
    }
}

void Midi7BitController::clearAllMappings()
{
    ccMappings.fill(-1);
    
    for (int i = 0; i < 16; ++i)
    {
        if (onMappingCleared)
            onMappingCleared(i);
    }
    
    DBG("Midi7BitController: Cleared all mappings");
}

bool Midi7BitController::isInLearnMode() const
{
    return learningMode;
}

int Midi7BitController::getLearnTarget() const
{
    return learnTargetSlider;
}

//==============================================================================
std::vector<Midi7BitController::MappingInfo> Midi7BitController::getAllMappings() const
{
    std::vector<MappingInfo> mappings;
    
    for (int i = 0; i < 16; ++i)
    {
        if (ccMappings[i] != -1)
        {
            MappingInfo info;
            info.sliderIndex = i;
            info.ccNumber = ccMappings[i];
            info.channel = 1; // Default channel for now - could be enhanced to store actual channel
            mappings.push_back(info);
        }
    }
    
    return mappings;
}

//==============================================================================
void Midi7BitController::timerCallback()
{
    updateContinuousMovement();
}

//==============================================================================
void Midi7BitController::handleLearnMode(int ccNumber, int ccValue, int channel)
{
    DBG("Midi7BitController::handleLearnMode called: CC=" << ccNumber << " Val=" << ccValue 
        << " Ch=" << channel << " targetSlider=" << learnTargetSlider);
        
    if (learnTargetSlider != -1)
    {
        DBG("Creating mapping: Slider " << learnTargetSlider << " -> CC " << ccNumber);
        
        // Map this CC to the target slider
        ccMappings[learnTargetSlider] = ccNumber;
        DBG("Updated ccMappings[" << learnTargetSlider << "] = " << ccNumber);
        
        // Store the slider index for the success message (before resetting)
        int mappedSliderIndex = learnTargetSlider;
        
        // Reset learn state but keep learn mode active
        learnTargetSlider = -1;
        
        // All UI updates must be on the message thread
        juce::MessageManager::callAsync([this, mappedSliderIndex, channel, ccNumber]() {
            // Notify about successful mapping
            if (onMappingLearned)
                onMappingLearned(mappedSliderIndex, ccNumber, channel);
                
            DBG("Midi7BitController: Called onMappingLearned(" << mappedSliderIndex << ", " << ccNumber << ", " << channel << ")");
        });
    }
    else
    {
        DBG("handleLearnMode called but learnTargetSlider is -1 (no slider selected)");
    }
}

int Midi7BitController::findSliderIndexForCC(int ccNumber) const
{
    // DEBUG: Show current mappings
    DBG("Looking for CC " << ccNumber << " in learned mappings:");
    for (int i = 0; i < 16; ++i)
    {
        if (ccMappings[i] != -1)
        {
            DBG("  Slider " << i << " -> CC " << ccMappings[i]);
        }
    }
    
    // Check learned mappings
    for (int i = 0; i < 16; ++i)
    {
        if (ccMappings[i] == ccNumber)
        {
            DBG("Found match: CC " << ccNumber << " -> Slider " << i);
            return i;
        }
    }
    
    DBG("No mapping found for CC " << ccNumber);
    return -1; // Not found
}

double Midi7BitController::calculateDistanceFromCenter(int ccValue) const
{
    if (ccValue >= DEADZONE_MIN && ccValue <= DEADZONE_MAX)
        return 0.0; // In deadzone
    
    if (ccValue > DEADZONE_MAX)
        return (ccValue - DEADZONE_MAX) / 59.0; // Distance from upper deadzone edge (normalized 0-1)
    else
        return (DEADZONE_MIN - ccValue) / 58.0; // Distance from lower deadzone edge (normalized 0-1)
}

double Midi7BitController::calculateExponentialSpeed(double distance) const
{
    // Exponential speed curve: slow near deadzone, fast at extremes
    double normalizedSpeed = std::pow(distance, SPEED_EXPONENT);
    return BASE_SPEED + (normalizedSpeed * (MAX_SPEED - BASE_SPEED));
}

void Midi7BitController::updateContinuousMovement()
{
    bool anySliderMoving = false;
    double currentTime = juce::Time::getMillisecondCounterHiRes();
    
    for (int i = 0; i < 16; ++i)
    {
        auto& state = controlStates[i];
        
        if (state.isMoving && state.isActive)
        {
            // Check if we've timed out (no new CC messages)
            if (currentTime - state.lastUpdateTime > MOVEMENT_TIMEOUT)
            {
                state.isMoving = false;
                state.isActive = false;
                continue;
            }
            
            // Check if slider is locked
            bool sliderIsLocked = false;
            if (isSliderLocked)
                sliderIsLocked = isSliderLocked(i);
                
            if (sliderIsLocked)
                continue;
            
            anySliderMoving = true;
            
            // Calculate movement delta (speed is in units per second)
            double deltaTime = 1.0 / 60.0; // 60fps
            double movementDelta = state.movementSpeed * deltaTime * state.movementDirection;
            
            // Get current slider value and apply movement
            double currentValue = 0.0;
            if (getSliderValue)
                currentValue = getSliderValue(i);
                
            double newValue = juce::jlimit(0.0, 16383.0, currentValue + movementDelta);
            
            // Update slider value through callback
            if (onSliderValueChanged)
                onSliderValueChanged(i, newValue);
        }
    }
    
    // Stop timer if no sliders are moving (thread-safe)
    if (!anySliderMoving)
    {
        juce::MessageManager::callAsync([this]() {
            stopTimer();
        });
    }
}