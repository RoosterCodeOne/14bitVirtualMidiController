// CustomLookAndFeel.h - Blueprint Technical Drawing Style
#pragma once
#include <JuceHeader.h>
#include <map>
#include "Core/SliderDisplayManager.h"

// Blueprint color palette
namespace BlueprintColors
{
    const juce::Colour background(0xFF1a1a2e);      // Dark navy base
    const juce::Colour panel(0xFF16213e);           // Slightly lighter navy for raised areas
    const juce::Colour windowBackground(0xFF1e2344); // Window background - slightly lighter than main
    const juce::Colour sectionBackground(0xFF242951); // Section background - lightest blue
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
    
    // Quantization support
    void setQuantizationEnabled(bool enabled)
    {
        quantizationEnabled = enabled;
    }
    
    void setQuantizationIncrement(double increment, double displayMin, double displayMax)
    {
        quantizationIncrement = increment;
        quantizationDisplayMin = displayMin;
        quantizationDisplayMax = displayMax;
    }
    
    juce::Colour getSliderColor() const
    {
        return sliderColor;
    }
    
    // Public method for drawing blueprint-style technical panel
    void drawExtendedModulePlate(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        // Solid panel background matching settings sections
        g.setColour(BlueprintColors::sectionBackground);
        g.fillRoundedRectangle(bounds, 2.0f);
        
        // Technical outline - dimmed cyan line
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
        
        // No mounting screws - clean technical appearance
    }
    
    // Blueprint-style button drawing method for consistent button styling
    void drawBlueprintButton(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text, 
                           bool isPressed, bool isHighlighted, bool isSelected = false, 
                           juce::Colour customColor = juce::Colour())
    {
        // Blueprint-style flat button background
        juce::Colour bgColor;
        juce::Colour activeColor = customColor.isTransparent() ? BlueprintColors::active : customColor;
        
        if (isPressed)
            bgColor = activeColor.darker(0.3f);
        else if (isSelected)
            bgColor = activeColor;
        else
            bgColor = BlueprintColors::panel;
            
        g.setColour(bgColor);
        g.fillRoundedRectangle(bounds, 2.0f);
        
        // Technical outline - thicker when pressed/highlighted
        float lineWidth = (isPressed || isHighlighted) ? 2.0f : 1.0f;
        juce::Colour outlineColor;
        if (isHighlighted)
            outlineColor = activeColor;
        else if (isSelected)
            outlineColor = activeColor.brighter(0.2f);
        else
            outlineColor = BlueprintColors::blueprintLines.withAlpha(0.6f);
            
        g.setColour(outlineColor);
        g.drawRoundedRectangle(bounds, 2.0f, lineWidth);
        
        // Draw button text with blueprint styling
        if (text.isNotEmpty())
        {
            g.setFont(juce::Font(11.0f, juce::Font::bold)); // Increased from 9.0f for better readability
            
            // Color based on state - ensure good contrast
            if (isHighlighted)
                g.setColour(activeColor);
            else if (isPressed)
                g.setColour(BlueprintColors::textPrimary.darker(0.2f));
            else if (isSelected)
            {
                // For better contrast on colored backgrounds, especially yellow
                if (customColor == juce::Colours::yellow)
                    g.setColour(juce::Colours::black); // Black text on yellow background
                else
                    g.setColour(juce::Colours::white); // White text on other colored backgrounds
            }
            else
                g.setColour(BlueprintColors::textPrimary);
                
            g.drawText(text, bounds, juce::Justification::centred);
        }
    }
    
    // Public drawing methods for external use
    void drawSliderTrack(juce::Graphics& g, juce::Rectangle<float> trackArea, juce::Colour trackColor, double sliderValue = 1.0, double minValue = 0.0, double maxValue = 16383.0, SliderOrientation orientation = SliderOrientation::Normal, double bipolarCenter = 8191.5)
    {
        // Blueprint-style rectangular track
        auto track = trackArea.reduced(2, 4);
        
        // Track background - solid dark fill
        g.setColour(BlueprintColors::background);
        g.fillRect(track);
        
        // Handle different orientations for visual display
        if (orientation == SliderOrientation::Normal)
        {
            // Calculate normalized fill level (0.0 = empty, 1.0 = full)
            double normalizedValue = juce::jlimit(0.0, 1.0, (sliderValue - minValue) / (maxValue - minValue));
            
            // Progressive fill from bottom up (normal orientation)
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
        }
        else if (orientation == SliderOrientation::Inverted)
        {
            // Calculate normalized fill level (0.0 = empty, 1.0 = full)
            double normalizedValue = juce::jlimit(0.0, 1.0, (sliderValue - minValue) / (maxValue - minValue));
            
            // Progressive fill from top down (inverted orientation)
            if (normalizedValue > 0.0)
            {
                float fillHeight = track.getHeight() * normalizedValue;
                auto fillArea = track.removeFromTop(fillHeight);
                
                // Gradient fill from bright to dark (inverted gradient)
                juce::ColourGradient fillGradient(
                    trackColor, fillArea.getTopLeft(),
                    BlueprintColors::background, fillArea.getBottomRight(),
                    false
                );
                
                g.setGradientFill(fillGradient);
                g.fillRect(fillArea);
            }
        }
        else if (orientation == SliderOrientation::Bipolar)
        {
            // Bipolar mode: fill from center point outward
            double normalizedCenter = juce::jlimit(0.0, 1.0, (bipolarCenter - minValue) / (maxValue - minValue));
            double normalizedValue = juce::jlimit(0.0, 1.0, (sliderValue - minValue) / (maxValue - minValue));
            
            float centerY = track.getY() + track.getHeight() * (1.0 - normalizedCenter);
            
            // Draw center line
            g.setColour(trackColor.withAlpha(0.8f));
            g.fillRect(juce::Rectangle<float>(track.getX(), centerY - 1.0f, track.getWidth(), 2.0f));
            
            // Fill from center to current value
            if (normalizedValue != normalizedCenter)
            {
                float valueY = track.getY() + track.getHeight() * (1.0 - normalizedValue);
                juce::Rectangle<float> fillArea;
                
                if (normalizedValue > normalizedCenter)
                {
                    // Fill upward from center
                    fillArea = juce::Rectangle<float>(track.getX(), valueY, track.getWidth(), centerY - valueY);
                }
                else
                {
                    // Fill downward from center
                    fillArea = juce::Rectangle<float>(track.getX(), centerY, track.getWidth(), valueY - centerY);
                }
                
                // Bipolar gradient fill
                juce::Point<float> gradientEnd = normalizedValue > normalizedCenter ? 
                    juce::Point<float>(fillArea.getCentreX(), fillArea.getY()) :
                    juce::Point<float>(fillArea.getCentreX(), fillArea.getBottom());
                    
                juce::ColourGradient fillGradient(
                    BlueprintColors::background.withAlpha(0.5f), fillArea.getCentre(),
                    trackColor, gradientEnd,
                    false
                );
                
                g.setGradientFill(fillGradient);
                g.fillRect(fillArea);
            }
        }
        
        // Technical outline
        g.setColour(BlueprintColors::blueprintLines);
        g.drawRect(track, 1.0f);
    }
    
    void drawTickMarks(juce::Graphics& g, juce::Rectangle<float> trackArea)
    {
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        
        auto tickArea = trackArea.reduced(0, 4);
        
        if (quantizationEnabled && quantizationIncrement > 0.0)
        {
            // Draw quantization step marks
            double displayRange = std::abs(quantizationDisplayMax - quantizationDisplayMin);
            if (displayRange > 0.001) // Ensure valid range
            {
                int numSteps = (int)std::floor(displayRange / quantizationIncrement);
                numSteps = juce::jlimit(1, 50, numSteps); // Limit number of ticks for visual clarity
                
                g.setColour(BlueprintColors::active.withAlpha(0.8f)); // Use active color for quantization ticks
                
                for (int i = 0; i <= numSteps; ++i)
                {
                    if (numSteps > 0) // Avoid division by zero
                    {
                        float normalizedPos = (float)i / (float)numSteps;
                        float y = tickArea.getBottom() - (normalizedPos * tickArea.getHeight());
                        
                        // Bounds check for y position
                        if (y >= tickArea.getY() && y <= tickArea.getBottom())
                        {
                            // All quantization ticks are major ticks
                            float tickLength = 8.0f;
                            float tickWidth = 1.5f; // Slightly thicker for quantization
                            
                            // Quantization tick marks - left side with distinctive style
                            g.fillRect(juce::Rectangle<float>(trackArea.getX() - tickLength - 2, y - tickWidth/2, tickLength, tickWidth));
                        }
                    }
                }
            }
        }
        else
        {
            // Draw standard aesthetic tick marks
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
    bool quantizationEnabled = false;
    double quantizationIncrement = 1.0;
    double quantizationDisplayMin = 0.0;
    double quantizationDisplayMax = 16383.0;
};

//==============================================================================
// Custom button look and feel for blueprint-style buttons
class CustomButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomButtonLookAndFeel() = default;
    
    // Set custom color for a specific button
    void setButtonColor(juce::Button* button, juce::Colour color)
    {
        buttonColors[button] = color;
    }
    
    // Remove button color mapping when button is destroyed
    void removeButtonColor(juce::Button* button)
    {
        buttonColors.erase(button);
    }
    
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        // Use the blueprint button drawing method
        CustomSliderLookAndFeel lookAndFeel;
        auto bounds = button.getLocalBounds().toFloat();
        auto text = button.getButtonText();
        
        // Check if this is a bank button that should show selected state
        bool isSelected = false;
        if (auto* toggleButton = dynamic_cast<juce::ToggleButton*>(&button))
        {
            isSelected = toggleButton->getToggleState();
        }
        
        // Get custom color for this button, if set
        juce::Colour customColor;
        auto it = buttonColors.find(&button);
        if (it != buttonColors.end())
        {
            customColor = it->second;
        }
        
        lookAndFeel.drawBlueprintButton(g, bounds, text, shouldDrawButtonAsDown, 
                                      shouldDrawButtonAsHighlighted, isSelected, customColor);
    }
    
    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        // Text is already drawn in drawButtonBackground, so we don't need to draw it again
    }
    
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        // Use the same blueprint button styling for toggle buttons
        drawButtonBackground(g, button, juce::Colour(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    }
    
private:
    std::map<juce::Button*, juce::Colour> buttonColors;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomButtonLookAndFeel)
};

//End CustomLookAndFeel.h
//=====================
