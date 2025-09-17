#include "Midi7BitController.h"

//==============================================================================
Midi7BitController::Midi7BitController()
{
    // Initialize current learn target
    currentLearnTarget.targetType = MidiTargetType::SliderValue;
    currentLearnTarget.sliderIndex = -1;
    currentLearnTarget.ccNumber = -1;
    currentLearnTarget.channel = -1;
    
    DBG("Midi7BitController: Created with extended target system");
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
    juce::String debugMsg = "Midi7BitController::processIncomingCC: CC=" + juce::String(ccNumber) + " Val=" + juce::String(ccValue) + " Ch=" + juce::String(channel) + " (learn mode: " + juce::String((int)learningMode) + " target type=" + juce::String((int)currentLearnTarget.targetType) + " slider=" + juce::String(currentLearnTarget.sliderIndex) + ")"; 
    DBG(debugMsg);
    
    // LEARN MODE: Process on ANY channel (hardware controllers use different channels)
    if (learningMode && currentLearnTarget.sliderIndex != -1)
    {
        DBG("LEARN MODE: Processing CC " + juce::String(ccNumber) + " from channel " + juce::String(channel));
        handleLearnMode(ccNumber, ccValue, channel);
        return; // Early return - learn mode takes priority
    }
    
    // Find target for this CC number and channel
    MidiTargetInfo* target = findTargetForCC(ccNumber, channel);
    if (!target)
    {
        DBG("No target mapped to CC " + juce::String(ccNumber) + " Ch " + juce::String(channel));
        return;
    }
    
    DBG("Found target: " + target->getDisplayName());
    
    // Process based on target type
    switch (target->targetType)
    {
        case MidiTargetType::SliderValue:
            processSliderTarget(*target, ccValue);
            break;
        case MidiTargetType::BankCycle:
            processBankCycleTarget(ccValue);
            break;
        case MidiTargetType::AutomationGO:
            processAutomationToggleTarget(*target, ccValue);
            break;
        case MidiTargetType::AutomationDelay:
        case MidiTargetType::AutomationAttack:
        case MidiTargetType::AutomationReturn:
        case MidiTargetType::AutomationCurve:
            processAutomationKnobTarget(*target, ccValue);
            break;
        case MidiTargetType::AutomationConfig:
            processAutomationConfigTarget(*target, ccValue);
            break;
    }
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
    currentLearnTarget.sliderIndex = -1;
    currentLearnTarget.targetType = MidiTargetType::SliderValue;
    
    if (onLearnModeChanged)
        onLearnModeChanged();
        
    DBG("Midi7BitController: Learn mode stopped");
}

void Midi7BitController::setLearnTarget(MidiTargetType targetType, int sliderIndex)
{
    currentLearnTarget.targetType = targetType;
    currentLearnTarget.sliderIndex = sliderIndex;
    currentLearnTarget.ccNumber = -1;
    currentLearnTarget.channel = -1;
    
    DBG("Midi7BitController: Learn target set to " + currentLearnTarget.getDisplayName());
}

void Midi7BitController::clearMapping(int sliderIndex)
{
    if (sliderIndex >= 0 && sliderIndex < 16)
    {
        // Remove all mappings for this slider
        targetMappings.erase(
            std::remove_if(targetMappings.begin(), targetMappings.end(),
                [sliderIndex](const MidiTargetInfo& target) {
                    return target.sliderIndex == sliderIndex;
                }),
            targetMappings.end());
        
        if (onMappingCleared)
            onMappingCleared(sliderIndex);
            
        DBG("Midi7BitController: Cleared all mappings for slider " + juce::String(sliderIndex));
    }
}

void Midi7BitController::clearTargetMapping(MidiTargetType targetType, int sliderIndex)
{
    // Remove specific target mapping
    targetMappings.erase(
        std::remove_if(targetMappings.begin(), targetMappings.end(),
            [targetType, sliderIndex](const MidiTargetInfo& target) {
                return target.targetType == targetType && target.sliderIndex == sliderIndex;
            }),
        targetMappings.end());
    
    MidiTargetInfo tempTarget{targetType, sliderIndex, -1, -1};
    DBG("Midi7BitController: Cleared mapping for " + tempTarget.getDisplayName());
}

void Midi7BitController::clearAllMappings()
{
    targetMappings.clear();
    
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

MidiTargetInfo Midi7BitController::getCurrentLearnTarget() const
{
    return currentLearnTarget;
}

//==============================================================================
std::vector<Midi7BitController::MappingInfo> Midi7BitController::getAllMappings() const
{
    std::vector<MappingInfo> mappings;
    
    // Convert targetMappings to MappingInfo format
    for (const auto& target : targetMappings)
    {
        MappingInfo info(target.targetType, target.sliderIndex, target.ccNumber, target.channel);
        mappings.push_back(info);
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
    juce::String debugMsg = "Midi7BitController::handleLearnMode called: CC=" + juce::String(ccNumber) + " Val=" + juce::String(ccValue) + " Ch=" + juce::String(channel) + " target=" + currentLearnTarget.getDisplayName();
    DBG(debugMsg);
        
    if (currentLearnTarget.sliderIndex != -1 || currentLearnTarget.targetType == MidiTargetType::BankCycle)
    {
        juce::String debugMsg = "Creating mapping: " + currentLearnTarget.getDisplayName() + " -> CC " + juce::String(ccNumber) + " Ch " + juce::String(channel);
        DBG(debugMsg);
        
        // Create new target mapping
        MidiTargetInfo newMapping = currentLearnTarget;
        newMapping.ccNumber = ccNumber;
        newMapping.channel = channel;
        
        // Remove any existing mapping for this CC/Channel combination
        targetMappings.erase(
            std::remove_if(targetMappings.begin(), targetMappings.end(),
                [ccNumber, channel](const MidiTargetInfo& target) {
                    return target.ccNumber == ccNumber && target.channel == channel;
                }),
            targetMappings.end());
        
        // Add the new mapping
        targetMappings.push_back(newMapping);
        
        // Store values for callback (before resetting)
        MidiTargetType mappedTargetType = currentLearnTarget.targetType;
        int mappedSliderIndex = currentLearnTarget.sliderIndex;
        
        // Reset learn state but keep learn mode active
        currentLearnTarget.sliderIndex = -1;
        
        // All UI updates must be on the message thread
        juce::MessageManager::callAsync([this, mappedTargetType, mappedSliderIndex, channel, ccNumber]() {
            // Notify about successful mapping
            if (onMappingLearned)
                onMappingLearned(mappedTargetType, mappedSliderIndex, ccNumber, channel);
                
            MidiTargetInfo tempTarget{mappedTargetType, mappedSliderIndex, ccNumber, channel};
            DBG("Midi7BitController: Called onMappingLearned for " + tempTarget.getDisplayName());
        });
    }
    else
    {
        DBG("handleLearnMode called but no valid target selected");
    }
}

MidiTargetInfo* Midi7BitController::findTargetForCC(int ccNumber, int channel)
{
    // Find target mapping for this CC and channel
    for (auto& target : targetMappings)
    {
        if (target.ccNumber == ccNumber && target.channel == channel)
        {
            return &target;
        }
    }
    
    DBG("No target mapping found for CC " + juce::String(ccNumber) + " Ch " + juce::String(channel));
    return nullptr;
}

void Midi7BitController::processSliderTarget(const MidiTargetInfo& target, int ccValue)
{
    int sliderIndex = target.sliderIndex;
    
    // Move all UI-related processing to the message thread
    juce::MessageManager::callAsync([this, sliderIndex, ccValue, target]() {
        // Update MIDI tracking display
        if (onMidiTooltipUpdate)
            onMidiTooltipUpdate(sliderIndex, target.channel, target.ccNumber, ccValue);
        
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
        bool isInDeadzone = (ccValue >= DEADZONE_MIN && ccValue <= DEADZONE_MAX);

        if (isInDeadzone)
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

        // Send initial value update to slider with deadzone info
        if (onSliderValueChanged)
        {
            // Convert 7-bit CC value to 14-bit range for slider
            double convertedValue = (double(ccValue) / 127.0) * 16383.0;
            onSliderValueChanged(sliderIndex, convertedValue, isInDeadzone);
        }
        
        // Trigger activity indicator
        if (onSliderActivityTrigger)
            onSliderActivityTrigger(sliderIndex);
    });
}

void Midi7BitController::processBankCycleTarget(int ccValue)
{
    // Trigger on CC value > threshold (like button press)
    if (ccValue >= 64)
    {
        DBG("Bank cycle triggered with CC value " + juce::String(ccValue));
        
        // Move to message thread for UI updates
        juce::MessageManager::callAsync([this]() {
            if (onBankCycleRequested)
                onBankCycleRequested();
        });
    }
}

void Midi7BitController::processAutomationToggleTarget(const MidiTargetInfo& target, int ccValue)
{
    // Simple click behavior - any CC value triggers a GO button click
    // This will start automation if stopped, or stop if already running
    int sliderIndex = target.sliderIndex;
    
    juce::String debugMsg = "Automation GO click for slider " + juce::String(sliderIndex) + " (CC value: " + juce::String(ccValue) + ")";
    DBG(debugMsg);
    
    // Move to message thread for UI updates (pass true for shouldStart, but the callback will handle toggle logic)
    juce::MessageManager::callAsync([this, sliderIndex]() {
        if (onAutomationToggle)
            onAutomationToggle(sliderIndex, true); // The callback handles the actual toggle logic
    });
}

void Midi7BitController::processAutomationKnobTarget(const MidiTargetInfo& target, int ccValue)
{
    // Convert 0-127 MIDI to knob's native range
    double normalizedValue = ccValue / 127.0;
    double knobValue = convertToKnobRange(target.targetType, normalizedValue);
    int sliderIndex = target.sliderIndex;
    
    juce::String debugMsg = "Automation knob " + target.getDisplayName() + ": " + juce::String(knobValue) + " (CC value: " + juce::String(ccValue) + ")";
    DBG(debugMsg);
    
    // Move to message thread for UI updates (direct control, no deadzone)
    juce::MessageManager::callAsync([this, sliderIndex, targetType = target.targetType, knobValue]() {
        if (onAutomationKnobChanged)
            onAutomationKnobChanged(sliderIndex, targetType, knobValue);
    });
}

double Midi7BitController::convertToKnobRange(MidiTargetType knobType, double normalizedValue) const
{
    // Convert normalized 0-1 value to knob's specific range
    switch (knobType)
    {
        case MidiTargetType::AutomationDelay:
        case MidiTargetType::AutomationAttack:
        case MidiTargetType::AutomationReturn:
            // Time values: 0-10 seconds (or could be 0-10000ms)
            return normalizedValue * 10.0;
            
        case MidiTargetType::AutomationCurve:
            // Curve values: typically -1.0 to +1.0 for exponential curves
            return (normalizedValue * 2.0) - 1.0;
            
        default:
            return normalizedValue;
    }
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

void Midi7BitController::processAutomationConfigTarget(const MidiTargetInfo& target, int ccValue)
{
    // Trigger automation config on CC value > threshold (like button press)
    if (ccValue < 64) return;
    
    if (target.configId.isEmpty())
    {
        DBG("AutomationConfig target has empty config ID");
        return;
    }
    
    juce::String debugMsg = "Automation config triggered: " + target.configId + " (CC value: " + juce::String(ccValue) + ")";
    DBG(debugMsg);
    
    // Move to message thread for config processing
    juce::MessageManager::callAsync([this, configId = target.configId, ccValue]() {
        if (onAutomationConfigTriggered)
            onAutomationConfigTriggered(configId, ccValue);
    });
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
            
            // Update slider value through callback (continuous movement is outside deadzone)
            if (onSliderValueChanged)
                onSliderValueChanged(i, newValue, false);
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
