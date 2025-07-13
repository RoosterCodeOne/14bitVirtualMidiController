// CustomLookAndFeel.h - Modular Synth Style
#pragma once
#include <JuceHeader.h>

//==============================================================================
class CustomSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Constructor with default color
    CustomSliderLookAndFeel(juce::Colour defaultColor = juce::Colours::cyan)
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
    
    // Public method for drawing extended plate that includes automation controls
    void drawExtendedModulePlate(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        // Main plate gradient (brushed aluminum effect)
        juce::ColourGradient plateGradient(
            juce::Colour(0xFFE8E8E8), bounds.getTopLeft(),
            juce::Colour(0xFFA0A0A0), bounds.getBottomRight(),
            false
        );
        plateGradient.addColour(0.3f, juce::Colour(0xFFD0D0D0));
        plateGradient.addColour(0.7f, juce::Colour(0xFFB8B8B8));
        
        g.setGradientFill(plateGradient);
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Plate border (subtle inset)
        g.setColour(juce::Colour(0xFF808080));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        
        // Inner highlight
        g.setColour(juce::Colour(0x40FFFFFF));
        g.drawRoundedRectangle(bounds.reduced(1), 3.0f, 1.0f);
        
        // Corner mounting screws with balanced positioning  
        float horizontalInset = 8.0f; // Reduced for narrower plate (was 8px)
        float verticalInset = 8.0f; // Keep vertical distance for balance
        
        // Top screws with balanced positioning
        drawMountingScrew(g, bounds.getTopLeft() + juce::Point<float>(horizontalInset, verticalInset));
        drawMountingScrew(g, bounds.getTopRight() + juce::Point<float>(-horizontalInset, verticalInset));
        
        // Bottom screws with balanced positioning
        drawMountingScrew(g, bounds.getBottomLeft() + juce::Point<float>(horizontalInset, -verticalInset));
        drawMountingScrew(g, bounds.getBottomRight() + juce::Point<float>(-horizontalInset, -verticalInset));
    }
    
    // Public drawing methods for external use
    void drawSliderTrack(juce::Graphics& g, juce::Rectangle<float> trackArea, juce::Colour trackColor)
    {
        // Create oval track shape
        auto ovalTrack = trackArea.reduced(4, 8);
        
        // Track background (deep inset shadow)
        juce::ColourGradient trackShadow(
            juce::Colour(0xFF202020), ovalTrack.getTopLeft(),
            juce::Colour(0xFF404040), ovalTrack.getBottomRight(),
            false
        );
        
        g.setGradientFill(trackShadow);
        g.fillRoundedRectangle(ovalTrack, ovalTrack.getWidth() / 2.0f);
        
        // Colored track fill (the active part)
        auto coloredTrack = ovalTrack.reduced(2);
        juce::ColourGradient colorGradient(
            trackColor.darker(0.8f), coloredTrack.getTopLeft(),
            trackColor.brighter(0.2f), coloredTrack.getBottomRight(),
            false
        );
        colorGradient.addColour(0.5f, trackColor);
        
        g.setGradientFill(colorGradient);
        g.fillRoundedRectangle(coloredTrack, coloredTrack.getWidth() / 2.0f);
        
        // Track highlight
        g.setColour(trackColor.brighter(0.4f).withAlpha(0.6f));
        auto highlightArea = coloredTrack.removeFromLeft(2).reduced(0, 4);
        g.fillRoundedRectangle(highlightArea, 1.0f);
        
        // Track border
        g.setColour(juce::Colour(0xFF303030));
        g.drawRoundedRectangle(ovalTrack, ovalTrack.getWidth() / 2.0f, 1.0f);
    }
    
    void drawTickMarks(juce::Graphics& g, juce::Rectangle<float> trackArea)
    {
        g.setColour(juce::Colour(0xFF606060));
        
        auto tickArea = trackArea.reduced(0, 8);
        int numTicks = 17; // Reduced from 21 to 17 for shorter track (better spacing)
        
        for (int i = 0; i <= numTicks; ++i)
        {
            float y = tickArea.getY() + (i * tickArea.getHeight() / numTicks);
            
            // Major ticks every 5
            bool isMajor = (i % 5 == 0);
            float tickLength = isMajor ? 12.0f : 8.0f; // Reduced from 16.0f/10.0f to 12.0f/8.0f for narrower track
            float tickWidth = isMajor ? 1.0f : 0.5f;
            
            // Left side ticks
            g.fillRect(trackArea.getX() - tickLength, y - tickWidth/2, tickLength, tickWidth);
            
            // Right side ticks
            g.fillRect(trackArea.getRight(), y - tickWidth/2, tickLength, tickWidth);
        }
    }
    
    void drawSliderThumb(juce::Graphics& g, float centerX, float centerY, juce::Colour trackColor)
    {
        float thumbWidth = 48.0f;  // Reduced from 64.0f to match narrower 30px track (25% reduction)
        float thumbHeight = 20.0f; // Reduced by 4px from 24px
        
        auto thumbBounds = juce::Rectangle<float>(thumbWidth, thumbHeight)
            .withCentre(juce::Point<float>(centerX, centerY));
        
        // Drop shadow (increased visibility)
        auto shadowBounds = thumbBounds.translated(1, 2);
        g.setColour(juce::Colour(0x80000000)); // Increased alpha from 0x40 to 0x80
        g.fillRoundedRectangle(shadowBounds, 6.0f);
        
        // Main thumb body gradient
        juce::ColourGradient thumbGradient(
            juce::Colour(0xFFF0F0F0), thumbBounds.getTopLeft(),
            juce::Colour(0xFFB0B0B0), thumbBounds.getBottomRight(),
            false
        );
        thumbGradient.addColour(0.4f, juce::Colour(0xFFE0E0E0));
        
        g.setGradientFill(thumbGradient);
        g.fillRoundedRectangle(thumbBounds, 6.0f);
        
        // Thumb border
        g.setColour(juce::Colour(0xFF808080));
        g.drawRoundedRectangle(thumbBounds, 6.0f, 1.0f);
        
        // Top highlight
        g.setColour(juce::Colour(0x60FFFFFF));
        auto highlight = thumbBounds.removeFromTop(3).reduced(2, 0);
        g.fillRoundedRectangle(highlight, 2.0f);
        
        // Center indicator dot (matches track color)
        float dotSize = 12.0f; // Doubled from 6.0f
        auto dotBounds = juce::Rectangle<float>(dotSize, dotSize)
            .withCentre(thumbBounds.getCentre());
        
        // Dot shadow
        g.setColour(juce::Colour(0x80000000));
        g.fillEllipse(dotBounds.translated(0.5f, 0.5f));
        
        // Colored dot
        juce::ColourGradient dotGradient(
            trackColor.brighter(0.3f), dotBounds.getTopLeft(),
            trackColor.darker(0.3f), dotBounds.getBottomRight(),
            false
        );
        
        g.setGradientFill(dotGradient);
        g.fillEllipse(dotBounds);
        
        // Dot highlight
        g.setColour(trackColor.brighter(0.6f).withAlpha(0.8f));
        auto dotHighlight = dotBounds.removeFromTop(2).removeFromLeft(2);
        g.fillEllipse(dotHighlight);
    }
    
private:
    juce::Colour sliderColor;
    
    void drawMountingScrew(juce::Graphics& g, juce::Point<float> center)
    {
        float radius = 6.0f; // Doubled from 3.0f
        auto screwBounds = juce::Rectangle<float>(radius * 2, radius * 2).withCentre(center);
        
        // Screw hole (dark)
        g.setColour(juce::Colour(0xFF404040));
        g.fillEllipse(screwBounds);
        
        // Screw highlight
        g.setColour(juce::Colour(0xFF606060));
        g.fillEllipse(screwBounds.reduced(0.5f));
        
        // Screw slot
        g.setColour(juce::Colour(0xFF202020));
        auto slotBounds = juce::Rectangle<float>(radius * 1.2f, 0.8f).withCentre(center);
        g.fillRect(slotBounds);
    }
};

//End CustomLookAndFeel.h
//=====================
