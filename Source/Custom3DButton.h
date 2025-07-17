// Custom3DButton.h - Protruding 3D Style Button Component
#pragma once
#include <JuceHeader.h>

//==============================================================================
class Custom3DButton : public juce::Button
{
public:
    Custom3DButton(const juce::String& buttonText = "GO") : juce::Button(buttonText)
    {
        setSize(35, 25);
        setButtonText(buttonText);
    }
    
    ~Custom3DButton() = default;
    
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Draw button shadow first (behind button)
        if (!shouldDrawButtonAsDown)
        {
            drawButtonShadow(g, bounds);
        }
        
        // Draw rectangular bezel border (similar to knob bezels)
        drawRectangularBezel(g, bounds);
        
        // Adjust bounds for pressed state
        auto buttonBounds = bounds.reduced(2.0f); // Account for bezel border
        if (shouldDrawButtonAsDown)
        {
            buttonBounds = buttonBounds.translated(1.0f, 1.0f).reduced(1.0f); // Inset and offset for pressed look
        }
        
        // Draw main button body
        drawButtonBody(g, buttonBounds, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        
        // Draw 3D borders
        draw3DBorders(g, buttonBounds, shouldDrawButtonAsDown);
        
        // Draw button text with emboss effect
        drawButtonText(g, buttonBounds, shouldDrawButtonAsDown);
    }
    
private:
    void drawRectangularBezel(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        // Dark metallic bezel ring around button perimeter (rectangular version of knob bezel)
        auto bezelBounds = bounds; // Use same bounds as button for perfect alignment
        
        // Outer bezel ring - dark gunmetal
        juce::ColourGradient bezelGradient(
            juce::Colour(0xFF303030), bezelBounds.getTopLeft(),
            juce::Colour(0xFF404040), bezelBounds.getBottomRight(),
            false
        );
        bezelGradient.addColour(0.5f, juce::Colour(0xFF353535));
        
        g.setGradientFill(bezelGradient);
        g.fillRoundedRectangle(bezelBounds, 4.0f);
        
        // Inner bezel shadow to create recessed effect
        auto innerBezel = bezelBounds.reduced(1.0f);
        g.setColour(juce::Colour(0xFF202020));
        g.drawRoundedRectangle(innerBezel, 4.0f, 1.0f);
        
        // Outer bezel highlight
        g.setColour(juce::Colour(0xFF505050));
        g.drawRoundedRectangle(bezelBounds, 4.0f, 0.5f);
    }

    void drawButtonShadow(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        // Drop shadow for raised appearance
        auto shadowBounds = bounds.translated(2.0f, 2.0f);
        g.setColour(juce::Colour(0x40000000));
        g.fillRoundedRectangle(shadowBounds, 4.0f);
    }
    
    void drawButtonBody(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHighlighted, bool isPressed)
    {
        juce::ColourGradient buttonGradient;
        
        if (isPressed)
        {
            // Pressed: Darker gradient (sunken appearance)
            buttonGradient = juce::ColourGradient(
                juce::Colour(0xFFA0A0A0), bounds.getTopLeft(),
                juce::Colour(0xFF808080), bounds.getBottomRight(),
                false
            );
            buttonGradient.addColour(0.3f, juce::Colour(0xFF909090));
        }
        else if (isHighlighted)
        {
            // Hover: Slightly brighter than normal
            buttonGradient = juce::ColourGradient(
                juce::Colour(0xFFF8F8F8), bounds.getTopLeft(),
                juce::Colour(0xFFC0C0C0), bounds.getBottomRight(),
                false
            );
            buttonGradient.addColour(0.3f, juce::Colour(0xFFE0E0E0));
        }
        else
        {
            // Normal: Light metallic gradient (raised appearance)
            buttonGradient = juce::ColourGradient(
                juce::Colour(0xFFF0F0F0), bounds.getTopLeft(),
                juce::Colour(0xFFB0B0B0), bounds.getBottomRight(),
                false
            );
            buttonGradient.addColour(0.3f, juce::Colour(0xFFD0D0D0));
        }
        
        g.setGradientFill(buttonGradient);
        g.fillRoundedRectangle(bounds, 4.0f);
    }
    
    void draw3DBorders(juce::Graphics& g, juce::Rectangle<float> bounds, bool isPressed)
    {
        if (isPressed)
        {
            // Pressed: Dark top/left edges (inset shadow)
            g.setColour(juce::Colour(0xFF606060));
            g.drawLine(juce::Line<float>(bounds.getTopLeft(), bounds.getTopRight()), 1.0f);
            g.drawLine(juce::Line<float>(bounds.getTopLeft(), bounds.getBottomLeft()), 1.0f);
            
            // Light bottom/right edges
            g.setColour(juce::Colour(0xFFE0E0E0));
            g.drawLine(juce::Line<float>(bounds.getBottomLeft(), bounds.getBottomRight()), 1.0f);
            g.drawLine(juce::Line<float>(bounds.getTopRight(), bounds.getBottomRight()), 1.0f);
        }
        else
        {
            // Raised: Light top/left edges (highlight)
            g.setColour(juce::Colour(0xFFFFFFFF));
            g.drawLine(juce::Line<float>(bounds.getTopLeft(), bounds.getTopRight()), 1.0f);
            g.drawLine(juce::Line<float>(bounds.getTopLeft(), bounds.getBottomLeft()), 1.0f);
            
            // Dark bottom/right edges (shadow)
            g.setColour(juce::Colour(0xFF808080));
            g.drawLine(juce::Line<float>(bounds.getBottomLeft(), bounds.getBottomRight()), 1.0f);
            g.drawLine(juce::Line<float>(bounds.getTopRight(), bounds.getBottomRight()), 1.0f);
            
            // Inner highlight for extra 3D effect
            auto innerBounds = bounds.reduced(1.0f);
            g.setColour(juce::Colour(0x60FFFFFF));
            g.drawLine(juce::Line<float>(innerBounds.getTopLeft(), innerBounds.getTopRight()), 1.0f);
            g.drawLine(juce::Line<float>(innerBounds.getTopLeft(), innerBounds.getBottomLeft()), 1.0f);
        }
        
        // Outer border
        g.setColour(juce::Colour(0xFF606060));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    }
    
    void drawButtonText(juce::Graphics& g, juce::Rectangle<float> bounds, bool isPressed)
    {
        auto text = getButtonText();
        if (text.isEmpty())
            return;
            
        // Set font - 10pt bold
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        
        // Text emboss effect
        if (!isPressed)
        {
            // Light shadow below text for raised effect
            g.setColour(juce::Colour(0x60FFFFFF));
            g.drawText(text, bounds.translated(0, 1), juce::Justification::centred);
            
            // Dark shadow above text for depth
            g.setColour(juce::Colour(0x40000000));
            g.drawText(text, bounds.translated(0, -0.5f), juce::Justification::centred);
        }
        else
        {
            // Pressed: darker text, different shadow
            g.setColour(juce::Colour(0x30000000));
            g.drawText(text, bounds.translated(0.5f, 0.5f), juce::Justification::centred);
        }
        
        // Main text
        if (isPressed)
            g.setColour(juce::Colour(0xFF404040)); // Darker when pressed
        else
            g.setColour(juce::Colour(0xFF202020)); // Normal dark text
            
        g.drawText(text, bounds, juce::Justification::centred);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Custom3DButton)
};