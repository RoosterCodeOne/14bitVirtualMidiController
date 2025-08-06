#pragma once
#include <JuceHeader.h>
#include "LearnZoneTypes.h"
#include "AutomationControlPanel.h"
#include "../CustomLookAndFeel.h"

//==============================================================================
/**
 * SliderLearnZones - Manages 5 distinct learn zones per slider:
 * 1. Slider Track Zone
 * 2. GO Button Zone  
 * 3. Delay Knob Zone
 * 4. Attack Knob Zone
 * 5. Return Knob Zone
 * 6. Curve Knob Zone
 */
class SliderLearnZones : public juce::Component
{
public:
    SliderLearnZones(int sliderIdx) : sliderIndex(sliderIdx)
    {
        setInterceptsMouseClicks(true, false);
        setVisible(false); // Hidden by default
        setAlwaysOnTop(true);
    }
    
    void createZones(juce::Rectangle<int> sliderTrackBounds, 
                    const AutomationControlPanel& automationPanel)
    {
        // Slider track zone - only the slider area
        sliderTrackZone = LearnZone(LearnZoneType::SliderTrack, sliderTrackBounds, 
                                   sliderIndex, MidiTargetType::SliderValue);
        
        // Get automation panel bounds to calculate offset
        auto automationPanelBounds = automationPanel.getBounds();
        
        // GO button zone - adjust bounds relative to slider coordinate system
        auto goButtonBounds = automationPanel.getGoButtonBounds();
        goButtonBounds.setPosition(goButtonBounds.getX() + automationPanelBounds.getX(),
                                  goButtonBounds.getY() + automationPanelBounds.getY());
        goButtonZone = LearnZone(LearnZoneType::AutomationGO, goButtonBounds,
                                sliderIndex, MidiTargetType::AutomationGO);
        
        // Individual knob zones - adjust bounds relative to slider coordinate system
        auto delayKnobBounds = automationPanel.getDelayKnobBounds();
        delayKnobBounds.setPosition(delayKnobBounds.getX() + automationPanelBounds.getX(),
                                   delayKnobBounds.getY() + automationPanelBounds.getY());
        delayKnobZone = LearnZone(LearnZoneType::AutomationDelay, delayKnobBounds,
                                 sliderIndex, MidiTargetType::AutomationDelay);
        
        auto attackKnobBounds = automationPanel.getAttackKnobBounds();
        attackKnobBounds.setPosition(attackKnobBounds.getX() + automationPanelBounds.getX(),
                                    attackKnobBounds.getY() + automationPanelBounds.getY());
        attackKnobZone = LearnZone(LearnZoneType::AutomationAttack, attackKnobBounds,
                                  sliderIndex, MidiTargetType::AutomationAttack);
        
        auto returnKnobBounds = automationPanel.getReturnKnobBounds();
        returnKnobBounds.setPosition(returnKnobBounds.getX() + automationPanelBounds.getX(),
                                    returnKnobBounds.getY() + automationPanelBounds.getY());
        returnKnobZone = LearnZone(LearnZoneType::AutomationReturn, returnKnobBounds,
                                  sliderIndex, MidiTargetType::AutomationReturn);
        
        auto curveKnobBounds = automationPanel.getCurveKnobBounds();
        curveKnobBounds.setPosition(curveKnobBounds.getX() + automationPanelBounds.getX(),
                                   curveKnobBounds.getY() + automationPanelBounds.getY());
        curveKnobZone = LearnZone(LearnZoneType::AutomationCurve, curveKnobBounds,
                                 sliderIndex, MidiTargetType::AutomationCurve);
        
        zonesCreated = true;
    }
    
    void updateZoneBounds(juce::Rectangle<int> sliderTrackBounds, 
                         const AutomationControlPanel& automationPanel)
    {
        if (!zonesCreated) return;
        
        // Get automation panel bounds to calculate offset
        auto automationPanelBounds = automationPanel.getBounds();
        
        // Update all zone bounds - adjust relative to slider coordinate system
        sliderTrackZone.bounds = sliderTrackBounds;
        
        auto goButtonBounds = automationPanel.getGoButtonBounds();
        goButtonBounds.setPosition(goButtonBounds.getX() + automationPanelBounds.getX(),
                                  goButtonBounds.getY() + automationPanelBounds.getY());
        goButtonZone.bounds = goButtonBounds;
        
        auto delayKnobBounds = automationPanel.getDelayKnobBounds();
        delayKnobBounds.setPosition(delayKnobBounds.getX() + automationPanelBounds.getX(),
                                   delayKnobBounds.getY() + automationPanelBounds.getY());
        delayKnobZone.bounds = delayKnobBounds;
        
        auto attackKnobBounds = automationPanel.getAttackKnobBounds();
        attackKnobBounds.setPosition(attackKnobBounds.getX() + automationPanelBounds.getX(),
                                    attackKnobBounds.getY() + automationPanelBounds.getY());
        attackKnobZone.bounds = attackKnobBounds;
        
        auto returnKnobBounds = automationPanel.getReturnKnobBounds();
        returnKnobBounds.setPosition(returnKnobBounds.getX() + automationPanelBounds.getX(),
                                    returnKnobBounds.getY() + automationPanelBounds.getY());
        returnKnobZone.bounds = returnKnobBounds;
        
        auto curveKnobBounds = automationPanel.getCurveKnobBounds();
        curveKnobBounds.setPosition(curveKnobBounds.getX() + automationPanelBounds.getX(),
                                   curveKnobBounds.getY() + automationPanelBounds.getY());
        curveKnobZone.bounds = curveKnobBounds;
    }
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        if (!isInLearnMode || !onZoneClicked || !zonesCreated) return;
        
        auto clickPoint = event.getPosition();
        
        // Check each zone in priority order (smallest to largest)
        if (delayKnobZone.contains(clickPoint))
        {
            onZoneClicked(delayKnobZone);
            currentActiveZone = &delayKnobZone;
        }
        else if (attackKnobZone.contains(clickPoint))
        {
            onZoneClicked(attackKnobZone);
            currentActiveZone = &attackKnobZone;
        }
        else if (returnKnobZone.contains(clickPoint))
        {
            onZoneClicked(returnKnobZone);
            currentActiveZone = &returnKnobZone;
        }
        else if (curveKnobZone.contains(clickPoint))
        {
            onZoneClicked(curveKnobZone);
            currentActiveZone = &curveKnobZone;
        }
        else if (goButtonZone.contains(clickPoint))
        {
            onZoneClicked(goButtonZone);
            currentActiveZone = &goButtonZone;
        }
        else if (sliderTrackZone.contains(clickPoint))
        {
            onZoneClicked(sliderTrackZone);
            currentActiveZone = &sliderTrackZone;
        }
        
        repaint();
    }
    
    void mouseMove(const juce::MouseEvent& event) override
    {
        if (!isInLearnMode || !zonesCreated) return;
        
        auto mousePoint = event.getPosition();
        LearnZone* newHoveredZone = nullptr;
        
        // Check which zone is being hovered (smallest to largest priority)
        if (delayKnobZone.contains(mousePoint))
            newHoveredZone = &delayKnobZone;
        else if (attackKnobZone.contains(mousePoint))
            newHoveredZone = &attackKnobZone;
        else if (returnKnobZone.contains(mousePoint))
            newHoveredZone = &returnKnobZone;
        else if (curveKnobZone.contains(mousePoint))
            newHoveredZone = &curveKnobZone;
        else if (goButtonZone.contains(mousePoint))
            newHoveredZone = &goButtonZone;
        else if (sliderTrackZone.contains(mousePoint))
            newHoveredZone = &sliderTrackZone;
        
        if (newHoveredZone != currentHoveredZone)
        {
            currentHoveredZone = newHoveredZone;
            repaint();
        }
    }
    
    void mouseExit(const juce::MouseEvent& event) override
    {
        currentHoveredZone = nullptr;
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        if (!isInLearnMode || !isVisible() || !zonesCreated) return;
        
        // Draw hover highlights for the currently hovered zone
        if (currentHoveredZone != nullptr)
        {
            drawZoneHighlight(g, *currentHoveredZone, true);
        }
        
        // Draw selection highlight for active zone
        if (currentActiveZone != nullptr)
        {
            drawZoneSelectionBrackets(g, *currentActiveZone);
        }
    }
    
    void setLearnModeActive(bool active)
    {
        isInLearnMode = active;
        setVisible(active);
        
        if (!active)
        {
            currentHoveredZone = nullptr;
            currentActiveZone = nullptr;
        }
        
        repaint();
    }
    
    bool getLearnModeActive() const { return isInLearnMode; }
    
    void clearActiveZone()
    {
        currentActiveZone = nullptr;
        repaint();
    }
    
    // Get specific zone bounds for external use
    juce::Rectangle<int> getSliderTrackZoneBounds() const { return sliderTrackZone.bounds; }
    juce::Rectangle<int> getGoButtonZoneBounds() const { return goButtonZone.bounds; }
    juce::Rectangle<int> getDelayKnobZoneBounds() const { return delayKnobZone.bounds; }
    juce::Rectangle<int> getAttackKnobZoneBounds() const { return attackKnobZone.bounds; }
    juce::Rectangle<int> getReturnKnobZoneBounds() const { return returnKnobZone.bounds; }
    juce::Rectangle<int> getCurveKnobZoneBounds() const { return curveKnobZone.bounds; }
    
    // Callbacks
    std::function<void(const LearnZone&)> onZoneClicked;
    
private:
    int sliderIndex;
    bool isInLearnMode = false;
    bool zonesCreated = false;
    
    // The 6 learn zones for this slider
    LearnZone sliderTrackZone;
    LearnZone goButtonZone;
    LearnZone delayKnobZone;
    LearnZone attackKnobZone;
    LearnZone returnKnobZone;
    LearnZone curveKnobZone;
    
    // Visual state
    LearnZone* currentHoveredZone = nullptr;
    LearnZone* currentActiveZone = nullptr;
    
    void drawZoneHighlight(juce::Graphics& g, const LearnZone& zone, bool isHovered)
    {
        auto bounds = zone.bounds.toFloat();
        juce::Colour highlightColor = BlueprintColors::warning; // Orange
        
        if (isHovered)
        {
            g.setColour(highlightColor.withAlpha(0.3f));
            g.fillRoundedRectangle(bounds, 2.0f);
        }
        
        g.setColour(highlightColor.withAlpha(isHovered ? 0.8f : 0.5f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 2.0f, isHovered ? 2.0f : 1.0f);
    }
    
    void drawZoneSelectionBrackets(juce::Graphics& g, const LearnZone& zone)
    {
        auto bounds = zone.bounds.toFloat();
        const float bracketLength = 8.0f;
        const float bracketThickness = 2.0f;
        const float cornerOffset = 2.0f;
        
        juce::Colour bracketColor = BlueprintColors::warning; // Orange brackets
        g.setColour(bracketColor);
        
        // Top-left bracket
        g.fillRect(bounds.getX() - cornerOffset, bounds.getY() - cornerOffset, 
                  bracketLength, bracketThickness); // Horizontal
        g.fillRect(bounds.getX() - cornerOffset, bounds.getY() - cornerOffset, 
                  bracketThickness, bracketLength); // Vertical
        
        // Top-right bracket
        g.fillRect(bounds.getRight() + cornerOffset - bracketLength, bounds.getY() - cornerOffset, 
                  bracketLength, bracketThickness); // Horizontal
        g.fillRect(bounds.getRight() + cornerOffset - bracketThickness, bounds.getY() - cornerOffset, 
                  bracketThickness, bracketLength); // Vertical
        
        // Bottom-left bracket
        g.fillRect(bounds.getX() - cornerOffset, bounds.getBottom() + cornerOffset - bracketThickness, 
                  bracketLength, bracketThickness); // Horizontal
        g.fillRect(bounds.getX() - cornerOffset, bounds.getBottom() + cornerOffset - bracketLength, 
                  bracketThickness, bracketLength); // Vertical
        
        // Bottom-right bracket
        g.fillRect(bounds.getRight() + cornerOffset - bracketLength, bounds.getBottom() + cornerOffset - bracketThickness, 
                  bracketLength, bracketThickness); // Horizontal
        g.fillRect(bounds.getRight() + cornerOffset - bracketThickness, bounds.getBottom() + cornerOffset - bracketLength, 
                  bracketThickness, bracketLength); // Vertical
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderLearnZones)
};