// SliderInteractionHandler.h - Mouse interaction and dragging logic for sliders
#pragma once
#include <JuceHeader.h>
#include "../Core/SliderDisplayManager.h"

//==============================================================================
class SliderInteractionHandler
{
public:
    SliderInteractionHandler() = default;
    
    // Handle mouse down events
    bool handleMouseDown(const juce::MouseEvent& event,
                        const juce::Rectangle<float>& visualThumbBounds,
                        bool isLocked,
                        double currentSliderValue,
                        std::function<void()> onSliderClick = nullptr)
    {
        // Handle learn mode (only if callback is set)
        if (onSliderClick)
            onSliderClick();
        
        auto localPos = event.getPosition().toFloat();
        
        if (visualThumbBounds.contains(localPos))
        {
            // Check if locked - prevent thumb dragging
            if (isLocked) return false;
            
            // Clicking on thumb - initiate grab behavior
            isDraggingThumb = true;
            dragStartValue = currentSliderValue;
            dragStartY = localPos.y;
            return true; // Handled - disable normal slider interaction
        }
        else
        {
            // Clicking outside thumb - allow normal jump-to-position behavior
            isDraggingThumb = false;
            return false; // Not handled - enable normal slider interaction
        }
    }
    
    // Handle mouse drag events
    bool handleMouseDrag(const juce::MouseEvent& event,
                        const juce::Rectangle<float>& trackBounds,
                        double sliderMin,
                        double sliderMax,
                        std::function<void(double)> onValueChanged,
                        SliderOrientation orientation = SliderOrientation::Normal)
    {
        if (isDraggingThumb)
        {
            // Custom thumb dragging behavior
            auto localPos = event.getPosition().toFloat();
            
            // Calculate drag distance based on orientation
            float dragDistance = dragStartY - localPos.y; // Inverted because Y increases downward
            
            // For inverted orientation, invert the drag direction
            if (orientation == SliderOrientation::Inverted)
            {
                dragDistance = -dragDistance;
            }
            
            // Convert drag distance to value change
            float trackHeight = trackBounds.getHeight();
            float valueRange = sliderMax - sliderMin;
            float valueDelta = (dragDistance / trackHeight) * valueRange;
            
            // Apply relative change from starting position
            double newValue = juce::jlimit(sliderMin, sliderMax, dragStartValue + valueDelta);
            
            if (onValueChanged)
                onValueChanged(newValue);
            
            return true; // Handled
        }
        
        return false; // Not handled
    }
    
    // Handle mouse up events
    bool handleMouseUp(const juce::MouseEvent& event)
    {
        if (isDraggingThumb)
        {
            // End custom thumb dragging
            isDraggingThumb = false;
            return true; // Handled - re-enable normal slider interaction
        }
        
        return false; // Not handled
    }
    
    // Check if currently dragging thumb
    bool isDragging() const { return isDraggingThumb; }
    
    // Reset interaction state
    void reset()
    {
        isDraggingThumb = false;
        dragStartValue = 0.0;
        dragStartY = 0.0f;
    }
    
private:
    bool isDraggingThumb = false;
    double dragStartValue = 0.0;
    float dragStartY = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderInteractionHandler)
};