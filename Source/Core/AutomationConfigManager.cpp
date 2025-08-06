#include "AutomationConfigManager.h"

//==============================================================================
AutomationConfigManager::AutomationConfigManager()
{
    // Set default config file path (next to executable)
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                      .getChildFile("14bit Virtual Midi Controller");
    appDataDir.createDirectory();
    configFile = appDataDir.getChildFile("automation_configs.json");
    
    // Load existing configs
    loadFromFile();
    
    DBG("AutomationConfigManager: Initialized with " + juce::String(savedConfigs.size()) + " configs");
}

AutomationConfigManager::~AutomationConfigManager()
{
    // Auto-save on destruction
    saveToFile();
    DBG("AutomationConfigManager: Destroyed");
}

//==============================================================================
juce::String AutomationConfigManager::saveConfig(const AutomationConfig& config)
{
    AutomationConfig configToSave = config;
    
    // Generate new ID if empty
    if (configToSave.id.isEmpty())
        configToSave.id = generateUniqueId();
    
    // Update timestamp
    configToSave.createdTime = juce::Time::getMillisecondCounterHiRes();
    
    // Store config
    savedConfigs[configToSave.id] = configToSave;
    
    DBG("AutomationConfigManager: Saved config '" + configToSave.name + "' with ID " + configToSave.id);
    
    // Auto-save to file
    notifySaveRequired();
    
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
    return AutomationConfig(); // Return default config
}

bool AutomationConfigManager::deleteConfig(const juce::String& configId)
{
    auto it = savedConfigs.find(configId);
    if (it != savedConfigs.end())
    {
        juce::String configName = it->second.name;
        
        // Remove any MIDI assignments for this config
        removeMidiAssignment(configId);
        
        // Remove from saved configs
        savedConfigs.erase(it);
        
        DBG("AutomationConfigManager: Deleted config '" + configName + "' with ID " + configId);
        
        // Auto-save to file
        notifySaveRequired();
        
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

bool AutomationConfigManager::hasConfig(const juce::String& configId) const
{
    return savedConfigs.find(configId) != savedConfigs.end();
}

//==============================================================================
void AutomationConfigManager::copyConfigFromSlider(int sliderIndex)
{
    if (onGetSliderConfig)
    {
        clipboardConfig = onGetSliderConfig();
        clipboardConfig.originalSliderIndex = sliderIndex;
        hasClipboard = true;
        
        DBG("AutomationConfigManager: Copied config from slider " + juce::String(sliderIndex));
    }
}

void AutomationConfigManager::pasteConfigToSlider(int sliderIndex)
{
    if (hasClipboard && onApplyConfigToSlider)
    {
        onApplyConfigToSlider(sliderIndex, clipboardConfig);
        DBG("AutomationConfigManager: Pasted config to slider " + juce::String(sliderIndex));
    }
}

bool AutomationConfigManager::hasClipboardConfig() const
{
    return hasClipboard;
}

AutomationConfig AutomationConfigManager::getClipboardConfig() const
{
    return clipboardConfig;
}

void AutomationConfigManager::clearClipboard()
{
    hasClipboard = false;
    clipboardConfig = AutomationConfig();
    DBG("AutomationConfigManager: Cleared clipboard");
}

//==============================================================================
void AutomationConfigManager::assignMidiToConfig(const juce::String& configId, int ccNumber, int channel)
{
    if (!hasConfig(configId))
    {
        DBG("AutomationConfigManager: Cannot assign MIDI - config ID not found: " + configId);
        return;
    }
    
    auto key = std::make_pair(ccNumber, channel);
    
    // Remove any existing assignment for this MIDI input
    auto existing = midiToConfigMap.find(key);
    if (existing != midiToConfigMap.end())
    {
        DBG("AutomationConfigManager: Replacing existing MIDI assignment CC " + juce::String(ccNumber) + " Ch " + juce::String(channel));
    }
    
    midiToConfigMap[key] = configId;
    
    auto config = loadConfig(configId);
    DBG("AutomationConfigManager: Assigned MIDI CC " + juce::String(ccNumber) + " Ch " + juce::String(channel) + " to config '" + config.name + "'");
    
    // Auto-save to file
    notifySaveRequired();
}

void AutomationConfigManager::removeMidiAssignment(const juce::String& configId)
{
    // Remove all MIDI assignments for this config
    for (auto it = midiToConfigMap.begin(); it != midiToConfigMap.end();)
    {
        if (it->second == configId)
        {
            DBG("AutomationConfigManager: Removed MIDI assignment CC " + juce::String(it->first.first) + " Ch " + juce::String(it->first.second));
            it = midiToConfigMap.erase(it);
        }
        else
        {
            ++it;
        }
    }
    
    // Auto-save to file
    notifySaveRequired();
}

void AutomationConfigManager::removeMidiAssignment(int ccNumber, int channel)
{
    auto key = std::make_pair(ccNumber, channel);
    auto it = midiToConfigMap.find(key);
    if (it != midiToConfigMap.end())
    {
        DBG("AutomationConfigManager: Removed MIDI assignment CC " + juce::String(ccNumber) + " Ch " + juce::String(channel));
        midiToConfigMap.erase(it);
        
        // Auto-save to file
        notifySaveRequired();
    }
}

juce::String AutomationConfigManager::getConfigForMidi(int ccNumber, int channel) const
{
    auto key = std::make_pair(ccNumber, channel);
    auto it = midiToConfigMap.find(key);
    if (it != midiToConfigMap.end())
    {
        return it->second;
    }
    return juce::String();
}

std::vector<std::pair<int, int>> AutomationConfigManager::getMidiAssignmentsForConfig(const juce::String& configId) const
{
    std::vector<std::pair<int, int>> assignments;
    
    for (const auto& pair : midiToConfigMap)
    {
        if (pair.second == configId)
        {
            assignments.push_back(pair.first);
        }
    }
    
    return assignments;
}

void AutomationConfigManager::triggerConfigFromMidi(int ccNumber, int channel, int ccValue)
{
    // Only trigger on high values (like button press)
    if (ccValue < 64) return;
    
    juce::String configId = getConfigForMidi(ccNumber, channel);
    if (configId.isEmpty())
    {
        DBG("AutomationConfigManager: No config assigned to MIDI CC " + juce::String(ccNumber) + " Ch " + juce::String(channel));
        return;
    }
    
    AutomationConfig config = loadConfig(configId);
    if (!config.isValid())
    {
        DBG("AutomationConfigManager: Invalid config loaded for ID " + configId);
        return;
    }
    
    DBG("AutomationConfigManager: MIDI triggered config '" + config.name + "' from CC " + juce::String(ccNumber) + " Ch " + juce::String(channel));
    
    // Find which slider this config should apply to
    int targetSlider = -1;
    if (onFindBestSlider)
    {
        targetSlider = onFindBestSlider(config);
    }
    else
    {
        // Default: use original slider if available
        targetSlider = config.originalSliderIndex;
        if (targetSlider < 0 || targetSlider >= 16)
            targetSlider = 0; // Default to first slider
    }
    
    // Apply config to slider
    if (onApplyConfigToSlider)
    {
        onApplyConfigToSlider(targetSlider, config);
        DBG("AutomationConfigManager: Applied config '" + config.name + "' to slider " + juce::String(targetSlider));
    }
    
    // Immediately start automation
    if (onStartAutomation)
    {
        // Small delay to ensure config is fully applied
        juce::Timer::callAfterDelay(50, [this, targetSlider]() {
            onStartAutomation(targetSlider);
            DBG("AutomationConfigManager: Started automation for slider " + juce::String(targetSlider));
        });
    }
}

//==============================================================================
void AutomationConfigManager::saveToFile()
{
    auto json = new juce::DynamicObject();
    
    // Save configs
    juce::Array<juce::var> configsArray;
    for (const auto& pair : savedConfigs)
    {
        configsArray.add(pair.second.toJson());
    }
    json->setProperty("configs", configsArray);
    
    // Save MIDI assignments
    juce::Array<juce::var> midiArray;
    for (const auto& pair : midiToConfigMap)
    {
        auto midiAssignment = new juce::DynamicObject();
        midiAssignment->setProperty("ccNumber", pair.first.first);
        midiAssignment->setProperty("channel", pair.first.second);
        midiAssignment->setProperty("configId", pair.second);
        midiArray.add(juce::var(midiAssignment));
    }
    json->setProperty("midiAssignments", midiArray);
    
    // Write to file
    juce::var jsonVar(json);
    juce::String jsonString = juce::JSON::toString(jsonVar, true);
    
    if (configFile.replaceWithText(jsonString))
    {
        DBG("AutomationConfigManager: Saved " + juce::String(savedConfigs.size()) + " configs and " + juce::String(midiToConfigMap.size()) + " MIDI assignments to " + configFile.getFullPathName());
    }
    else
    {
        DBG("AutomationConfigManager: Failed to save to " + configFile.getFullPathName());
    }
}

void AutomationConfigManager::loadFromFile()
{
    if (!configFile.existsAsFile())
    {
        DBG("AutomationConfigManager: Config file doesn't exist, starting with empty configs");
        return;
    }
    
    juce::String jsonString = configFile.loadFileAsString();
    if (jsonString.isEmpty())
    {
        DBG("AutomationConfigManager: Config file is empty");
        return;
    }
    
    juce::var jsonVar = juce::JSON::parse(jsonString);
    if (!jsonVar.isObject())
    {
        DBG("AutomationConfigManager: Invalid JSON in config file");
        return;
    }
    
    auto* jsonObj = jsonVar.getDynamicObject();
    
    // Load configs
    if (jsonObj->hasProperty("configs"))
    {
        auto configsArray = jsonObj->getProperty("configs");
        if (configsArray.isArray())
        {
            for (const auto& configVar : *configsArray.getArray())
            {
                AutomationConfig config = AutomationConfig::fromJson(configVar);
                if (config.isValid())
                {
                    savedConfigs[config.id] = config;
                }
            }
        }
    }
    
    // Load MIDI assignments
    if (jsonObj->hasProperty("midiAssignments"))
    {
        auto midiArray = jsonObj->getProperty("midiAssignments");
        if (midiArray.isArray())
        {
            for (const auto& midiVar : *midiArray.getArray())
            {
                if (auto* midiObj = midiVar.getDynamicObject())
                {
                    int ccNumber = midiObj->getProperty("ccNumber");
                    int channel = midiObj->getProperty("channel");
                    juce::String configId = midiObj->getProperty("configId").toString();
                    
                    if (!configId.isEmpty() && hasConfig(configId))
                    {
                        midiToConfigMap[std::make_pair(ccNumber, channel)] = configId;
                    }
                }
            }
        }
    }
    
    DBG("AutomationConfigManager: Loaded " + juce::String(savedConfigs.size()) + " configs and " + juce::String(midiToConfigMap.size()) + " MIDI assignments from " + configFile.getFullPathName());
}

void AutomationConfigManager::setConfigFilePath(const juce::File& filePath)
{
    configFile = filePath;
    DBG("AutomationConfigManager: Config file path set to " + configFile.getFullPathName());
}

//==============================================================================
juce::String AutomationConfigManager::generateUniqueId() const
{
    juce::String id;
    do {
        id = juce::Uuid().toString();
    } while (hasConfig(id));
    
    return id;
}

void AutomationConfigManager::notifySaveRequired()
{
    // Auto-save after a short delay to batch multiple changes
    juce::Timer::callAfterDelay(1000, [this]() {
        saveToFile();
    });
}