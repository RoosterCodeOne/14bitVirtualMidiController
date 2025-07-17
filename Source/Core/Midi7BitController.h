#pragma once
#include <JuceHeader.h>
#include <functional>
#include <array>
#include <vector>

//==============================================================================
/**
 * Midi7BitController handles 7-bit to 14-bit MIDI control conversion, learn mode, and continuous movement
 * Extracted from DebugMidiController to provide clean separation of concerns
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
    void setLearnTarget(int sliderIndex);
    void clearMapping(int sliderIndex);
    void clearAllMappings();
    bool isInLearnMode() const;
    int getLearnTarget() const;
    
    // Mapping information
    struct MappingInfo {
        int sliderIndex;
        int ccNumber;
        int channel;
    };
    std::vector<MappingInfo> getAllMappings() const;
    
    // Callbacks for parent components
    std::function<void(int sliderIndex, double newValue)> onSliderValueChanged;
    std::function<void(int sliderIndex, int ccNumber, int channel)> onMappingLearned;
    std::function<void(int sliderIndex)> onMappingCleared;
    std::function<void()> onLearnModeChanged;
    std::function<void(int sliderIndex, int channel, int ccNumber, int ccValue)> onMidiTooltipUpdate;
    std::function<void(int sliderIndex)> onSliderActivityTrigger;
    std::function<bool(int sliderIndex)> isSliderLocked;
    std::function<double(int sliderIndex)> getSliderValue;
    
private:
    // Timer callback for continuous movement
    void timerCallback() override;
    
    // Internal processing methods
    void handleLearnMode(int ccNumber, int ccValue, int channel);
    int findSliderIndexForCC(int ccNumber) const;
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
    std::array<int, 16> ccMappings; // -1 = not mapped
    bool learningMode = false;
    int learnTargetSlider = -1;
    
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