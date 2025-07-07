// SimpleSliderControl.h - DEBUG VERSION ---------------
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

//==============================================================================
class SimpleSliderControl : public juce::Component, public juce::Timer
{
public:
    SimpleSliderControl(int sliderIndex, std::function<void(int, int)> midiCallback)
        : index(sliderIndex), sendMidiCallback(midiCallback), sliderColor(juce::Colours::cyan)
    {
        DBG("SimpleSliderControl constructor START - index: " + juce::String(index));
        
        // Main slider with custom look
        addAndMakeVisible(mainSlider);
        DBG("Added mainSlider to view");
        
        mainSlider.setSliderStyle(juce::Slider::LinearVertical);
        mainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        mainSlider.setRange(0.0, 16383.0, 1.0);
        DBG("Set slider basic properties");
        
        // Initialize look and feel with default color
        customLookAndFeel.setSliderColor(sliderColor);
        DBG("Set look and feel color");
        
        mainSlider.setLookAndFeel(&customLookAndFeel);
        DBG("Applied look and feel to slider");
        
        // Safe callback - update label and send MIDI
        mainSlider.onValueChange = [this]() {
            DBG("Slider value changed - index: " + juce::String(index));
            if (!isAutomating) // Only update if not currently automating
            {
                int value = (int)mainSlider.getValue();
                updateDisplayValue();
                if (sendMidiCallback)
                    sendMidiCallback(index, value);
            }
        };
        
        // Manual override detection
        mainSlider.onDragStart = [this]() {
            DBG("Slider drag start - index: " + juce::String(index));
            if (isAutomating)
            {
                DBG("Manual override detected for slider " + juce::String(index));
                stopTimer();
                isAutomating = false;
                goButton.setButtonText("GO");
            }
        };
        
        // Current value label
        addAndMakeVisible(currentValueLabel);
        currentValueLabel.setText("0", juce::dontSendNotification);
        currentValueLabel.setJustificationType(juce::Justification::centred);
        currentValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::black);
        currentValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        DBG("Created current value label");
        
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
        DBG("Created delay controls");
        
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
        DBG("Created attack controls");
        
        // Target value input
        addAndMakeVisible(targetInput);
        targetInput.setInputRestrictions(0, "-0123456789.");
        targetInput.setText("8192", juce::dontSendNotification);
        targetInput.onReturnKey = [this]() { validateTargetValue(); };
        targetInput.onFocusLost = [this]() { validateTargetValue(); };
        
        addAndMakeVisible(targetLabel);
        targetLabel.setText("Target:", juce::dontSendNotification);
        targetLabel.attachToComponent(&targetInput, true);
        DBG("Created target controls");
        
        // GO button with automation functionality
        addAndMakeVisible(goButton);
        goButton.setButtonText("GO");
        goButton.onClick = [this]() { startAutomation(); };
        DBG("Created GO button");
        
        DBG("SimpleSliderControl constructor COMPLETE - index: " + juce::String(index));
    }
    
    ~SimpleSliderControl()
    {
        DBG("SimpleSliderControl destructor START - index: " + juce::String(index));
        
        // CRITICAL: Stop timer before destruction
        stopTimer();
        DBG("Timer stopped");
        
        // CRITICAL: Remove look and feel before destruction
        mainSlider.setLookAndFeel(nullptr);
        DBG("Look and feel removed");
        
        DBG("SimpleSliderControl destructor COMPLETE - index: " + juce::String(index));
    }
    
    void resized() override
    {
        DBG("SimpleSliderControl resized START - index: " + juce::String(index));
        
        auto area = getLocalBounds();
        DBG("Got local bounds: " + area.toString());
        
        // Main slider takes most of the space
        auto sliderArea = area.removeFromTop(area.getHeight() - 120);
        mainSlider.setBounds(sliderArea);
        DBG("Set main slider bounds: " + sliderArea.toString());
        
        // Current value label
        auto labelArea = area.removeFromTop(25);
        currentValueLabel.setBounds(labelArea);
        DBG("Set label bounds: " + labelArea.toString());
        
        // Delay slider
        auto delayArea = area.removeFromTop(25);
        delaySlider.setBounds(delayArea.removeFromLeft(delayArea.getWidth() - 50));
        DBG("Set delay slider bounds");
        
        // Attack slider
        auto attackArea = area.removeFromTop(25);
        attackSlider.setBounds(attackArea.removeFromLeft(attackArea.getWidth() - 50));
        DBG("Set attack slider bounds");
        
        // Target and GO button
        auto bottomArea = area.removeFromTop(25);
        goButton.setBounds(bottomArea.removeFromRight(40));
        bottomArea.removeFromRight(5); // spacing
        targetInput.setBounds(bottomArea.removeFromRight(60));
        DBG("Set bottom controls bounds");
        
        DBG("SimpleSliderControl resized COMPLETE - index: " + juce::String(index));
    }
    
    void paint(juce::Graphics& g) override
    {
        DBG("SimpleSliderControl paint called - index: " + juce::String(index));
        // Add a simple background so we can see if paint is being called
        g.fillAll(juce::Colours::transparentBlack);
    }
    
    double getValue() const { return mainSlider.getValue(); }
    
    // Set custom display range
    void setDisplayRange(double minVal, double maxVal)
    {
        DBG("Setting display range for slider " + juce::String(index) + ": " +
            juce::String(minVal) + " to " + juce::String(maxVal));
        displayMin = minVal;
        displayMax = maxVal;
        updateDisplayValue();
    }
    
    // Set slider color - now updates the look and feel safely
    void setSliderColor(juce::Colour color)
    {
        DBG("Setting slider color for index " + juce::String(index));
        sliderColor = color;
        customLookAndFeel.setSliderColor(color);
        mainSlider.repaint();
    }
    
    // Timer callback for automation
    void timerCallback() override
    {
        if (!isAutomating) return;
        
        double currentTime = juce::Time::getMillisecondCounterHiRes();
        double elapsed = (currentTime - automationStartTime) / 1000.0;
        
        if (elapsed < delayTime)
        {
            return;
        }
        
        double attackElapsed = elapsed - delayTime;
        
        if (attackElapsed >= attackTime)
        {
            mainSlider.setValue(targetValue, juce::dontSendNotification);
            updateDisplayValue();
            if (sendMidiCallback)
                sendMidiCallback(index, targetValue);
            
            stopTimer();
            isAutomating = false;
            goButton.setButtonText("GO");
            
            DBG("Animation complete for slider " + juce::String(index));
            return;
        }
        
        double progress = attackElapsed / attackTime;
        double currentValue = startValue + (targetValue - startValue) * progress;
        
        mainSlider.setValue(currentValue, juce::dontSendNotification);
        updateDisplayValue();
        if (sendMidiCallback)
            sendMidiCallback(index, (int)currentValue);
    }
    
private:
    void updateDisplayValue()
    {
        DBG("Updating display value for slider " + juce::String(index));
        double actualValue = mainSlider.getValue();
        double normalizedValue = actualValue / 16383.0;
        double displayValue = displayMin + (normalizedValue * (displayMax - displayMin));
        currentValueLabel.setText(juce::String(displayValue, 2), juce::dontSendNotification);
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
        targetInput.setText(juce::String(displayValue, 2), juce::dontSendNotification);
        
        DBG("Validated target display value: " + juce::String(displayValue) + " for slider " + juce::String(index));
    }
    
    void startAutomation()
    {
        if (isAutomating) return;
        
        validateTargetValue();
        auto targetText = targetInput.getText();
        if (targetText.isEmpty()) return;
        
        double targetDisplayValue = targetText.getDoubleValue();
        double normalizedValue = (targetDisplayValue - displayMin) / (displayMax - displayMin);
        targetValue = normalizedValue * 16383.0;
        
        startValue = mainSlider.getValue();
        
        if (std::abs(targetValue - startValue) < 1.0)
        {
            DBG("Already at target value for slider " + juce::String(index));
            return;
        }
        
        delayTime = delaySlider.getValue();
        attackTime = attackSlider.getValue();
        
        DBG("Starting automation for slider " + juce::String(index));
        
        if (attackTime <= 0.0)
        {
            mainSlider.setValue(targetValue, juce::dontSendNotification);
            updateDisplayValue();
            if (sendMidiCallback)
                sendMidiCallback(index, targetValue);
            DBG("Instant change for slider " + juce::String(index));
            return;
        }
        
        isAutomating = true;
        automationStartTime = juce::Time::getMillisecondCounterHiRes();
        goButton.setButtonText("...");
        
        startTimer(16);
    }
    
    int index;
    std::function<void(int, int)> sendMidiCallback;
    CustomSliderLookAndFeel customLookAndFeel;
    juce::Slider mainSlider;
    juce::Label currentValueLabel;
    juce::Slider delaySlider, attackSlider;
    juce::Label delayLabel, attackLabel, targetLabel;
    juce::TextEditor targetInput;
    juce::TextButton goButton;
    
    bool isAutomating = false;
    double automationStartTime = 0.0;
    double startValue = 0.0;
    double targetValue = 0.0;
    double delayTime = 0.0;
    double attackTime = 0.0;
    
    double displayMin = 0.0;
    double displayMax = 16383.0;
    
    juce::Colour sliderColor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSliderControl)
};

//End SimpleSliderControl.h
//=====================
