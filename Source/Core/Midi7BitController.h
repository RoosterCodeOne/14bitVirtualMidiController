#pragma once
#include <JuceHeader.h>
#include <functional>
#include <array>
#include <vector>
#include <unordered_map>

//==============================================================================
/**
 * MIDI Target Types for expanded input system
 */
enum class MidiTargetType
{
    SliderValue,        // Existing - slider position with deadzone
    BankCycle,          // NEW - cycles through banks  
    AutomationGO,       // NEW - automation start/stop toggle
    AutomationDelay,    // NEW - delay knob (direct value, no deadzone)
    AutomationAttack,   // NEW - attack knob (direct value, no deadzone)
    AutomationReturn,   // NEW - return knob (direct value, no deadzone)
    AutomationCurve,    // NEW - curve knob (direct value, no deadzone)
    AutomationConfig    // NEW - automation config trigger (load + start)
};

/**
 * MIDI Target Information for mapping system
 */
struct MidiTargetInfo
{
    MidiTargetType targetType = MidiTargetType::SliderValue;
    int sliderIndex = -1;     // For per-slider targets (GO, knobs). -1 for global targets
    int ccNumber = -1;
    int channel = -1;
    juce::String configId;    // For AutomationConfig targets - stores config ID
    
    // Helper methods
    bool isPerSliderTarget() const
    {
        return targetType != MidiTargetType::BankCycle;
    }
    
    bool isKnobTarget() const
    {
        return targetType == MidiTargetType::AutomationDelay ||
               targetType == MidiTargetType::AutomationAttack ||
               targetType == MidiTargetType::AutomationReturn ||
               targetType == MidiTargetType::AutomationCurve;
    }
    
    bool isConfigTarget() const
    {
        return targetType == MidiTargetType::AutomationConfig;
    }
    
    juce::String getDisplayName() const
    {
        switch (targetType)
        {
            case MidiTargetType::SliderValue:
                return "Slider " + juce::String(sliderIndex + 1) + " Value";
            case MidiTargetType::BankCycle:
                return "Bank Cycle";
            case MidiTargetType::AutomationGO:
                return "Slider " + juce::String(sliderIndex + 1) + " GO Button";
            case MidiTargetType::AutomationDelay:
                return "Slider " + juce::String(sliderIndex + 1) + " Delay";
            case MidiTargetType::AutomationAttack:
                return "Slider " + juce::String(sliderIndex + 1) + " Attack";
            case MidiTargetType::AutomationReturn:
                return "Slider " + juce::String(sliderIndex + 1) + " Return";
            case MidiTargetType::AutomationCurve:
                return "Slider " + juce::String(sliderIndex + 1) + " Curve";
            case MidiTargetType::AutomationConfig:
                return "Automation Config: " + configId.substring(0, 8) + "...";
            default:
                return "Unknown";
        }
    }
};

/**
 * Midi7BitController handles 7-bit to 14-bit MIDI control conversion, learn mode, and continuous movement
 * Extracted from DebugMidiController to provide clean separation of concerns
 * Extended to support multiple target types beyond just slider values
 */
class Midi7BitController : public juce::Timer
{
public:
    Midi7BitController();
    ~Midi7BitController();
    
    // MIDI processing
    void processIncomingCC(int ccNumber, int ccValue, int channel);
    
    // Learn mode management
    void startLearnMode();
    void stopLearnMode();
    void setLearnTarget(MidiTargetType targetType, int sliderIndex = -1);
    void clearMapping(int sliderIndex);
    void clearTargetMapping(MidiTargetType targetType, int sliderIndex = -1);
    void clearAllMappings();
    bool isInLearnMode() const;
    MidiTargetInfo getCurrentLearnTarget() const;
    
    // Mapping information
    struct MappingInfo {
        MidiTargetType targetType;
        int sliderIndex;
        int ccNumber;
        int channel;
        
        // Legacy compatibility
        MappingInfo(int slider, int cc, int ch) 
            : targetType(MidiTargetType::SliderValue), sliderIndex(slider), ccNumber(cc), channel(ch) {}
        MappingInfo(MidiTargetType type, int slider, int cc, int ch)
            : targetType(type), sliderIndex(slider), ccNumber(cc), channel(ch) {}
    };
    std::vector<MappingInfo> getAllMappings() const;
    
    // Callbacks for parent components
    std::function<void(int sliderIndex, double newValue)> onSliderValueChanged;
    std::function<void(MidiTargetType targetType, int sliderIndex, int ccNumber, int channel)> onMappingLearned;
    std::function<void(int sliderIndex)> onMappingCleared;
    std::function<void()> onLearnModeChanged;
    std::function<void(int sliderIndex, int channel, int ccNumber, int ccValue)> onMidiTooltipUpdate;
    std::function<void(int sliderIndex)> onSliderActivityTrigger;
    std::function<bool(int sliderIndex)> isSliderLocked;
    std::function<double(int sliderIndex)> getSliderValue;
    
    // New callbacks for extended target types
    std::function<void()> onBankCycleRequested;
    std::function<void(int sliderIndex, bool ignored)> onAutomationToggle; // ignored param for compatibility - acts like GO button click
    std::function<void(int sliderIndex, MidiTargetType knobType, double knobValue)> onAutomationKnobChanged;
    std::function<void(const juce::String& configId, int ccValue)> onAutomationConfigTriggered;
    
private:
    // Timer callback for continuous movement
    void timerCallback() override;
    
    // Internal processing methods
    void handleLearnMode(int ccNumber, int ccValue, int channel);
    MidiTargetInfo* findTargetForCC(int ccNumber, int channel);
    void processSliderTarget(const MidiTargetInfo& target, int ccValue);
    void processBankCycleTarget(int ccValue);
    void processAutomationToggleTarget(const MidiTargetInfo& target, int ccValue);
    void processAutomationKnobTarget(const MidiTargetInfo& target, int ccValue);
    void processAutomationConfigTarget(const MidiTargetInfo& target, int ccValue);
    double convertToKnobRange(MidiTargetType knobType, double normalizedValue) const;
    double calculateDistanceFromCenter(int ccValue) const;
    double calculateExponentialSpeed(double distance) const;
    void updateContinuousMovement();
    
    // 7-bit control state for each slider
    struct Midi7BitControlState {
        int lastCCValue = 63; // Default to deadzone center
        double lastUpdateTime = 0.0;
        double movementSpeed = 0.0;
        double movementDirection = 0.0;
        bool isMoving = false;
        bool isActive = false;
    };
    
    // Member variables
    std::array<Midi7BitControlState, 16> controlStates;
    std::vector<MidiTargetInfo> targetMappings; // New comprehensive mapping system
    bool learningMode = false;
    MidiTargetInfo currentLearnTarget; // Current target being learned
    
    // Constants
    static constexpr double MOVEMENT_TIMEOUT = 100.0; // milliseconds
    static constexpr int DEADZONE_MIN = 58;
    static constexpr int DEADZONE_MAX = 68;
    static constexpr int DEADZONE_CENTER = 63;
    static constexpr double BASE_SPEED = 50.0;
    static constexpr double MAX_SPEED = 8000.0;
    static constexpr double SPEED_EXPONENT = 3.0;
    static constexpr int TIMER_INTERVAL = 16; // ~60fps
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Midi7BitController)
};