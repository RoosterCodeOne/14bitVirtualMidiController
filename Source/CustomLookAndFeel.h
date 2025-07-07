// CustomLookAndFeel.h ---------------
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
        auto value = slider.getValue();
        float norm = juce::jmap<float>(value, slider.getMinimum(), slider.getMaximum(), 0.0f, 1.0f);
        sliderPos = juce::jmap(norm, (float)y + height - 4.0f, (float)y + 4.0f);

        int sidePadding = 4;
        int topBottomPadding = 4;
        auto trackBounds = juce::Rectangle<float>((float)x + sidePadding, (float)y + topBottomPadding,
                                                  (float)width - 2 * sidePadding, (float)height - 2 * topBottomPadding);
        auto filled = trackBounds.withTop(sliderPos);

        // Background track
        g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
        g.fillRoundedRectangle(trackBounds, 8.0f);

        // Filled portion (value) - now uses stored color
        g.setColour(sliderColor);
        g.fillRoundedRectangle(filled, 8.0f);

        // Thumb
        auto thumbRadius = 8.0f;
        g.setColour(juce::Colours::white);
        g.fillEllipse(trackBounds.getCentreX() - thumbRadius, sliderPos - thumbRadius,
                      thumbRadius * 2.0f, thumbRadius * 2.0f);

        // Thumb border
        g.setColour(juce::Colours::darkgrey);
        g.drawEllipse(trackBounds.getCentreX() - thumbRadius, sliderPos - thumbRadius,
                      thumbRadius * 2.0f, thumbRadius * 2.0f, 1.5f);
    }

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;
        layout.sliderBounds = slider.getLocalBounds();
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
    
private:
    juce::Colour sliderColor;
};

//End CustomLookAndFeel.h
//=====================
