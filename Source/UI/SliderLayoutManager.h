// SliderLayoutManager.h - Component positioning and bounds calculation for slider controls
#pragma once
#include <JuceHeader.h>
#include "../Core/SliderDisplayManager.h"

//==============================================================================
class SliderLayoutManager
{
public:
    struct SliderBounds
    {
        juce::Rectangle<int> utilityBar;
        juce::Rectangle<int> sliderArea;
        juce::Rectangle<int> valueLabel;
        juce::Rectangle<float> midiIndicator;
        juce::Rectangle<int> lockLabel;
        juce::Rectangle<int> automationArea;
        juce::Rectangle<int> trackBounds;
        juce::Rectangle<int> sliderInteractionBounds;
    };
    
    SliderLayoutManager() = default;
    
    // Calculate all layout bounds for a slider control
    SliderBounds calculateSliderBounds(const juce::Rectangle<int>& totalBounds, bool showAutomation = true) const
    {
        SliderBounds bounds;
        auto area = totalBounds;
        
        // Utility bar at top (16px height)
        bounds.utilityBar = area.removeFromTop(16);
        area.removeFromTop(4); // spacing after utility bar
        
        // Main slider area calculation depends on automation visibility
        if (showAutomation)
        {
            // Normal layout with automation controls
            int automationControlsHeight = 200;
            int availableSliderHeight = area.getHeight() - automationControlsHeight;
            int reducedSliderHeight = (int)(availableSliderHeight * 0.70);
            bounds.sliderArea = area.removeFromTop(reducedSliderHeight);
        }
        else
        {
            // Expanded layout - slider takes most of the remaining space
            // Leave space for value label and controls at bottom
            int bottomControlsHeight = 30; // Space for value display + MIDI indicator + controls
            int expandedSliderHeight = area.getHeight() - bottomControlsHeight;
            bounds.sliderArea = area.removeFromTop(expandedSliderHeight);
        }
        
        bounds.trackBounds = bounds.sliderArea.withWidth(20).withCentre(bounds.sliderArea.getCentre());
        
        // Calculate precise slider interaction bounds for proper thumb alignment
        float thumbHeight = 24.0f;
        int trackY = bounds.trackBounds.getY();
        int trackHeight = bounds.trackBounds.getHeight();
        int thumbHalfHeight = (int)(thumbHeight / 2.0f);
        
        int sliderY = trackY + thumbHalfHeight;
        int sliderHeight = trackHeight - (int)thumbHeight + 3;
        
        bounds.sliderInteractionBounds = juce::Rectangle<int>(
            bounds.trackBounds.getX(), 
            sliderY, 
            bounds.trackBounds.getWidth(), 
            sliderHeight
        );
        
        area.removeFromTop(4); // spacing before value label
        
        if (showAutomation)
        {
            // Normal layout with automation controls below
            // Current value label
            auto labelArea = area.removeFromTop(20);
            bounds.valueLabel = labelArea.reduced(4, 0);
            
            // MIDI activity indicator - positioned above value label on left side
            bounds.midiIndicator = juce::Rectangle<float>(5, labelArea.getY() - 15, 10, 10);
            
            // Lock label - positioned above value label on right side
            int lockLabelX = totalBounds.getWidth() - 25;
            bounds.lockLabel = juce::Rectangle<int>(lockLabelX, labelArea.getY() - 15, 20, 10);
            
            area.removeFromTop(4); // spacing before automation controls
            
            // Remaining area for automation controls
            bounds.automationArea = area;
        }
        else
        {
            // Expanded layout - value label at bottom with padding
            // Current value label at bottom
            auto labelArea = area.removeFromBottom(20);
            bounds.valueLabel = labelArea.reduced(4, 0);
            
            // MIDI activity indicator and lock label positioned above value label
            bounds.midiIndicator = juce::Rectangle<float>(5, labelArea.getY() - 15, 10, 10);
            
            int lockLabelX = totalBounds.getWidth() - 25;
            bounds.lockLabel = juce::Rectangle<int>(lockLabelX, labelArea.getY() - 15, 20, 10);
            
            // No automation area in expanded mode
            bounds.automationArea = juce::Rectangle<int>();
        }
        
        return bounds;
    }
    
    // Calculate visual track bounds for parent component drawing
    juce::Rectangle<int> calculateVisualTrackBounds(const juce::Rectangle<int>& totalBounds, bool showAutomation = true) const
    {
        auto area = totalBounds;
        area.removeFromTop(20); // utility bar + spacing
        
        if (showAutomation)
        {
            // Normal layout with automation controls
            int automationControlsHeight = 200;
            int availableSliderHeight = area.getHeight() - automationControlsHeight;
            int reducedSliderHeight = (int)(availableSliderHeight * 0.70);
            auto sliderArea = area.removeFromTop(reducedSliderHeight);
            return sliderArea.withWidth(20).withCentre(sliderArea.getCentre());
        }
        else
        {
            // Expanded layout - slider takes most space
            int bottomControlsHeight = 30;
            int expandedSliderHeight = area.getHeight() - bottomControlsHeight;
            auto sliderArea = area.removeFromTop(expandedSliderHeight);
            return sliderArea.withWidth(20).withCentre(sliderArea.getCentre());
        }
    }
    
    // Calculate thumb position based on slider value and track bounds
    juce::Point<float> calculateThumbPosition(const juce::Rectangle<int>& trackBounds,
                                             double sliderValue,
                                             double sliderMin,
                                             double sliderMax,
                                             SliderOrientation orientation = SliderOrientation::Normal) const
    {
        auto trackBoundsFloat = trackBounds.toFloat();
        
        // Calculate normalized value
        float norm = juce::jmap<float>(sliderValue, sliderMin, sliderMax, 0.0f, 1.0f);
        
        // For hardware-realistic behavior, thumb center should align with track edges
        float trackTop = trackBoundsFloat.getY() + 4.0f;
        float trackBottom = trackBoundsFloat.getBottom() - 4.0f;
        
        // Apply orientation-specific visual mapping
        float thumbY;
        if (orientation == SliderOrientation::Inverted)
        {
            // For inverted: visually flip the position
            // When slider value is high (near max), show thumb near bottom
            // When slider value is low (near min), show thumb near top
            thumbY = juce::jmap(norm, trackTop, trackBottom);
        }
        else
        {
            // For normal: standard mapping
            // When slider value is high (near max), show thumb near top  
            // When slider value is low (near min), show thumb near bottom
            thumbY = juce::jmap(norm, trackBottom, trackTop);
        }
        
        return juce::Point<float>(trackBoundsFloat.getCentreX(), thumbY);
    }
    
    // Get visual thumb bounds for hit-testing
    juce::Rectangle<float> calculateVisualThumbBounds(const juce::Point<float>& thumbPosition) const
    {
        float thumbWidth = 28.0f;   // Match CustomLookAndFeel::drawSliderThumb
        float thumbHeight = 12.0f;  // Match CustomLookAndFeel::drawSliderThumb
        
        return juce::Rectangle<float>(thumbWidth, thumbHeight).withCentre(thumbPosition);
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderLayoutManager)
};