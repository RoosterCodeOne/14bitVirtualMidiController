#pragma once
#include <JuceHeader.h>
#include "../Core/Midi7BitController.h"
#include "../CustomLookAndFeel.h"

//==============================================================================
/**
 * LearnModeOverlay - Invisible overlay that captures clicks during learn mode
 * Provides visual feedback and routes clicks to learn mode handler
 */
class LearnModeOverlay : public juce::Component
{
public:
    LearnModeOverlay()
    {
        setInterceptsMouseClicks(true, false);
        setVisible(false); // Hidden by default
        setAlwaysOnTop(true);
    }
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        if (onTargetClicked && isEnabled)
        {
            onTargetClicked(currentTargetType, targetSliderIndex);
        }
    }
    
    void mouseEnter(const juce::MouseEvent& event) override
    {
        isHovered = true;
        updateTooltip();
        repaint();
    }
    
    void mouseExit(const juce::MouseEvent& event) override
    {
        isHovered = false;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        if (!isEnabled || !isVisible()) return;
        
        auto bounds = getLocalBounds().toFloat();
        
        // Learn mode highlight
        juce::Colour highlightColor = BlueprintColors::active();
        if (isHovered)
        {
            // Brighter when hovered
            g.setColour(highlightColor.withAlpha(0.3f));
            g.fillRoundedRectangle(bounds, 2.0f);
        }
        
        // Learn mode border
        g.setColour(highlightColor.withAlpha(isHovered ? 0.8f : 0.5f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 2.0f, isHovered ? 2.0f : 1.0f);
        
        // Draw learn icon or text
        if (bounds.getWidth() > 30 && bounds.getHeight() > 20)
        {
            g.setColour(highlightColor);
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.drawText("LEARN", bounds, juce::Justification::centred);
        }
    }
    
    void setTargetInfo(MidiTargetType targetType, int sliderIndex = -1)
    {
        currentTargetType = targetType;
        targetSliderIndex = sliderIndex;
        
        // Update tooltip based on target type
        updateTooltip();
    }
    
    void setEnabled(bool enabled)
    {
        isEnabled = enabled;
        setVisible(enabled);
        repaint();
    }
    
    bool getEnabled() const { return isEnabled; }
    
    void setLearnModeActive(bool active)
    {
        setEnabled(active);
    }
    
    // Callbacks
    std::function<void(MidiTargetType, int)> onTargetClicked;
    
private:
    MidiTargetType currentTargetType = MidiTargetType::SliderValue;
    int targetSliderIndex = -1;
    bool isEnabled = false;
    bool isHovered = false;
    
    void updateTooltip()
    {
        juce::String tooltipText;
        
        switch (currentTargetType)
        {
            case MidiTargetType::SliderValue:
                tooltipText = "Click to assign slider " + juce::String(targetSliderIndex + 1) + " control";
                break;
            case MidiTargetType::BankCycle:
                tooltipText = "Click to assign bank cycling";
                break;
            case MidiTargetType::AutomationGO:
                tooltipText = "Click to assign automation toggle for slider " + juce::String(targetSliderIndex + 1);
                break;
            case MidiTargetType::AutomationDelay:
                tooltipText = "Click to assign delay knob for slider " + juce::String(targetSliderIndex + 1);
                break;
            case MidiTargetType::AutomationAttack:
                tooltipText = "Click to assign attack knob for slider " + juce::String(targetSliderIndex + 1);
                break;
            case MidiTargetType::AutomationReturn:
                tooltipText = "Click to assign return knob for slider " + juce::String(targetSliderIndex + 1);
                break;
            case MidiTargetType::AutomationCurve:
                tooltipText = "Click to assign curve knob for slider " + juce::String(targetSliderIndex + 1);
                break;
            case MidiTargetType::AutomationConfig:
                tooltipText = "Click to assign automation config trigger";
                break;
            default:
                tooltipText = "Click to assign MIDI control";
                break;
        }
        
        // Note: setTooltip not available on base Component class in this JUCE version
        // Tooltip functionality handled through mouse events
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LearnModeOverlay)
};