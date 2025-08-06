#pragma once
#include <JuceHeader.h>
#include "../Core/Midi7BitController.h"

//==============================================================================
/**
 * LearnZoneTypes - Defines all learnable zone types and structures
 */

// Complete learn zone types for all targetable components
enum class LearnZoneType 
{
    BankButtons,         // Single zone covering all 4 bank buttons
    SliderTrack,         // Per-slider: just the slider track area  
    AutomationGO,        // Per-slider: the GO button specifically
    AutomationDelay,     // Per-slider: Delay knob individually
    AutomationAttack,    // Per-slider: Attack knob individually  
    AutomationReturn,    // Per-slider: Return knob individually
    AutomationCurve      // Per-slider: Curve knob individually
};

struct LearnZone
{
    LearnZoneType zoneType;
    juce::Rectangle<int> bounds;
    int sliderIndex = -1;           // -1 for BankButtons, 0-15 for slider-specific zones
    MidiTargetType midiTargetType;  // Corresponding MIDI target type
    
    LearnZone() = default;
    
    LearnZone(LearnZoneType type, juce::Rectangle<int> zoneBounds, int index, MidiTargetType targetType)
        : zoneType(type), bounds(zoneBounds), sliderIndex(index), midiTargetType(targetType)
    {}
    
    bool isValid() const 
    { 
        return !bounds.isEmpty(); 
    }
    
    bool contains(juce::Point<int> point) const 
    { 
        return bounds.contains(point); 
    }
    
    juce::String getDisplayName() const
    {
        juce::String name;
        
        switch (zoneType)
        {
            case LearnZoneType::BankButtons:
                name = "Bank Cycling";
                break;
            case LearnZoneType::SliderTrack:
                name = "Slider " + juce::String(sliderIndex + 1) + " Value";
                break;
            case LearnZoneType::AutomationGO:
                name = "Slider " + juce::String(sliderIndex + 1) + " GO Button";
                break;
            case LearnZoneType::AutomationDelay:
                name = "Slider " + juce::String(sliderIndex + 1) + " Delay Knob";
                break;
            case LearnZoneType::AutomationAttack:
                name = "Slider " + juce::String(sliderIndex + 1) + " Attack Knob";
                break;
            case LearnZoneType::AutomationReturn:
                name = "Slider " + juce::String(sliderIndex + 1) + " Return Knob";
                break;
            case LearnZoneType::AutomationCurve:
                name = "Slider " + juce::String(sliderIndex + 1) + " Curve Knob";
                break;
            default:
                name = "Unknown Zone";
                break;
        }
        
        return name;
    }
};