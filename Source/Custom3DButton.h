// Custom3DButton.h - Blueprint Technical Drawing Style Button
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

//==============================================================================
class Custom3DButton : public juce::Button
{
public:
    Custom3DButton(const juce::String& buttonText = "GO") : juce::Button(buttonText)
    {
        setSize(35, 25);
        setButtonText(buttonText);
    }
    
    ~Custom3DButton() = default;
    
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
        auto bounds = getLocalBounds().toFloat();
        
        // Blueprint-style flat button
        g.setColour(shouldDrawButtonAsDown ? BlueprintColors::active.darker(0.3f) : BlueprintColors::panel);
        g.fillRoundedRectangle(bounds, 2.0f);
        
        // Technical outline - thicker when pressed/highlighted
        float lineWidth = (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted) ? 2.0f : 1.0f;
        g.setColour(shouldDrawButtonAsHighlighted ? BlueprintColors::active : BlueprintColors::blueprintLines);
        g.drawRoundedRectangle(bounds, 2.0f, lineWidth);
        
        // Draw button text
        drawButtonText(g, bounds, shouldDrawButtonAsDown, shouldDrawButtonAsHighlighted);
    }
    
private:
    void drawButtonText(juce::Graphics& g, juce::Rectangle<float> bounds, bool isPressed, bool isHighlighted)
    {
        auto text = getButtonText();
        if (text.isEmpty())
            return;
            
        // Blueprint-style text
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Custom3DButton)
};