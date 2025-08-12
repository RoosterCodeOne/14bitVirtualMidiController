// AutomationConfig.h - Data structure for automation configuration persistence
#pragma once
#include <JuceHeader.h>
#include <cmath>
#include "../Components/AutomationControlPanel.h"

//==============================================================================
struct AutomationConfig
{
    // Core automation parameters
    double targetValue = 8192.0;           // Target value in display units
    double delayTime = 0.0;                // Delay knob value
    double attackTime = 1.0;               // Attack knob value  
    double returnTime = 0.0;               // Return knob value
    double curveValue = 1.0;               // Curve knob value
    AutomationControlPanel::TimeMode timeMode = AutomationControlPanel::TimeMode::Seconds; // Seconds vs Beats
    
    // Metadata
    juce::String name;                     // User-defined name
    juce::String id;                       // Unique identifier (UUID)
    double createdTime = 0.0;              // Timestamp
    int originalSliderIndex = -1;          // Where it was created (-1 if copied)
    
    // Default constructor
    AutomationConfig() = default;
    
    // Constructor from automation panel values
    AutomationConfig(const juce::String& configName,
                    double target, double delay, double attack, 
                    double returnVal, double curve, 
                    AutomationControlPanel::TimeMode mode,
                    int sliderIndex = -1)
        : targetValue(target), delayTime(delay), attackTime(attack),
          returnTime(returnVal), curveValue(curve), timeMode(mode),
          name(configName), originalSliderIndex(sliderIndex)
    {
        id = generateUniqueId();
        createdTime = juce::Time::getCurrentTime().toMilliseconds();
    }
    
    // Serialization methods
    juce::var toVar() const
    {
        auto obj = new juce::DynamicObject();
        obj->setProperty("name", name);
        obj->setProperty("id", id);
        obj->setProperty("targetValue", targetValue);
        obj->setProperty("delayTime", delayTime);
        obj->setProperty("attackTime", attackTime);
        obj->setProperty("returnTime", returnTime);
        obj->setProperty("curveValue", curveValue);
        obj->setProperty("timeMode", static_cast<int>(timeMode));
        obj->setProperty("createdTime", createdTime);
        obj->setProperty("originalSliderIndex", originalSliderIndex);
        
        return juce::var(obj);
    }
    
    static AutomationConfig fromVar(const juce::var& var)
    {
        AutomationConfig config;
        if (auto* obj = var.getDynamicObject())
        {
            config.name = obj->getProperty("name").toString();
            config.id = obj->getProperty("id").toString();
            config.targetValue = static_cast<double>(obj->getProperty("targetValue"));
            config.delayTime = static_cast<double>(obj->getProperty("delayTime"));
            config.attackTime = static_cast<double>(obj->getProperty("attackTime"));
            config.returnTime = static_cast<double>(obj->getProperty("returnTime"));
            config.curveValue = static_cast<double>(obj->getProperty("curveValue"));
            config.timeMode = static_cast<AutomationControlPanel::TimeMode>(
                static_cast<int>(obj->getProperty("timeMode")));
            config.createdTime = static_cast<double>(obj->getProperty("createdTime"));
            config.originalSliderIndex = static_cast<int>(obj->getProperty("originalSliderIndex"));
        }
        return config;
    }
    
    // Validation
    bool isValid() const 
    { 
        return !name.isEmpty() && !id.isEmpty(); 
    }
    
    // Equality comparison for clipboard operations
    bool operator==(const AutomationConfig& other) const
    {
        const double tolerance = 0.001;
        return std::abs(targetValue - other.targetValue) < tolerance &&
               std::abs(delayTime - other.delayTime) < tolerance &&
               std::abs(attackTime - other.attackTime) < tolerance &&
               std::abs(returnTime - other.returnTime) < tolerance &&
               std::abs(curveValue - other.curveValue) < tolerance &&
               timeMode == other.timeMode;
    }
    
    bool operator!=(const AutomationConfig& other) const
    {
        return !(*this == other);
    }
    
    // Create a copy with new metadata for copying between sliders
    AutomationConfig createCopy(int newSliderIndex = -1) const
    {
        AutomationConfig copy = *this;
        copy.id = generateUniqueId();
        copy.createdTime = juce::Time::getCurrentTime().toMilliseconds();
        copy.originalSliderIndex = newSliderIndex;
        // Keep same name but could be modified by caller if needed
        return copy;
    }
    
    // Get display string for UI (includes time mode indicator)
    juce::String getDisplayName() const
    {
        if (name.isEmpty())
            return "Unnamed Config";
            
        juce::String displayName = name;
        
        // Add time mode indicator
        if (timeMode == AutomationControlPanel::TimeMode::Beats)
            displayName += " [BEAT]";
        else
            displayName += " [SEC]";
            
        // Add MIDI indicator if has non-zero timing
        if (delayTime > 0.0 || attackTime > 0.0 || returnTime > 0.0)
            displayName += " ♪";
            
        return displayName;
    }
    
private:
    static juce::String generateUniqueId()
    {
        // Generate UUID-like identifier
        juce::Random random;
        juce::String uuid;
        
        // Generate random hex string (simplified UUID format)
        for (int i = 0; i < 8; ++i)
            uuid += juce::String::toHexString(random.nextInt(16));
        uuid += "-";
        for (int i = 0; i < 4; ++i)
            uuid += juce::String::toHexString(random.nextInt(16));
        uuid += "-";
        for (int i = 0; i < 4; ++i)
            uuid += juce::String::toHexString(random.nextInt(16));
        uuid += "-";
        for (int i = 0; i < 4; ++i)
            uuid += juce::String::toHexString(random.nextInt(16));
        uuid += "-";
        for (int i = 0; i < 12; ++i)
            uuid += juce::String::toHexString(random.nextInt(16));
            
        return uuid.toUpperCase();
    }
};