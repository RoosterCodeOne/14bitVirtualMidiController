#include "AutomationEngine.h"

//==============================================================================
AutomationEngine::AutomationEngine()
{
    DBG("AutomationEngine: Created");
}

AutomationEngine::~AutomationEngine()
{
    // Stop timer before destruction
    stopTimer();
    
    DBG("AutomationEngine: Destroyed");
}

//==============================================================================
void AutomationEngine::startAutomation(int sliderIndex, const AutomationParams& params)
{
    if (sliderIndex < 0 || sliderIndex >= 16)
        return;
        
    auto& automation = automations[sliderIndex];
    
    // Don't start if already automating
    if (automation.isActive)
        return;
    
    // Check if there's enough change to warrant automation
    if (std::abs(params.targetValue - params.startValue) < MIN_VALUE_CHANGE)
    {
        DBG("AutomationEngine: Target too close to start value, skipping automation");
        return;
    }
    
    // Validate attack time
    if (params.attackTime <= 0.0)
    {
        // Instant change - just update the value directly
        if (onValueUpdate)
            onValueUpdate(sliderIndex, params.targetValue);
        return;
    }
    
    // Set up automation
    automation.isActive = true;
    automation.isInReturnPhase = false;
    automation.startTime = juce::Time::getMillisecondCounterHiRes();
    automation.originalValue = params.startValue; // Store original for return phase
    automation.params = params;
    automation.sliderIndex = sliderIndex;
    
    DBG("AutomationEngine: Started automation for slider " << sliderIndex 
        << " from " << params.startValue << " to " << params.targetValue
        << " (delay=" << params.delayTime << "s, attack=" << params.attackTime 
        << "s, return=" << params.returnTime << "s, curve=" << params.curveValue << ")");
    
    // Notify state change
    if (onAutomationStateChanged)
        onAutomationStateChanged(sliderIndex, true);
    
    // Start timer if not already running
    if (!isTimerRunning())
        startTimer(TIMER_INTERVAL);
}

void AutomationEngine::stopAutomation(int sliderIndex)
{
    if (sliderIndex < 0 || sliderIndex >= 16)
        return;
        
    auto& automation = automations[sliderIndex];
    
    if (!automation.isActive)
        return;
        
    automation.isActive = false;
    automation.isInReturnPhase = false;
    
    DBG("AutomationEngine: Stopped automation for slider " << sliderIndex);
    
    // Notify state change
    if (onAutomationStateChanged)
        onAutomationStateChanged(sliderIndex, false);
    
    // Stop timer if no more active automations
    if (!hasAnyActiveAutomations())
        stopTimer();
}

void AutomationEngine::stopAllAutomations()
{
    bool hadActiveAutomations = false;
    
    for (int i = 0; i < 16; ++i)
    {
        if (automations[i].isActive)
        {
            automations[i].isActive = false;
            automations[i].isInReturnPhase = false;
            hadActiveAutomations = true;
            
            // Notify state change
            if (onAutomationStateChanged)
                onAutomationStateChanged(i, false);
        }
    }
    
    if (hadActiveAutomations)
    {
        stopTimer();
        DBG("AutomationEngine: Stopped all automations");
    }
}

bool AutomationEngine::isSliderAutomating(int sliderIndex) const
{
    if (sliderIndex < 0 || sliderIndex >= 16)
        return false;
        
    return automations[sliderIndex].isActive;
}

void AutomationEngine::handleManualOverride(int sliderIndex)
{
    if (isSliderAutomating(sliderIndex))
    {
        stopAutomation(sliderIndex);
        DBG("AutomationEngine: Manual override detected for slider " << sliderIndex);
    }
}

//==============================================================================
void AutomationEngine::timerCallback()
{
    for (auto& automation : automations)
    {
        if (automation.isActive)
        {
            updateAutomation(automation);
        }
    }
    
    // Stop timer if no more active automations
    if (!hasAnyActiveAutomations())
        stopTimer();
}

//==============================================================================
double AutomationEngine::applyCurve(double progress, double curveValue) const
{
    // Clamp progress to valid range
    progress = juce::jlimit(0.0, 1.0, progress);
    
    if (curveValue < 1.0)
    {
        // Exponential (0.0 = full exponential, slow start/fast finish)
        double exponent = 1.0 + (1.0 - curveValue) * 3.0; // Range: 1.0 to 4.0
        return std::pow(progress, exponent);
    }
    else if (curveValue > 1.0)
    {
        // Logarithmic (2.0 = full logarithmic, fast start/slow finish)
        double exponent = 1.0 / (1.0 + (curveValue - 1.0) * 3.0); // Range: 1.0 to 0.25
        return std::pow(progress, exponent);
    }
    else
    {
        // Linear (curveValue == 1.0)
        return progress;
    }
}

void AutomationEngine::updateAutomation(SliderAutomation& automation)
{
    double currentTime = juce::Time::getMillisecondCounterHiRes();
    double elapsed = (currentTime - automation.startTime) / 1000.0; // Convert to seconds
    
    const auto& params = automation.params;
    
    if (elapsed < params.delayTime)
    {
        // DELAY PHASE: Still waiting, no action needed
        return;
    }
    else if (elapsed < params.delayTime + params.attackTime)
    {
        // ATTACK PHASE: Move from start to target with curve applied
        double attackElapsed = elapsed - params.delayTime;
        double progress = attackElapsed / params.attackTime;
        double curvedProgress = applyCurve(progress, params.curveValue);
        double currentValue = params.startValue + (params.targetValue - params.startValue) * curvedProgress;
        
        if (onValueUpdate)
            onValueUpdate(automation.sliderIndex, currentValue);
    }
    else if (params.returnTime > 0.0 && elapsed < params.delayTime + params.attackTime + params.returnTime)
    {
        // RETURN PHASE: Move from target back to original
        if (!automation.isInReturnPhase)
        {
            automation.isInReturnPhase = true;
            DBG("AutomationEngine: Entering return phase for slider " << automation.sliderIndex);
        }
        
        double returnElapsed = elapsed - params.delayTime - params.attackTime;
        double progress = returnElapsed / params.returnTime;
        // Create proper inverse curve: exponential becomes logarithmic and vice versa
        double inverseCurve;
        if (params.curveValue < 1.0)
        {
            // Attack was exponential, return should be logarithmic
            inverseCurve = 1.0 + (1.0 - params.curveValue); // Maps 0.0->2.0, 1.0->1.0
        }
        else if (params.curveValue > 1.0)
        {
            // Attack was logarithmic, return should be exponential  
            inverseCurve = 1.0 - (params.curveValue - 1.0); // Maps 2.0->0.0, 1.0->1.0
        }
        else
        {
            // Attack was linear, return stays linear
            inverseCurve = 1.0;
        }
        double curvedProgress = applyCurve(progress, inverseCurve);
        double currentValue = params.targetValue + (automation.originalValue - params.targetValue) * curvedProgress;
        
        if (onValueUpdate)
            onValueUpdate(automation.sliderIndex, currentValue);
    }
    else
    {
        // AUTOMATION COMPLETE
        completeAutomation(automation);
    }
}

void AutomationEngine::completeAutomation(SliderAutomation& automation)
{
    // If we had a return phase, end at original value; otherwise end at target
    double finalValue = (automation.params.returnTime > 0.0) ? automation.originalValue : automation.params.targetValue;
    
    if (onValueUpdate)
        onValueUpdate(automation.sliderIndex, finalValue);
    
    DBG("AutomationEngine: Completed automation for slider " << automation.sliderIndex 
        << " with final value " << finalValue);
    
    // Mark as inactive
    automation.isActive = false;
    automation.isInReturnPhase = false;
    
    // Notify state change
    if (onAutomationStateChanged)
        onAutomationStateChanged(automation.sliderIndex, false);
}

bool AutomationEngine::hasAnyActiveAutomations() const
{
    for (const auto& automation : automations)
    {
        if (automation.isActive)
            return true;
    }
    return false;
}