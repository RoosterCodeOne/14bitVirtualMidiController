// SimpleSliderControl.h - Production Version with MIDI Activity Indicators and Preset Support
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "CustomKnob.h"
#include "CustomLEDInput.h"
#include "Custom3DButton.h"

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
          attackKnob("ATTACK", 0.0, 30.0, CustomKnob::Small), 
          delayKnob("DELAY", 0.0, 10.0, CustomKnob::Small), 
          returnKnob("RETURN", 0.0, 30.0, CustomKnob::Small),
          curveKnob("CURVE", 0.0, 2.0, CustomKnob::Small)
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
                isReturning = false; // Reset return phase flag
                goButton3D.setButtonText("GO");
            }
        };
        
        // Slider number label
        addAndMakeVisible(sliderNumberLabel);
        sliderNumberLabel.setText(juce::String(sliderIndex + 1), juce::dontSendNotification);
        sliderNumberLabel.setJustificationType(juce::Justification::centred);
        sliderNumberLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        sliderNumberLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        
        // Lock label (acting as button)
        addAndMakeVisible(lockLabel);
        lockLabel.setText("U", juce::dontSendNotification); // U for Unlocked by default
        lockLabel.setJustificationType(juce::Justification::centred);
        lockLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        lockLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold)); // Large font to fill space
        lockLabel.onClick = [this]() { toggleLock(); };
        
        // Current value label
        addAndMakeVisible(currentValueLabel);
        currentValueLabel.setText("0", juce::dontSendNotification);
        currentValueLabel.setJustificationType(juce::Justification::centred);
        currentValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::black);
        currentValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        currentValueLabel.setFont(juce::FontOptions(12.0f));
        
        // Attack knob (large)
        addAndMakeVisible(attackKnob);
        attackKnob.setValue(1.0);
        
        // Delay knob (small)
        addAndMakeVisible(delayKnob);
        delayKnob.setValue(0.0);
        
        // Return knob (small)
        addAndMakeVisible(returnKnob);
        returnKnob.setValue(0.0);
        
        // Curve knob (small) - controls automation curve shape
        addAndMakeVisible(curveKnob);
        curveKnob.setValue(1.0); // Default to linear
        
        // Target value input - LED display style
        addAndMakeVisible(targetLEDInput);
        targetLEDInput.setValidatedValue(8192); // Will be converted to display range
        
        // 3D GO button with automation functionality
        addAndMakeVisible(goButton3D);
        goButton3D.onClick = [this]() { 
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
        
        // Utility bar at top (16px height)  
        auto utilityBar = area.removeFromTop(16);
        
        // Slider number label in utility bar (centered)
        sliderNumberLabel.setBounds(utilityBar);
        
        area.removeFromTop(4); // spacing after utility bar
        
        // Main slider - invisible, positioned for mouse interaction in track area
        int automationControlsHeight = 155; // Increased from 145 to 155 (10px more for automation area)
        int availableSliderHeight = area.getHeight() - automationControlsHeight;
        int reducedSliderHeight = (int)(availableSliderHeight * 0.80); // 20% reduction
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
        
        // Current value label
        auto labelArea = area.removeFromTop(20);
        currentValueLabel.setBounds(labelArea);
        
        // MIDI activity indicator - positioned above currentValueLabel on left side
        midiIndicatorBounds = juce::Rectangle<float>(5, labelArea.getY() - 15, 10, 10);
        
        // Lock label - positioned above currentValueLabel on right side, aligned with MIDI indicator
        int lockLabelX = getWidth() - 25; // 5px from right edge, 20px width
        lockLabel.setBounds(lockLabelX, labelArea.getY() - 15, 20, 10);
        
        area.removeFromTop(4); // spacing before automation controls
        
        // New automation layout: GO button, LED input, then knob group
        auto automationArea = area;
        
        // GO button first - moved 12px to the right for zig-zag layout
        auto buttonArea = automationArea.removeFromTop(33); // Height for 3D button + spacing (increased by 3px)
        int buttonX = (buttonArea.getWidth() - 35) / 2 + 12; // Moved 12px to the right
        goButton3D.setBounds(buttonX, buttonArea.getY() + 5, 35, 25); // Increased top padding by 2px
        
        automationArea.removeFromTop(7); // spacing after button (increased by 2px)
        
        // Target LED input - centered horizontally below button
        auto targetArea = automationArea.removeFromTop(28); // Height for LED input + spacing (increased by 3px)
        int targetX = (targetArea.getWidth() - 50) / 2;
        targetLEDInput.setBounds(targetX, targetArea.getY() + 2, 50, 20); // Increased top padding by 2px
        
        automationArea.removeFromTop(7); // spacing after target (increased by 2px)
        
        // Knob group arrangement in vertical zig-zag pattern:
        // Pattern from top to bottom:
        // 1. Delay: top-right position
        // 2. Attack: middle-left position  
        // 3. Return: middle-right position
        // 4. Curve: bottom-left position
        auto knobColumnArea = automationArea;
        
        // Define column area within automation section - reduced spacing by 50%
        int leftX = knobColumnArea.getX() + 15;       // Left column X position (moved closer to center)
        int rightX = knobColumnArea.getRight() - 43;  // Right column X position (moved closer to center)
        int knobSpacing = 23; // Vertical spacing between knob centers (18 + 5px)
        int startY = knobColumnArea.getY() + 2;       // Top margin (moved up 8px from 10 to 2)
        
        // Position knobs in zig-zag pattern:
        delayKnob.setBounds(rightX, startY, 28, 35);                    // 1. Right - Delay
        attackKnob.setBounds(leftX - 1, startY + knobSpacing, 31, 39);  // 2. Left - Attack (10% larger: 28*1.1=31, 35*1.1=39)
        returnKnob.setBounds(rightX, startY + (knobSpacing * 2), 28, 35); // 3. Right - Return
        curveKnob.setBounds(leftX, startY + (knobSpacing * 3), 28, 35);   // 4. Left - Curve
    }
    
    void paint(juce::Graphics& g) override
    {
        // Draw MIDI activity indicator above value label
        juce::Colour indicatorColor = juce::Colours::orange;
        float alpha = midiActivityState ? 1.0f : 0.2f;
        
        g.setColour(indicatorColor.withAlpha(alpha));
        g.fillRect(midiIndicatorBounds);
        
        // Optional: thin outline
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRect(midiIndicatorBounds, 1.0f);
    }
    
    void paintOverChildren(juce::Graphics& g) override
    {
        // Draw arrow from display value to target value ON TOP of all other components
        drawDirectionalArrow(g);
    }
    
    double getValue() const { return mainSlider.getValue(); }
    
    // Methods for parent component to get visual track and thumb positions
    juce::Rectangle<int> getVisualTrackBounds() const 
    {
        // Calculate the full visual track area based on current layout
        auto area = getLocalBounds();
        area.removeFromTop(20); // utility bar + spacing
        int automationControlsHeight = 155; // Increased from 145 to 155 (10px more for automation area)
        int availableSliderHeight = area.getHeight() - automationControlsHeight;
        int reducedSliderHeight = (int)(availableSliderHeight * 0.80); // Match resized() method
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
        targetLEDInput.setValidRange(minVal, maxVal);
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
        
        // Handle automation with three phases: delay, attack, return
        if (isAutomating)
        {
            double elapsed = (currentTime - automationStartTime) / 1000.0;
            
            if (elapsed < delayTime)
            {
                // DELAY PHASE: Still waiting
                // Button shows "STOP" throughout automation
            }
            else if (elapsed < delayTime + attackTime)
            {
                // ATTACK PHASE: Move from start to target with curve applied
                double attackElapsed = elapsed - delayTime;
                double progress = attackElapsed / attackTime;
                double curvedProgress = applyCurve(progress, curveKnob.getValue());
                double currentValue = startMidiValue + (targetMidiValue - startMidiValue) * curvedProgress;
                
                mainSlider.setValue(currentValue, juce::dontSendNotification);
                updateDisplayValue();
                if (sendMidiCallback)
                    sendMidiCallback(index, (int)currentValue);
            }
            else if (returnTime > 0.0 && elapsed < delayTime + attackTime + returnTime)
            {
                // RETURN PHASE: Move from target back to original
                if (!isReturning)
                {
                    isReturning = true;
                }
                
                double returnElapsed = elapsed - delayTime - attackTime;
                double progress = returnElapsed / returnTime;
                double invertedCurve = 2.0 - curveKnob.getValue(); // Invert curve for return phase
                double curvedProgress = applyCurve(progress, invertedCurve);
                double currentValue = targetMidiValue + (originalValue - targetMidiValue) * curvedProgress;
                
                mainSlider.setValue(currentValue, juce::dontSendNotification);
                updateDisplayValue();
                if (sendMidiCallback)
                    sendMidiCallback(index, (int)currentValue);
            }
            else
            {
                // AUTOMATION COMPLETE
                // If we had a return phase, end at original value; otherwise end at target
                double finalValue = (returnTime > 0.0) ? originalValue : targetMidiValue;
                
                mainSlider.setValue(finalValue, juce::dontSendNotification);
                updateDisplayValue();
                if (sendMidiCallback)
                    sendMidiCallback(index, (int)finalValue);
                
                isAutomating = false;
                isReturning = false;
                goButton3D.setButtonText("GO");
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
        updateDisplayValue();
        if (sendMidiCallback)
            sendMidiCallback(index, (int)newValue);
    }
    
    // For MIDI input - updates slider without triggering output (prevents feedback loops)
    void setValueFromMIDI(double newValue)
    {
        mainSlider.setValue(newValue, juce::dontSendNotification);
        updateDisplayValue();
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
    
private:
    double applyCurve(double progress, double curveValue)
    {
        if (curveValue < 1.0)
        {
            // Exponential (0.0 to 1.0)
            double factor = curveValue; // 0.0 = full exponential, 1.0 = linear
            return std::pow(progress, 2.0 - factor);
        }
        else
        {
            // Logarithmic (1.0 to 2.0)  
            double factor = curveValue - 1.0; // 0.0 = linear, 1.0 = full logarithmic
            return std::pow(progress, 1.0 - factor);
        }
    }
    
    void drawDirectionalArrow(juce::Graphics& g)
    {
        // Calculate start point: center bottom of currentValueLabel (display value) moved 15px left
        auto displayBounds = currentValueLabel.getBounds();
        auto startPoint = juce::Point<float>(displayBounds.getCentreX() - 15, displayBounds.getBottom() + 4);
        
        // Calculate end point: center top of targetLEDInput (straight down) moved 15px left
        auto targetBounds = targetLEDInput.getBounds();
        auto endPoint = juce::Point<float>(displayBounds.getCentreX() - 15, targetBounds.getY() - 4);
        
        // Only draw arrow if there's enough vertical space
        if (endPoint.y - startPoint.y > 15)
        {
            // Draw arrow line (straight down)
            g.setColour(juce::Colours::darkgrey.withAlpha(0.8f)); // Increased alpha for better visibility
            juce::Line<float> arrowLine(startPoint, endPoint);
            g.drawLine(arrowLine, 2.0f); // Thicker line for visibility
            
            // Draw arrowhead pointing toward target
            drawArrowhead(g, startPoint, endPoint);
        }
    }
    
    void drawArrowhead(juce::Graphics& g, juce::Point<float> start, juce::Point<float> end)
    {
        // Calculate direction vector
        auto direction = end - start;
        auto length = direction.getDistanceFromOrigin();
        
        if (length > 0)
        {
            // Normalize direction
            direction = direction / length;
            
            // Arrowhead size
            float arrowheadSize = 5.0f;
            
            // Calculate arrowhead points
            auto arrowheadTip = end;
            auto arrowheadBase = end - (direction * arrowheadSize);
            
            // Perpendicular vector for arrowhead wings
            auto perpendicular = juce::Point<float>(-direction.y, direction.x) * (arrowheadSize * 0.5f);
            
            auto wing1 = arrowheadBase + perpendicular;
            auto wing2 = arrowheadBase - perpendicular;
            
            // Draw arrowhead triangle
            juce::Path arrowheadPath;
            arrowheadPath.startNewSubPath(arrowheadTip);
            arrowheadPath.lineTo(wing1);
            arrowheadPath.lineTo(wing2);
            arrowheadPath.closeSubPath();
            
            g.setColour(juce::Colours::darkgrey.withAlpha(0.8f)); // Match line alpha
            g.fillPath(arrowheadPath);
        }
    }
    
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
        
        targetLEDInput.setValidatedValue(displayValue);
    }
    
    void validateTargetValue()
    {
        // LED input handles its own validation
        double displayValue = targetLEDInput.getValidatedValue();
        targetLEDInput.setValidatedValue(displayValue);
    }
    
    void startAutomation()
    {
        if (isAutomating) return;
        
        validateTargetValue();
        double targetDisplayValue = targetLEDInput.getValidatedValue();
        targetMidiValue = displayToMidiValue(targetDisplayValue);
        
        startMidiValue = mainSlider.getValue();
        originalValue = startMidiValue; // Store original value for return phase
        
        if (std::abs(targetMidiValue - startMidiValue) < 1.0)
        {
            return; // Already at target
        }
        
        delayTime = delayKnob.getValue();
        attackTime = attackKnob.getValue();
        returnTime = returnKnob.getValue();
        isReturning = false; // Reset return phase flag
        
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
        
        // Set button text to STOP during automation
        goButton3D.setButtonText("STOP");
        
        startTimer(16); // ~60fps updates
    }
    
    void stopAutomation()
    {
        if (!isAutomating) return;
        
        isAutomating = false;
        isReturning = false; // Reset return phase flag
        goButton3D.setButtonText("GO");
        
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
    CustomKnob attackKnob, delayKnob, returnKnob, curveKnob;
    CustomLEDInput targetLEDInput;
    Custom3DButton goButton3D;
    
    bool isAutomating = false;
    bool lockState = false; // Lock state for manual controls
    double automationStartTime = 0.0;
    double startMidiValue = 0.0;
    double targetMidiValue = 0.0;
    double originalValue = 0.0; // Value at start of automation (for return phase)
    double delayTime = 0.0;
    double attackTime = 0.0;
    double returnTime = 0.0;
    bool isReturning = false; // Flag for return phase
    
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
