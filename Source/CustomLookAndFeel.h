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
        // Main plate gradient (brushed aluminum effect - darkened for better contrast)
        juce::ColourGradient plateGradient(
            juce::Colour(0xFFD0D0D0), bounds.getTopLeft(),    // Darkened from 0xFFE8E8E8
            juce::Colour(0xFF909090), bounds.getBottomRight(), // Darkened from 0xFFA0A0A0
            false
        );
        plateGradient.addColour(0.3f, juce::Colour(0xFFB8B8B8)); // Darkened from 0xFFD0D0D0
        plateGradient.addColour(0.7f, juce::Colour(0xFFA0A0A0)); // Darkened from 0xFFB8B8B8
        
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
    void drawSliderTrack(juce::Graphics& g, juce::Rectangle<float> trackArea, juce::Colour trackColor, double sliderValue = 1.0, double minValue = 0.0, double maxValue = 16383.0)
    {
        // Create oval track shape
        auto ovalTrack = trackArea.reduced(4, 8);
        
        // Track background (deep inset shadow) - always draw full background
        juce::ColourGradient trackShadow(
            juce::Colour(0xFF202020), ovalTrack.getTopLeft(),
            juce::Colour(0xFF404040), ovalTrack.getBottomRight(),
            false
        );
        
        g.setGradientFill(trackShadow);
        g.fillRoundedRectangle(ovalTrack, ovalTrack.getWidth() / 2.0f);
        
        // Calculate normalized fill level (0.0 = empty, 1.0 = full)
        double normalizedValue = juce::jlimit(0.0, 1.0, (sliderValue - minValue) / (maxValue - minValue));
        
        // Only draw colored fill if there's something to fill
        if (normalizedValue > 0.0)
        {
            // Colored track fill (the active part) - desaturated and matte
            auto coloredTrack = ovalTrack.reduced(2);
            
            // Calculate fill height from bottom up (normalizedValue 0.0 = no fill, 1.0 = full fill)
            float fillHeight = coloredTrack.getHeight() * normalizedValue;
            auto fillArea = coloredTrack.removeFromBottom(fillHeight);
            
            // Manual desaturation by reducing saturation in HSV space
            float hue = trackColor.getHue();
            float saturation = trackColor.getSaturation();
            float brightness = trackColor.getBrightness();
            auto desaturatedColor = juce::Colour::fromHSV(hue, saturation * 0.6f, brightness * 0.9f, trackColor.getFloatAlpha()); // Reduce saturation by 40% and darken slightly
            
            juce::ColourGradient colorGradient(
                desaturatedColor.brighter(0.1f), fillArea.getTopLeft(),    // Brighter at top (natural lighting)
                desaturatedColor.darker(0.6f), fillArea.getBottomRight(),   // Darker at bottom
                false
            );
            colorGradient.addColour(0.5f, desaturatedColor);
            
            g.setGradientFill(colorGradient);
            g.fillRoundedRectangle(fillArea, fillArea.getWidth() / 2.0f);
            
            // Track highlight - using desaturated color, only in filled area
            if (fillArea.getWidth() > 4) // Only draw highlight if there's enough space
            {
                g.setColour(desaturatedColor.brighter(0.3f).withAlpha(0.5f));
                auto highlightArea = fillArea.removeFromLeft(2).reduced(0, 4);
                g.fillRoundedRectangle(highlightArea, 1.0f);
            }
        }
        
        // Track border - always draw full border
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
        // DJ fader style thumb - 25% wider with horizontal colored line
        float thumbWidth = 32.5f;   // 25% wider than previous 26px (26 * 1.25 = 32.5)
        float thumbHeight = 14.0f;  // Keep same height
        
        auto thumbBounds = juce::Rectangle<float>(thumbWidth, thumbHeight)
            .withCentre(juce::Point<float>(centerX, centerY));
        
        // Drop shadow
        auto shadowBounds = thumbBounds.translated(1, 2);
        g.setColour(juce::Colour(0x80000000));
        g.fillRoundedRectangle(shadowBounds, 1.0f); // Less rounded for more rectangular look
        
        // Main thumb body gradient - more rectangular/linear appearance
        juce::ColourGradient thumbGradient(
            juce::Colour(0xFFF8F8F8), thumbBounds.getTopLeft(),
            juce::Colour(0xFFB8B8B8), thumbBounds.getBottomRight(),
            false
        );
        thumbGradient.addColour(0.3f, juce::Colour(0xFFE8E8E8));
        thumbGradient.addColour(0.7f, juce::Colour(0xFFD0D0D0));
        
        g.setGradientFill(thumbGradient);
        g.fillRoundedRectangle(thumbBounds, 1.0f); // Less rounded corners (reduced from 2.0f)
        
        // Calculate center line position before any thumb modifications
        float lineHeight = 4.0f;
        float lineWidth = thumbWidth - 1.0f; // Horizontal line across most of thumb width
        
        // Manual desaturation to match track color
        float hue = trackColor.getHue();
        float saturation = trackColor.getSaturation();
        float brightness = trackColor.getBrightness();
        auto desaturatedThumbColor = juce::Colour::fromHSV(hue, saturation * 0.6f, brightness * 0.9f, trackColor.getFloatAlpha()); // Match track desaturation
        
        auto centerLine = juce::Rectangle<float>(lineWidth, lineHeight)
            .withCentre(thumbBounds.getCentre()); // Use original thumbBounds center for perfect alignment
        
        // Thumb border
        g.setColour(juce::Colour(0xFF707070));
        g.drawRoundedRectangle(thumbBounds, 1.0f, 1.0f);
        
        // Top highlight - linear style
        g.setColour(juce::Colour(0x80FFFFFF));
        auto highlight = thumbBounds.removeFromTop(2).reduced(1, 0);
        g.fillRoundedRectangle(highlight, 0.5f);
        
        // Line shadow
        g.setColour(juce::Colour(0x60000000));
        g.fillRoundedRectangle(centerLine.translated(0.5f, 0.5f), 0.5f);
        
        // Colored horizontal line
        juce::ColourGradient lineGradient(
            desaturatedThumbColor.brighter(0.2f), centerLine.getTopLeft(),
            desaturatedThumbColor.darker(0.2f), centerLine.getBottomRight(),
            false
        );
        
        g.setGradientFill(lineGradient);
        g.fillRoundedRectangle(centerLine, 0.5f);
        
        // Line highlight
        g.setColour(desaturatedThumbColor.brighter(0.4f).withAlpha(0.7f));
        auto lineHighlight = centerLine.removeFromTop(1).removeFromLeft(1);
        g.fillRoundedRectangle(lineHighlight, 0.25f);
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
