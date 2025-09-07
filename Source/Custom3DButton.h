// Custom3DButton.h - Blueprint Technical Drawing Style Button
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "UI/GlobalUIScale.h"

//==============================================================================
class Custom3DButton : public juce::Button, public GlobalUIScale::ScaleChangeListener
{
public:
    Custom3DButton(const juce::String& buttonText = "GO") : juce::Button(buttonText), isSelected(false)
    {
        auto& scale = GlobalUIScale::getInstance();
        setSize(scale.getScaled(35), scale.getScaled(25));
        setButtonText(buttonText);
        
        // Register for scale change notifications
        scale.addScaleChangeListener(this);
    }
    
    ~Custom3DButton()
    {
        // Unregister from scale change notifications
        GlobalUIScale::getInstance().removeScaleChangeListener(this);
    }
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        // CRITICAL: Check for right-click FIRST, before any button processing
        if (event.mods.isRightButtonDown())
        {
            // Forward right-clicks to AutomationControlPanel for context menu
            forwardRightClickToAutomationPanel(event);
            return; // Don't process as button interaction
        }
        
        // Only handle left-clicks for normal button interaction
        if (event.mods.isLeftButtonDown())
        {
            // Call parent Button class implementation for normal button behavior
            juce::Button::mouseDown(event);
        }
    }
    
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto& scale = GlobalUIScale::getInstance();
        auto bounds = getLocalBounds().toFloat();
        
        // Blueprint-style flat button with scaled corner radius
        juce::Colour bgColor;
        if (shouldDrawButtonAsDown)
            bgColor = BlueprintColors::active.darker(0.3f);
        else if (isSelected)
            bgColor = BlueprintColors::active.withAlpha(0.7f);  // Same alpha as other active buttons
        else
            bgColor = BlueprintColors::panel;
            
        g.setColour(bgColor);
        g.fillRoundedRectangle(bounds, scale.getScaled(2.0f));
        
        // Technical outline - thicker when pressed/highlighted/selected - scaled line widths
        float lineWidth = (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted || isSelected) ? scale.getScaled(2.0f) : scale.getScaled(1.0f);
        juce::Colour outlineColor;
        if (shouldDrawButtonAsHighlighted)
            outlineColor = BlueprintColors::active;
        else if (isSelected)
            outlineColor = BlueprintColors::active.brighter(0.2f);
        else
            outlineColor = BlueprintColors::blueprintLines;
            
        g.setColour(outlineColor);
        g.drawRoundedRectangle(bounds, scale.getScaled(2.0f), lineWidth);
        
        // Draw button text
        drawButtonText(g, bounds, shouldDrawButtonAsDown, shouldDrawButtonAsHighlighted);
    }
    
    // Scale change notification implementation
    void scaleFactorChanged(float newScale) override
    {
        // Update button size for new scale - maintain default 35x25 ratio
        auto& scale = GlobalUIScale::getInstance();
        setSize(scale.getScaled(35), scale.getScaled(25));
        repaint();
    }
    
    // Selected state for highlighting when automation is active
    void setSelected(bool selected)
    {
        if (isSelected != selected)
        {
            isSelected = selected;
            repaint();
        }
    }
    
    bool getSelected() const { return isSelected; }
    
private:
    void drawButtonText(juce::Graphics& g, juce::Rectangle<float> bounds, bool isPressed, bool isHighlighted)
    {
        auto text = getButtonText();
        if (text.isEmpty())
            return;
            
        // Blueprint-style text with scaled font
        auto& scale = GlobalUIScale::getInstance();
        auto scaledFont = scale.getScaledFont(9.0f);
        scaledFont = scaledFont.boldened(); // Apply bold styling
        g.setFont(scaledFont);
        
        // Color based on state
        if (isHighlighted)
            g.setColour(BlueprintColors::active);
        else if (isPressed)
            g.setColour(BlueprintColors::textPrimary.darker(0.2f));
        else
            g.setColour(BlueprintColors::textPrimary);
            
        g.drawText(text, bounds, juce::Justification::centred);
    }
    
    // Helper method to forward right-clicks to automation panel parent
    void forwardRightClickToAutomationPanel(const juce::MouseEvent& event)
    {
        DBG("Custom3DButton: Forwarding right-click from position " + juce::String(event.getPosition().x) + ", " + juce::String(event.getPosition().y));
        
        // Since GO button is directly inside AutomationControlPanel, forward to direct parent
        if (auto* parent = getParentComponent())
        {
            DBG("Custom3DButton: Forwarding to parent component");
            // Convert coordinates to parent's coordinate system and forward the event
            auto parentEvent = event.getEventRelativeTo(parent);
            parent->mouseDown(parentEvent);
        }
        else
        {
            DBG("Custom3DButton: No parent found to forward right-click to!");
        }
    }
    
    bool isSelected;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Custom3DButton)
};