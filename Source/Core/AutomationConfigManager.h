// AutomationConfigManager.h - Management system for automation configuration persistence
#pragma once
#include <JuceHeader.h>
#include "AutomationConfig.h"
#include "../Components/AutomationControlPanel.h"
#include <map>
#include <vector>

//==============================================================================
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
    bool configExists(const juce::String& configId);
    
    // Get configs sorted by name for UI display
    std::vector<AutomationConfig> getConfigsSortedByName();
    
    // Get config by name (returns first match if duplicates exist)
    AutomationConfig getConfigByName(const juce::String& name);
    bool configNameExists(const juce::String& name);
    
    // Copy/paste clipboard system
    void copyConfigFromSlider(int sliderIndex, const AutomationConfig& config);
    bool pasteConfigToSlider(int sliderIndex, AutomationConfig& outConfig);
    bool hasClipboardConfig() const;
    void clearClipboard();
    
    // File persistence
    void saveToFile();
    void loadFromFile();
    juce::File getConfigFile();
    
    // Config creation helpers
    juce::String generateUniqueId();
    AutomationConfig createConfigFromPanel(const AutomationControlPanel& panel, 
                                         const juce::String& name, 
                                         int sliderIndex);
    
    // Statistics and info
    int getConfigCount() const { return static_cast<int>(savedConfigs.size()); }
    bool hasAnyConfigs() const { return !savedConfigs.empty(); }
    
    // Config validation and cleanup
    void validateConfigs();
    void removeInvalidConfigs();
    
    // File management info
    bool configFileExists() const;
    juce::String getConfigFilePath() const;
    juce::int64 getConfigFileSize() const;
    juce::Time getConfigFileLastModified() const;
    
    // Debug and maintenance
    void debugPrintAllConfigs();
    juce::String getDebugInfo();
    
private:
    std::map<juce::String, AutomationConfig> savedConfigs;
    AutomationConfig clipboardConfig;
    bool hasClipboard = false;
    
    // File management
    juce::File appDataDir;
    juce::File configFile;
    
    void ensureConfigDirectoryExists();
    bool writeConfigsToFile(const juce::var& configData);
    juce::var loadConfigsFromFile();
    
    // Internal helpers
    void addOrUpdateConfig(const AutomationConfig& config);
    void removeConfigById(const juce::String& configId);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationConfigManager)
};