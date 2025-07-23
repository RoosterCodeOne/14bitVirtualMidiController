// MainControllerLayout.h - Layout management for main controller components
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"

//==============================================================================
class MainControllerLayout
{
public:
    struct LayoutBounds
    {
        juce::Rectangle<int> topArea;
        juce::Rectangle<int> contentArea;
        juce::Rectangle<int> tooltipArea;
    };
    
    struct Constants
    {
        static constexpr int SLIDER_PLATE_WIDTH = 110;
        static constexpr int SLIDER_GAP = 10;
        static constexpr int SETTINGS_PANEL_WIDTH = 350;
        static constexpr int TOP_AREA_HEIGHT = 50;
        static constexpr int TOOLTIP_HEIGHT = 25;
        static constexpr int VERTICAL_GAP = 10;
    };
    
    MainControllerLayout() = default;
    
    // Calculate main layout bounds based on current state
    LayoutBounds calculateLayoutBounds(const juce::Rectangle<int>& totalBounds,
                                     bool isEightSliderMode,
                                     bool isInSettingsMode,
                                     bool isInLearnMode) const
    {
        LayoutBounds bounds;
        
        const int contentAreaWidth = isEightSliderMode ? 970 : 490;
        
        // Calculate content area bounds
        int contentX = (isInSettingsMode || isInLearnMode) ? 
            Constants::SETTINGS_PANEL_WIDTH : 
            (totalBounds.getWidth() - contentAreaWidth) / 2;
        int contentY = Constants::TOP_AREA_HEIGHT + Constants::VERTICAL_GAP;
        int contentHeight = totalBounds.getHeight() - Constants::TOP_AREA_HEIGHT - 
                           Constants::TOOLTIP_HEIGHT - (2 * Constants::VERTICAL_GAP) + 8;
        
        bounds.contentArea = juce::Rectangle<int>(contentX, contentY, contentAreaWidth, contentHeight);
        
        // Calculate top area bounds
        if ((isInSettingsMode) || (isInLearnMode))
        {
            bounds.topArea = juce::Rectangle<int>(0, 0, totalBounds.getWidth(), Constants::TOP_AREA_HEIGHT);
        }
        else
        {
            bounds.topArea = juce::Rectangle<int>(contentX, 0, contentAreaWidth, Constants::TOP_AREA_HEIGHT);
        }
        
        // Calculate tooltip area
        bounds.tooltipArea = totalBounds.withHeight(Constants::TOOLTIP_HEIGHT).withBottomY(totalBounds.getBottom());
        
        return bounds;
    }
    
    // Layout sliders within content area  
    template<typename SliderType>
    void layoutSliders(juce::OwnedArray<SliderType>& sliderControls,
                      const juce::Rectangle<int>& contentArea,
                      int visibleSliderCount,
                      std::function<int(int)> getVisibleSliderIndex) const
    {
        // Calculate total width needed for the slider rack
        int totalSliderWidth = (visibleSliderCount * Constants::SLIDER_PLATE_WIDTH) + 
                              ((visibleSliderCount - 1) * Constants::SLIDER_GAP);
        
        // Center sliders within the provided content area
        int startX = contentArea.getX() + (contentArea.getWidth() - totalSliderWidth) / 2;
        
        // Set bounds for each visible slider
        for (int i = 0; i < visibleSliderCount; ++i)
        {
            int sliderIndex = getVisibleSliderIndex(i);
            if (sliderIndex < sliderControls.size())
            {
                int xPos = startX + (i * (Constants::SLIDER_PLATE_WIDTH + Constants::SLIDER_GAP));
                auto sliderBounds = juce::Rectangle<int>(xPos, contentArea.getY(), 
                                                        Constants::SLIDER_PLATE_WIDTH, contentArea.getHeight());
                sliderControls[sliderIndex]->setBounds(sliderBounds);
                sliderControls[sliderIndex]->repaint();
            }
        }
    }
    
    // Layout top area components (settings, learn, banks, mode)
    void layoutTopAreaComponents(const juce::Rectangle<int>& topAreaBounds,
                               juce::Component& settingsButton,
                               juce::Component& learnButton,
                               juce::Component& bankAButton,
                               juce::Component& bankBButton,
                               juce::Component& bankCButton,
                               juce::Component& bankDButton,
                               juce::Component& modeButton,
                               juce::Component& showingLabel) const
    {
        // Settings button - positioned on left within top area bounds
        int settingsButtonX = topAreaBounds.getX() + 10;
        int settingsButtonY = topAreaBounds.getY() + 23;
        settingsButton.setBounds(settingsButtonX, settingsButtonY, 100, 20);
        
        // Learn button - positioned closer to settings button
        int learnButtonX = settingsButtonX + 105;
        learnButton.setBounds(learnButtonX, settingsButtonY, 50, 20);
        
        // Bank buttons - positioned as 2x2 grid in top right of top area
        const int buttonWidth = 35;
        const int buttonHeight = 20;
        const int buttonSpacing = 5;
        const int rightMargin = 10;
        
        int gridWidth = (2 * buttonWidth) + buttonSpacing;
        int gridStartX = topAreaBounds.getRight() - rightMargin - gridWidth;
        int gridStartY = topAreaBounds.getY() + 3;
        
        // Top row: A and B buttons
        bankAButton.setBounds(gridStartX, gridStartY, buttonWidth, buttonHeight);
        bankBButton.setBounds(gridStartX + buttonWidth + buttonSpacing, gridStartY, buttonWidth, buttonHeight);
        
        // Bottom row: C and D buttons
        bankCButton.setBounds(gridStartX, gridStartY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
        bankDButton.setBounds(gridStartX + buttonWidth + buttonSpacing, gridStartY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
        
        // Mode button - positioned to the left of C bank button with "Showing:" label
        int showingLabelX = gridStartX - 85;
        int modeButtonX = gridStartX - 40;
        showingLabel.setBounds(showingLabelX, gridStartY + buttonHeight + buttonSpacing, 40, 20);
        modeButton.setBounds(modeButtonX, gridStartY + buttonHeight + buttonSpacing, 30, 20);
    }
    
    // Layout tooltip components at bottom
    void layoutTooltips(const juce::Rectangle<int>& totalArea,
                       juce::Component& movementSpeedLabel,
                       juce::Component& windowSizeLabel,
                       bool isInSettingsMode,
                       bool isInLearnMode,
                       bool isEightSliderMode) const
    {
        const int contentAreaWidth = isEightSliderMode ? 970 : 490;
        auto tooltipArea = totalArea.withHeight(Constants::TOOLTIP_HEIGHT).withBottomY(totalArea.getBottom());
        
        if ((isInSettingsMode) || (isInLearnMode))
        {
            // Settings or learn open: tooltips span remaining width after panel
            auto adjustedTooltipArea = tooltipArea.withTrimmedLeft(Constants::SETTINGS_PANEL_WIDTH);
            auto leftTooltip = adjustedTooltipArea.removeFromLeft(adjustedTooltipArea.getWidth() / 2);
            movementSpeedLabel.setBounds(leftTooltip);
            windowSizeLabel.setBounds(adjustedTooltipArea);
        }
        else
        {
            // Settings closed: tooltips positioned within content area bounds
            int contentAreaX = (totalArea.getWidth() - contentAreaWidth) / 2;
            auto contentTooltipArea = tooltipArea.withX(contentAreaX).withWidth(contentAreaWidth);
            auto leftTooltip = contentTooltipArea.removeFromLeft(contentTooltipArea.getWidth() / 2);
            movementSpeedLabel.setBounds(leftTooltip);
            windowSizeLabel.setBounds(contentTooltipArea);
        }
    }
    
    // Draw blueprint grid overlay for content area
    void drawBlueprintGrid(juce::Graphics& g, const juce::Rectangle<int>& contentAreaBounds) const
    {
        CustomSliderLookAndFeel lookAndFeelGrid;
        lookAndFeelGrid.drawBlueprintGrid(g, contentAreaBounds);
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainControllerLayout)
};