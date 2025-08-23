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
        Separator2 = 204,
        ResetAutomation = 205
    };
    
    AutomationContextMenu(AutomationConfigManager& configManager)
        : configManager(configManager), currentSliderIndex(-1)
    {
        DBG("AutomationContextMenu created");
    }
    
    ~AutomationContextMenu()
    {
        DBG("AutomationContextMenu destroyed, last slider index was: " + juce::String(currentSliderIndex));
    }
    
    void showForSlider(int sliderIndex, juce::Point<int> position, juce::Component* parentComponent, std::shared_ptr<AutomationContextMenu> self = nullptr)
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
                
                // MIDI assignments not implemented in current system
                // Future enhancement: add MIDI assignment indicators
                
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
        
        // Reset automation
        addItem(ResetAutomation, "Reset Automation");
        
        addSeparator();
        
        // Management
        addItem(ManageConfigs, "Manage Configs...");
        
        // Show menu at the clicked position (convert to global coordinates)
        auto globalPos = parentComponent->localPointToGlobal(position);
        
        #if JUCE_MODAL_LOOPS_PERMITTED
        int result = show(0, globalPos.x, globalPos.y);
        handleMenuResult(result);
        // In synchronous mode, shared_ptr will handle cleanup automatically
        #else
        showMenuAsync(juce::PopupMenu::Options()
                         .withTargetScreenArea(juce::Rectangle<int>(globalPos.x, globalPos.y, 1, 1)),
                     [this, self](int result) { 
                         handleMenuResult(result); 
                         // self shared_ptr keeps this object alive until callback completes
                     });
        #endif
    }
    
    // Callbacks for menu actions
    std::function<void(int sliderIndex)> onSaveConfig;
    std::function<void(int sliderIndex, const juce::String& configId)> onLoadConfig;
    std::function<void(int sliderIndex)> onCopyConfig;
    std::function<void(int sliderIndex)> onPasteConfig;
    std::function<void(int sliderIndex)> onResetAutomation;
    std::function<void()> onManageConfigs;
    
private:
    AutomationConfigManager& configManager;
    int currentSliderIndex = -1;
    std::map<int, juce::String> configIdMap; // Menu item ID -> Config ID
    
    void handleMenuResult(int result)
    {
        try {
            if (result == 0) return; // User cancelled
            
            // Validate slider index
            if (currentSliderIndex < 0 || currentSliderIndex >= 16) {
                DBG("ERROR: Invalid slider index in handleMenuResult: " + juce::String(currentSliderIndex));
                return;
            }
            
            switch (result)
            {
                case SaveConfig:
                    if (onSaveConfig) {
                        DBG("Calling onSaveConfig for slider " + juce::String(currentSliderIndex));
                        onSaveConfig(currentSliderIndex);
                    } else {
                        DBG("ERROR: onSaveConfig callback is null");
                    }
                    break;
                    
                case CopyConfig:
                    if (onCopyConfig) {
                        DBG("Calling onCopyConfig for slider " + juce::String(currentSliderIndex));
                        onCopyConfig(currentSliderIndex);
                    } else {
                        DBG("ERROR: onCopyConfig callback is null");
                    }
                    break;
                    
                case PasteConfig:
                    if (onPasteConfig) {
                        DBG("Calling onPasteConfig for slider " + juce::String(currentSliderIndex));
                        onPasteConfig(currentSliderIndex);
                    } else {
                        DBG("ERROR: onPasteConfig callback is null");
                    }
                    break;
                    
                case ResetAutomation:
                    if (onResetAutomation) {
                        DBG("Calling onResetAutomation for slider " + juce::String(currentSliderIndex));
                        onResetAutomation(currentSliderIndex);
                    } else {
                        DBG("ERROR: onResetAutomation callback is null");
                    }
                    break;
                    
                case ManageConfigs:
                    if (onManageConfigs) {
                        DBG("Calling onManageConfigs");
                        onManageConfigs();
                    } else {
                        DBG("ERROR: onManageConfigs callback is null");
                    }
                    break;
                    
                default:
                    // Check if it's a load config item
                    if (result >= LoadConfigStart && result <= LoadConfigEnd)
                    {
                        auto it = configIdMap.find(result);
                        if (it != configIdMap.end() && onLoadConfig)
                        {
                            if (!it->second.isEmpty()) {
                                DBG("Calling onLoadConfig for slider " + juce::String(currentSliderIndex) + " with config: " + it->second);
                                onLoadConfig(currentSliderIndex, it->second);
                            } else {
                                DBG("ERROR: Config ID is empty for menu item " + juce::String(result));
                            }
                        } else {
                            DBG("ERROR: Load config callback not found or null for menu item " + juce::String(result));
                        }
                    } else {
                        DBG("ERROR: Unknown menu result: " + juce::String(result));
                    }
                    break;
            }
        }
        catch (const std::exception& e) {
            DBG("EXCEPTION in handleMenuResult: " + juce::String(e.what()));
        }
        catch (...) {
            DBG("UNKNOWN EXCEPTION in handleMenuResult");
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationContextMenu)
};