// MainControllerLayout.h - Layout management for main controller components
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"
#include "GlobalUIScale.h"

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
        static int getSliderPlateWidth() { return GlobalUIScale::getInstance().getScaled(110); }
        static int getSliderGap() { return GlobalUIScale::getInstance().getScaled(10); }
        static int getSettingsPanelWidth() { return GlobalUIScale::getInstance().getScaled(350); }
        static int getTopAreaHeight() { return GlobalUIScale::getInstance().getScaled(50); }
        static int getTooltipHeight() { return GlobalUIScale::getInstance().getScaled(25); }
        static int getVerticalGap() { return GlobalUIScale::getInstance().getScaled(10); }
    };
    
    MainControllerLayout() = default;
    
    // Calculate main layout bounds based on current state
    LayoutBounds calculateLayoutBounds(const juce::Rectangle<int>& totalBounds,
                                     bool isEightSliderMode,
                                     bool isInSettingsMode,
                                     bool isInLearnMode) const
    {
        LayoutBounds bounds;
        
        const int contentAreaWidth = isEightSliderMode ? GlobalUIScale::getInstance().getScaled(970) : GlobalUIScale::getInstance().getScaled(490);
        
        // Calculate content area bounds
        int contentX = (isInSettingsMode || isInLearnMode) ? 
            Constants::getSettingsPanelWidth() : 
            (totalBounds.getWidth() - contentAreaWidth) / 2;
        int contentY = Constants::getTopAreaHeight() + Constants::getVerticalGap();
        int contentHeight = totalBounds.getHeight() - Constants::getTopAreaHeight() - 
                           Constants::getTooltipHeight() - (2 * Constants::getVerticalGap()) + GlobalUIScale::getInstance().getScaled(8);
        
        bounds.contentArea = juce::Rectangle<int>(contentX, contentY, contentAreaWidth, contentHeight);
        
        // Calculate top area bounds
        if ((isInSettingsMode) || (isInLearnMode))
        {
            bounds.topArea = juce::Rectangle<int>(0, 0, totalBounds.getWidth(), Constants::getTopAreaHeight());
        }
        else
        {
            bounds.topArea = juce::Rectangle<int>(contentX, 0, contentAreaWidth, Constants::getTopAreaHeight());
        }
        
        // Calculate tooltip area
        bounds.tooltipArea = totalBounds.withHeight(Constants::getTooltipHeight()).withBottomY(totalBounds.getBottom());
        
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
        int totalSliderWidth = (visibleSliderCount * Constants::getSliderPlateWidth()) + 
                              ((visibleSliderCount - 1) * Constants::getSliderGap());
        
        // Center sliders within the provided content area
        int startX = contentArea.getX() + (contentArea.getWidth() - totalSliderWidth) / 2;
        
        // Set bounds for each visible slider
        for (int i = 0; i < visibleSliderCount; ++i)
        {
            int sliderIndex = getVisibleSliderIndex(i);
            if (sliderIndex < sliderControls.size())
            {
                int xPos = startX + (i * (Constants::getSliderPlateWidth() + Constants::getSliderGap()));
                auto sliderBounds = juce::Rectangle<int>(xPos, contentArea.getY(), 
                                                        Constants::getSliderPlateWidth(), contentArea.getHeight());
                sliderControls[sliderIndex]->setBounds(sliderBounds);
                sliderControls[sliderIndex]->repaint();
            }
        }
    }
    
    // Layout top area components (settings, learn, monitor, banks, mode)
    void layoutTopAreaComponents(const juce::Rectangle<int>& topAreaBounds,
                               juce::Component& settingsButton,
                               juce::Component& learnButton,
                               juce::Component& monitorButton,
                               juce::Component& bankAButton,
                               juce::Component& bankBButton,
                               juce::Component& bankCButton,
                               juce::Component& bankDButton,
                               juce::Component& modeButton,
                               juce::Component& showingLabel) const
    {
        auto& scale = GlobalUIScale::getInstance();
        
        // Settings button - positioned on left within top area bounds
        int settingsButtonX = topAreaBounds.getX() + scale.getScaled(10);
        int settingsButtonY = topAreaBounds.getY() + scale.getScaled(23);
        settingsButton.setBounds(settingsButtonX, settingsButtonY, scale.getScaled(75), scale.getScaled(20));
        
        // Learn button - positioned closer to settings button
        int learnButtonX = settingsButtonX + scale.getScaled(80);
        learnButton.setBounds(learnButtonX, settingsButtonY, scale.getScaled(45), scale.getScaled(20));
        
        // MIDI Monitor button - positioned next to Learn button
        int monitorButtonX = learnButtonX + scale.getScaled(50);
        monitorButton.setBounds(monitorButtonX, settingsButtonY, scale.getScaled(80), scale.getScaled(20));
        
        // Bank buttons - positioned as 2x2 grid in top right of top area
        const int buttonWidth = scale.getScaled(35);
        const int buttonHeight = scale.getScaled(20);
        const int buttonSpacing = scale.getScaled(5);
        const int rightMargin = scale.getScaled(10);
        
        int gridWidth = (2 * buttonWidth) + buttonSpacing;
        int gridStartX = topAreaBounds.getRight() - rightMargin - gridWidth;
        int gridStartY = topAreaBounds.getY() + scale.getScaled(3);
        
        // Top row: A and B buttons
        bankAButton.setBounds(gridStartX, gridStartY, buttonWidth, buttonHeight);
        bankBButton.setBounds(gridStartX + buttonWidth + buttonSpacing, gridStartY, buttonWidth, buttonHeight);
        
        // Bottom row: C and D buttons
        bankCButton.setBounds(gridStartX, gridStartY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
        bankDButton.setBounds(gridStartX + buttonWidth + buttonSpacing, gridStartY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
        
        // Mode button - positioned to the left of C bank button with "Showing:" label
        int showingLabelX = gridStartX - scale.getScaled(100);
        int modeButtonX = gridStartX - scale.getScaled(40);
        showingLabel.setBounds(showingLabelX, gridStartY + buttonHeight + buttonSpacing, scale.getScaled(55), scale.getScaled(20));
        modeButton.setBounds(modeButtonX, gridStartY + buttonHeight + buttonSpacing, scale.getScaled(30), scale.getScaled(20));
    }
    
    // Layout tooltip components at bottom
    void layoutTooltips(const juce::Rectangle<int>& totalArea,
                       juce::Component& actionTooltipLabel,
                       juce::Component& windowSizeLabel,
                       bool isInSettingsMode,
                       bool isInLearnMode,
                       bool isEightSliderMode) const
    {
        auto& scale = GlobalUIScale::getInstance();
        const int contentAreaWidth = isEightSliderMode ? scale.getScaled(970) : scale.getScaled(490);
        auto tooltipArea = totalArea.withHeight(Constants::getTooltipHeight()).withBottomY(totalArea.getBottom());
        
        if ((isInSettingsMode) || (isInLearnMode))
        {
            // Settings or learn open: tooltips span remaining width after panel
            auto adjustedTooltipArea = tooltipArea.withTrimmedLeft(Constants::getSettingsPanelWidth());
            auto leftTooltip = adjustedTooltipArea.removeFromLeft(adjustedTooltipArea.getWidth() / 2);
            actionTooltipLabel.setBounds(leftTooltip);
            windowSizeLabel.setBounds(adjustedTooltipArea);
        }
        else
        {
            // Settings closed: tooltips positioned within content area bounds
            int contentAreaX = (totalArea.getWidth() - contentAreaWidth) / 2;
            auto contentTooltipArea = tooltipArea.withX(contentAreaX).withWidth(contentAreaWidth);
            auto leftTooltip = contentTooltipArea.removeFromLeft(contentTooltipArea.getWidth() / 2);
            actionTooltipLabel.setBounds(leftTooltip);
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