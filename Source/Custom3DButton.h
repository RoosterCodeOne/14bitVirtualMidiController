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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Custom3DButton)
};