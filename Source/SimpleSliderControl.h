// SimpleSliderControl.h - Production Version with MIDI Activity Indicators and Preset Support
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

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
        : index(sliderIndex), sendMidiCallback(midiCallback), sliderColor(juce::Colours::cyan)
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
            if (!isAutomating) // Only update if not currently automating
            {
                int value = (int)mainSlider.getValue();
                updateDisplayValue();
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
            
            if (isAutomating)
            {
                stopTimer();
                isAutomating = false;
                goButton.setButtonText("GO");
            }
        };
        
        // Slider number label
        addAndMakeVisible(sliderNumberLabel);
        sliderNumberLabel.setText(juce::String(sliderIndex + 1), juce::dontSendNotification);
        sliderNumberLabel.setJustificationType(juce::Justification::centred);
        sliderNumberLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        sliderNumberLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        
        // Lock label (acting as button)
        addAndMakeVisible(lockLabel);
        lockLabel.setText("U", juce::dontSendNotification); // U for Unlocked by default
        lockLabel.setJustificationType(juce::Justification::centred);
        lockLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        lockLabel.setFont(juce::FontOptions(18.0f, juce::Font::bold)); // Large font to fill space
        lockLabel.onClick = [this]() { toggleLock(); };
        
        // Current value label
        addAndMakeVisible(currentValueLabel);
        currentValueLabel.setText("0", juce::dontSendNotification);
        currentValueLabel.setJustificationType(juce::Justification::centred);
        currentValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::black);
        currentValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        
        // Delay slider
        addAndMakeVisible(delaySlider);
        delaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
        delaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        delaySlider.setRange(0.0, 10.0, 0.1);
        delaySlider.setValue(0.0);
        delaySlider.setTextValueSuffix(" s");
        
        addAndMakeVisible(delayLabel);
        delayLabel.setText("Delay:", juce::dontSendNotification);
        delayLabel.attachToComponent(&delaySlider, true);
        
        // Attack slider
        addAndMakeVisible(attackSlider);
        attackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        attackSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        attackSlider.setRange(0.0, 30.0, 0.1);
        attackSlider.setValue(1.0);
        attackSlider.setTextValueSuffix(" s");
        
        addAndMakeVisible(attackLabel);
        attackLabel.setText("Attack:", juce::dontSendNotification);
        attackLabel.attachToComponent(&attackSlider, true);
        
        // Target value input - shows display range values
        addAndMakeVisible(targetInput);
        targetInput.setInputRestrictions(0, "-0123456789.");
        targetInput.setText("8192", juce::dontSendNotification); // Will be converted to display range
        targetInput.onReturnKey = [this]() { validateTargetValue(); };
        targetInput.onFocusLost = [this]() { validateTargetValue(); };
        
        addAndMakeVisible(targetLabel);
        targetLabel.setText("Target:", juce::dontSendNotification);
        targetLabel.attachToComponent(&targetInput, true);
        
        // GO button with automation functionality
        addAndMakeVisible(goButton);
        goButton.setButtonText("GO");
        goButton.onClick = [this]() { 
            if (isAutomating)
                stopAutomation();
            else
                startAutomation();
        };
        
        // Initialize display with default range
        updateDisplayValue();
        updateTargetDisplayValue();
    }
    
    ~SimpleSliderControl()
    {
        // CRITICAL: Stop timer before destruction
        stopTimer();
        
        // CRITICAL: Remove look and feel before destruction
        mainSlider.setLookAndFeel(nullptr);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        
        // Utility bar at top (20px height)  
        auto utilityBar = area.removeFromTop(20);
        
        // MIDI activity indicator in utility bar (left side)
        midiIndicatorBounds = juce::Rectangle<float>(2, 2, 10, 10);
        
        // Lock label in utility bar (right side)
        lockLabel.setBounds(utilityBar.removeFromRight(20));
        
        // Slider number label in utility bar (remaining center space)
        sliderNumberLabel.setBounds(utilityBar);
        
        area.removeFromTop(5); // spacing after utility bar
        
        // Main slider - invisible, positioned for mouse interaction in track area
        auto sliderArea = area.removeFromTop(area.getHeight() - 120);
        auto trackBounds = sliderArea.withWidth(40).withCentre(sliderArea.getCentre());
        
        // Position slider for mouse interaction (accounting for thumb travel)
        float thumbHeight = 24.0f;
        auto interactionBounds = trackBounds;
        interactionBounds.setHeight(trackBounds.getHeight() - thumbHeight + 10.0f); // Add 10px to height (was 6px)
        interactionBounds.setY(trackBounds.getY() + (thumbHeight / 2.0f) - 5.0f); // Offset by half the added height
        
        mainSlider.setBounds(interactionBounds.toNearestInt());
        
        area.removeFromTop(5); // spacing before value label
        
        // Current value label
        auto labelArea = area.removeFromTop(25);
        currentValueLabel.setBounds(labelArea);
        
        area.removeFromTop(5); // spacing before automation controls
        
        // Automation controls - centered and properly spaced
        auto automationArea = area;
        int controlWidth = automationArea.getWidth() - 20; // Leave margins
        int controlsStartX = (automationArea.getWidth() - controlWidth) / 2;
        
        // Delay slider
        auto delayArea = automationArea.removeFromTop(25);
        delaySlider.setBounds(delayArea.removeFromLeft(controlWidth - 50).translated(controlsStartX, 0));
        
        automationArea.removeFromTop(3); // spacing between controls
        
        // Attack slider
        auto attackArea = automationArea.removeFromTop(25);
        attackSlider.setBounds(attackArea.removeFromLeft(controlWidth - 50).translated(controlsStartX, 0));
        
        automationArea.removeFromTop(3); // spacing before target row
        
        // Target and GO button - centered
        auto bottomArea = automationArea.removeFromTop(25);
        int targetRowWidth = 105; // 60 for input + 5 spacing + 40 for button
        int targetRowStartX = (bottomArea.getWidth() - targetRowWidth) / 2;
        
        goButton.setBounds(targetRowStartX + 65, bottomArea.getY(), 40, bottomArea.getHeight());
        targetInput.setBounds(targetRowStartX, bottomArea.getY(), 60, bottomArea.getHeight());
    }
    
    void paint(juce::Graphics& g) override
    {
        // Draw MIDI activity indicator in utility bar
        juce::Colour indicatorColor = juce::Colours::orange;
        float alpha = midiActivityState ? 1.0f : 0.2f;
        
        g.setColour(indicatorColor.withAlpha(alpha));
        g.fillRect(midiIndicatorBounds);
        
        // Optional: thin outline
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRect(midiIndicatorBounds, 1.0f);
    }
    
    double getValue() const { return mainSlider.getValue(); }
    
    // Methods for parent component to get visual track and thumb positions
    juce::Rectangle<int> getVisualTrackBounds() const 
    {
        // Calculate the full visual track area based on current layout
        auto area = getLocalBounds();
        area.removeFromTop(25); // utility bar + spacing
        auto sliderArea = area.removeFromTop(area.getHeight() - 120);
        return sliderArea.withWidth(40).withCentre(sliderArea.getCentre());
    }
    
    juce::Point<float> getThumbPosition() const 
    {
        auto trackBounds = getVisualTrackBounds().toFloat();
        
        // Calculate thumb position based on slider value
        auto value = mainSlider.getValue();
        float norm = juce::jmap<float>(value, mainSlider.getMinimum(), mainSlider.getMaximum(), 0.0f, 1.0f);
        
        // Map to the usable track area (excluding thumb size)
        float thumbHeight = 24.0f;
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
        updateDisplayValue();
        updateTargetDisplayValue();
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
                               lockState ? juce::Colours::orange : juce::Colours::lightgrey);
            
            // Enable/disable slider interaction
            mainSlider.setInterceptsMouseClicks(!lockState, !lockState);
        }
    }
    
    // Set custom display range - this is the key method for display mapping
    void setDisplayRange(double minVal, double maxVal)
    {
        displayMin = minVal;
        displayMax = maxVal;
        updateDisplayValue();
        updateTargetDisplayValue();
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
        
        // Start timer if not already running (for activity timeout)
        if (!isTimerRunning())
            startTimer(16); // ~60fps for smooth timeout
            
        repaint(); // Immediate visual feedback
    }
    
    // Timer callback for automation and MIDI activity timeout
    void timerCallback() override
    {
        double currentTime = juce::Time::getMillisecondCounterHiRes();
        
        // Handle automation
        if (isAutomating)
        {
            double elapsed = (currentTime - automationStartTime) / 1000.0;
            
            if (elapsed < delayTime)
            {
                // Still in delay phase
            }
            else
            {
                double attackElapsed = elapsed - delayTime;
                
                if (attackElapsed >= attackTime)
                {
                    // Automation complete
                    mainSlider.setValue(targetMidiValue, juce::dontSendNotification);
                    updateDisplayValue();
                    if (sendMidiCallback)
                        sendMidiCallback(index, (int)targetMidiValue);
                    
                    isAutomating = false;
                    goButton.setButtonText("GO");
                }
                else
                {
                    // Continue automation
                    double progress = attackElapsed / attackTime;
                    double currentValue = startMidiValue + (targetMidiValue - startMidiValue) * progress;
                    
                    mainSlider.setValue(currentValue, juce::dontSendNotification);
                    updateDisplayValue();
                    if (sendMidiCallback)
                        sendMidiCallback(index, (int)currentValue);
                }
            }
        }
        
        // Handle MIDI activity timeout
        if (midiActivityState && (currentTime - lastMidiSendTime) > MIDI_ACTIVITY_DURATION)
        {
            midiActivityState = false;
            repaint(); // Redraw to show inactive state
        }
        
        // Stop timer if no longer needed
        if (!isAutomating && !midiActivityState)
        {
            stopTimer();
        }
    }
    
    // Toggle lock state
    void toggleLock()
    {
        setLocked(!lockState);
    }
    
    void setDelayTime(double delay)
    {
        delaySlider.setValue(delay, juce::dontSendNotification);
    }

    double getDelayTime() const
    {
        return delaySlider.getValue();
    }

    void setAttackTime(double attack)
    {
        attackSlider.setValue(attack, juce::dontSendNotification);
    }

    double getAttackTime() const
    {
        return attackSlider.getValue();
    }
    
    // For keyboard movement - updates slider without changing target input
    void setValueFromKeyboard(double newValue)
    {
        mainSlider.setValue(newValue, juce::dontSendNotification);
        updateDisplayValue();
        if (sendMidiCallback)
            sendMidiCallback(index, (int)newValue);
    }
    
private:
    // Convert MIDI value (0-16383) to display value based on custom range
    double midiToDisplayValue(double midiValue) const
    {
        double normalized = midiValue / 16383.0;
        return displayMin + (normalized * (displayMax - displayMin));
    }
    
    // Convert display value to MIDI value (0-16383)
    double displayToMidiValue(double displayValue) const
    {
        double normalized = (displayValue - displayMin) / (displayMax - displayMin);
        return juce::jlimit(0.0, 16383.0, normalized * 16383.0);
    }
    
    void updateDisplayValue()
    {
        double midiValue = mainSlider.getValue();
        double displayValue = midiToDisplayValue(midiValue);
        
        // Format the display value nicely
        juce::String displayText;
        if (std::abs(displayValue) < 0.01)
            displayText = "0";
        else if (std::abs(displayValue - std::round(displayValue)) < 0.01)
            displayText = juce::String((int)std::round(displayValue));
        else
            displayText = juce::String(displayValue, 2);
            
        currentValueLabel.setText(displayText, juce::dontSendNotification);
    }
    
    void updateTargetDisplayValue()
    {
        // Update the target input to show the current value in display range
        double midiValue = mainSlider.getValue();
        double displayValue = midiToDisplayValue(midiValue);
        
        juce::String displayText;
        if (std::abs(displayValue - std::round(displayValue)) < 0.01)
            displayText = juce::String((int)std::round(displayValue));
        else
            displayText = juce::String(displayValue, 2);
            
        targetInput.setText(displayText, juce::dontSendNotification);
    }
    
    void validateTargetValue()
    {
        auto text = targetInput.getText();
        if (text.isEmpty())
        {
            targetInput.setText(juce::String(displayMin), juce::dontSendNotification);
            return;
        }
        
        double displayValue = text.getDoubleValue();
        displayValue = juce::jlimit(displayMin, displayMax, displayValue);
        
        // Format nicely
        juce::String displayText;
        if (std::abs(displayValue - std::round(displayValue)) < 0.01)
            displayText = juce::String((int)std::round(displayValue));
        else
            displayText = juce::String(displayValue, 2);
            
        targetInput.setText(displayText, juce::dontSendNotification);
    }
    
    void startAutomation()
    {
        if (isAutomating) return;
        
        validateTargetValue();
        auto targetText = targetInput.getText();
        if (targetText.isEmpty()) return;
        
        double targetDisplayValue = targetText.getDoubleValue();
        targetMidiValue = displayToMidiValue(targetDisplayValue);
        
        startMidiValue = mainSlider.getValue();
        
        if (std::abs(targetMidiValue - startMidiValue) < 1.0)
        {
            return; // Already at target
        }
        
        delayTime = delaySlider.getValue();
        attackTime = attackSlider.getValue();
        
        if (attackTime <= 0.0)
        {
            // Instant change
            mainSlider.setValue(targetMidiValue, juce::dontSendNotification);
            updateDisplayValue();
            if (sendMidiCallback)
                sendMidiCallback(index, (int)targetMidiValue);
            return;
        }
        
        isAutomating = true;
        automationStartTime = juce::Time::getMillisecondCounterHiRes();
        goButton.setButtonText("STOP");
        
        startTimer(16); // ~60fps updates
    }
    
    void stopAutomation()
    {
        if (!isAutomating) return;
        
        isAutomating = false;
        goButton.setButtonText("GO");
        
        // Stop the timer if not needed for MIDI activity
        if (!midiActivityState)
            stopTimer();
    }
    
    int index;
    std::function<void(int, int)> sendMidiCallback;
    CustomSliderLookAndFeel customLookAndFeel;
    juce::Slider mainSlider;
    juce::Label sliderNumberLabel;
    ClickableLabel lockLabel;
    juce::Label currentValueLabel;
    juce::Slider delaySlider, attackSlider;
    juce::Label delayLabel, attackLabel, targetLabel;
    juce::TextEditor targetInput;
    juce::TextButton goButton;
    
    bool isAutomating = false;
    bool lockState = false; // Lock state for manual controls
    double automationStartTime = 0.0;
    double startMidiValue = 0.0;
    double targetMidiValue = 0.0;
    double delayTime = 0.0;
    double attackTime = 0.0;
    
    // Display mapping variables
    double displayMin = 0.0;
    double displayMax = 16383.0;
    
    // MIDI activity indicator variables
    bool midiActivityState = false;
    double lastMidiSendTime = 0.0;
    static constexpr double MIDI_ACTIVITY_DURATION = 100.0; // milliseconds
    juce::Rectangle<float> midiIndicatorBounds;
    
    juce::Colour sliderColor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSliderControl)
};
