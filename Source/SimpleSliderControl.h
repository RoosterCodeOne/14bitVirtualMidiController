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
    SimpleSliderControl(int sliderIndex, std::function<void(int, int)> midiCallback)
        : index(sliderIndex), sendMidiCallback(midiCallback), sliderColor(juce::Colours::cyan),
          attackKnob("ATTACK", 0.0, 30.0, CustomKnob::Smaller), 
          delayKnob("DELAY", 0.0, 10.0, CustomKnob::Smaller), 
          returnKnob("RETURN", 0.0, 30.0, CustomKnob::Smaller),
          curveKnob("CURVE", 0.0, 2.0, CustomKnob::Smaller)
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
                int value = (int)mainSlider.getValue();
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
        
        // Attack knob (large)
        addAndMakeVisible(attackKnob);
        attackKnob.setValue(1.0);
        attackKnob.onValueChanged = [this](double newValue) {
            updateVisualizerParameters();
        };
        
        // Delay knob (small)
        addAndMakeVisible(delayKnob);
        delayKnob.setValue(0.0);
        delayKnob.onValueChanged = [this](double newValue) {
            updateVisualizerParameters();
        };
        
        // Return knob (small)
        addAndMakeVisible(returnKnob);
        returnKnob.setValue(0.0);
        returnKnob.onValueChanged = [this](double newValue) {
            updateVisualizerParameters();
        };
        
        // Curve knob (small) - controls automation curve shape
        addAndMakeVisible(curveKnob);
        curveKnob.setValue(1.0); // Default to linear
        curveKnob.onValueChanged = [this](double newValue) {
            updateVisualizerParameters();
        };
        
        // Target value input - LED display style
        addAndMakeVisible(targetLEDInput);
        targetLEDInput.setValidatedValue(8192); // Will be converted to display range
        
        // 3D GO button with automation functionality
        addAndMakeVisible(goButton3D);
        goButton3D.onClick = [this]() { 
            if (automationEngine.isSliderAutomating(index))
                automationEngine.stopAutomation(index);
            else
                startAutomation();
        };
        
        // Automation visualizer - blueprint-style curve display
        addAndMakeVisible(automationVisualizer);
        automationVisualizer.setParameters(delayKnob.getValue(), attackKnob.getValue(), 
                                         returnKnob.getValue(), curveKnob.getValue());
        
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
        
        // Utility bar at top (16px height)  
        auto utilityBar = area.removeFromTop(16);
        
        // Slider number label in utility bar (centered)
        sliderNumberLabel.setBounds(utilityBar);
        
        area.removeFromTop(4); // spacing after utility bar
        
        // Main slider - invisible, positioned for mouse interaction in track area
        int automationControlsHeight = 200; // Increased to accommodate taller knobs and increased spacing
        int availableSliderHeight = area.getHeight() - automationControlsHeight;
        int reducedSliderHeight = (int)(availableSliderHeight * 0.70); // Reduced from 80% to 70% (10% further reduction)
        auto sliderArea = area.removeFromTop(reducedSliderHeight);
        auto trackBounds = sliderArea.withWidth(20).withCentre(sliderArea.getCentre()); // Reduced from 30px to 20px
        
        // Position slider for mouse interaction (proper thumb travel alignment)
        float thumbHeight = 24.0f; // Use visual thumb height for proper alignment
        
        // Calculate precise values to avoid rounding errors
        int trackY = trackBounds.getY();
        int trackHeight = trackBounds.getHeight();
        int thumbHalfHeight = (int)(thumbHeight / 2.0f); // 12px
        
        // MainSlider positioning for exact thumb center alignment
        int sliderY = trackY + thumbHalfHeight;
        int sliderHeight = trackHeight - (int)thumbHeight + 3; // Add 3px to align bottom edge with thumb center
        
        auto interactionBounds = juce::Rectangle<int>(
            trackBounds.getX(), 
            sliderY, 
            trackBounds.getWidth(), 
            sliderHeight
        );
        
        mainSlider.setBounds(interactionBounds);
        
        area.removeFromTop(4); // spacing before value label
        
        // Current value label - restored to original width
        auto labelArea = area.removeFromTop(20);
        auto reducedLabelArea = labelArea.reduced(4, 0); // Reduce width by 4px on each side
        currentValueLabel.setBounds(reducedLabelArea);
        
        // MIDI activity indicator - positioned above currentValueLabel on left side
        midiIndicatorBounds = juce::Rectangle<float>(5, labelArea.getY() - 15, 10, 10);
        
        // Lock label - positioned above currentValueLabel on right side, aligned with MIDI indicator
        int lockLabelX = getWidth() - 25; // 5px from right edge, 20px width
        lockLabel.setBounds(lockLabelX, labelArea.getY() - 15, 20, 10);
        
        area.removeFromTop(4); // spacing before automation controls
        
        // New automation layout: Centered GO button between display and target values
        auto automationArea = area;
        
        // GO button - centered on slider plate, equidistant from display and target
        auto buttonArea = automationArea.removeFromTop(33);
        int buttonX = (buttonArea.getWidth() - 35) / 2; // Centered horizontally
        goButton3D.setBounds(buttonX, buttonArea.getY() + 5, 35, 25);
        
        automationArea.removeFromTop(7); // spacing after button
        
        // Target LED input - centered horizontally below button, same width as display
        auto targetArea = automationArea.removeFromTop(28);
        int displayWidth = reducedLabelArea.getWidth(); // Match display value width
        int targetX = (targetArea.getWidth() - displayWidth) / 2;
        targetLEDInput.setBounds(targetX, targetArea.getY() + 2, displayWidth, 20);
        
        automationArea.removeFromTop(7); // spacing after target
        
        // Knob group arrangement in 2x2 grid:
        // [DELAY]   [ATTACK]
        // [RETURN]  [CURVE]
        auto knobColumnArea = automationArea;
        
        // Calculate 2x2 grid positioning
        int centerX = knobColumnArea.getCentreX();
        int startY = knobColumnArea.getY() + 8;
        
        // Grid layout parameters
        int knobWidth = 42;
        int knobHeight = 57;
        int horizontalSpacing = 15; // Space between columns
        int verticalSpacing = 10;   // Space between rows
        
        // Calculate grid dimensions
        int totalGridWidth = (2 * knobWidth) + horizontalSpacing;
        int totalGridHeight = (2 * knobHeight) + verticalSpacing;
        
        // Calculate starting position to center the grid
        int gridStartX = centerX - (totalGridWidth / 2);
        
        // 2x2 Grid positions:
        // Top row: [DELAY] [ATTACK]
        int delayX = gridStartX;
        int attackX = gridStartX + knobWidth + horizontalSpacing;
        int topRowY = startY;
        
        delayKnob.setBounds(delayX, topRowY, knobWidth, knobHeight);
        attackKnob.setBounds(attackX, topRowY, knobWidth, knobHeight);
        
        // Bottom row: [RETURN] [CURVE]
        int returnX = gridStartX;
        int curveX = gridStartX + knobWidth + horizontalSpacing;
        int bottomRowY = startY + knobHeight + verticalSpacing;
        
        returnKnob.setBounds(returnX, bottomRowY, knobWidth, knobHeight);
        curveKnob.setBounds(curveX, bottomRowY, knobWidth, knobHeight);
        
        // Automation visualizer - positioned below knob grid
        int visualizerY = bottomRowY + knobHeight + 8; // Small gap after knobs
        int visualizerWidth = totalGridWidth; // Match grid width
        int visualizerHeight = 60; // Fixed height for curve display
        int visualizerX = gridStartX; // Align with knob grid
        
        automationVisualizer.setBounds(visualizerX, visualizerY, visualizerWidth, visualizerHeight);
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
    
    // Methods for parent component to get visual track and thumb positions
    juce::Rectangle<int> getVisualTrackBounds() const 
    {
        // Calculate the full visual track area based on current layout
        auto area = getLocalBounds();
        area.removeFromTop(20); // utility bar + spacing
        int automationControlsHeight = 200; // Increased to accommodate taller knobs and increased spacing
        int availableSliderHeight = area.getHeight() - automationControlsHeight;
        int reducedSliderHeight = (int)(availableSliderHeight * 0.70); // Match resized() method
        auto sliderArea = area.removeFromTop(reducedSliderHeight);
        return sliderArea.withWidth(20).withCentre(sliderArea.getCentre()); // Updated to 20px width
    }
    
    juce::Point<float> getThumbPosition() const 
    {
        auto trackBounds = getVisualTrackBounds().toFloat();
        
        // Calculate thumb position based on slider value
        auto value = mainSlider.getValue();
        float norm = juce::jmap<float>(value, mainSlider.getMinimum(), mainSlider.getMaximum(), 0.0f, 1.0f);
        
        // Map to the usable track area (excluding thumb size)
        float thumbHeight = 24.0f; // Reverted to original for thumb positioning
        float usableTrackHeight = trackBounds.getHeight() - thumbHeight;
        float trackTop = trackBounds.getY() + (thumbHeight / 2.0f);
        float trackBottom = trackTop + usableTrackHeight;
        
        float thumbY = juce::jmap(norm, trackBottom, trackTop);
        return juce::Point<float>(trackBounds.getCentreX(), thumbY);
    }
    
    // Preset support methods
    void setValue(double newValue)
    {
        mainSlider.setValue(newValue, juce::dontSendNotification);
        displayManager.setMidiValue(newValue);
        targetLEDInput.setValidatedValue(displayManager.getDisplayValue());
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
        targetLEDInput.setValidRange(minVal, maxVal);
        targetLEDInput.setValidatedValue(displayManager.getDisplayValue());
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
        delayKnob.setValue(delay);
    }

    double getDelayTime() const
    {
        return delayKnob.getValue();
    }

    void setAttackTime(double attack)
    {
        attackKnob.setValue(attack);
    }

    double getAttackTime() const
    {
        return attackKnob.getValue();
    }

    void setReturnTime(double returnVal)
    {
        returnKnob.setValue(returnVal);
    }

    double getReturnTime() const
    {
        return returnKnob.getValue();
    }
    
    void setCurveValue(double curve)
    {
        curveKnob.setValue(curve);
    }

    double getCurveValue() const
    {
        return curveKnob.getValue();
    }
    
    // For keyboard movement - updates slider without changing target input
    void setValueFromKeyboard(double newValue)
    {
        mainSlider.setValue(newValue, juce::dontSendNotification);
        displayManager.setMidiValue(newValue);
        if (sendMidiCallback)
            sendMidiCallback(index, (int)newValue);
    }
    
    // For MIDI input - updates slider without triggering output (prevents feedback loops)
    void setValueFromMIDI(double newValue)
    {
        mainSlider.setValue(newValue, juce::dontSendNotification);
        displayManager.setMidiValue(newValue);
        // Note: No sendMidiCallback to prevent feedback loops
        
        // Trigger activity indicator to show MIDI input activity
        triggerMidiActivity();
    }
    
    // Callback for slider click (used for learn mode)
    std::function<void()> onSliderClick;
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        // Handle learn mode (only if callback is set)
        if (onSliderClick)
            onSliderClick();
        
        // Pass to base class for normal handling
        juce::Component::mouseDown(event);
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
        targetLEDInput.setValidatedValue(displayManager.getDisplayValue());
    }
    
    void setupAutomationEngine()
    {
        // Set up automation value update callback
        automationEngine.onValueUpdate = [this](int sliderIndex, double newValue) {
            if (sliderIndex == index)
            {
                mainSlider.setValue(newValue, juce::dontSendNotification);
                displayManager.setMidiValue(newValue);
                if (sendMidiCallback)
                    sendMidiCallback(index, (int)newValue);
            }
        };
        
        // Set up automation state change callback
        automationEngine.onAutomationStateChanged = [this](int sliderIndex, bool isAutomating) {
            if (sliderIndex == index)
            {
                goButton3D.setButtonText(isAutomating ? "STOP" : "GO");
                
                // Update visualizer state based on automation status
                if (isAutomating)
                {
                    automationVisualizer.lockCurveForAutomation();
                }
                else
                {
                    automationVisualizer.unlockCurve();
                }
            }
        };
        
        // Note: Progress updates would require adding onProgressUpdate callback to AutomationEngine
        // For now, the visualizer will show state changes and curve updates
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
    
    void updateVisualizerParameters()
    {
        automationVisualizer.setParameters(delayKnob.getValue(), attackKnob.getValue(),
                                         returnKnob.getValue(), curveKnob.getValue());
    }
    
    
    
    void validateTargetValue()
    {
        // LED input handles its own validation
        double displayValue = targetLEDInput.getValidatedValue();
        targetLEDInput.setValidatedValue(displayValue);
    }
    
    void startAutomation()
    {
        if (automationEngine.isSliderAutomating(index)) return;
        
        validateTargetValue();
        double targetDisplayValue = targetLEDInput.getValidatedValue();
        displayManager.setTargetDisplayValue(targetDisplayValue);
        double targetMidiValue = displayManager.getTargetMidiValue();
        double startMidiValue = mainSlider.getValue();
        
        // Set up automation parameters
        AutomationEngine::AutomationParams params;
        params.delayTime = delayKnob.getValue();
        params.attackTime = attackKnob.getValue();
        params.returnTime = returnKnob.getValue();
        params.curveValue = curveKnob.getValue();
        params.startValue = startMidiValue;
        params.targetValue = targetMidiValue;
        
        // Start automation through engine
        automationEngine.startAutomation(index, params);
    }
    
    
    int index;
    std::function<void(int, int)> sendMidiCallback;
    CustomSliderLookAndFeel customLookAndFeel;
    juce::Slider mainSlider;
    juce::Label sliderNumberLabel;
    ClickableLabel lockLabel;
    juce::Label currentValueLabel;
    CustomKnob attackKnob, delayKnob, returnKnob, curveKnob;
    CustomLEDInput targetLEDInput;
    Custom3DButton goButton3D;
    AutomationEngine automationEngine;
    SliderDisplayManager displayManager;
    AutomationVisualizer automationVisualizer;
    
    bool lockState = false; // Lock state for manual controls
    
    // MIDI activity indicator variables
    bool midiActivityState = false;
    double lastMidiSendTime = 0.0;
    static constexpr double MIDI_ACTIVITY_DURATION = 100.0; // milliseconds
    juce::Rectangle<float> midiIndicatorBounds;
    
    // MIDI learn markers
    bool showLearnMarkers = false;
    
    juce::Colour sliderColor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSliderControl)
};
