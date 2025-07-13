// CustomKnob.h - Professional Audio Knob Component
#pragma once
#include <JuceHeader.h>

//==============================================================================
class CustomKnob : public juce::Component
{
public:
    enum KnobSize
    {
        Large = 40,
        Small = 28
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
        // Drop shadow - 2px offset down and right, soft black shadow
        auto shadowArea = knobArea.translated(2, 2).toFloat();
        g.setColour(juce::Colour(0x4D000000)); // Alpha 0.3 (77/255)
        g.fillEllipse(shadowArea);
    }
    
    void drawKnobBody(juce::Graphics& g, juce::Rectangle<int> knobArea)
    {
        auto knobBounds = knobArea.toFloat();
        auto center = knobBounds.getCentre();
        
        // Main knob body with matte finish - subtle radial gradient (lighter in center)
        juce::ColourGradient knobGradient(
            juce::Colour(0xFFF0F0F0), center, // Lighter center
            juce::Colour(0xFFE8E8E8), knobBounds.getTopLeft(), // Base matte gray
            true // Radial gradient
        );
        knobGradient.addColour(0.7f, juce::Colour(0xFFE8E8E8)); // Base color
        knobGradient.addColour(1.0f, juce::Colour(0xFFD8D8D8)); // Slightly darker edge
        
        g.setGradientFill(knobGradient);
        g.fillEllipse(knobBounds);
        
        // Outer ring with slightly darker gray for definition
        g.setColour(juce::Colour(0xFFD0D0D0));
        g.drawEllipse(knobBounds, 1.0f);
        
        // 3D bevel effects
        // Top highlight (upper arc)
        auto highlightBounds = knobBounds.reduced(1.0f);
        g.setColour(juce::Colour(0xFFF5F5F5)); // Very light gray
        juce::Path highlightPath;
        highlightPath.addArc(highlightBounds.getX(), highlightBounds.getY(), 
                           highlightBounds.getWidth(), highlightBounds.getHeight(),
                           juce::MathConstants<float>::pi * 1.2f, // Start angle (upper left)
                           juce::MathConstants<float>::pi * 1.8f, // End angle (upper right)
                           true);
        g.strokePath(highlightPath, juce::PathStrokeType(1.5f));
        
        // Bottom shadow (lower arc)
        g.setColour(juce::Colour(0xFFC0C0C0)); // Darker shadow
        juce::Path shadowPath;
        shadowPath.addArc(highlightBounds.getX(), highlightBounds.getY(),
                         highlightBounds.getWidth(), highlightBounds.getHeight(),
                         juce::MathConstants<float>::pi * 0.2f, // Start angle (lower right)
                         juce::MathConstants<float>::pi * 0.8f, // End angle (lower left)
                         true);
        g.strokePath(shadowPath, juce::PathStrokeType(1.0f));
    }
    
    void drawKnobIndicator(juce::Graphics& g, juce::Rectangle<int> knobArea)
    {
        auto knobBounds = knobArea.toFloat();
        auto center = knobBounds.getCentre();
        
        // Calculate angle based on current value (225 degrees total rotation)
        // At value 0: 225 degrees (bottom-left)
        // At max value: 90 degrees (bottom-right) - achieved by going clockwise 225 degrees
        double valueNormalized = (currentValue - minVal) / (maxVal - minVal);
        double angleDegrees = 225.0 + (valueNormalized * 225.0);
        
        // Handle wraparound at 360 degrees to ensure proper angle range
        if (angleDegrees >= 360.0)
            angleDegrees -= 360.0;
            
        double angleRadians = angleDegrees * juce::MathConstants<double>::pi / 180.0;
        
        // Indicator line from center to 80% of radius
        float radius = knobBounds.getWidth() * 0.4f; // 80% of radius (knob radius is 50%)
        float lineEndX = center.x + std::cos(angleRadians) * radius;
        float lineEndY = center.y + std::sin(angleRadians) * radius;
        
        // Draw indicator line (2px wide, black)
        g.setColour(juce::Colours::black);
        juce::Line<float> indicatorLine(center.x, center.y, lineEndX, lineEndY);
        g.drawLine(indicatorLine, 2.0f);
    }
    
    void drawLabel(juce::Graphics& g, juce::Rectangle<int> labelArea)
    {
        // Text label underneath the knob, centered (9pt font) - moved down 1px
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(9.0f));
        auto adjustedLabelArea = labelArea.translated(0, 1); // Move down 1px
        g.drawText(label, adjustedLabelArea, juce::Justification::centredTop);
        
        // Draw current value below label
        juce::String valueText;
        if (std::abs(currentValue - std::round(currentValue)) < 0.01)
            valueText = juce::String((int)std::round(currentValue));
        else
            valueText = juce::String(currentValue, 1);
            
        g.setFont(juce::FontOptions(9.0f));
        g.setColour(juce::Colour(0xFF404040)); // Dark gray for better readability
        auto valueArea = labelArea.removeFromBottom(12);
        g.drawText(valueText, valueArea, juce::Justification::centred);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomKnob)
};