#pragma once
#include <JuceHeader.h>
#include <map>
#include <vector>
#include <functional>
#include "../Components/AutomationControlPanel.h"

//==============================================================================
/**
 * AutomationConfig - Complete automation setup that can be saved/loaded/triggered
 */
struct AutomationConfig
{
    juce::String name;                          // User-defined name
    double targetValue;                         // Target value in display units
    double delayTime;                           // Delay knob value
    double attackTime;                          // Attack knob value  
    double returnTime;                          // Return knob value
    double curveValue;                          // Curve knob value
    AutomationControlPanel::TimeMode timeMode;  // Seconds vs Beats
    
    // Metadata
    juce::String id;                            // Unique identifier
    double createdTime;                         // Timestamp
    int originalSliderIndex;                    // Where it was created
    
    // Constructor
    AutomationConfig()
        : name("New Config")
        , targetValue(8192.0)
        , delayTime(0.0)
        , attackTime(1.0)
        , returnTime(0.0)
        , curveValue(1.0)
        , timeMode(AutomationControlPanel::TimeMode::Seconds)
        , id(juce::Uuid().toString())
        , createdTime(juce::Time::getMillisecondCounterHiRes())
        , originalSliderIndex(-1)
    {
    }
    
    // Copy constructor for clipboard operations
    AutomationConfig(const AutomationConfig& other) = default;
    AutomationConfig& operator=(const AutomationConfig& other) = default;
    
    // Validation
    bool isValid() const
    {
        return !name.isEmpty() && !id.isEmpty();
    }
    
    // JSON serialization
    juce::var toJson() const
    {
        auto json = new juce::DynamicObject();
        json->setProperty("name", name);
        json->setProperty("targetValue", targetValue);
        json->setProperty("delayTime", delayTime);
        json->setProperty("attackTime", attackTime);
        json->setProperty("returnTime", returnTime);
        json->setProperty("curveValue", curveValue);
        json->setProperty("timeMode", (int)timeMode);
        json->setProperty("id", id);
        json->setProperty("createdTime", createdTime);
        json->setProperty("originalSliderIndex", originalSliderIndex);
        return juce::var(json);
    }
    
    // JSON deserialization
    static AutomationConfig fromJson(const juce::var& json)
    {
        AutomationConfig config;
        if (auto* obj = json.getDynamicObject())
        {
            config.name = obj->getProperty("name").toString();
            config.targetValue = obj->getProperty("targetValue");
            config.delayTime = obj->getProperty("delayTime");
            config.attackTime = obj->getProperty("attackTime");
            config.returnTime = obj->getProperty("returnTime");
            config.curveValue = obj->getProperty("curveValue");
            config.timeMode = (AutomationControlPanel::TimeMode)(int)obj->getProperty("timeMode");
            config.id = obj->getProperty("id").toString();
            config.createdTime = obj->getProperty("createdTime");
            config.originalSliderIndex = obj->getProperty("originalSliderIndex");
        }
        return config;
    }
};

//==============================================================================
/**
 * AutomationConfigManager - Manages saving, loading, and MIDI triggering of automation configs
 */
class AutomationConfigManager
{
public:
    AutomationConfigManager();
    ~AutomationConfigManager();
    
    // Configuration management
    juce::String saveConfig(const AutomationConfig& config);
    AutomationConfig loadConfig(const juce::String& configId);
    bool deleteConfig(const juce::String& configId);
    std::vector<AutomationConfig> getAllConfigs();
    bool hasConfig(const juce::String& configId) const;
    
    // Copy/paste system
    void copyConfigFromSlider(int sliderIndex);
    void pasteConfigToSlider(int sliderIndex);
    bool hasClipboardConfig() const;
    AutomationConfig getClipboardConfig() const;
    void clearClipboard();
    
    // MIDI assignment
    void assignMidiToConfig(const juce::String& configId, int ccNumber, int channel);
    void removeMidiAssignment(const juce::String& configId);
    void removeMidiAssignment(int ccNumber, int channel);
    juce::String getConfigForMidi(int ccNumber, int channel) const;
    std::vector<std::pair<int, int>> getMidiAssignmentsForConfig(const juce::String& configId) const;
    void triggerConfigFromMidi(int ccNumber, int channel, int ccValue);
    
    // File persistence
    void saveToFile();
    void loadFromFile();
    void setConfigFilePath(const juce::File& filePath);
    
    // Callbacks for external integration
    std::function<AutomationConfig()> onGetSliderConfig;           // Get current slider's automation config
    std::function<void(int, const AutomationConfig&)> onApplyConfigToSlider;  // Apply config to slider
    std::function<void(int)> onStartAutomation;                   // Start automation for slider
    std::function<int(const AutomationConfig&)> onFindBestSlider; // Find best slider for config
    
private:
    std::map<juce::String, AutomationConfig> savedConfigs;
    AutomationConfig clipboardConfig;
    bool hasClipboard = false;
    std::map<std::pair<int,int>, juce::String> midiToConfigMap;    // (CC,Channel) -> ConfigID
    
    juce::File configFile;
    
    // Helper methods
    juce::String generateUniqueId() const;
    void notifySaveRequired();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationConfigManager)
};