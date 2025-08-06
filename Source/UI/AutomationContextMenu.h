#pragma once
#include <JuceHeader.h>
#include "../Core/AutomationConfigManager.h"

//==============================================================================
/**
 * AutomationContextMenu - Right-click context menu for automation areas
 * Provides save, load, copy/paste functionality for automation configurations
 */
class AutomationContextMenu : public juce::PopupMenu
{
public:
    // Menu item IDs
    enum MenuItems 
    {
        SaveConfig = 1,
        LoadConfigStart = 100,      // 100-199 reserved for config list
        LoadConfigEnd = 199,
        CopyConfig = 200,
        PasteConfig = 201,
        ManageConfigs = 202,
        Separator1 = 203,
        Separator2 = 204
    };
    
    AutomationContextMenu(AutomationConfigManager& configManager)
        : configManager(configManager)
    {
    }
    
    void showForSlider(int sliderIndex, juce::Point<int> position, juce::Component* parentComponent)
    {
        currentSliderIndex = sliderIndex;
        
        // Clear previous menu items
        clear();
        
        // Save current config
        addItem(SaveConfig, "Save Config As...");
        
        // Load config submenu
        juce::PopupMenu loadSubmenu;
        auto allConfigs = configManager.getAllConfigs();
        
        if (!allConfigs.empty())
        {
            for (size_t i = 0; i < allConfigs.size() && i < 99; ++i)
            {
                const auto& config = allConfigs[i];
                int itemId = LoadConfigStart + (int)i;
                
                // Add config info to menu text
                juce::String menuText = config.name;
                
                // Add time mode indicator
                if (config.timeMode == AutomationControlPanel::TimeMode::Beats)
                    menuText += " (Beats)";
                else
                    menuText += " (Sec)";
                
                // Add MIDI assignment indicator if any
                auto midiAssignments = configManager.getMidiAssignmentsForConfig(config.id);
                if (!midiAssignments.empty())
                {
                    menuText += " [MIDI]";
                }
                
                loadSubmenu.addItem(itemId, menuText);
                
                // Store config ID for later retrieval
                configIdMap[itemId] = config.id;
            }
        }
        else
        {
            loadSubmenu.addItem(-1, "No saved configs", false);
        }
        
        addSubMenu("Load Config", loadSubmenu);
        
        addSeparator();
        
        // Copy/Paste
        addItem(CopyConfig, "Copy Config");
        addItem(PasteConfig, "Paste Config", configManager.hasClipboardConfig());
        
        addSeparator();
        
        // Management
        addItem(ManageConfigs, "Manage Configs...");
        
        // Show menu at the clicked position (convert to global coordinates)
        auto globalPos = parentComponent->localPointToGlobal(position);
        
        #if JUCE_MODAL_LOOPS_PERMITTED
        int result = show(0, globalPos.x, globalPos.y);
        handleMenuResult(result);
        #else
        showMenuAsync(juce::PopupMenu::Options()
                         .withTargetScreenArea(juce::Rectangle<int>(globalPos.x, globalPos.y, 1, 1)),
                     [this](int result) { handleMenuResult(result); });
        #endif
    }
    
    // Callbacks for menu actions
    std::function<void(int sliderIndex)> onSaveConfig;
    std::function<void(int sliderIndex, const juce::String& configId)> onLoadConfig;
    std::function<void(int sliderIndex)> onCopyConfig;
    std::function<void(int sliderIndex)> onPasteConfig;
    std::function<void()> onManageConfigs;
    
private:
    AutomationConfigManager& configManager;
    int currentSliderIndex = -1;
    std::map<int, juce::String> configIdMap; // Menu item ID -> Config ID
    
    void handleMenuResult(int result)
    {
        if (result == 0) return; // User cancelled
        
        switch (result)
        {
            case SaveConfig:
                if (onSaveConfig)
                    onSaveConfig(currentSliderIndex);
                break;
                
            case CopyConfig:
                if (onCopyConfig)
                    onCopyConfig(currentSliderIndex);
                break;
                
            case PasteConfig:
                if (onPasteConfig)
                    onPasteConfig(currentSliderIndex);
                break;
                
            case ManageConfigs:
                if (onManageConfigs)
                    onManageConfigs();
                break;
                
            default:
                // Check if it's a load config item
                if (result >= LoadConfigStart && result <= LoadConfigEnd)
                {
                    auto it = configIdMap.find(result);
                    if (it != configIdMap.end() && onLoadConfig)
                    {
                        onLoadConfig(currentSliderIndex, it->second);
                    }
                }
                break;
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationContextMenu)
};