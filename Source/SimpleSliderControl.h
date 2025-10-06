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
#include "Core/AutomationConfigManager.h"
#include "Core/AutomationConfig.h"
#include "Components/SliderInteractionHandler.h"
#include "Components/AutomationControlPanel.h"
#include "Components/SliderLearnZones.h"
#include "UI/GlobalUIScale.h"
#include "UI/SliderLayoutManager.h"
#include "UI/AutomationContextMenu.h"
#include "UI/SliderContextMenu.h"
#include "UI/AutomationSaveDialog.h"
#include "UI/AutomationConfigManagementWindow.h"
#include "UI/GlobalUIScale.h"

//==============================================================================
// MIDI Input Mode for slider control
enum class MidiInputMode
{
    Direct,    // Traditional direct Hardware MIDI -> App Slider connection
    Deadzone   // Smoothed Hardware MIDI -> Helper Knob -> App Slider with deadzone processing
};

//==============================================================================
// Invisible helper knob for deadzone smoothing
class InvisibleHelperKnob : public juce::Component, public juce::Timer
{
public:
    InvisibleHelperKnob()
    {
        // Initialize to center position (8191.5 for 14-bit)
        currentPosition = 8191.5;
        targetPosition = 8191.5;
        lastMidiValue = 8191.5;

        // Start smoothing timer
        startTimer(16); // ~60fps for smooth interpolation
    }

    ~InvisibleHelperKnob() override
    {
        stopTimer();
    }

    // Set the MIDI input mode for this helper knob
    void setMidiInputMode(MidiInputMode mode)
    {
        midiMode = mode;

        if (mode == MidiInputMode::Direct)
        {
            // In direct mode, disable smoothing and pass through values immediately
            currentPosition = lastMidiValue;
            targetPosition = lastMidiValue;
            isSmoothing = false;
            stopTimer();
        }
        else
        {
            // In deadzone mode, enable smoothing
            startTimer(16);
        }
    }

    // Process incoming MIDI value based on current mode
    void processMidiInput(double midiValue, bool isInDeadzone = false)
    {
        lastMidiValue = midiValue;

        if (midiMode == MidiInputMode::Direct)
        {
            // Direct mode: immediate 1:1 mapping
            currentPosition = midiValue;
            targetPosition = midiValue;

            // Immediately notify listeners
            if (onValueChanged)
                onValueChanged(currentPosition);
        }
        else
        {
            // Deadzone mode: process through deadzone logic
            processDeadzoneInput(midiValue, isInDeadzone);
        }
    }

    // Get current output value for the app slider
    double getCurrentValue() const { return currentPosition; }

    // Get the last received MIDI value
    double getLastMidiValue() const { return lastMidiValue; }

    // Check if currently smoothing to target
    bool isSmoothingActive() const { return isSmoothing; }

    // Callback when value changes
    std::function<void(double newValue)> onValueChanged;

private:
    void timerCallback() override
    {
        if (midiMode == MidiInputMode::Direct || !isSmoothing)
            return;

        // Smooth interpolation towards target
        double difference = targetPosition - currentPosition;

        if (std::abs(difference) < 0.1)
        {
            // Close enough - snap to target and stop smoothing
            currentPosition = targetPosition;
            isSmoothing = false;

            if (onValueChanged)
                onValueChanged(currentPosition);
            return;
        }

        // Apply smoothing with configurable rate
        double smoothingRate = 0.1; // Adjust for smoother/faster response
        currentPosition += difference * smoothingRate;

        if (onValueChanged)
            onValueChanged(currentPosition);
    }

    void processDeadzoneInput(double midiValue, bool isInDeadzone)
    {
        if (!isInDeadzone)
        {
            // Outside deadzone - update target position
            targetPosition = midiValue;

            // Start smoothing if not already active
            if (!isSmoothing)
            {
                isSmoothing = true;
                startTimer(16);
            }
        }
        else
        {
            // Inside deadzone - maintain current target but allow continued smoothing
            // This preserves the last valid position when hardware stops sending
        }
    }

    MidiInputMode midiMode = MidiInputMode::Deadzone;
    double currentPosition = 8191.5;
    double targetPosition = 8191.5;
    double lastMidiValue = 8191.5;
    bool isSmoothing = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InvisibleHelperKnob)
};

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
class SimpleSliderControl : public juce::Component, 
                            public juce::Timer, 
                            public GlobalUIScale::ScaleChangeListener
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
                double rawValue = mainSlider.getValue();
                
                // For inverted orientation, if this was a click-to-jump (not a drag), invert the value
                if (displayManager.getOrientation() == SliderOrientation::Inverted && 
                    !interactionHandler.isDragging() && !isSettingValueProgrammatically)
                {
                    // Invert the value: clicking high on track should give low value
                    double min = mainSlider.getMinimum();
                    double max = mainSlider.getMaximum();
                    rawValue = max - (rawValue - min);
                    // Set the inverted value without triggering this callback again
                    isSettingValueProgrammatically = true;
                    mainSlider.setValue(rawValue, juce::dontSendNotification);
                    isSettingValueProgrammatically = false;
                }
                
                double quantizedValue = quantizeValue(rawValue);
                if (quantizedValue != rawValue) {
                    // Update slider to quantized value without triggering callback
                    isSettingValueProgrammatically = true;
                    mainSlider.setValue(quantizedValue, juce::dontSendNotification);
                    isSettingValueProgrammatically = false;
                }
                int value = (int)quantizedValue;
                // Manual slider change - allow snap on value changes
                displayManager.setMidiValueWithSnap(value, true);
                
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
            
            // Set drag state for movement-aware snapping
            displayManager.setDragState(true);
            
            if (automationEngine.isSliderAutomating(index))
            {
                automationEngine.handleManualOverride(index);
            }
        };
        
        // Drag end detection for snap behavior
        mainSlider.onDragEnd = [this]() {
            // Clear drag state - allows snapping to occur
            displayManager.setDragState(false);
            
            // Trigger a final value update that can snap after drag ends
            juce::Timer::callAfterDelay(50, [this]() {
                double currentValue = mainSlider.getValue();
                displayManager.setMidiValueWithSnap(currentValue, true); // Allow snap after drag ends
            });
        };
        
        // Slider number label
        addAndMakeVisible(sliderNumberLabel);
        sliderNumberLabel.setText(juce::String(sliderIndex + 1), juce::dontSendNotification);
        sliderNumberLabel.setJustificationType(juce::Justification::centred);
        sliderNumberLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        sliderNumberLabel.setFont(GlobalUIScale::getInstance().getScaledFont(11.0f).boldened());
        
        // Lock label (acting as button)
        addAndMakeVisible(lockLabel);
        lockLabel.setText("U", juce::dontSendNotification); // U for Unlocked by default
        lockLabel.setJustificationType(juce::Justification::centred);
        lockLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        lockLabel.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened()); // Large font to fill space
        lockLabel.onClick = [this]() { toggleLock(); };
        
        // Current value label
        addAndMakeVisible(currentValueLabel);
        currentValueLabel.setText("0", juce::dontSendNotification);
        currentValueLabel.setJustificationType(juce::Justification::centred);
        currentValueLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        currentValueLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        // Match LED input font style with proper scaling
        auto& scale = GlobalUIScale::getInstance();
        juce::Font ledFont(juce::FontOptions("Monaco", scale.getScaled(12.0f), juce::Font::plain));
        if (!ledFont.getTypefaceName().contains("Monaco")) {
            ledFont = juce::Font(juce::FontOptions("Courier New", scale.getScaled(12.0f), juce::Font::plain));
        }
        currentValueLabel.setFont(ledFont);
        
        // Automation control panel - handles knobs, buttons, visualizer, target input
        addAndMakeVisible(automationControlPanel);
        automationControlPanel.onGoButtonClicked = [this]() {
            if (automationEngine.isSliderAutomating(index))
            {
                automationEngine.stopAutomation(index);
                automationControlPanel.updateGoButtonState(false);
                if (onAutomationToggled) onAutomationToggled(index, false);
            }
            else
            {
                startAutomation();
                automationControlPanel.updateGoButtonState(automationEngine.isSliderAutomating(index));
                if (onAutomationToggled) onAutomationToggled(index, true);
            }
        };
        automationControlPanel.onKnobValueChanged = [this](double newValue) {
            // Knob values changed - no specific action needed here
        };
        automationControlPanel.onTimeModeChanged = [this](AutomationControlPanel::TimeMode mode) {
            if (onTimeModeChanged) onTimeModeChanged(index, mode);
        };
        
        // Set up context menu callback for automation area
        automationControlPanel.onContextMenuRequested = [this](juce::Point<int> automationLocalPos) {
            // Convert from automation panel coordinates to slider control coordinates
            auto sliderLocalPos = automationControlPanel.getBounds().getTopLeft() + automationLocalPos;
            showAutomationContextMenu(sliderLocalPos);
        };
        
        // Set up learn mode callback for automation control panel
        automationControlPanel.onLearnModeTargetClicked = [this](MidiTargetType targetType, int sliderIndex) {
            if (onLearnModeTargetClicked)
                onLearnModeTargetClicked(targetType, sliderIndex);
        };
        
        // Initialize default time mode
        automationControlPanel.setTimeMode(TimeMode::Seconds);
        
        // Set up display manager callbacks
        setupDisplayManager();
        
        // Set up automation engine callbacks
        setupAutomationEngine();
        
        // Initialize learn zones
        setupLearnZones();

        // Initialize helper knob system (starts in Direct mode)
        setupHelperKnobSystem();

        // Initialize GO button state based on current automation status
        automationControlPanel.updateGoButtonState(automationEngine.isSliderAutomating(index));
        
        // Register for scale change notifications
        GlobalUIScale::getInstance().addScaleChangeListener(this);
    }
    
    ~SimpleSliderControl()
    {
        // CRITICAL: Stop automation and timer before destruction
        automationEngine.stopAutomation(index);
        stopTimer();

        // Clean up helper knob system
        if (helperKnob)
        {
            removeChildComponent(helperKnob.get());
            helperKnob.reset();
        }

        // Remove scale change listener
        GlobalUIScale::getInstance().removeScaleChangeListener(this);

        // CRITICAL: Remove look and feel before destruction
        mainSlider.setLookAndFeel(nullptr);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        
        // Check automation visibility setting
        bool showAutomation = automationControlPanel.isVisible();
        
        // Calculate layout bounds using SliderLayoutManager with automation visibility
        auto bounds = layoutManager.calculateSliderBounds(area, showAutomation);
        
        // Slider number label in utility bar (centered)
        sliderNumberLabel.setBounds(bounds.utilityBar);
        
        // Position main slider for mouse interaction - ensure bounds match visual track
        if (showAutomation)
        {
            mainSlider.setBounds(bounds.sliderInteractionBounds);
        }
        else
        {
            // For expanded layout, calculate the visual track bounds and use them for interaction
            auto visualTrackBounds = layoutManager.calculateVisualTrackBounds(area, showAutomation);
            
            // Extend the interaction bounds to cover the full expanded slider height
            // but keep the visual track width for consistency
            auto expandedInteractionBounds = juce::Rectangle<int>(
                visualTrackBounds.getX(),
                bounds.sliderArea.getY(),
                visualTrackBounds.getWidth(),
                bounds.sliderArea.getHeight()
            );
            
            mainSlider.setBounds(expandedInteractionBounds);
        }
        
        // Current value label
        currentValueLabel.setBounds(bounds.valueLabel);
        
        // MIDI activity indicator and lock label
        midiIndicatorBounds = bounds.midiIndicator;
        lockLabel.setBounds(bounds.lockLabel);
        
        // Automation control panel gets the remaining automation area (empty when hidden)
        if (showAutomation)
        {
            automationControlPanel.setBounds(bounds.automationArea);
        }
        
        // Update learn zone bounds after layout
        updateLearnZoneBounds();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto& scale = GlobalUIScale::getInstance();
        
        // Draw MIDI activity indicator above value label - blueprint style
        juce::Colour indicatorColor = BlueprintColors::warning;
        float alpha = midiActivityState ? 1.0f : 0.2f;
        
        g.setColour(indicatorColor.withAlpha(alpha));
        g.fillRect(midiIndicatorBounds);
        
        // Technical outline
        g.setColour(BlueprintColors::blueprintLines);
        g.drawRect(midiIndicatorBounds, scale.getScaledLineThickness());
        
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
    double getDisplayValue() const { return displayManager.getDisplayValue(); }
    double getCenterValue() const { return displayManager.getCenterValue(); }
    bool isInSnapZone(double displayValue) const { return displayManager.isInSnapZone(displayValue); }
    
    void setTimeMode(TimeMode mode)
    {
        automationControlPanel.setTimeMode(mode);
    }
    
    TimeMode getTimeMode() const { return automationControlPanel.getTimeMode(); }
    
    // Methods for parent component to get visual track and thumb positions
    juce::Rectangle<int> getVisualTrackBounds() const 
    {
        bool showAutomation = automationControlPanel.isVisible();
        return layoutManager.calculateVisualTrackBounds(getLocalBounds(), showAutomation);
    }
    
    juce::Point<float> getThumbPosition() const 
    {
        auto trackBounds = getVisualTrackBounds();
        return layoutManager.calculateThumbPosition(trackBounds, mainSlider.getValue(),
                                                   mainSlider.getMinimum(), mainSlider.getMaximum(),
                                                   displayManager.getOrientation());
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
        isSettingValueProgrammatically = true;
        mainSlider.setValue(quantizedValue, juce::dontSendNotification);
        isSettingValueProgrammatically = false;
        
        // Use snap-aware method for external value setting
        displayManager.setMidiValueWithSnap(quantizedValue, true);
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
            
            // Notify about lock state change
            if (onLockStateChanged) onLockStateChanged(index, lockState);
        }
    }
    
    // Set custom display range - this is the key method for display mapping
    void setDisplayRange(double minVal, double maxVal)
    {
        displayManager.setDisplayRangePreservingCurrentValue(minVal, maxVal);
        // Update step increment to ensure smart formatting is recalculated
        displayManager.setStepIncrement(stepIncrement);
        automationControlPanel.setTargetRange(minVal, maxVal);
        automationControlPanel.setTargetValue(displayManager.getDisplayValue());
        
        // Update the main slider with the preserved MIDI value to maintain position
        isSettingValueProgrammatically = true;
        mainSlider.setValue(displayManager.getMidiValue(), juce::dontSendNotification);
        isSettingValueProgrammatically = false;
        
        // Trigger parent repaint to update visual thumb position
        if (auto* parent = getParentComponent())
            parent->repaint();
    }
    
    // Set slider orientation - this is crucial for fixing orientation persistence
    void setOrientation(SliderOrientation orientation)
    {
        // Store current position before orientation change
        double currentMidiValue = displayManager.getMidiValue();
        
        displayManager.setOrientation(orientation);
        
        // Restore the same MIDI value to maintain logical position
        isSettingValueProgrammatically = true;
        mainSlider.setValue(currentMidiValue, juce::dontSendNotification);
        isSettingValueProgrammatically = false;
        
        repaint(); // Force visual update for orientation changes
    }
    
    // Set bipolar settings - required for bipolar center calculation
    void setBipolarSettings(const BipolarSettings& settings)
    {
        // Store current position before bipolar settings change
        double currentMidiValue = displayManager.getMidiValue();
        
        displayManager.setBipolarSettings(settings);
        
        // Restore the same MIDI value to maintain logical position
        isSettingValueProgrammatically = true;
        mainSlider.setValue(currentMidiValue, juce::dontSendNotification);
        isSettingValueProgrammatically = false;
        
        repaint(); // Force visual update for bipolar changes
    }
    
    // Get current orientation
    SliderOrientation getOrientation() const
    {
        return displayManager.getOrientation();
    }
    
    // Get current bipolar settings
    BipolarSettings getBipolarSettings() const
    {
        return displayManager.getBipolarSettings();
    }
    
    // Update snap-to-center settings
    void setSnapToCenter(bool enabled)
    {
        auto settings = getBipolarSettings();
        settings.snapToCenter = enabled;
        setBipolarSettings(settings);
    }
    
    void setSnapThreshold(SnapThreshold threshold)
    {
        auto settings = getBipolarSettings();
        settings.snapThreshold = threshold;
        setBipolarSettings(settings);
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
    
    // Update slider display name
    void setDisplayName(const juce::String& name)
    {
        if (name.isEmpty())
            sliderNumberLabel.setText(juce::String(index + 1), juce::dontSendNotification); // Just the number
        else
            sliderNumberLabel.setText(name, juce::dontSendNotification); // Custom name
        sliderNumberLabel.repaint();
    }
    
    // Set step increment for quantization
    void setStepIncrement(double increment)
    {
        // Validate increment - must be positive or zero for disable
        stepIncrement = juce::jmax(0.0, increment);
        
        // Update display manager with new step increment for smart formatting
        displayManager.setStepIncrement(stepIncrement);
        
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
        // Enable keyboard navigation mode for smart timing
        displayManager.setKeyboardNavigationMode(true);
        
        double quantizedValue = quantizeValue(newValue);
        isSettingValueProgrammatically = true;
        mainSlider.setValue(quantizedValue, juce::dontSendNotification);
        isSettingValueProgrammatically = false;
        
        // Use snap-aware method for keyboard input
        displayManager.setMidiValueWithSnap(quantizedValue, true);
        if (sendMidiCallback)
            sendMidiCallback(index, (int)quantizedValue);
            
        // Reset keyboard navigation mode after a short delay
        juce::Timer::callAfterDelay(200, [this]() {
            displayManager.setKeyboardNavigationMode(false);
        });
    }
    
    // Get the effective step size for keyboard movement (128 for 7-bit, 1 for 14-bit)
    double getEffectiveStepSize() const
    {
        // Check if this is 7-bit mode
        if (is14BitMode && !is14BitMode(index))
        {
            return 128.0; // 7-bit mode uses 128-unit steps
        }
        return 1.0; // 14-bit mode uses single-unit steps
    }
    
    // For MIDI input - updates slider without triggering output (prevents feedback loops)
    void setValueFromMIDI(double newValue)
    {
        double quantizedValue = quantizeValue(newValue);
        isSettingValueProgrammatically = true;
        mainSlider.setValue(quantizedValue, juce::dontSendNotification);
        isSettingValueProgrammatically = false;
        
        // Use snap-aware method for external MIDI input
        displayManager.setMidiValueWithSnap(quantizedValue, true);
        // Note: No sendMidiCallback to prevent feedback loops
        
        // Trigger activity indicator to show MIDI input activity
        triggerMidiActivity();
    }

    // MIDI Input Mode Management
    void setMidiInputMode(MidiInputMode mode)
    {
        if (midiInputMode == mode)
            return; // No change needed

        midiInputMode = mode;

        if (mode == MidiInputMode::Deadzone)
        {
            // Only create helper knob if slider has MIDI mapping
            bool hasMapping = hasMidiMapping ? hasMidiMapping() : false;

            if (hasMapping)
            {
                // Create helper knob if not exists
                if (!helperKnob)
                {
                    helperKnob = std::make_unique<InvisibleHelperKnob>();
                    addChildComponent(*helperKnob);

                    // Set up helper knob callback to update main slider
                    helperKnob->onValueChanged = [this](double newValue) {
                        updateSliderFromHelper(newValue);
                    };
                }

                // Configure helper knob for deadzone mode
                helperKnob->setMidiInputMode(MidiInputMode::Deadzone);

                // Don't initialize helper knob with current slider value
                // This prevents sliders from moving when the app starts
                // Helper knob will be positioned by first MIDI input
            }
            else
            {
                // No MIDI mapping - destroy helper knob if it exists
                if (helperKnob)
                {
                    removeChildComponent(helperKnob.get());
                    helperKnob.reset();
                }
            }
        }
        else // Direct mode
        {
            // Destroy helper knob - not needed for direct mode
            if (helperKnob)
            {
                removeChildComponent(helperKnob.get());
                helperKnob.reset();
            }
        }
    }

    MidiInputMode getMidiInputMode() const
    {
        return midiInputMode;
    }

    // Updated MIDI input method that routes through helper knob system
    void setValueFromMIDIWithMode(double newValue, bool isInDeadzone = false)
    {
        if (midiInputMode == MidiInputMode::Direct)
        {
            // Direct mode: use existing direct method
            setValueFromMIDI(newValue);
        }
        else
        {
            // Deadzone mode: route through helper knob
            if (helperKnob)
            {
                helperKnob->processMidiInput(newValue, isInDeadzone);
            }
            else
            {
                // Fallback to direct if helper knob not available
                setValueFromMIDI(newValue);
            }
        }
    }

    // Callback for slider click (used for learn mode)
    std::function<void()> onSliderClick;
    
    // Callback to check if slider is in 14-bit mode (for quantization)
    std::function<bool(int)> is14BitMode;

    // Callback to check if slider has MIDI mapping (for helper knob activation)
    std::function<bool()> hasMidiMapping;

    // Callback for learn zone clicks (new learn system)
    std::function<void(const LearnZone&)> onLearnZoneClicked;
    
    // Learn zone management (public interface)
    void activateAllLearnZones(bool active)
    {
        if (learnZones)
        {
            learnZones->setLearnModeActive(active);
            if (active)
                updateLearnZoneBounds();
        }
    }
    
    void clearActiveLearnZone()
    {
        if (learnZones)
            learnZones->clearActiveZone();
    }
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        // Right-click context menu (only when not in learn mode)
        if (event.mods.isRightButtonDown() && !showLearnMarkers)
        {
            // Check if click is specifically in automation control area
            if (automationControlPanel.getBounds().contains(event.getPosition()))
            {
                showAutomationContextMenu(event.getPosition());
                return;
            }

            // Otherwise, show slider context menu
            showSliderContextMenu(event.getPosition());
            return;
        }

        // Handle learn mode clicks first
        if (showLearnMarkers)
        {
            handleLearnModeClick(event);
            return;
        }

        // Set drag state for movement tracking
        displayManager.setDragState(true);

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
            isSettingValueProgrammatically = true;
            mainSlider.setValue(newValue, juce::dontSendNotification);
            isSettingValueProgrammatically = false;
            // Update during manual drag - use drag flag to prevent snapping
            displayManager.setMidiValueWithSnap(newValue, true, true); // isDragUpdate = true
            if (sendMidiCallback)
                sendMidiCallback(index, (int)newValue);
            
            // Trigger parent repaint to update visual thumb position
            if (auto* parent = getParentComponent())
                parent->repaint();
        }, displayManager.getOrientation());
        
        if (!handled)
        {
            // Pass to base class for normal handling
            juce::Component::mouseDrag(event);
        }
    }
    
    void mouseUp(const juce::MouseEvent& event) override
    {
        // Clear drag state - allows snapping to occur
        displayManager.setDragState(false);
        
        bool handled = interactionHandler.handleMouseUp(event);
        
        if (handled)
        {
            // Re-enable normal slider interaction
            mainSlider.setInterceptsMouseClicks(true, true);
        }
        
        // Pass to base class for normal handling first
        juce::Component::mouseUp(event);
        
        // Trigger a final value update that can snap after drag ends
        juce::Timer::callAfterDelay(50, [this]() {
            double currentValue = mainSlider.getValue();
            displayManager.setMidiValueWithSnap(currentValue, true); // Allow snap after drag ends
        });
    }
    
    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        // Don't reset if slider is locked
        if (lockState) return;
        
        // Don't interfere if automation is currently running
        if (automationEngine.isSliderAutomating(index)) return;
        
        double resetValue;
        SliderOrientation orientation = displayManager.getOrientation();
        
        switch (orientation)
        {
            case SliderOrientation::Normal:
                // Reset to minimum value (bottom position)
                resetValue = displayManager.displayToMidi(displayManager.getDisplayMin());
                break;
                
            case SliderOrientation::Inverted:
                // Reset to maximum value (top position) 
                resetValue = displayManager.displayToMidi(displayManager.getDisplayMax());
                break;
                
            case SliderOrientation::Bipolar:
                // Reset to center value (middle position)
                resetValue = displayManager.displayToMidi(displayManager.getCenterValue());
                break;
                
            default:
                // Fallback to minimum
                resetValue = displayManager.displayToMidi(displayManager.getDisplayMin());
                break;
        }
        
        // Clamp to valid MIDI range
        resetValue = juce::jlimit(0.0, 16383.0, resetValue);
        
        // Set the value with notification to trigger MIDI output and UI updates
        isSettingValueProgrammatically = true;
        mainSlider.setValue(resetValue, juce::sendNotification);
        isSettingValueProgrammatically = false;
        
        // Trigger parent repaint to update visual thumb position
        if (auto* parent = getParentComponent())
            parent->repaint();
    }
    
    // Set learn markers visibility
    void setShowLearnMarkers(bool show) 
    { 
        showLearnMarkers = show;
        
        // Enable/disable automation control panel learn mode
        automationControlPanel.setLearnModeActive(show, index);
        
        repaint(); 
    }
    
    // Set automation visibility and update layout
    void setAutomationVisible(bool shouldShow)
    {
        automationControlPanel.setVisible(shouldShow);
        
        // Force immediate layout recalculation
        resized();
        
        // Force visual update
        repaint();
        
        // Also update parent component if needed
        if (auto* parent = getParentComponent())
            parent->repaint();
    }
    
    // Automation config management
    void setConfigManager(AutomationConfigManager* manager) { configManager = manager; }
    
    // Automation highlighting for config management window
    void setAutomationHighlighted(bool highlighted) 
    { 
        if (isAutomationHighlighted != highlighted)
        {
            isAutomationHighlighted = highlighted;
            automationControlPanel.setHighlighted(highlighted); // Set highlighting on automation panel
        }
    }
    
    bool isAutomationComponentsHighlighted() const { return isAutomationHighlighted; }
    
    AutomationConfig getCurrentAutomationConfig() const
    {
        // Use the parameterized constructor which automatically generates ID and timestamp
        juce::String configName = "Slider " + juce::String(index + 1) + " Config";
        AutomationConfig config(
            configName,
            displayManager.getDisplayValue(),
            automationControlPanel.getDelayTime(),
            automationControlPanel.getAttackTime(),
            automationControlPanel.getReturnTime(),
            automationControlPanel.getCurveValue(),
            automationControlPanel.getTimeMode(),
            index
        );
        return config;
    }
    
    void applyAutomationConfig(const AutomationConfig& config)
    {
        // Apply all config values to this slider
        automationControlPanel.setTargetValue(config.targetValue);
        automationControlPanel.setDelayTime(config.delayTime);
        automationControlPanel.setAttackTime(config.attackTime);
        automationControlPanel.setReturnTime(config.returnTime);
        automationControlPanel.setCurveValue(config.curveValue);
        automationControlPanel.setTimeMode(config.timeMode);
        
        DBG("Applied automation config '" + config.name + "' to slider " + juce::String(index));
    }
    
    void showAutomationContextMenu(juce::Point<int> position)
    {
        try {
            // Safety checks
            if (!configManager) {
                DBG("ERROR: configManager is null in showAutomationContextMenu");
                return;
            }
            
            // Validate slider index
            if (index < 0 || index >= 16) {
                DBG("ERROR: Invalid slider index in showAutomationContextMenu: " + juce::String(index));
                return;
            }
            
            DBG("Creating AutomationContextMenu for slider " + juce::String(index) + " at position (" + juce::String(position.x) + ", " + juce::String(position.y) + ")");
            
            // Create context menu with shared ownership to prevent double-delete
            auto contextMenu = std::make_shared<AutomationContextMenu>(*configManager);
            
            // Capture the slider index by value to ensure it's not corrupted
            const int sliderIdx = this->index;
        
        // Set up context menu callbacks with captured slider index
        contextMenu->onSaveConfig = [this, sliderIdx](int) {  // Ignore passed index, use captured
            DBG("=== onSaveConfig callback started for slider " + juce::String(sliderIdx));
            
            try {
                // Safety check for configManager
                if (!configManager) {
                    DBG("ERROR: configManager is null!");
                    return;
                }
                
                // Open management window in Save mode instead of modal dialog
                if (onOpenConfigManagement) {
                    DBG("Opening config management window in Save mode for slider " + juce::String(sliderIdx));
                    onOpenConfigManagement(sliderIdx, AutomationConfigManagementWindow::Mode::Save);
                    
                    // Enable green highlighting to show what's being saved
                    setAutomationHighlighted(true);
                } else {
                    DBG("ERROR: onOpenConfigManagement callback is null!");
                }
                
                DBG("=== onSaveConfig callback completed successfully");
            }
            catch (const std::exception& e) {
                DBG("EXCEPTION in onSaveConfig: " + juce::String(e.what()));
            }
            catch (...) {
                DBG("UNKNOWN EXCEPTION in onSaveConfig");
            }
        };
        
        contextMenu->onLoadConfig = [this, sliderIdx](int, const juce::String& configId) {  // Ignore passed index
            try {
                // Safety check for configManager
                if (!configManager) {
                    DBG("ERROR: configManager is null in onLoadConfig!");
                    return;
                }
                
                if (configId.isEmpty()) {
                    DBG("ERROR: configId is empty in onLoadConfig!");
                    return;
                }
                
                DBG("Loading config: " + configId + " for slider " + juce::String(sliderIdx));
                auto config = configManager->loadConfig(configId);
                if (config.isValid())
                {
                    applyAutomationConfig(config);
                    DBG("Successfully loaded and applied config: " + configId);

                    // Notify about load action
                    if (onAutomationConfigLoaded) onAutomationConfigLoaded(sliderIdx, config.name);
                }
                else
                {
                    DBG("ERROR: Loaded config is invalid: " + configId);
                }
            }
            catch (const std::exception& e) {
                DBG("EXCEPTION in onLoadConfig: " + juce::String(e.what()));
            }
            catch (...) {
                DBG("UNKNOWN EXCEPTION in onLoadConfig");
            }
        };
        
        contextMenu->onCopyConfig = [this, sliderIdx](int) {  // Ignore passed index, use captured
            DBG("=== onCopyConfig callback started for slider " + juce::String(sliderIdx));
            
            try {
                // Safety check for configManager
                if (!configManager) {
                    DBG("ERROR: configManager is null in onCopyConfig!");
                    return;
                }
                
                DBG("Extracting current automation config from panel...");
                
                // Extract current automation config from automation panel
                AutomationConfig config;
                double targetValue, delayTime, attackTime, returnTime, curveValue;
                AutomationControlPanel::TimeMode timeMode;
                
                DBG("Calling automationControlPanel.extractCurrentConfig...");
                automationControlPanel.extractCurrentConfig(targetValue, delayTime, attackTime, returnTime, curveValue, timeMode);
                
                DBG("Creating AutomationConfig object...");
                config = AutomationConfig("Copied Config", targetValue, delayTime, attackTime, returnTime, curveValue, timeMode, sliderIdx);
                
                DBG("Calling configManager->copyConfigFromSlider...");
                configManager->copyConfigFromSlider(sliderIdx, config);
                DBG("Successfully copied config from slider " + juce::String(sliderIdx));

                // Notify about copy action
                if (onAutomationConfigCopied) onAutomationConfigCopied(sliderIdx);

                DBG("=== onCopyConfig callback completed successfully");
            }
            catch (const std::exception& e) {
                DBG("EXCEPTION in onCopyConfig: " + juce::String(e.what()));
            }
            catch (...) {
                DBG("UNKNOWN EXCEPTION in onCopyConfig");
            }
        };
        
        contextMenu->onPasteConfig = [this, sliderIdx](int) {  // Ignore passed index, use captured
            try {
                // Safety check for configManager
                if (!configManager) {
                    DBG("ERROR: configManager is null in onPasteConfig!");
                    return;
                }
                
                DBG("Attempting to paste config to slider " + juce::String(sliderIdx));
                AutomationConfig config;
                if (configManager->pasteConfigToSlider(sliderIdx, config))
                {
                    if (config.isValid()) {
                        // Apply the pasted config to automation panel
                        automationControlPanel.applyConfig(config.targetValue, config.delayTime, config.attackTime,
                                                  config.returnTime, config.curveValue, config.timeMode);
                        DBG("Successfully pasted and applied config to slider " + juce::String(sliderIdx));

                        // Notify about paste action
                        if (onAutomationConfigPasted) onAutomationConfigPasted(sliderIdx);
                    } else {
                        DBG("ERROR: Pasted config is invalid for slider " + juce::String(sliderIdx));
                    }
                }
                else
                {
                    DBG("No config available to paste for slider " + juce::String(sliderIdx));
                }
            }
            catch (const std::exception& e) {
                DBG("EXCEPTION in onPasteConfig: " + juce::String(e.what()));
            }
            catch (...) {
                DBG("UNKNOWN EXCEPTION in onPasteConfig");
            }
        };
        
        contextMenu->onResetAutomation = [this, sliderIdx](int) {  // Ignore passed index, use captured
            try {
                DBG("Attempting to reset automation for slider " + juce::String(sliderIdx));
                
                // Reset automation parameters to defaults
                automationControlPanel.resetToDefaults();

                // Notify about reset action
                if (onAutomationConfigReset) onAutomationConfigReset(sliderIdx);

                DBG("Successfully reset automation parameters for slider " + juce::String(sliderIdx));
            }
            catch (const std::exception& e) {
                DBG("EXCEPTION in onResetAutomation: " + juce::String(e.what()));
            }
            catch (...) {
                DBG("UNKNOWN EXCEPTION in onResetAutomation");
            }
        };
        
        contextMenu->onManageConfigs = [this, sliderIdx]() {
            if (onOpenConfigManagement) {
                DBG("Opening config management window in Manage mode");
                onOpenConfigManagement(sliderIdx, AutomationConfigManagementWindow::Mode::Manage);
            } else if (onConfigManagementRequested) {
                onConfigManagementRequested();
            }
        };
        
        // Show context menu with captured index
        DBG("Showing context menu for slider " + juce::String(sliderIdx));
        contextMenu->showForSlider(sliderIdx, position, this, contextMenu);
        DBG("Context menu completed for slider " + juce::String(sliderIdx));
        
        // shared_ptr will automatically clean up when it goes out of scope
        
        } catch (const std::exception& e) {
            DBG("EXCEPTION in showAutomationContextMenu: " + juce::String(e.what()));
        } catch (...) {
            DBG("UNKNOWN EXCEPTION in showAutomationContextMenu");
        }
    }

    void showSliderContextMenu(juce::Point<int> position)
    {
        try {
            // Validate slider index
            if (index < 0 || index >= 16) {
                DBG("ERROR: Invalid slider index in showSliderContextMenu: " + juce::String(index));
                return;
            }

            DBG("Creating SliderContextMenu for slider " + juce::String(index) + " at position (" + juce::String(position.x) + ", " + juce::String(position.y) + ")");

            // Create context menu with shared ownership to prevent double-delete
            auto contextMenu = std::make_shared<SliderContextMenu>();

            // Capture the slider index by value to ensure it's not corrupted
            const int sliderIdx = this->index;

            // Set up context menu callbacks with captured slider index

            // Range preset callback
            contextMenu->onRangePresetSelected = [this, sliderIdx](int, int rangeType) {
                DBG("Range preset " + juce::String(rangeType) + " selected for slider " + juce::String(sliderIdx));
                if (onRangePresetSelected) {
                    onRangePresetSelected(sliderIdx, rangeType);
                }
            };

            // Copy slider callback
            contextMenu->onCopySlider = [this, sliderIdx](int) {
                DBG("Copy slider " + juce::String(sliderIdx));
                if (onCopySlider) {
                    onCopySlider(sliderIdx);
                }
            };

            // Paste slider callback
            contextMenu->onPasteSlider = [this, sliderIdx](int) {
                DBG("Paste slider " + juce::String(sliderIdx));
                if (onPasteSlider) {
                    onPasteSlider(sliderIdx);
                }
            };

            // Reset slider callback
            contextMenu->onResetSlider = [this, sliderIdx](int) {
                DBG("Reset slider " + juce::String(sliderIdx));
                if (onResetSlider) {
                    onResetSlider(sliderIdx);
                }
            };

            // Set all in bank callback
            contextMenu->onSetAllInBank = [this, sliderIdx](int) {
                DBG("Set all in bank to value of slider " + juce::String(sliderIdx));
                // TODO: Implement set all in bank functionality
            };

            // Set all sliders callback
            contextMenu->onSetAllSliders = [this, sliderIdx](int) {
                DBG("Set all sliders to value of slider " + juce::String(sliderIdx));
                // TODO: Implement set all sliders functionality
            };

            // Copy to bank callback
            contextMenu->onCopyToBank = [this, sliderIdx](int) {
                DBG("Copy slider " + juce::String(sliderIdx) + " settings to all in bank");
                // TODO: Implement copy to bank functionality
            };

            // Copy to all callback
            contextMenu->onCopyToAll = [this, sliderIdx](int) {
                DBG("Copy slider " + juce::String(sliderIdx) + " settings to all sliders");
                // TODO: Implement copy to all functionality
            };

            // Show context menu with captured index
            // Get clipboard status from callback if available
            bool hasClipboard = false;
            if (hasClipboardData) {
                hasClipboard = hasClipboardData();
            }
            DBG("Showing slider context menu for slider " + juce::String(sliderIdx));
            contextMenu->showForSlider(sliderIdx, position, this, hasClipboard, contextMenu);
            DBG("Slider context menu completed for slider " + juce::String(sliderIdx));

            // shared_ptr will automatically clean up when it goes out of scope

        } catch (const std::exception& e) {
            DBG("EXCEPTION in showSliderContextMenu: " + juce::String(e.what()));
        } catch (...) {
            DBG("UNKNOWN EXCEPTION in showSliderContextMenu");
        }
    }

    // Automation config callbacks
    std::function<void()> onConfigManagementRequested;
    std::function<void(int sliderIndex, AutomationConfigManagementWindow::Mode mode)> onOpenConfigManagement;
    
    // Automation start/stop callback
    std::function<void(int sliderIndex, bool isStarting)> onAutomationToggled;
    
    // Time mode change callback
    std::function<void(int sliderIndex, AutomationControlPanel::TimeMode mode)> onTimeModeChanged;
    
    // Lock state change callback
    std::function<void(int sliderIndex, bool isLocked)> onLockStateChanged;

    // Automation config callbacks
    std::function<void(int sliderIndex)> onAutomationConfigCopied;
    std::function<void(int sliderIndex)> onAutomationConfigPasted;
    std::function<void(int sliderIndex)> onAutomationConfigReset;
    std::function<void(int sliderIndex, const juce::String& configName)> onAutomationConfigLoaded;

    // Slider context menu callbacks
    std::function<void(int sliderIndex, int rangeType)> onRangePresetSelected;
    std::function<void(int sliderIndex)> onCopySlider;
    std::function<void(int sliderIndex)> onPasteSlider;
    std::function<void(int sliderIndex)> onResetSlider;
    std::function<bool()> hasClipboardData;

    // Learn mode callbacks
    std::function<void(MidiTargetType, int)> onLearnModeTargetClicked;
    
    void handleLearnModeClick(const juce::MouseEvent& event)
    {
        auto pos = event.getPosition();
        MidiTargetType targetType = MidiTargetType::SliderValue;
        
        // Check if click is in automation control panel
        if (automationControlPanel.getBounds().contains(pos))
        {
            // Let the automation control panel handle the specific component detection
            auto relativePos = pos - automationControlPanel.getBounds().getTopLeft();
            
            // Check specific automation components
            if (automationControlPanel.getGoButtonBounds().contains(relativePos))
                targetType = MidiTargetType::AutomationGO;
            else if (automationControlPanel.getDelayKnobBounds().contains(relativePos))
                targetType = MidiTargetType::AutomationDelay;
            else if (automationControlPanel.getAttackKnobBounds().contains(relativePos))
                targetType = MidiTargetType::AutomationAttack;
            else if (automationControlPanel.getReturnKnobBounds().contains(relativePos))
                targetType = MidiTargetType::AutomationReturn;
            else if (automationControlPanel.getCurveKnobBounds().contains(relativePos))
                targetType = MidiTargetType::AutomationCurve;
            else
                return; // Click wasn't on a specific automation component
        }
        else
        {
            // Click was on slider track area
            targetType = MidiTargetType::SliderValue;
        }
        
        // Notify the learn mode system
        if (onLearnModeTargetClicked)
            onLearnModeTargetClicked(targetType, index);
    }
    
    // Snap logic moved to SliderDisplayManager::setMidiValueWithSnap()
    // This ensures consistent snap behavior across all interaction paths
    
    // Scale change notification implementation
    void scaleFactorChanged(float newScale) override
    {
        // Update fonts to new scale
        auto& scale = GlobalUIScale::getInstance();
        sliderNumberLabel.setFont(scale.getScaledFont(11.0f).boldened());
        lockLabel.setFont(scale.getScaledFont(14.0f).boldened());
        
        // Update current value label font to new scale
        juce::Font ledFont(juce::FontOptions("Monaco", scale.getScaled(12.0f), juce::Font::plain));
        if (!ledFont.getTypefaceName().contains("Monaco")) {
            ledFont = juce::Font(juce::FontOptions("Courier New", scale.getScaled(12.0f), juce::Font::plain));
        }
        currentValueLabel.setFont(ledFont);
        
        // Trigger layout updates when scale changes
        resized();
        repaint();
        
        // Notify child components that support scaling
        automationControlPanel.repaint();
        
        // Update learn zone bounds for new scale
        updateLearnZoneBounds();
    }
    
private:
    void setupDisplayManager()
    {
        // Set up display manager callbacks
        displayManager.onDisplayTextChanged = [this](const juce::String& text) {
            currentValueLabel.setText(text, juce::dontSendNotification);
        };
        
        // Set up snap callback for consistent behavior
        displayManager.onSnapToCenter = [this](double snappedMidiValue) {
            // Update slider to snapped value
            isSettingValueProgrammatically = true;
            mainSlider.setValue(snappedMidiValue, juce::dontSendNotification);
            isSettingValueProgrammatically = false;
            
            // Send MIDI output and trigger visual updates
            if (sendMidiCallback)
                sendMidiCallback(index, (int)snappedMidiValue);
                
            // Trigger parent repaint to update visual thumb position
            if (auto* parent = getParentComponent())
                parent->repaint();
        };
        
        // Initialize display with current slider value
        displayManager.setMidiValue(mainSlider.getValue());
        
        // Initialize target LED input with current display value
        automationControlPanel.setTargetValue(displayManager.getDisplayValue());
    }

    void updateSliderFromHelper(double helperValue)
    {
        // Update main slider from helper knob value (used in Deadzone mode)
        double quantizedValue = quantizeValue(helperValue);

        isSettingValueProgrammatically = true;
        mainSlider.setValue(quantizedValue, juce::dontSendNotification);
        isSettingValueProgrammatically = false;

        // Update display manager with smooth value (no snapping from helper)
        displayManager.setMidiValue(quantizedValue);

        // Send MIDI output
        if (sendMidiCallback)
            sendMidiCallback(index, (int)quantizedValue);

        // Trigger parent repaint to update visual thumb position
        if (auto* parent = getParentComponent())
            parent->repaint();

        // Trigger activity indicator to show ongoing movement
        triggerMidiActivity();
    }

    void setupAutomationEngine()
    {
        // Set up automation value update callback
        automationEngine.onValueUpdate = [this](int sliderIndex, double newValue) {
            if (sliderIndex == index)
            {
                double quantizedValue = quantizeValue(newValue);
                isSettingValueProgrammatically = true;
                mainSlider.setValue(quantizedValue, juce::dontSendNotification);
                isSettingValueProgrammatically = false;
                // Automation should never snap - maintain smooth, precise movement
                displayManager.setMidiValue(quantizedValue);
                if (sendMidiCallback)
                    sendMidiCallback(index, (int)quantizedValue);
            }
        };
        
        // Set up automation state change callback
        automationEngine.onAutomationStateChanged = [this](int sliderIndex, bool isAutomating) {
            if (sliderIndex == index)
            {
                // Update GO button state based on automation status
                automationControlPanel.updateGoButtonState(isAutomating);
                
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
    
    void setupLearnZones()
    {
        // Create learn zones for this slider
        learnZones = std::make_unique<SliderLearnZones>(index);
        addChildComponent(*learnZones);
        
        // Set up learn zone callback
        learnZones->onZoneClicked = [this](const LearnZone& zone) {
            if (onLearnZoneClicked)
                onLearnZoneClicked(zone);
        };
    }

    void setupHelperKnobSystem()
    {
        // Initialize in Direct mode by default (no helper knob created)
        // Helper knob will be created when switching to Deadzone mode
        midiInputMode = MidiInputMode::Direct;
        helperKnob = nullptr;
    }

    void updateLearnZoneBounds()
    {
        if (!learnZones) return;
        
        // Update learn zone bounds based on current layout
        auto sliderTrackBounds = getVisualTrackBounds();
        learnZones->createZones(sliderTrackBounds, automationControlPanel);
        learnZones->setBounds(getLocalBounds());
    }
    
    void drawLearnModeMarkers(juce::Graphics& g)
    {
        auto& scale = GlobalUIScale::getInstance();
        auto bounds = getLocalBounds().toFloat();
        float markerSize = scale.getScaled(8.0f);
        float markerThickness = scale.getScaled(2.0f);
        
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
        
        // Update GO button state to show "STOP" and highlighting
        automationControlPanel.updateGoButtonState(true);
    }
    
private:
    // Quantize value to step increments based on display range
    double quantizeValue(double midiValue) const
    {
        // Check for 7-bit quantization first
        if (is14BitMode && !is14BitMode(index))
        {
            // This is 7-bit mode - quantize to 128 discrete steps (0-127)
            // Scale 0-16383 to 0-127, then back to 0-16383 in discrete steps
            int value7bit = (int)((midiValue / 16383.0) * 127.0 + 0.5); // Round to nearest
            value7bit = juce::jlimit(0, 127, value7bit);
            double quantizedValue = (double)(value7bit * 128);
            
            // Debug logging for 7-bit quantization
            if (quantizedValue != midiValue || midiValue > 16000.0)
            {
                juce::String rangeIndicator = (midiValue < 8191.5) ? "Lower" : "Upper";
                double percentageOfRange = (midiValue / 16383.0) * 100.0;
                bool isInDeadZone = (midiValue > 16256.0 && midiValue <= 16383.0);
                DBG("7-bit Quantization: Input " << midiValue << " (" << rangeIndicator << " " << (int)percentageOfRange << "%)"
                    << " -> 7bit(" << value7bit << ") -> Output " << quantizedValue 
                    << (isInDeadZone ? " [DEAD ZONE]" : ""));
            }
            
            return quantizedValue; // Scale back to 14-bit range in 128-step increments
        }
        
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
    AutomationConfigManager* configManager = nullptr;
    
    // MIDI activity indicator variables
    bool midiActivityState = false;
    double lastMidiSendTime = 0.0;
    static constexpr double MIDI_ACTIVITY_DURATION = 100.0; // milliseconds
    juce::Rectangle<float> midiIndicatorBounds;
    
    // MIDI learn markers
    bool showLearnMarkers = false;
    
    // Automation highlighting for config management
    bool isAutomationHighlighted = false;
    
    // Flag to prevent infinite recursion when setting values programmatically
    bool isSettingValueProgrammatically = false;
    
    // Learn zones for individual component targeting
    std::unique_ptr<SliderLearnZones> learnZones;

    // MIDI Input Mode and Helper Knob System
    MidiInputMode midiInputMode = MidiInputMode::Direct; // Default to Direct mode
    std::unique_ptr<InvisibleHelperKnob> helperKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSliderControl)
};
