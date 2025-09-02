// WindowManager.h - Window and constraint management for settings and learn modes
#pragma once
#include <JuceHeader.h>
#include "GlobalUIScale.h"

// Forward declarations
class SettingsWindow;

//==============================================================================
class WindowManager
{
public:
    WindowManager() = default;
    
    // Update window constraints based on current mode
    void updateWindowConstraints(juce::Component* topLevelComponent,
                                bool isEightSliderMode,
                                bool isInSettingsMode,
                                bool isInLearnMode,
                                int settingsPanelWidth) const
    {
        auto& scale = GlobalUIScale::getInstance();
        
        if (auto* documentWindow = dynamic_cast<juce::DocumentWindow*>(topLevelComponent))
        {
            if (auto* constrainer = documentWindow->getConstrainer())
            {
                // Fixed window widths based on mode - now scale-aware
                int fixedWidth = isEightSliderMode ? scale.getScaled(970) : scale.getScaled(490);
                
                if (isInSettingsMode || isInLearnMode)
                {
                    // Add panel width to fixed width (panel width is already scaled from MainControllerLayout)
                    fixedWidth += settingsPanelWidth;
                }
                
                // FIXED: Scale-aware height constraints
                int optimalHeight = scale.getScaled(660);
                int minHeight = scale.getScaled(580);  // Minimum functional height
                int maxHeight = scale.getScaled(800);  // Maximum reasonable height
                
                // Set both min and max to the same value to prevent resizing
                constrainer->setMinimumWidth(fixedWidth);
                constrainer->setMaximumWidth(fixedWidth);
                
                // Set height constraints with scaling support
                constrainer->setMinimumHeight(minHeight);
                constrainer->setMaximumHeight(maxHeight);
            }
        }
    }
    
    // Handle settings window toggle
    void toggleSettingsWindow(juce::Component* topLevelComponent,
                            juce::Component& settingsWindow,
                            juce::Component* learnWindow,
                            bool& isInSettingsMode,
                            bool& isInLearnMode,
                            bool isEightSliderMode,
                            int settingsPanelWidth,
                            std::function<void()> onLearnModeExit = nullptr,
                            std::function<int()> getCurrentBank = nullptr) const
    {
        // Close learn mode if it's open
        if (isInLearnMode && onLearnModeExit)
        {
            onLearnModeExit();
        }
        
        isInSettingsMode = !isInSettingsMode;
        
        // Update constraints BEFORE resizing
        updateWindowConstraints(topLevelComponent, isEightSliderMode, isInSettingsMode, false, settingsPanelWidth);
        
        if (topLevelComponent)
        {
            auto& scale = GlobalUIScale::getInstance();
            // Calculate target window dimensions with proper height scaling
            int contentAreaWidth = isEightSliderMode ? scale.getScaled(970) : scale.getScaled(490);
            int targetWidth = isInSettingsMode ? (contentAreaWidth + settingsPanelWidth) : contentAreaWidth;
            int targetHeight = scale.getScaled(660);  // Optimal scaled height
            
            // Resize window instantly with both width and height
            topLevelComponent->setSize(targetWidth, targetHeight);
            
            if (isInSettingsMode)
            {
                // Show settings window
                settingsWindow.setVisible(true);
                settingsWindow.toFront(true);
                
                // Sync bank selection when settings window is first shown
                if (getCurrentBank)
                {
                    int currentBank = getCurrentBank();
                    // Cast to SettingsWindow to access updateBankSelection method
                    if (auto* settings = dynamic_cast<SettingsWindow*>(&settingsWindow))
                    {
                        settings->updateBankSelection(currentBank);
                    }
                }
            }
            else
            {
                // Hide settings window
                settingsWindow.setVisible(false);
            }
        }
    }
    
    // Handle learn window toggle
    void toggleLearnWindow(juce::Component* topLevelComponent,
                          juce::Component& learnWindow,
                          juce::Component& settingsWindow,
                          bool& isInLearnMode,
                          bool& isInSettingsMode,
                          bool isEightSliderMode,
                          int settingsPanelWidth,
                          std::function<void()> onLearnModeEnter = nullptr,
                          std::function<void()> onLearnModeExit = nullptr) const
    {
        // Close settings mode if it's open
        if (isInSettingsMode)
        {
            isInSettingsMode = false;
            settingsWindow.setVisible(false);
        }
        
        isInLearnMode = !isInLearnMode;
        
        if (isInLearnMode)
        {
            // Enter learn mode
            if (onLearnModeEnter) onLearnModeEnter();
            
            // Show learn window
            updateWindowConstraints(topLevelComponent, isEightSliderMode, false, isInLearnMode, settingsPanelWidth);
            
            if (topLevelComponent)
            {
                auto& scale = GlobalUIScale::getInstance();
                int contentAreaWidth = isEightSliderMode ? scale.getScaled(970) : scale.getScaled(490);
                int targetWidth = contentAreaWidth + settingsPanelWidth;
                topLevelComponent->setSize(targetWidth, topLevelComponent->getHeight());
                
                learnWindow.setVisible(true);
                learnWindow.toFront(true);
            }
        }
        else
        {
            // Exit learn mode
            if (onLearnModeExit) onLearnModeExit();
            
            learnWindow.setVisible(false);
            
            // Resize window back
            updateWindowConstraints(topLevelComponent, isEightSliderMode, false, false, settingsPanelWidth);
            if (topLevelComponent)
            {
                auto& scale = GlobalUIScale::getInstance();
                int contentAreaWidth = isEightSliderMode ? scale.getScaled(970) : scale.getScaled(490);
                topLevelComponent->setSize(contentAreaWidth, topLevelComponent->getHeight());
            }
        }
    }
    
    // Position settings or learn window
    void positionSideWindow(juce::Component& window,
                           const juce::Rectangle<int>& totalBounds,
                           int topAreaHeight,
                           int settingsPanelWidth) const
    {
        int windowY = topAreaHeight;
        int windowHeight = totalBounds.getHeight() - windowY;
        window.setBounds(0, windowY, settingsPanelWidth, windowHeight);
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WindowManager)
};