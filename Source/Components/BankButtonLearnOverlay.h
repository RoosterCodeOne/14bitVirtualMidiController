#pragma once
#include <JuceHeader.h>
#include "LearnModeOverlay.h"

//==============================================================================
/**
 * BankButtonLearnOverlay - Learn mode overlay specifically for bank buttons
 */
class BankButtonLearnOverlay : public LearnModeOverlay
{
public:
    BankButtonLearnOverlay() : LearnModeOverlay()
    {
        setTargetInfo(MidiTargetType::BankCycle, -1);
    }
    
    void paint(juce::Graphics& g) override
    {
        if (!getEnabled() || !isVisible()) return;
        
        auto bounds = getLocalBounds().toFloat();
        
        // Bank cycling specific highlight
        juce::Colour highlightColor = BlueprintColors::active;
        if (isHovered)
        {
            // Brighter when hovered
            g.setColour(highlightColor.withAlpha(0.4f));
            g.fillRoundedRectangle(bounds, 4.0f);
        }
        
        // Learn mode border with bank-specific styling
        g.setColour(highlightColor.withAlpha(isHovered ? 0.9f : 0.6f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 4.0f, isHovered ? 2.5f : 1.5f);
        
        // Draw bank cycling icon
        if (bounds.getWidth() > 40 && bounds.getHeight() > 20)
        {
            g.setColour(highlightColor);
            g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
            g.drawText("BANK\nLEARN", bounds, juce::Justification::centred);
        }
        else if (bounds.getWidth() > 20)
        {
            g.setColour(highlightColor);
            g.setFont(juce::FontOptions(8.0f, juce::Font::bold));
            g.drawText("LEARN", bounds, juce::Justification::centred);
        }
    }
    
    void mouseEnter(const juce::MouseEvent& event) override
    {
        isHovered = true;
        // Note: setTooltip not available on base Component class in this JUCE version
        repaint();
    }
    
    void mouseExit(const juce::MouseEvent& event) override
    {
        isHovered = false;
        repaint();
    }
    
private:
    bool isHovered = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BankButtonLearnOverlay)
};