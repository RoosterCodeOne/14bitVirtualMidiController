// SimpleSliderControl.h - Production Version with Blueprint Technical Drawing Style
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "CustomKnob.h"
#include "CustomLEDInput.h"
#include "Custom3DButton.h"
#include "AutomationVisualizer.h"
#include "Core/AutomationEngine.h"
#include "Core/SliderDisplayManager.h"
#include "Components/SliderInteractionHandler.h"
#include "Components/AutomationControlPanel.h"
#include "UI/SliderLayoutManager.h"

//==============================================================================
// Custom clickable label for lock functionality
class ClickableLabel : public juce::Label
{
public:
    std::function<void()> onClick;
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        if (onClick)
            onClick();
    }
};

//==============================================================================
class SimpleSliderControl : public juce::Component, public juce::Timer
{
public:
    // Import time mode from automation control panel
    using TimeMode = AutomationControlPanel::TimeMode;
    SimpleSliderControl(int sliderIndex, std::function<void(int, int)> midiCallback)
        : index(sliderIndex), sendMidiCallback(midiCallback), sliderColor(juce::Colours::cyan),
  automationControlPanel()
    {
        // Main slider with custom look
        addAndMakeVisible(mainSlider);
        mainSlider.setSliderStyle(juce::Slider::LinearVertical);
        mainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        mainSlider.setRange(0.0, 16383.0, 1.0); // Always 0-16383 internally for MIDI
        
        // Initialize look and feel with default color
        customLookAndFeel.setSliderColor(sliderColor);
        mainSlider.setLookAndFeel(&customLookAndFeel);
        
        // Safe callback - update label and send MIDI
        mainSlider.onValueChange = [this]() {
            if (!automationEngine.isSliderAutomating(index)) // Only update if not currently automating
            {
                double quantizedValue = quantizeValue(mainSlider.getValue());
                if (quantizedValue != mainSlider.getValue()) {
                    // Update slider to quantized value without triggering callback
                    mainSlider.setValue(quantizedValue, juce::dontSendNotification);
                }
                int value = (int)quantizedValue;
                displayManager.setMidiValue(value);
                if (sendMidiCallback)
                    sendMidiCallback(index, value);
                // Trigger parent repaint to update visual thumb position
                if (auto* parent = getParentComponent())
                    parent->repaint();
            }
        };
        
        // Manual override detection
        mainSlider.onDragStart = [this]() {
            // Check if locked - prevent manual dragging
            if (lockState) return;
            
            if (automationEngine.isSliderAutomating(index))
            {
                automationEngine.handleManualOverride(index);
            }
        };
        
        // Slider number label
        addAndMakeVisible(sliderNumberLabel);
        sliderNumberLabel.setText(juce::String(sliderIndex + 1), juce::dontSendNotification);
        sliderNumberLabel.setJustificationType(juce::Justification::centred);
        sliderNumberLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        sliderNumberLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        
        // Lock label (acting as button)
        addAndMakeVisible(lockLabel);
        lockLabel.setText("U", juce::dontSendNotification); // U for Unlocked by default
        lockLabel.setJustificationType(juce::Justification::centred);
        lockLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        lockLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold)); // Large font to fill space
        lockLabel.onClick = [this]() { toggleLock(); };
        
        // Current value label
        addAndMakeVisible(currentValueLabel);
        currentValueLabel.setText("0", juce::dontSendNotification);
        currentValueLabel.setJustificationType(juce::Justification::centred);
        currentValueLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        currentValueLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        // Match LED input font style
        juce::Font ledFont("Monaco", 12.0f, juce::Font::plain);
        if (!ledFont.getTypefaceName().contains("Monaco")) {
            ledFont = juce::Font("Courier New", 12.0f, juce::Font::plain);
        }
        currentValueLabel.setFont(ledFont);
        
        // Automation control panel - handles knobs, buttons, visualizer, target input
        addAndMakeVisible(automationControlPanel);
        automationControlPanel.onGoButtonClicked = [this]() {
            if (automationEngine.isSliderAutomating(index))
                automationEngine.stopAutomation(index);
            else
                startAutomation();
        };
        automationControlPanel.onKnobValueChanged = [this](double newValue) {
            // Knob values changed - no specific action needed here
        };
        
        // Initialize default time mode
        automationControlPanel.setTimeMode(TimeMode::Seconds);
        
        // Set up display manager callbacks
        setupDisplayManager();
        
        // Set up automation engine callbacks
        setupAutomationEngine();
    }
    
    ~SimpleSliderControl()
    {
        // CRITICAL: Stop automation and timer before destruction
        automationEngine.stopAutomation(index);
        stopTimer();
        
        // CRITICAL: Remove look and feel before destruction
        mainSlider.setLookAndFeel(nullptr);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        
        // Calculate layout bounds using SliderLayoutManager
        auto bounds = layoutManager.calculateSliderBounds(area);
        
        // Slider number label in utility bar (centered)
        sliderNumberLabel.setBounds(bounds.utilityBar);
        
        // Position main slider for mouse interaction
        mainSlider.setBounds(bounds.sliderInteractionBounds);
        
        // Current value label
        currentValueLabel.setBounds(bounds.valueLabel);
        
        // MIDI activity indicator and lock label
        midiIndicatorBounds = bounds.midiIndicator;
        lockLabel.setBounds(bounds.lockLabel);
        
        // Automation control panel gets the remaining automation area
        automationControlPanel.setBounds(bounds.automationArea);
    }
    
    void paint(juce::Graphics& g) override
    {
        // Draw MIDI activity indicator above value label - blueprint style
        juce::Colour indicatorColor = BlueprintColors::warning;
        float alpha = midiActivityState ? 1.0f : 0.2f;
        
        g.setColour(indicatorColor.withAlpha(alpha));
        g.fillRect(midiIndicatorBounds);
        
        // Technical outline
        g.setColour(BlueprintColors::blueprintLines);
        g.drawRect(midiIndicatorBounds, 1.0f);
        
        // Draw learn mode corner markers
        if (showLearnMarkers)
        {
            drawLearnModeMarkers(g);
        }
    }
    
    void paintOverChildren(juce::Graphics& g) override
    {
        // Signal flow lines removed for cleaner blueprint aesthetic
    }
    
    double getValue() const { return mainSlider.getValue(); }
    
    void setTimeMode(TimeMode mode)
    {
        automationControlPanel.setTimeMode(mode);
    }
    
    TimeMode getTimeMode() const { return automationControlPanel.getTimeMode(); }
    
    // Methods for parent component to get visual track and thumb positions
    juce::Rectangle<int> getVisualTrackBounds() const 
    {
        return layoutManager.calculateVisualTrackBounds(getLocalBounds());
    }
    
    juce::Point<float> getThumbPosition() const 
    {
        auto trackBounds = getVisualTrackBounds();
        return layoutManager.calculateThumbPosition(trackBounds, mainSlider.getValue(),
                                                   mainSlider.getMinimum(), mainSlider.getMaximum());
    }
    
    // Get visual thumb bounds for hit-testing
    juce::Rectangle<float> getVisualThumbBounds() const
    {
        auto thumbPos = getThumbPosition();
        return layoutManager.calculateVisualThumbBounds(thumbPos);
    }
    
    // Preset support methods
    void setValue(double newValue)
    {
        double quantizedValue = quantizeValue(newValue);
        mainSlider.setValue(quantizedValue, juce::dontSendNotification);
        displayManager.setMidiValue(quantizedValue);
        automationControlPanel.setTargetValue(displayManager.getDisplayValue());
    }
    
    bool isLocked() const { return lockState; }
    
    void setLocked(bool shouldBeLocked)
    {
        if (lockState != shouldBeLocked)
        {
            lockState = shouldBeLocked;
            
            // Update label text and color
            lockLabel.setText(lockState ? "L" : "U", juce::dontSendNotification);
            lockLabel.setColour(juce::Label::textColourId,
                               lockState ? BlueprintColors::warning : BlueprintColors::textSecondary);
            
            // Enable/disable slider interaction
            mainSlider.setInterceptsMouseClicks(!lockState, !lockState);
        }
    }
    
    // Set custom display range - this is the key method for display mapping
    void setDisplayRange(double minVal, double maxVal)
    {
        displayManager.setDisplayRange(minVal, maxVal);
        automationControlPanel.setTargetRange(minVal, maxVal);
        automationControlPanel.setTargetValue(displayManager.getDisplayValue());
    }
    
    // Set slider color
    void setSliderColor(juce::Colour color)
    {
        sliderColor = color;
        customLookAndFeel.setSliderColor(color);
        // Trigger parent repaint since visuals are now drawn there
        if (auto* parent = getParentComponent())
            parent->repaint();
    }
    
    // Set step increment for quantization
    void setStepIncrement(double increment)
    {
        // Validate increment - must be positive or zero for disable
        stepIncrement = juce::jmax(0.0, increment);
        
        bool shouldQuantize = stepIncrement > 0.0;
        customLookAndFeel.setQuantizationEnabled(shouldQuantize);
        
        if (shouldQuantize)
        {
            double displayRange = displayManager.getDisplayMax() - displayManager.getDisplayMin();
            
            // Ensure increment is not too small (causes performance issues) or too large
            if (stepIncrement < 0.001) stepIncrement = 0.001;
            if (stepIncrement > displayRange) stepIncrement = displayRange / 2.0;
            
            // Update tick marks for quantization steps
            customLookAndFeel.setQuantizationIncrement(stepIncrement, 
                displayManager.getDisplayMin(), 
                displayManager.getDisplayMax());
        }
        
        // Trigger parent repaint to update tick marks
        if (auto* parent = getParentComponent())
            parent->repaint();
    }
    
    // Get slider color
    juce::Colour getSliderColor() const
    {
        return sliderColor;
    }
    
    // Debug method to get mainSlider bounds
    juce::Rectangle<int> getMainSliderBounds() const
    {
        return mainSlider.getBounds();
    }
    
    // Trigger MIDI activity indicator
    void triggerMidiActivity()
    {
        midiActivityState = true;
        lastMidiSendTime = juce::Time::getMillisecondCounterHiRes();
        
        // Always restart timer for clean timeout behavior
        startTimer(16); // ~60fps for smooth timeout
        
        repaint(); // Immediate visual feedback
    }
    
    // Timer callback for MIDI activity timeout only
    void timerCallback() override
    {
        if (!midiActivityState) return;
        
        double currentTime = juce::Time::getMillisecondCounterHiRes();
        double elapsed = currentTime - lastMidiSendTime;
        
        // Check for timeout
        if (elapsed > MIDI_ACTIVITY_DURATION)
        {
            midiActivityState = false;
            stopTimer();
            repaint(); // Redraw to show inactive state
        }
    }
    
    // Toggle lock state
    void toggleLock()
    {
        setLocked(!lockState);
    }
    
    void setDelayTime(double delay)
    {
        automationControlPanel.setDelayTime(delay);
    }

    double getDelayTime() const
    {
        return automationControlPanel.getDelayTime();
    }

    void setAttackTime(double attack)
    {
        automationControlPanel.setAttackTime(attack);
    }

    double getAttackTime() const
    {
        return automationControlPanel.getAttackTime();
    }

    void setReturnTime(double returnVal)
    {
        automationControlPanel.setReturnTime(returnVal);
    }

    double getReturnTime() const
    {
        return automationControlPanel.getReturnTime();
    }
    
    void setCurveValue(double curve)
    {
        automationControlPanel.setCurveValue(curve);
    }

    double getCurveValue() const
    {
        return automationControlPanel.getCurveValue();
    }
    
    // For keyboard movement - updates slider without changing target input
    void setValueFromKeyboard(double newValue)
    {
        double quantizedValue = quantizeValue(newValue);
        mainSlider.setValue(quantizedValue, juce::dontSendNotification);
        displayManager.setMidiValue(quantizedValue);
        if (sendMidiCallback)
            sendMidiCallback(index, (int)quantizedValue);
    }
    
    // For MIDI input - updates slider without triggering output (prevents feedback loops)
    void setValueFromMIDI(double newValue)
    {
        double quantizedValue = quantizeValue(newValue);
        mainSlider.setValue(quantizedValue, juce::dontSendNotification);
        displayManager.setMidiValue(quantizedValue);
        // Note: No sendMidiCallback to prevent feedback loops
        
        // Trigger activity indicator to show MIDI input activity
        triggerMidiActivity();
    }
    
    // Callback for slider click (used for learn mode)
    std::function<void()> onSliderClick;
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        bool handled = interactionHandler.handleMouseDown(event, getVisualThumbBounds(), lockState,
                                                         mainSlider.getValue(), onSliderClick);
        
        // Enable/disable normal slider interaction based on whether we're handling custom dragging
        mainSlider.setInterceptsMouseClicks(!handled, !handled);
        
        if (!handled)
        {
            // Pass to base class for normal handling
            juce::Component::mouseDown(event);
        }
    }
    
    void mouseDrag(const juce::MouseEvent& event) override
    {
        bool handled = interactionHandler.handleMouseDrag(event, getVisualTrackBounds().toFloat(),
                                                         mainSlider.getMinimum(), mainSlider.getMaximum(),
                                                         [this](double newValue) {
            mainSlider.setValue(newValue, juce::dontSendNotification);
            displayManager.setMidiValue(newValue);
            if (sendMidiCallback)
                sendMidiCallback(index, (int)newValue);
            
            // Trigger parent repaint to update visual thumb position
            if (auto* parent = getParentComponent())
                parent->repaint();
        });
        
        if (!handled)
        {
            // Pass to base class for normal handling
            juce::Component::mouseDrag(event);
        }
    }
    
    void mouseUp(const juce::MouseEvent& event) override
    {
        bool handled = interactionHandler.handleMouseUp(event);
        
        if (handled)
        {
            // Re-enable normal slider interaction
            mainSlider.setInterceptsMouseClicks(true, true);
        }
        
        // Pass to base class for normal handling
        juce::Component::mouseUp(event);
    }
    
    // Set learn markers visibility
    void setShowLearnMarkers(bool show) 
    { 
        showLearnMarkers = show; 
        repaint(); 
    }
    
private:
    void setupDisplayManager()
    {
        // Set up display manager callbacks
        displayManager.onDisplayTextChanged = [this](const juce::String& text) {
            currentValueLabel.setText(text, juce::dontSendNotification);
        };
        
        // Initialize display with current slider value
        displayManager.setMidiValue(mainSlider.getValue());
        
        // Initialize target LED input with current display value
        automationControlPanel.setTargetValue(displayManager.getDisplayValue());
    }
    
    void setupAutomationEngine()
    {
        // Set up automation value update callback
        automationEngine.onValueUpdate = [this](int sliderIndex, double newValue) {
            if (sliderIndex == index)
            {
                double quantizedValue = quantizeValue(newValue);
                mainSlider.setValue(quantizedValue, juce::dontSendNotification);
                displayManager.setMidiValue(quantizedValue);
                if (sendMidiCallback)
                    sendMidiCallback(index, (int)quantizedValue);
            }
        };
        
        // Set up automation state change callback
        automationEngine.onAutomationStateChanged = [this](int sliderIndex, bool isAutomating) {
            if (sliderIndex == index)
            {
                // Update GO button text through automation control panel if needed
                // (Currently handled by the panel itself)
                
                // Update visualizer state based on automation status
                auto& visualizer = automationControlPanel.getAutomationVisualizer();
                if (isAutomating)
                {
                    // Pass current knob values for precise animation timing
                    visualizer.lockCurveForAutomation(
                        automationControlPanel.getDelayTime(), 
                        automationControlPanel.getAttackTime(), 
                        automationControlPanel.getReturnTime()
                    );
                }
                else
                {
                    visualizer.unlockCurve();
                }
            }
        };
        
        // Animation is now self-contained in the visualizer - no callbacks needed
    }
    
    
    
    void drawLearnModeMarkers(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        float markerSize = 8.0f;
        float markerThickness = 2.0f;
        
        g.setColour(BlueprintColors::warning);
        
        // Top-left corner
        g.fillRect(bounds.getX(), bounds.getY(), markerSize, markerThickness);
        g.fillRect(bounds.getX(), bounds.getY(), markerThickness, markerSize);
        
        // Top-right corner  
        g.fillRect(bounds.getRight() - markerSize, bounds.getY(), markerSize, markerThickness);
        g.fillRect(bounds.getRight() - markerThickness, bounds.getY(), markerThickness, markerSize);
        
        // Bottom-left corner
        g.fillRect(bounds.getX(), bounds.getBottom() - markerThickness, markerSize, markerThickness);
        g.fillRect(bounds.getX(), bounds.getBottom() - markerSize, markerThickness, markerSize);
        
        // Bottom-right corner
        g.fillRect(bounds.getRight() - markerSize, bounds.getBottom() - markerThickness, markerSize, markerThickness);
        g.fillRect(bounds.getRight() - markerThickness, bounds.getBottom() - markerSize, markerThickness, markerSize);
    }
    
    
    
    void validateTargetValue()
    {
        // LED input handles its own validation through automation control panel
        double displayValue = automationControlPanel.getTargetValue();
        automationControlPanel.setTargetValue(displayValue);
    }
    
    void startAutomation()
    {
        if (automationEngine.isSliderAutomating(index)) return;
        
        validateTargetValue();
        double targetDisplayValue = automationControlPanel.getTargetValue();
        displayManager.setTargetDisplayValue(targetDisplayValue);
        double targetMidiValue = displayManager.getTargetMidiValue();
        double startMidiValue = mainSlider.getValue();
        
        // Set up automation parameters
        AutomationEngine::AutomationParams params;
        params.delayTime = automationControlPanel.getDelayTime();
        params.attackTime = automationControlPanel.getAttackTime();
        params.returnTime = automationControlPanel.getReturnTime();
        params.curveValue = automationControlPanel.getCurveValue();
        params.startValue = startMidiValue;
        params.targetValue = targetMidiValue;
        
        // Start automation through engine
        automationEngine.startAutomation(index, params);
    }
    
private:
    // Quantize value to step increments based on display range
    double quantizeValue(double midiValue) const
    {
        if (stepIncrement <= 0.0) return midiValue; // No quantization
        
        // Convert MIDI value to display value for quantization
        double displayValue = displayManager.midiToDisplay(midiValue);
        double displayMin = displayManager.getDisplayMin();
        double displayMax = displayManager.getDisplayMax();
        
        // Handle invalid range
        double displayRange = displayMax - displayMin;
        if (std::abs(displayRange) < 0.001)
        {
            // Range too small - return clamped MIDI value
            return juce::jlimit(0.0, 16383.0, midiValue);
        }
        
        // Handle edge cases
        if (stepIncrement >= std::abs(displayRange))
        {
            // Step is larger than range - snap to closest endpoint
            double middleDisplay = (displayMin + displayMax) / 2.0;
            double distToMin = std::abs(displayValue - displayMin);
            double distToMax = std::abs(displayValue - displayMax);
            double snapTarget = (distToMin <= distToMax) ? displayMin : displayMax;
            return displayManager.displayToMidi(snapTarget);
        }
        
        // Calculate number of steps and quantize
        // Handle both positive and negative ranges
        double relativeValue = displayValue - displayMin;
        double stepNumber = std::round(relativeValue / stepIncrement);
        double quantizedDisplay = displayMin + (stepNumber * stepIncrement);
        
        // Clamp to display range (handles both positive and negative ranges)
        if (displayMin <= displayMax)
        {
            quantizedDisplay = juce::jlimit(displayMin, displayMax, quantizedDisplay);
        }
        else
        {
            quantizedDisplay = juce::jlimit(displayMax, displayMin, quantizedDisplay);
        }
        
        // Convert back to MIDI value and clamp to MIDI range
        double quantizedMidi = displayManager.displayToMidi(quantizedDisplay);
        return juce::jlimit(0.0, 16383.0, quantizedMidi);
    }

public:
    
    
    // Core properties
    int index;
    std::function<void(int, int)> sendMidiCallback;
    juce::Colour sliderColor;
    bool lockState = false;
    double stepIncrement = 0.0; // 0 = disabled, >0 = quantization enabled
    
    // UI Components
    CustomSliderLookAndFeel customLookAndFeel;
    juce::Slider mainSlider;
    juce::Label sliderNumberLabel;
    ClickableLabel lockLabel;
    juce::Label currentValueLabel;
    
    // Modular Components
    AutomationControlPanel automationControlPanel;
    SliderInteractionHandler interactionHandler;
    SliderLayoutManager layoutManager;
    
    // Core Systems
    AutomationEngine automationEngine;
    SliderDisplayManager displayManager;
    
    // MIDI activity indicator variables
    bool midiActivityState = false;
    double lastMidiSendTime = 0.0;
    static constexpr double MIDI_ACTIVITY_DURATION = 100.0; // milliseconds
    juce::Rectangle<float> midiIndicatorBounds;
    
    // MIDI learn markers
    bool showLearnMarkers = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSliderControl)
};
