// CustomKnob.h - Blueprint Technical Drawing Style Knob
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

//==============================================================================
class CustomKnob : public juce::Component
{
public:
    enum KnobSize
    {
        Large = 42,
        Medium = 35,  // 10% larger than Small (32 * 1.1 = 35.2, rounded to 35)
        Small = 32,
        Smaller = 28  // 10% smaller than Small (32 * 0.9 = 28.8, rounded to 28)
    };
    
    enum class TimeMode { Seconds, Beats };
    
    CustomKnob(const juce::String& labelText, double minValue = 0.0, double maxValue = 10.0, KnobSize size = Small)
        : label(labelText), minVal(minValue), maxVal(maxValue), knobSize(size), currentValue(minValue), timeMode(TimeMode::Seconds)
    {
        setSize(knobSize + 14, knobSize + 29); // Extra space for label and 2px bezel on each side (4px total + 4px more for height)
    }
    
    ~CustomKnob() = default;
    
    void setValue(double newValue)
    {
        newValue = juce::jlimit(minVal, maxVal, newValue);
        if (currentValue != newValue)
        {
            currentValue = newValue;
            repaint();
            if (onValueChanged)
                onValueChanged(currentValue);
        }
    }
    
    double getValue() const
    {
        return currentValue;
    }
    
    void setRange(double newMinValue, double newMaxValue)
    {
        minVal = newMinValue;
        maxVal = newMaxValue;
        setValue(currentValue); // Clamp to new range
    }
    
    std::function<void(double)> onValueChanged;
    
    void setTimeMode(TimeMode mode)
    {
        timeMode = mode;
        repaint(); // Refresh display when hovering
    }
    
    TimeMode getTimeMode() const { return timeMode; }
    
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
        auto bounds = getLocalBounds();
        // Allocate enough space for knob + bezel (knobSize + 4 pixels)
        auto knobAreaHeight = knobSize + 4;
        auto knobAreaBounds = bounds.removeFromTop(knobAreaHeight);
        auto knobArea = knobAreaBounds.withSizeKeepingCentre(knobSize, knobSize);
        auto labelArea = bounds;
        
        drawKnobShadow(g, knobArea);
        drawKnobBezel(g, knobArea);
        drawKnobBody(g, knobArea);
        drawKnobIndicator(g, knobArea);
        drawLabel(g, labelArea);
    }
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        dragStartValue = currentValue;
        dragStartY = event.position.y;
    }
    
    void mouseDrag(const juce::MouseEvent& event) override
    {
        // Vertical drag sensitivity - 100 pixels for full range
        double dragDistance = dragStartY - event.position.y;
        double sensitivity = (maxVal - minVal) / 100.0;
        double newValue = dragStartValue + (dragDistance * sensitivity);
        
        setValue(newValue);
    }
    
    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        // Reset to default value based on knob type (determined by label)
        double defaultValue = 0.0; // Default for most knobs
        
        if (label.containsIgnoreCase("ATTACK"))
            defaultValue = 1.0; // Attack default: 1.0 seconds (fast attack)
        else if (label.containsIgnoreCase("CURVE") || label.containsIgnoreCase("SLOPE"))
            defaultValue = 1.0; // Curve/Slope default: 1.0 (linear)
        else
            defaultValue = 0.0; // Delay and Return default: 0.0 seconds
        
        // Clamp to valid range
        defaultValue = juce::jlimit(minVal, maxVal, defaultValue);
        
        // Set the value which will trigger the callback and repaint
        setValue(defaultValue);
    }
    
private:
    juce::String label;
    double minVal, maxVal, currentValue;
    int knobSize;
    double dragStartValue = 0.0;
    float dragStartY = 0.0f;
    bool isHovered = false;
    TimeMode timeMode;
    
    juce::String secondsToBeats(double seconds) const
    {
        // Beat conversion - independent scale using musical notation
        // 1/16, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16 (representing 16th notes to 16 bars)
        
        static const struct {
            double seconds;
            juce::String text;
        } beatValues[] = {
            {0.0, "1/16"},
            {0.5, "1/8"}, 
            {1.0, "1/4"},
            {2.0, "1/2"},
            {4.0, "1"},
            {8.0, "2"},
            {12.0, "4"},
            {16.0, "8"},
            {20.0, "16"}
        };
        
        // Find the closest beat value
        juce::String closestBeat = "1/16";
        double minDistance = std::abs(seconds - 0.0);
        
        for (const auto& beat : beatValues)
        {
            double distance = std::abs(seconds - beat.seconds);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestBeat = beat.text;
            }
        }
        
        return closestBeat;
    }
    
    void drawKnobShadow(juce::Graphics& g, juce::Rectangle<int> knobArea)
    {
        // No shadow for flat blueprint design
    }
    
    void drawKnobBezel(juce::Graphics& g, juce::Rectangle<int> knobArea)
    {
        // No bezel for flat blueprint design
    }
    
    void drawKnobBody(juce::Graphics& g, juce::Rectangle<int> knobArea)
    {
        auto knobBounds = knobArea.toFloat();
        
        // Flat circular body with blueprint styling
        g.setColour(BlueprintColors::panel);
        g.fillEllipse(knobBounds);
        
        // Technical outline
        g.setColour(BlueprintColors::blueprintLines);
        g.drawEllipse(knobBounds, 2.0f);
    }
    
    void drawKnobIndicator(juce::Graphics& g, juce::Rectangle<int> knobArea)
    {
        auto knobBounds = knobArea.toFloat();
        auto center = knobBounds.getCentre();
        
        // Calculate angle - same rotation logic as before
        double valueNormalized = (currentValue - minVal) / (maxVal - minVal);
        double angleDegrees = 135.0 + (valueNormalized * 270.0);
        
        if (angleDegrees >= 360.0)
            angleDegrees -= 360.0;
            
        double angleRadians = angleDegrees * juce::MathConstants<double>::pi / 180.0;
        
        // Technical indicator line - bright cyan
        float radius = knobBounds.getWidth() * 0.4f;
        float lineEndX = center.x + std::cos(angleRadians) * radius;
        float lineEndY = center.y + std::sin(angleRadians) * radius;
        
        g.setColour(BlueprintColors::active);
        juce::Line<float> indicatorLine(center.x, center.y, lineEndX, lineEndY);
        g.drawLine(indicatorLine, 3.0f);
        
        // Small circle at end of line for technical appearance
        g.fillEllipse(lineEndX - 2, lineEndY - 2, 4, 4);
    }
    
    void drawLabel(juce::Graphics& g, juce::Rectangle<int> labelArea)
    {
        // Blueprint-style text
        g.setColour(BlueprintColors::textPrimary);
        g.setFont(juce::FontOptions(9.0f));
        auto adjustedLabelArea = labelArea.translated(0, 1);
        
        if (isHovered)
        {
            // Show current value when hovered with cyan highlight
            g.setColour(BlueprintColors::active);
            juce::String valueText;
            
            if (timeMode == TimeMode::Beats)
            {
                // Show beat notation
                valueText = secondsToBeats(currentValue);
            }
            else
            {
                // Show seconds value
                if (std::abs(currentValue - std::round(currentValue)) < 0.01)
                    valueText = juce::String((int)std::round(currentValue));
                else
                    valueText = juce::String(currentValue, 1);
            }
            
            g.drawText(valueText, adjustedLabelArea, juce::Justification::centredTop);
        }
        else
        {
            // Show label when not hovered
            g.drawText(label, adjustedLabelArea, juce::Justification::centredTop);
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomKnob)
};