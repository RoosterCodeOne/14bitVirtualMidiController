// CustomLEDInput.h - Retro LED Display Style Input Component
#pragma once
#include <JuceHeader.h>
#include "UI/GlobalUIScale.h"

//==============================================================================
class CustomLEDInput : public juce::TextEditor, public GlobalUIScale::ScaleChangeListener
{
public:
    CustomLEDInput() : juce::TextEditor()
    {
        setupLEDStyle();
        
        // Register for scale change notifications
        GlobalUIScale::getInstance().addScaleChangeListener(this);
    }
    
    ~CustomLEDInput()
    {
        // Unregister from scale change notifications
        GlobalUIScale::getInstance().removeScaleChangeListener(this);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Draw LED display background and border
        drawLEDBackground(g, bounds);
        
        // Let parent handle text rendering with our custom styling
        juce::TextEditor::paint(g);
        
        // Add optional scan lines effect
        drawScanLines(g, bounds);
    }
    
    void setValidRange(double minValue, double maxValue)
    {
        minVal = minValue;
        maxVal = maxValue;
    }
    
    void setValidatedValue(double value)
    {
        value = juce::jlimit(minVal, maxVal, value);
        
        juce::String displayText;
        if (std::abs(value - std::round(value)) < 0.01)
            displayText = juce::String((int)std::round(value));
        else
            displayText = juce::String(value, 2);
            
        setText(displayText, false);
    }
    
    double getValidatedValue() const
    {
        auto text = getText();
        if (text.isEmpty())
            return minVal;
            
        double value = text.getDoubleValue();
        return juce::jlimit(minVal, maxVal, value);
    }
    
    // Override to validate input on focus lost
    void focusLost(FocusChangeType cause) override
    {
        validateAndFormat();
        juce::TextEditor::focusLost(cause);
    }
    
    // Override to validate input on return key
    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::returnKey)
        {
            validateAndFormat();
            return true;
        }
        return juce::TextEditor::keyPressed(key);
    }
    
    // Scale change notification implementation
    void scaleFactorChanged(float newScale) override
    {
        updateFontScale();
    }
    
private:
    double minVal = 0.0;
    double maxVal = 16383.0;
    
    void setupLEDStyle()
    {
        // Set size
        setSize(50, 20);
        
        // Numeric input only
        setInputRestrictions(0, "-0123456789.");
        
        // Set up LED-style font with scaling
        updateFontScale();
        
        // LED colors - white text on dark green/black background
        setColour(juce::TextEditor::textColourId, juce::Colours::white); // White LED text
        setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF001100)); // Very dark green
        setColour(juce::TextEditor::highlightColourId, juce::Colour(0x4000FF00)); // Green highlight
        setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack); // No outline
        setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xFF00AA00)); // Darker green focus
        setColour(juce::CaretComponent::caretColourId, juce::Colour(0xFF00FF00)); // Bright green caret
        
        // Center text alignment
        setJustification(juce::Justification::centred);
        
        // Single line only
        setMultiLine(false);
        setReturnKeyStartsNewLine(false);
        
        // Border settings
        setBorder(juce::BorderSize<int>(2));
    }
    
    void updateFontScale()
    {
        // Update LED-style font with current scale
        auto& scale = GlobalUIScale::getInstance();
        juce::Font ledFont = scale.getScaledFont("Monaco", 12.0f, juce::Font::plain);
        if (!ledFont.getTypefaceName().contains("Monaco"))
        {
            // Fallback to Courier New if Monaco isn't available
            ledFont = scale.getScaledFont("Courier New", 12.0f, juce::Font::plain);
        }
        setFont(ledFont);
        
        // Apply font to all existing text and force text editor to recalculate layout
        applyFontToAllText(ledFont);
        
        // Force immediate visual update and text layout recalculation
        repaint();
        resized();
    }
    
    void drawLEDBackground(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        // Outer metallic housing with inset effect
        auto outerBounds = bounds;
        g.setColour(juce::Colour(0xFF404040)); // Dark metallic gray
        g.fillRoundedRectangle(outerBounds, 3.0f);
        
        // Inset border effect - dark shadow on top/left, light on bottom/right
        auto borderBounds = bounds.reduced(0.5f);
        g.setColour(juce::Colour(0xFF606060)); // Light edge (bottom/right)
        g.drawRoundedRectangle(borderBounds, 2.5f, 1.0f);
        
        g.setColour(juce::Colour(0xFF202020)); // Dark edge (top/left)
        g.drawLine(borderBounds.getX(), borderBounds.getY(), 
                  borderBounds.getRight(), borderBounds.getY(), 1.0f); // Top
        g.drawLine(borderBounds.getX(), borderBounds.getY(), 
                  borderBounds.getX(), borderBounds.getBottom(), 1.0f); // Left
        
        // Inner LED display area with dark green background
        auto innerBounds = bounds.reduced(2.0f);
        
        // Vintage LED background - very dark green with subtle gradient
        juce::ColourGradient ledGradient(
            juce::Colour(0xFF001100), innerBounds.getTopLeft(), // Very dark green
            juce::Colour(0xFF000800), innerBounds.getBottomRight(), // Even darker green/black
            false
        );
        g.setGradientFill(ledGradient);
        g.fillRoundedRectangle(innerBounds, 1.5f);
        
        // Inner glow effect (subtle dark green glow)
        if (hasKeyboardFocus(true) || getText().isNotEmpty())
        {
            auto glowBounds = innerBounds.reduced(0.5f);
            g.setColour(juce::Colour(0x1000AA00)); // Very subtle dark green glow
            g.fillRoundedRectangle(glowBounds, 1.0f);
        }
        
        // Recessed inner shadow for authentic inset look
        g.setColour(juce::Colour(0x40000000)); // Dark shadow
        auto shadowBounds = innerBounds.removeFromTop(1.0f);
        g.fillRoundedRectangle(shadowBounds, 1.0f);
    }
    
    void drawScanLines(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        // Vintage LED scan lines for authentic retro look
        auto scanArea = bounds.reduced(3.0f);
        g.setColour(juce::Colour(0x06004400)); // Very subtle dark green lines
        
        // Draw thin horizontal scan lines every 2 pixels
        for (float y = scanArea.getY() + 1; y < scanArea.getBottom(); y += 2)
        {
            g.drawHorizontalLine((int)y, scanArea.getX(), scanArea.getRight());
        }
    }
    
    void validateAndFormat()
    {
        auto text = getText();
        if (text.isEmpty())
        {
            setText(juce::String(minVal), false);
            return;
        }
        
        double value = text.getDoubleValue();
        value = juce::jlimit(minVal, maxVal, value);
        
        // Format nicely
        juce::String displayText;
        if (std::abs(value - std::round(value)) < 0.01)
            displayText = juce::String((int)std::round(value));
        else
            displayText = juce::String(value, 2);
            
        setText(displayText, false);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLEDInput)
};