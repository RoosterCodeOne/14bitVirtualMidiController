// PresetManager.h - Simple Preset System for Virtual MIDI Controller
#pragma once
#include <JuceHeader.h>

//==============================================================================
struct SliderPreset
{
    int ccNumber = 0;
    double minRange = 0.0;
    double maxRange = 16383.0;
    int colorId = 1; // ComboBox selected ID
    double currentValue = 0.0;
    bool isLocked = false;
    double delayTime = 0.0;
    double attackTime = 1.0;
    double returnTime = 0.0;
    double curveValue = 1.0;
    int orientation = 0; // 0=Normal, 1=Inverted, 2=Bipolar
    double bipolarCenter = 8191.5; // Default center value
    
    juce::var toVar() const
    {
        auto obj = new juce::DynamicObject();
        obj->setProperty("ccNumber", ccNumber);
        obj->setProperty("minRange", minRange);
        obj->setProperty("maxRange", maxRange);
        obj->setProperty("colorId", colorId);
        obj->setProperty("currentValue", currentValue);
        obj->setProperty("isLocked", isLocked);
        obj->setProperty("delayTime", delayTime);
        obj->setProperty("attackTime", attackTime);
        obj->setProperty("returnTime", returnTime);
        obj->setProperty("curveValue", curveValue);
        obj->setProperty("orientation", orientation);
        obj->setProperty("bipolarCenter", bipolarCenter);

        return juce::var(obj);
    }
    
    void fromVar(const juce::var& data)
    {
        if (auto* obj = data.getDynamicObject())
        {
            ccNumber = obj->hasProperty("ccNumber") ? (int)obj->getProperty("ccNumber") : 0;
            minRange = obj->hasProperty("minRange") ? (double)obj->getProperty("minRange") : 0.0;
            maxRange = obj->hasProperty("maxRange") ? (double)obj->getProperty("maxRange") : 16383.0;
            colorId = obj->hasProperty("colorId") ? (int)obj->getProperty("colorId") : 1;
            currentValue = obj->hasProperty("currentValue") ? (double)obj->getProperty("currentValue") : 0.0;
            isLocked = obj->hasProperty("isLocked") ? (bool)obj->getProperty("isLocked") : false;
            delayTime = obj->hasProperty("delayTime") ? (double)obj->getProperty("delayTime") : 0.0;    
            attackTime = obj->hasProperty("attackTime") ? (double)obj->getProperty("attackTime") : 1.0;
            returnTime = obj->hasProperty("returnTime") ? (double)obj->getProperty("returnTime") : 0.0;
            curveValue = obj->hasProperty("curveValue") ? (double)obj->getProperty("curveValue") : 1.0;
            orientation = obj->hasProperty("orientation") ? (int)obj->getProperty("orientation") : 0;
            bipolarCenter = obj->hasProperty("bipolarCenter") ? (double)obj->getProperty("bipolarCenter") : 8191.5;

        }
    }
};

//==============================================================================
struct ControllerPreset
{
    juce::String name = "Untitled";
    int midiChannel = 1;
    double bpm = 120.0; // Default BPM
    juce::Array<SliderPreset> sliders;
    
    ControllerPreset()
    {
        // Initialize with 16 default sliders
        for (int i = 0; i < 16; ++i)
        {
            SliderPreset slider;
            slider.ccNumber = i;
            
            // Set default colors based on bank
            int bankIndex = i / 4;
            switch (bankIndex)
            {
                case 0: slider.colorId = 2; break; // Red
                case 1: slider.colorId = 3; break; // Blue
                case 2: slider.colorId = 4; break; // Green
                case 3: slider.colorId = 5; break; // Yellow
                default: slider.colorId = 1; break; // Default
            }
            
            sliders.add(slider);
        }
    }
    
    juce::var toVar() const
    {
        auto obj = new juce::DynamicObject();
        obj->setProperty("name", name);
        obj->setProperty("midiChannel", midiChannel);
        obj->setProperty("bpm", bpm);
        
        juce::Array<juce::var> sliderArray;
        for (const auto& slider : sliders)
            sliderArray.add(slider.toVar());
        obj->setProperty("sliders", sliderArray);
        
        return juce::var(obj);
    }
    
    void fromVar(const juce::var& data)
    {
        if (auto* obj = data.getDynamicObject())
        {
            name = obj->hasProperty("name") ? obj->getProperty("name").toString() : "Untitled";
            midiChannel = obj->hasProperty("midiChannel") ? (int)obj->getProperty("midiChannel") : 1;
            bpm = obj->hasProperty("bpm") ? (double)obj->getProperty("bpm") : 120.0;
            
            sliders.clear();
            if (obj->hasProperty("sliders"))
            {
                if (auto sliderArray = obj->getProperty("sliders").getArray())
                {
                    for (const auto& sliderVar : *sliderArray)
                    {
                        SliderPreset slider;
                        slider.fromVar(sliderVar);
                        sliders.add(slider);
                    }
                }
            }
            
            // Ensure we always have 16 sliders
            while (sliders.size() < 16)
            {
                SliderPreset slider;
                slider.ccNumber = sliders.size();
                
                // Set default colors based on bank for new sliders
                int bankIndex = sliders.size() / 4;
                switch (bankIndex)
                {
                    case 0: slider.colorId = 2; break; // Red
                    case 1: slider.colorId = 3; break; // Blue
                    case 2: slider.colorId = 4; break; // Green
                    case 3: slider.colorId = 5; break; // Yellow
                    default: slider.colorId = 1; break; // Default
                }
                
                sliders.add(slider);
            }
        }
    }
};

//==============================================================================
class PresetManager
{
public:
    PresetManager()
    {
        // Get app data directory for storing presets
        auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("VirtualMidiController");
        
        if (!appDataDir.exists())
            appDataDir.createDirectory();
        
        presetDirectory = appDataDir.getChildFile("Presets");
        if (!presetDirectory.exists())
            presetDirectory.createDirectory();
        
        autoSaveFile = appDataDir.getChildFile("current_state.json");
        
        // Load any existing preset files
        refreshPresetList();
    }
    
    // Save current state as auto-save
    bool autoSaveCurrentState(const ControllerPreset& preset)
    {
        auto json = juce::JSON::toString(preset.toVar());
        return autoSaveFile.replaceWithText(json);
    }
    
    // Load auto-saved state
    ControllerPreset loadAutoSavedState()
    {
        ControllerPreset preset;
        
        if (autoSaveFile.existsAsFile())
        {
            auto jsonText = autoSaveFile.loadFileAsString();
            auto jsonVar = juce::JSON::parse(jsonText);
            if (jsonVar.isObject())
                preset.fromVar(jsonVar);
        }
        
        return preset;
    }
    
    // Save preset to file
    bool savePreset(const ControllerPreset& preset, const juce::String& filename)
    {
        auto file = presetDirectory.getChildFile(filename + ".json");
        auto json = juce::JSON::toString(preset.toVar());
        
        if (file.replaceWithText(json))
        {
            refreshPresetList();
            return true;
        }
        return false;
    }
    
    // Load preset from file
    ControllerPreset loadPreset(const juce::String& filename)
    {
        ControllerPreset preset;
        auto file = presetDirectory.getChildFile(filename + ".json");
        
        if (file.existsAsFile())
        {
            auto jsonText = file.loadFileAsString();
            auto jsonVar = juce::JSON::parse(jsonText);
            if (jsonVar.isObject())
                preset.fromVar(jsonVar);
        }
        
        return preset;
    }
    
    // Get list of available presets
    juce::StringArray getPresetNames() const
    {
        return presetNames;
    }
    
    // Delete a preset
    bool deletePreset(const juce::String& filename)
    {
        auto file = presetDirectory.getChildFile(filename + ".json");
        if (file.deleteFile())
        {
            refreshPresetList();
            return true;
        }
        return false;
    }
    
    // Get current preset directory
    juce::File getPresetDirectory() const
    {
        return presetDirectory;
    }

    // Set new preset directory
    void setPresetDirectory(const juce::File& newDirectory)
    {
        presetDirectory = newDirectory;
        
        // Ensure the directory exists
        if (!presetDirectory.exists())
            presetDirectory.createDirectory();
            
        // Refresh the preset list for the new directory
        refreshPresetList();
    }
    
private:
    juce::File presetDirectory;
    juce::File autoSaveFile;
    juce::StringArray presetNames;
    
    void refreshPresetList()
    {
        presetNames.clear();
        
        if (presetDirectory.exists())
        {
            auto files = presetDirectory.findChildFiles(juce::File::findFiles, false, "*.json");
            for (const auto& file : files)
            {
                presetNames.add(file.getFileNameWithoutExtension());
            }
        }
        
        presetNames.sort(false);
    }
};
