// CustomLookAndFeel.h - Blueprint Technical Drawing Style
#pragma once
#include <JuceHeader.h>

// Blueprint color palette
namespace BlueprintColors
{
    const juce::Colour background(0xFF1a1a2e);      // Dark navy base
    const juce::Colour panel(0xFF16213e);           // Slightly lighter navy for raised areas
    const juce::Colour blueprintLines(0xFF00d4ff);  // Bright cyan for technical lines
    const juce::Colour textPrimary(0xFFe8e8e8);     // Soft white
    const juce::Colour textSecondary(0xFFa0b4cc);   // Blue-gray
    const juce::Colour active(0xFF00d4ff);          // Bright cyan for active/focus
    const juce::Colour warning(0xFFff8c42);         // Warm amber for warning/learn
    const juce::Colour success(0xFF4ade80);         // Soft green
    const juce::Colour inactive(0xFF4a5568);        // Muted gray
}

//==============================================================================
class CustomSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Constructor with default color
    CustomSliderLookAndFeel(juce::Colour defaultColor = BlueprintColors::active)
        : sliderColor(defaultColor) {}
    
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        // Make slider completely invisible - all visuals handled by parent component
        // No drawing here
    }
    
    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;
        auto bounds = slider.getLocalBounds().toFloat();
        
        // The slider bounds should now be exactly the track area since that's all the slider component represents
        layout.sliderBounds = bounds.toNearestInt();
        
        return layout;
    }
    
    // Safe method to update color
    void setSliderColor(juce::Colour newColor)
    {
        sliderColor = newColor;
    }
    
    juce::Colour getSliderColor() const
    {
        return sliderColor;
    }
    
    // Public method for drawing blueprint-style technical panel
    void drawExtendedModulePlate(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        // Solid panel background with no gradients
        g.setColour(BlueprintColors::panel);
        g.fillRoundedRectangle(bounds, 2.0f);
        
        // Technical outline - thin cyan line
        g.setColour(BlueprintColors::blueprintLines);
        g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
        
        // No mounting screws - clean technical appearance
    }
    
    // Public drawing methods for external use
    void drawSliderTrack(juce::Graphics& g, juce::Rectangle<float> trackArea, juce::Colour trackColor, double sliderValue = 1.0, double minValue = 0.0, double maxValue = 16383.0)
    {
        // Blueprint-style rectangular track
        auto track = trackArea.reduced(2, 4);
        
        // Track background - solid dark fill
        g.setColour(BlueprintColors::background);
        g.fillRect(track);
        
        // Calculate normalized fill level (0.0 = empty, 1.0 = full)
        double normalizedValue = juce::jlimit(0.0, 1.0, (sliderValue - minValue) / (maxValue - minValue));
        
        // Progressive fill from bottom up
        if (normalizedValue > 0.0)
        {
            float fillHeight = track.getHeight() * normalizedValue;
            auto fillArea = track.removeFromBottom(fillHeight);
            
            // Gradient fill from dark to bright cyan
            juce::ColourGradient fillGradient(
                BlueprintColors::background, fillArea.getTopLeft(),
                trackColor, fillArea.getBottomRight(),
                false
            );
            
            g.setGradientFill(fillGradient);
            g.fillRect(fillArea);
        }
        
        // Technical outline
        g.setColour(BlueprintColors::blueprintLines);
        g.drawRect(track, 1.0f);
    }
    
    void drawTickMarks(juce::Graphics& g, juce::Rectangle<float> trackArea)
    {
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        
        auto tickArea = trackArea.reduced(0, 4);
        int numTicks = 11; // Reduced for cleaner blueprint appearance
        
        for (int i = 0; i <= numTicks; ++i)
        {
            float y = tickArea.getY() + (i * tickArea.getHeight() / numTicks);
            
            // Major ticks every 5, minor ticks in between
            bool isMajor = (i % 5 == 0);
            float tickLength = isMajor ? 8.0f : 4.0f;
            float tickWidth = 1.0f; // Consistent thin lines
            
            // Technical tick marks - left side only for cleaner look
            g.fillRect(trackArea.getX() - tickLength - 2, y - tickWidth/2, tickLength, tickWidth);
        }
    }
    
    void drawSliderThumb(juce::Graphics& g, float centerX, float centerY, juce::Colour trackColor)
    {
        // Blueprint-style flat rectangular thumb
        float thumbWidth = 28.0f;
        float thumbHeight = 12.0f;
        
        auto thumbBounds = juce::Rectangle<float>(thumbWidth, thumbHeight)
            .withCentre(juce::Point<float>(centerX, centerY));
        
        // No shadow for flat design
        
        // Solid flat body
        g.setColour(BlueprintColors::panel);
        g.fillRect(thumbBounds);
        
        // Technical outline
        g.setColour(BlueprintColors::blueprintLines);
        g.drawRect(thumbBounds, 1.0f);
        
        // Horizontal indicator line using track color
        float lineHeight = 2.0f;
        float lineWidth = thumbWidth - 6.0f;
        
        auto centerLine = juce::Rectangle<float>(lineWidth, lineHeight)
            .withCentre(thumbBounds.getCentre());
        
        g.setColour(trackColor);
        g.fillRect(centerLine);
    }
    
    // Add blueprint grid drawing method
    void drawBlueprintGrid(juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.1f));
        
        int gridSpacing = 20;
        
        // Vertical lines
        for (int x = bounds.getX(); x < bounds.getRight(); x += gridSpacing)
        {
            g.drawVerticalLine(x, bounds.getY(), bounds.getBottom());
        }
        
        // Horizontal lines
        for (int y = bounds.getY(); y < bounds.getBottom(); y += gridSpacing)
        {
            g.drawHorizontalLine(y, bounds.getX(), bounds.getRight());
        }
    }

private:
    juce::Colour sliderColor;
};

//End CustomLookAndFeel.h
//=====================
