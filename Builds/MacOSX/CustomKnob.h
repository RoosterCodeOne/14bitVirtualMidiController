// CustomKnob.h - Professional Audio Knob Component
#pragma once
#include <JuceHeader.h>

//==============================================================================
class CustomKnob : public juce::Component
{
public:
    enum KnobSize
    {
        Large = 50,
        Small = 35
    };
    
    CustomKnob(const juce::String& labelText, double minValue = 0.0, double maxValue = 10.0, KnobSize size = Small)
        : label(labelText), minVal(minValue), maxVal(maxValue), knobSize(size), currentValue(minValue)
    {
        setSize(knobSize + 10, knobSize + 25); // Extra space for label
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
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        auto knobArea = bounds.removeFromTop(knobSize).withSizeKeepingCentre(knobSize, knobSize);
        auto labelArea = bounds;
        
        drawKnobShadow(g, knobArea);
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
    
private:
    juce::String label;
    double minVal, maxVal, currentValue;
    int knobSize;
    double dragStartValue = 0.0;
    float dragStartY = 0.0f;
    
    void drawKnobShadow(juce::Graphics& g, juce::Rectangle<int> knobArea)
    {
        // Drop shadow - 2px offset, semi-transparent black
        auto shadowArea = knobArea.translated(2, 2).toFloat();
        g.setColour(juce::Colour(0x40000000));
        g.fillEllipse(shadowArea);
    }
    
    void drawKnobBody(juce::Graphics& g, juce::Rectangle<int> knobArea)
    {
        auto knobBounds = knobArea.toFloat();
        
        // Metallic gradient (light gray to darker gray)
        juce::ColourGradient knobGradient(
            juce::Colour(0xFFE8E8E8), knobBounds.getTopLeft(),
            juce::Colour(0xFF808080), knobBounds.getBottomRight(),
            false
        );
        knobGradient.addColour(0.3f, juce::Colour(0xFFD0D0D0));
        knobGradient.addColour(0.7f, juce::Colour(0xFFA0A0A0));
        
        g.setGradientFill(knobGradient);
        g.fillEllipse(knobBounds);
        
        // 3D beveled edge effect - outer rim
        g.setColour(juce::Colour(0xFF606060));
        g.drawEllipse(knobBounds, 1.0f);
        
        // Inner highlight for 3D effect
        auto innerBounds = knobBounds.reduced(2.0f);
        g.setColour(juce::Colour(0x60FFFFFF));
        g.drawEllipse(innerBounds, 1.0f);
        
        // Center raised area
        auto centerBounds = knobBounds.reduced(knobSize * 0.2f);
        juce::ColourGradient centerGradient(
            juce::Colour(0xFFF0F0F0), centerBounds.getTopLeft(),
            juce::Colour(0xFFB0B0B0), centerBounds.getBottomRight(),
            false
        );
        g.setGradientFill(centerGradient);
        g.fillEllipse(centerBounds);
        
        // Center rim
        g.setColour(juce::Colour(0xFF808080));
        g.drawEllipse(centerBounds, 0.5f);
    }
    
    void drawKnobIndicator(juce::Graphics& g, juce::Rectangle<int> knobArea)
    {
        auto knobBounds = knobArea.toFloat();
        auto center = knobBounds.getCentre();
        
        // Calculate angle based on current value (270 degrees of rotation)
        double valueNormalized = (currentValue - minVal) / (maxVal - minVal);
        double angleRadians = juce::MathConstants<double>::pi * 1.35 * (valueNormalized - 0.5) + juce::MathConstants<double>::pi * 0.5;
        
        // Indicator line from center towards edge (like clock hand)
        float radius = knobBounds.getWidth() * 0.35f;
        float lineEndX = center.x + std::cos(angleRadians) * radius;
        float lineEndY = center.y + std::sin(angleRadians) * radius;
        
        // Draw indicator line (2px wide)
        g.setColour(juce::Colour(0xFF404040));
        juce::Line<float> indicatorLine(center.x, center.y, lineEndX, lineEndY);
        g.drawLine(indicatorLine, 2.0f);
        
        // Small dot at the end of the line
        g.fillEllipse(lineEndX - 2, lineEndY - 2, 4, 4);
    }
    
    void drawLabel(juce::Graphics& g, juce::Rectangle<int> labelArea)
    {
        // Text label underneath the knob, centered (11pt font)
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(11.0f));
        g.drawText(label, labelArea, juce::Justification::centredTop);
        
        // Draw current value below label
        juce::String valueText;
        if (std::abs(currentValue - std::round(currentValue)) < 0.01)
            valueText = juce::String((int)std::round(currentValue));
        else
            valueText = juce::String(currentValue, 1);
            
        g.setFont(juce::FontOptions(9.0f));
        g.setColour(juce::Colours::lightgrey);
        auto valueArea = labelArea.removeFromBottom(12);
        g.drawText(valueText, valueArea, juce::Justification::centred);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomKnob)
};