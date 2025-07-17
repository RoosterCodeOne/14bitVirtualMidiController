#pragma once
#include <JuceHeader.h>
#include <functional>
#include <array>

//==============================================================================
/**
 * AutomationEngine handles complex delay/attack/return automation with curve calculations
 * Extracted from SimpleSliderControl to provide clean separation of concerns
 */
class AutomationEngine : public juce::Timer
{
public:
    struct AutomationParams {
        double delayTime = 0.0;     // Delay before movement starts (seconds)
        double attackTime = 1.0;    // Time to move from start to target (seconds)
        double returnTime = 0.0;    // Time to return to original value (seconds, 0 = no return)
        double curveValue = 1.0;    // Curve shape: 0.0-1.0 = exponential, 1.0 = linear, 1.0-2.0 = logarithmic
        double startValue = 0.0;    // Starting MIDI value (0-16383)
        double targetValue = 0.0;   // Target MIDI value (0-16383)
    };
    
    AutomationEngine();
    ~AutomationEngine();
    
    // Automation control
    void startAutomation(int sliderIndex, const AutomationParams& params);
    void stopAutomation(int sliderIndex);
    void stopAllAutomations();
    bool isSliderAutomating(int sliderIndex) const;
    
    // Manual override detection
    void handleManualOverride(int sliderIndex);
    
    // Callbacks for parent components
    std::function<void(int sliderIndex, double newValue)> onValueUpdate;
    std::function<void(int sliderIndex, bool isAutomating)> onAutomationStateChanged;
    
private:
    // Internal automation state for each slider
    struct SliderAutomation {
        bool isActive = false;
        bool isInReturnPhase = false;
        double startTime = 0.0;
        double originalValue = 0.0;  // Value at start of automation (for return phase)
        AutomationParams params;
        int sliderIndex = -1;
    };
    
    // Timer callback for automation updates
    void timerCallback() override;
    
    // Internal processing methods
    double applyCurve(double progress, double curveValue) const;
    void updateAutomation(SliderAutomation& automation);
    void completeAutomation(SliderAutomation& automation);
    bool hasAnyActiveAutomations() const;
    
    // Member variables
    std::array<SliderAutomation, 16> automations;
    
    // Constants
    static constexpr int TIMER_INTERVAL = 16; // ~60fps updates
    static constexpr double MIN_VALUE_CHANGE = 1.0; // Minimum change to start automation
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationEngine)
};