// AutomationConfigManager.cpp - Implementation of automation configuration management system
#include "AutomationConfigManager.h"

//==============================================================================
AutomationConfigManager::AutomationConfigManager()
{
    // Set config file path in application data directory
    appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                     .getChildFile("14bit Virtual Midi Controller");
    configFile = appDataDir.getChildFile("automation_configs.json");
    
    // Ensure directory exists and load existing configs
    ensureConfigDirectoryExists();
    loadFromFile();
    
    DBG("AutomationConfigManager: Initialized with " + juce::String(savedConfigs.size()) + " configs");
}

AutomationConfigManager::~AutomationConfigManager()
{
    // Auto-save on destruction
    saveToFile();
    DBG("AutomationConfigManager: Destroyed, saved " + juce::String(savedConfigs.size()) + " configs");
}

//==============================================================================
// Configuration Management
juce::String AutomationConfigManager::saveConfig(const AutomationConfig& config)
{
    if (!config.isValid())
    {
        DBG("AutomationConfigManager: Cannot save invalid config");
        return juce::String();
    }
    
    AutomationConfig configToSave = config;
    
    // Generate new ID if empty
    if (configToSave.id.isEmpty())
        configToSave.id = generateUniqueId();
    
    // Update timestamp
    configToSave.createdTime = juce::Time::getCurrentTime().toMilliseconds();
    
    // Store config
    addOrUpdateConfig(configToSave);
    
    DBG("AutomationConfigManager: Saved config '" + configToSave.name + "' with ID " + configToSave.id);
    
    // Auto-save to file
    saveToFile();
    
    return configToSave.id;
}

AutomationConfig AutomationConfigManager::loadConfig(const juce::String& configId)
{
    auto it = savedConfigs.find(configId);
    if (it != savedConfigs.end())
    {
        DBG("AutomationConfigManager: Loaded config '" + it->second.name + "' with ID " + configId);
        return it->second;
    }
    
    DBG("AutomationConfigManager: Config not found with ID " + configId);
    return AutomationConfig(); // Return invalid config
}

bool AutomationConfigManager::deleteConfig(const juce::String& configId)
{
    auto it = savedConfigs.find(configId);
    if (it != savedConfigs.end())
    {
        juce::String configName = it->second.name;
        
        // Remove from saved configs
        removeConfigById(configId);
        
        DBG("AutomationConfigManager: Deleted config '" + configName + "' with ID " + configId);
        
        // Auto-save to file
        saveToFile();
        
        return true;
    }
    
    return false;
}

std::vector<AutomationConfig> AutomationConfigManager::getAllConfigs()
{
    std::vector<AutomationConfig> configs;
    configs.reserve(savedConfigs.size());
    
    for (const auto& pair : savedConfigs)
    {
        configs.push_back(pair.second);
    }
    
    // Sort by creation time (newest first)
    std::sort(configs.begin(), configs.end(), 
        [](const AutomationConfig& a, const AutomationConfig& b) {
            return a.createdTime > b.createdTime;
        });
    
    return configs;
}

bool AutomationConfigManager::configExists(const juce::String& configId)
{
    return savedConfigs.find(configId) != savedConfigs.end();
}

std::vector<AutomationConfig> AutomationConfigManager::getConfigsSortedByName()
{
    std::vector<AutomationConfig> configs;
    configs.reserve(savedConfigs.size());
    
    for (const auto& pair : savedConfigs)
    {
        configs.push_back(pair.second);
    }
    
    // Sort by name alphabetically
    std::sort(configs.begin(), configs.end(), 
        [](const AutomationConfig& a, const AutomationConfig& b) {
            return a.name.compareIgnoreCase(b.name) < 0;
        });
    
    return configs;
}

AutomationConfig AutomationConfigManager::getConfigByName(const juce::String& name)
{
    for (const auto& pair : savedConfigs)
    {
        if (pair.second.name.equalsIgnoreCase(name))
        {
            return pair.second;
        }
    }
    
    DBG("AutomationConfigManager: Config not found with name '" + name + "'");
    return AutomationConfig(); // Return invalid config
}

bool AutomationConfigManager::configNameExists(const juce::String& name)
{
    for (const auto& pair : savedConfigs)
    {
        if (pair.second.name.equalsIgnoreCase(name))
            return true;
    }
    return false;
}

//==============================================================================
// Copy/Paste Clipboard System
void AutomationConfigManager::copyConfigFromSlider(int sliderIndex, const AutomationConfig& config)
{
    clipboardConfig = config;
    clipboardConfig.originalSliderIndex = sliderIndex;
    hasClipboard = true;
    
    DBG("AutomationConfigManager: Copied config from slider " + juce::String(sliderIndex));
}

bool AutomationConfigManager::pasteConfigToSlider(int sliderIndex, AutomationConfig& outConfig)
{
    if (!hasClipboard)
    {
        DBG("AutomationConfigManager: Cannot paste - no config in clipboard");
        return false;
    }
    
    outConfig = clipboardConfig;
    DBG("AutomationConfigManager: Pasted config to slider " + juce::String(sliderIndex));
    return true;
}

bool AutomationConfigManager::hasClipboardConfig() const
{
    return hasClipboard;
}

void AutomationConfigManager::clearClipboard()
{
    hasClipboard = false;
    clipboardConfig = AutomationConfig();
    DBG("AutomationConfigManager: Cleared clipboard");
}

//==============================================================================
// File Persistence
void AutomationConfigManager::saveToFile()
{
    ensureConfigDirectoryExists();
    
    // Create root JSON object
    juce::var configArray;
    for (const auto& pair : savedConfigs)
    {
        configArray.append(pair.second.toVar());
    }
    
    juce::var rootObj = new juce::DynamicObject();
    rootObj.getDynamicObject()->setProperty("automationConfigs", configArray);
    rootObj.getDynamicObject()->setProperty("version", 1);
    rootObj.getDynamicObject()->setProperty("lastSaved", juce::Time::getCurrentTime().toMilliseconds());
    
    // Convert to JSON string
    juce::String jsonString = juce::JSON::toString(rootObj, true);
    
    if (writeConfigsToFile(rootObj))
    {
        DBG("AutomationConfigManager: Saved " + juce::String(savedConfigs.size()) + " configs to file");
    }
    else
    {
        DBG("AutomationConfigManager: Failed to save configs to file");
    }
}

void AutomationConfigManager::loadFromFile()
{
    if (!configFile.existsAsFile())
    {
        DBG("AutomationConfigManager: Config file doesn't exist, starting with empty configs");
        return;
    }
    
    juce::var configData = loadConfigsFromFile();
    if (!configData.isObject())
    {
        DBG("AutomationConfigManager: Invalid or empty config file");
        return;
    }
    
    auto* rootObj = configData.getDynamicObject();
    if (!rootObj)
    {
        DBG("AutomationConfigManager: Failed to parse config file as JSON object");
        return;
    }
    
    // Load configs array
    if (rootObj->hasProperty("automationConfigs"))
    {
        auto configsVar = rootObj->getProperty("automationConfigs");
        if (configsVar.isArray())
        {
            auto* configsArray = configsVar.getArray();
            for (const auto& configVar : *configsArray)
            {
                AutomationConfig config = AutomationConfig::fromVar(configVar);
                if (config.isValid())
                {
                    addOrUpdateConfig(config);
                }
                else
                {
                    DBG("AutomationConfigManager: Skipped invalid config during load");
                }
            }
        }
    }
    
    DBG("AutomationConfigManager: Loaded " + juce::String(savedConfigs.size()) + " configs from file");
}

juce::File AutomationConfigManager::getConfigFile()
{
    return configFile;
}

//==============================================================================
// Config Creation Helpers
juce::String AutomationConfigManager::generateUniqueId()
{
    juce::String id;
    do {
        // Generate random UUID-like identifier
        juce::Random random;
        id = "";
        
        // Generate hex string segments
        for (int i = 0; i < 8; ++i)
            id += juce::String::toHexString(random.nextInt(16));
        id += "-";
        for (int i = 0; i < 4; ++i)
            id += juce::String::toHexString(random.nextInt(16));
        id += "-";
        for (int i = 0; i < 4; ++i)
            id += juce::String::toHexString(random.nextInt(16));
        id += "-";
        for (int i = 0; i < 4; ++i)
            id += juce::String::toHexString(random.nextInt(16));
        id += "-";
        for (int i = 0; i < 12; ++i)
            id += juce::String::toHexString(random.nextInt(16));
            
        id = id.toUpperCase();
        
    } while (configExists(id)); // Ensure uniqueness
    
    return id;
}

AutomationConfig AutomationConfigManager::createConfigFromPanel(const AutomationControlPanel& panel, 
                                                              const juce::String& name, 
                                                              int sliderIndex)
{
    AutomationConfig config(name,
                           panel.getTargetValue(),
                           panel.getDelayTime(),
                           panel.getAttackTime(),
                           panel.getReturnTime(),
                           panel.getCurveValue(),
                           panel.getTimeMode(),
                           sliderIndex);
    
    return config;
}

//==============================================================================
// Config Validation and Cleanup
void AutomationConfigManager::validateConfigs()
{
    std::vector<juce::String> invalidIds;
    
    for (const auto& pair : savedConfigs)
    {
        if (!pair.second.isValid())
        {
            invalidIds.push_back(pair.first);
        }
    }
    
    for (const auto& id : invalidIds)
    {
        DBG("AutomationConfigManager: Removing invalid config with ID " + id);
        removeConfigById(id);
    }
    
    if (!invalidIds.empty())
    {
        saveToFile(); // Save after cleanup
    }
}

void AutomationConfigManager::removeInvalidConfigs()
{
    validateConfigs(); // Same implementation
}

//==============================================================================
// File Management Info
bool AutomationConfigManager::configFileExists() const
{
    return configFile.existsAsFile();
}

juce::String AutomationConfigManager::getConfigFilePath() const
{
    return configFile.getFullPathName();
}

juce::int64 AutomationConfigManager::getConfigFileSize() const
{
    return configFile.getSize();
}

juce::Time AutomationConfigManager::getConfigFileLastModified() const
{
    return configFile.getLastModificationTime();
}

//==============================================================================
// Debug and Maintenance
void AutomationConfigManager::debugPrintAllConfigs()
{
    DBG("AutomationConfigManager: === CONFIG DEBUG DUMP ===");
    DBG("Total configs: " + juce::String(savedConfigs.size()));
    DBG("Config file: " + configFile.getFullPathName());
    
    for (const auto& pair : savedConfigs)
    {
        const auto& config = pair.second;
        DBG("Config: '" + config.name + "' [" + config.id + "]");
        DBG("  Target: " + juce::String(config.targetValue));
        DBG("  Timing: D=" + juce::String(config.delayTime) + 
            " A=" + juce::String(config.attackTime) + 
            " R=" + juce::String(config.returnTime));
        DBG("  Curve: " + juce::String(config.curveValue));
        DBG("  Mode: " + juce::String(config.timeMode == AutomationControlPanel::TimeMode::Seconds ? "SEC" : "BEAT"));
        DBG("  Created: " + juce::Time(static_cast<juce::int64>(config.createdTime)).toString(true, true));
        DBG("  Original Slider: " + juce::String(config.originalSliderIndex));
    }
    
    DBG("AutomationConfigManager: === END DEBUG DUMP ===");
}

juce::String AutomationConfigManager::getDebugInfo()
{
    juce::String info;
    info += "AutomationConfigManager Debug Info\n";
    info += "==================================\n";
    info += "Config Count: " + juce::String(savedConfigs.size()) + "\n";
    info += "Config File: " + configFile.getFullPathName() + "\n";
    info += "File Exists: " + juce::String(configFile.existsAsFile() ? "Yes" : "No") + "\n";
    
    if (configFile.existsAsFile())
    {
        info += "File Size: " + juce::String(configFile.getSize()) + " bytes\n";
        info += "Last Modified: " + configFile.getLastModificationTime().toString(true, true) + "\n";
    }
    
    info += "Clipboard: " + juce::String(hasClipboard ? "Has Config" : "Empty") + "\n";
    
    if (hasClipboard)
    {
        info += "Clipboard Config: '" + clipboardConfig.name + "'\n";
    }
    
    return info;
}

//==============================================================================
// Private Helper Methods
void AutomationConfigManager::ensureConfigDirectoryExists()
{
    if (!appDataDir.isDirectory())
    {
        if (!appDataDir.createDirectory())
        {
            DBG("AutomationConfigManager: Failed to create config directory: " + appDataDir.getFullPathName());
        }
        else
        {
            DBG("AutomationConfigManager: Created config directory: " + appDataDir.getFullPathName());
        }
    }
}

bool AutomationConfigManager::writeConfigsToFile(const juce::var& configData)
{
    juce::String jsonString = juce::JSON::toString(configData, true);
    
    if (configFile.replaceWithText(jsonString))
    {
        return true;
    }
    else
    {
        DBG("AutomationConfigManager: Failed to write to file: " + configFile.getFullPathName());
        return false;
    }
}

juce::var AutomationConfigManager::loadConfigsFromFile()
{
    juce::String jsonString = configFile.loadFileAsString();
    if (jsonString.isEmpty())
    {
        DBG("AutomationConfigManager: Config file is empty");
        return juce::var();
    }
    
    juce::var jsonVar = juce::JSON::parse(jsonString);
    if (jsonVar.isVoid())
    {
        DBG("AutomationConfigManager: Failed to parse JSON from config file");
        return juce::var();
    }
    
    return jsonVar;
}

void AutomationConfigManager::addOrUpdateConfig(const AutomationConfig& config)
{
    savedConfigs[config.id] = config;
}

void AutomationConfigManager::removeConfigById(const juce::String& configId)
{
    auto it = savedConfigs.find(configId);
    if (it != savedConfigs.end())
    {
        savedConfigs.erase(it);
    }
}