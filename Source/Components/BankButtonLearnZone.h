#pragma once
#include <JuceHeader.h>
#include "LearnZoneTypes.h"
#include "../CustomLookAndFeel.h"

//==============================================================================
/**
 * BankButtonLearnZone - Single learn zone covering entire bank button grid
 * Always available in learn mode for bank cycling assignment
 */
class BankButtonLearnZone : public juce::Component
{
public:
    BankButtonLearnZone()
    {
        setInterceptsMouseClicks(true, false);
        setVisible(false); // Hidden by default
        setAlwaysOnTop(true);
    }
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        if (isInLearnMode && onZoneClicked)
        {
            LearnZone zone(LearnZoneType::BankButtons, getBounds(), -1, MidiTargetType::BankCycle);
            onZoneClicked(zone);
        }
    }
    
    void mouseEnter(const juce::MouseEvent& event) override
    {
        isHovered = true;
        repaint();
    }
    
    void mouseExit(const juce::MouseEvent& event) override
    {
        isHovered = false;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        if (!isInLearnMode || !isVisible()) return;
        
        auto bounds = getLocalBounds().toFloat();
        
        // Bank cycling specific highlight
        juce::Colour highlightColor = BlueprintColors::warning(); // Orange color
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
        if (bounds.getWidth() > 60 && bounds.getHeight() > 30)
        {
            g.setColour(highlightColor);
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.drawText("BANK\\nLEARN", bounds, juce::Justification::centred);
        }
        else if (bounds.getWidth() > 30)
        {
            g.setColour(highlightColor);
            g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
            g.drawText("LEARN", bounds, juce::Justification::centred);
        }
    }
    
    void setLearnModeActive(bool active)
    {
        isInLearnMode = active;
        setVisible(active);
        repaint();
    }
    
    bool getLearnModeActive() const { return isInLearnMode; }
    
    // Callbacks
    std::function<void(const LearnZone&)> onZoneClicked;
    
private:
    bool isInLearnMode = false;
    bool isHovered = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BankButtonLearnZone)
};