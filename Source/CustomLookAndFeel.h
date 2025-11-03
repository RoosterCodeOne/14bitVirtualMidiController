// CustomLookAndFeel.h - Blueprint Technical Drawing Style with Theme Support
#pragma once
#include <JuceHeader.h>
#include <map>
#include "Core/SliderDisplayManager.h"
#include "UI/GlobalUIScale.h"
#include "UI/ThemeManager.h"

// Dynamic color palette that automatically reflects the current theme
// All colors now fetch from ThemeManager, making the entire UI theme-aware
namespace BlueprintColors
{
    // Inline functions that fetch current theme colors dynamically
    // No macros used to avoid name collisions with function parameters
    inline juce::Colour background() { return Theme::palette().background; }
    inline juce::Colour panel() { return Theme::palette().panel; }
    inline juce::Colour windowBackground() { return Theme::palette().windowBackground; }
    inline juce::Colour sectionBackground() { return Theme::palette().sectionBackground; }
    inline juce::Colour inputBackground() { return Theme::palette().inputBackground; }
    inline juce::Colour blueprintLines() { return Theme::palette().blueprintLines; }
    inline juce::Colour textPrimary() { return Theme::palette().textPrimary; }
    inline juce::Colour textSecondary() { return Theme::palette().textSecondary; }
    inline juce::Colour active() { return Theme::palette().active; }
    inline juce::Colour warning() { return Theme::palette().warning; }
    inline juce::Colour success() { return Theme::palette().success; }
    inline juce::Colour inactive() { return Theme::palette().inactive; }
}

//==============================================================================
class CustomSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Constructor with default color
    CustomSliderLookAndFeel(juce::Colour defaultColor = BlueprintColors::active())
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
        auto& scale = GlobalUIScale::getInstance();
        
        // Solid panel background matching settings sections
        g.setColour(BlueprintColors::sectionBackground());
        g.fillRoundedRectangle(bounds, scale.getScaledCornerRadius());
        
        // Technical outline - dimmed cyan line
        g.setColour(BlueprintColors::blueprintLines().withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, scale.getScaledCornerRadius(), scale.getScaledLineThickness());
        
        // No mounting screws - clean technical appearance
    }
    
    // Blueprint-style button drawing method for consistent button styling
    void drawBlueprintButton(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text, 
                           bool isPressed, bool isHighlighted, bool isSelected = false, 
                           juce::Colour customColor = juce::Colour())
    {
        auto& scale = GlobalUIScale::getInstance();
        
        // Blueprint-style flat button background
        juce::Colour bgColor;
        juce::Colour activeColor = customColor.isTransparent() ? BlueprintColors::active() : customColor;
        
        if (isPressed)
            bgColor = activeColor.darker(0.3f);
        else if (isSelected)
            bgColor = activeColor.withAlpha(0.7f);
        else
            bgColor = BlueprintColors::panel();
            
        g.setColour(bgColor);
        g.fillRoundedRectangle(bounds, scale.getScaledCornerRadius());
        
        // Technical outline - thicker when pressed/highlighted
        float lineWidth = (isPressed || isHighlighted) ? scale.getScaled(2.0f) : scale.getScaledLineThickness();
        juce::Colour outlineColor;
        if (isHighlighted)
            outlineColor = activeColor;
        else if (isSelected)
            outlineColor = activeColor.brighter(0.2f);
        else
            outlineColor = BlueprintColors::blueprintLines().withAlpha(0.6f);
            
        g.setColour(outlineColor);
        g.drawRoundedRectangle(bounds, scale.getScaledCornerRadius(), lineWidth);
        
        // Draw button text with blueprint styling
        if (text.isNotEmpty())
        {
            g.setFont(scale.getScaledFont(11.0f).boldened()); // Scale font and make bold
            
            // Color based on state - ensure good contrast
            if (isHighlighted)
                g.setColour(activeColor);
            else if (isPressed)
                g.setColour(BlueprintColors::textPrimary().darker(0.2f));
            else if (isSelected)
            {
                // For better contrast on colored backgrounds, especially yellow
                if (customColor == juce::Colours::yellow)
                    g.setColour(juce::Colours::black); // Black text on yellow background
                else
                    g.setColour(juce::Colours::white); // White text on other colored backgrounds
            }
            else
                g.setColour(BlueprintColors::textPrimary());
                
            g.drawText(text, bounds, juce::Justification::centred);
        }
    }
    
    // Public drawing methods for external use
    void drawSliderTrack(juce::Graphics& g, juce::Rectangle<float> trackArea, juce::Colour trackColor, double sliderValue = 1.0, double minValue = 0.0, double maxValue = 16383.0, SliderOrientation orientation = SliderOrientation::Normal, double bipolarCenter = 8191.5, bool isInSnapZone = false)
    {
        auto& scale = GlobalUIScale::getInstance();
        
        // Blueprint-style rectangular track
        auto track = trackArea.reduced(scale.getScaled(2), scale.getScaled(4));
        
        // Track background - solid dark fill
        g.setColour(BlueprintColors::background());
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
                    BlueprintColors::background(), fillArea.getTopLeft(),
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
                    BlueprintColors::background(), fillArea.getBottomRight(),
                    false
                );
                
                g.setGradientFill(fillGradient);
                g.fillRect(fillArea);
            }
        }
        else if (orientation == SliderOrientation::Bipolar)
        {
            // Bipolar mode: fill from center point outward
            // Center line position based on bipolarCenter's position within the display range
            double normalizedCenter = juce::jlimit(0.0, 1.0, (bipolarCenter - minValue) / (maxValue - minValue));
            double normalizedValue = juce::jlimit(0.0, 1.0, (sliderValue - minValue) / (maxValue - minValue));
            
            float centerY = track.getY() + track.getHeight() * (1.0 - normalizedCenter);
            float valueY = track.getY() + track.getHeight() * (1.0 - normalizedValue);
            
            // Draw center line with enhanced visual feedback when in snap zone
            if (isInSnapZone)
            {
                // Enhanced center line when in snap zone - brighter and thicker
                g.setColour(trackColor.brighter(0.3f));
                g.fillRect(juce::Rectangle<float>(track.getX() - scale.getScaled(2.0f), centerY - scale.getScaled(2.0f), track.getWidth() + scale.getScaled(4.0f), scale.getScaled(4.0f)));
                
                // Add subtle glow effect
                g.setColour(trackColor.withAlpha(0.4f));
                g.fillRect(juce::Rectangle<float>(track.getX() - scale.getScaled(4.0f), centerY - scale.getScaled(3.0f), track.getWidth() + scale.getScaled(8.0f), scale.getScaled(6.0f)));
            }
            else
            {
                // Normal center line
                g.setColour(trackColor.withAlpha(0.8f));
                g.fillRect(juce::Rectangle<float>(track.getX(), centerY - scale.getScaled(1.0f), track.getWidth(), scale.getScaled(2.0f)));
            }
            
            // Fill from center to current value (only if not exactly at center)
            if (std::abs(normalizedValue - normalizedCenter) > 0.001) // Small threshold to avoid tiny fills
            {
                juce::Rectangle<float> fillArea;
                juce::Point<float> gradientStart, gradientEnd;
                
                if (normalizedValue > normalizedCenter)
                {
                    // Value is above center - fill upward from center
                    fillArea = juce::Rectangle<float>(track.getX(), valueY, track.getWidth(), centerY - valueY);
                    gradientStart = juce::Point<float>(fillArea.getCentreX(), centerY);       // Start at center line
                    gradientEnd = juce::Point<float>(fillArea.getCentreX(), valueY);         // End at current value
                }
                else
                {
                    // Value is below center - fill downward from center  
                    fillArea = juce::Rectangle<float>(track.getX(), centerY, track.getWidth(), valueY - centerY);
                    gradientStart = juce::Point<float>(fillArea.getCentreX(), centerY);       // Start at center line
                    gradientEnd = juce::Point<float>(fillArea.getCentreX(), valueY);         // End at current value
                }
                
                // Create gradient from center (brighter) to current position (darker)
                juce::ColourGradient fillGradient(
                    trackColor, gradientStart,                                       // Brighter at center
                    BlueprintColors::background().withAlpha(0.2f), gradientEnd,      // Darker at current value (thumb)
                    false
                );
                
                g.setGradientFill(fillGradient);
                g.fillRect(fillArea);
            }
        }
        
        // Technical outline
        g.setColour(BlueprintColors::blueprintLines());
        g.drawRect(track, scale.getScaledLineThickness());
    }
    
    void drawTickMarks(juce::Graphics& g, juce::Rectangle<float> trackArea)
    {
        auto& scale = GlobalUIScale::getInstance();
        
        g.setColour(BlueprintColors::blueprintLines().withAlpha(0.6f));
        
        auto tickArea = trackArea.reduced(0, scale.getScaled(4));
        
        if (quantizationEnabled && quantizationIncrement > 0.0)
        {
            // Draw quantization step marks
            double displayRange = std::abs(quantizationDisplayMax - quantizationDisplayMin);
            if (displayRange > 0.001) // Ensure valid range
            {
                int numSteps = (int)std::floor(displayRange / quantizationIncrement);
                numSteps = juce::jlimit(1, 50, numSteps); // Limit number of ticks for visual clarity
                
                g.setColour(BlueprintColors::active().withAlpha(0.8f)); // Use active color for quantization ticks
                
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
                            float tickLength = scale.getScaled(8.0f);
                            float tickWidth = scale.getScaled(1.5f); // Slightly thicker for quantization
                            
                            // Quantization tick marks - left side with distinctive style
                            g.fillRect(juce::Rectangle<float>(trackArea.getX() - tickLength - scale.getScaled(2), y - tickWidth/2, tickLength, tickWidth));
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
                float tickLength = isMajor ? scale.getScaled(8.0f) : scale.getScaled(4.0f);
                float tickWidth = scale.getScaled(1.0f); // Consistent thin lines
                
                // Technical tick marks - left side only for cleaner look
                g.fillRect(trackArea.getX() - tickLength - scale.getScaled(2), y - tickWidth/2, tickLength, tickWidth);
            }
        }
    }
    
    void drawSliderThumb(juce::Graphics& g, float centerX, float centerY, juce::Colour trackColor)
    {
        auto& scale = GlobalUIScale::getInstance();
        
        // Blueprint-style flat rectangular thumb
        float thumbWidth = scale.getScaled(28.0f);
        float thumbHeight = scale.getScaled(12.0f);
        
        auto thumbBounds = juce::Rectangle<float>(thumbWidth, thumbHeight)
            .withCentre(juce::Point<float>(centerX, centerY));
        
        // No shadow for flat design
        
        // Solid flat body
        g.setColour(BlueprintColors::panel());
        g.fillRect(thumbBounds);
        
        // Technical outline
        g.setColour(BlueprintColors::blueprintLines());
        g.drawRect(thumbBounds, scale.getScaledLineThickness());
        
        // Horizontal indicator line using track color
        float lineHeight = scale.getScaled(2.0f);
        float lineWidth = thumbWidth - scale.getScaled(6.0f);
        
        auto centerLine = juce::Rectangle<float>(lineWidth, lineHeight)
            .withCentre(thumbBounds.getCentre());
        
        g.setColour(trackColor);
        g.fillRect(centerLine);
    }
    
    // Add blueprint grid drawing method
    void drawBlueprintGrid(juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        auto& scale = GlobalUIScale::getInstance();
        
        g.setColour(BlueprintColors::blueprintLines().withAlpha(0.1f));
        
        int gridSpacing = scale.getScaled(20);
        
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
    
    // Scale change notification method
    void updateForNewScale()
    {
        // This method can be called by components when scale changes
        // All drawing operations already use scaled values, so no specific updates needed
        // But this provides a hook for any future scale-dependent calculations
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
    
    // Scale change notification method
    void updateForNewScale()
    {
        // This method can be called by components when scale changes
        // All drawing operations already use scaled values, so no specific updates needed
        // But this provides a hook for any future scale-dependent calculations
    }
    
private:
    std::map<juce::Button*, juce::Colour> buttonColors;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomButtonLookAndFeel)
};

//End CustomLookAndFeel.h
//=====================
