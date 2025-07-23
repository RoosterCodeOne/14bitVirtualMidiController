// AutomationControlPanel.h - Automation controls (knobs, buttons, target input)
#pragma once
#include <JuceHeader.h>
#include "../CustomKnob.h"
#include "../CustomLEDInput.h"
#include "../Custom3DButton.h"
#include "../AutomationVisualizer.h"
#include "../CustomLookAndFeel.h"

//==============================================================================
class AutomationControlPanel : public juce::Component
{
public:
    // Time mode enumeration
    enum class TimeMode { Seconds, Beats };
    
    AutomationControlPanel()
        : attackKnob("ATTACK", 0.0, 30.0, CustomKnob::Smaller), 
          delayKnob("DELAY", 0.0, 10.0, CustomKnob::Smaller), 
          returnKnob("RETURN", 0.0, 30.0, CustomKnob::Smaller),
          curveKnob("CURVE", 0.0, 2.0, CustomKnob::Smaller),
          currentTimeMode(TimeMode::Seconds)
    {
        setupKnobs();
        setupButtons();
        setupLabels();
        setupVisualizer();
        setupTargetInput();
        
        // Set default time mode
        setTimeMode(TimeMode::Seconds);
    }
    
    ~AutomationControlPanel()
    {
        secButton.setLookAndFeel(nullptr);
        beatButton.setLookAndFeel(nullptr);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        
        // GO button - centered horizontally, at top
        auto buttonArea = area.removeFromTop(33);
        int buttonX = (buttonArea.getWidth() - 35) / 2;
        goButton3D.setBounds(buttonX, buttonArea.getY() + 5, 35, 25);
        
        area.removeFromTop(7); // spacing after button
        
        // Target LED input - centered horizontally below button
        auto targetArea = area.removeFromTop(28);
        int displayWidth = getWidth() - 8; // Match parent's reduced width
        int targetX = (targetArea.getWidth() - displayWidth) / 2;
        targetLEDInput.setBounds(targetX, targetArea.getY() + 2, displayWidth, 20);
        
        area.removeFromTop(7); // spacing after target
        
        // Calculate dimensions for automation visualizer and knob grid
        int knobWidth = 42;
        int knobHeight = 57;
        int horizontalSpacing = 9;
        int verticalSpacing = 2;
        int totalGridWidth = (2 * knobWidth) + horizontalSpacing;
        int totalGridHeight = (2 * knobHeight) + verticalSpacing;
        int centerX = area.getCentreX();
        int gridStartX = centerX - (totalGridWidth / 2);
        
        // Automation visualizer - positioned above knob grid
        int visualizerWidth = totalGridWidth;
        int visualizerHeight = 60;
        int visualizerX = gridStartX;
        int visualizerY = area.getY() - 2;
        
        automationVisualizer.setBounds(visualizerX, visualizerY, visualizerWidth, visualizerHeight);
        
        // Knob group arrangement in 2x2 grid positioned below visualizer:
        // [DELAY]   [ATTACK]
        // [RETURN]  [CURVE]
        int knobStartY = visualizerY + visualizerHeight + 8;
        
        // Top row: [DELAY] [ATTACK]
        int delayX = gridStartX;
        int attackX = gridStartX + knobWidth + horizontalSpacing;
        int topRowY = knobStartY;
        
        delayKnob.setBounds(delayX, topRowY, knobWidth, knobHeight);
        attackKnob.setBounds(attackX, topRowY, knobWidth, knobHeight);
        
        // Bottom row: [RETURN] [CURVE]
        int returnX = gridStartX;
        int curveX = gridStartX + knobWidth + horizontalSpacing;
        int bottomRowY = knobStartY + knobHeight + verticalSpacing - 6;
        
        returnKnob.setBounds(returnX, bottomRowY, knobWidth, knobHeight);
        curveKnob.setBounds(curveX, bottomRowY, knobWidth, knobHeight);
        
        // Time mode toggle buttons below knob grid
        int toggleStartY = bottomRowY + knobHeight + 1;
        int buttonWidth = 16;
        int buttonHeight = 12;
        int buttonSpacing = 1;
        int labelSpacing = 2;
        
        // Calculate total width: label + gap + button + spacing + button + gap + label
        int labelWidth = 24;
        int totalToggleWidth = labelWidth + labelSpacing + buttonWidth + buttonSpacing + buttonWidth + labelSpacing + labelWidth;
        
        // Ensure buttons fit within the knob grid width
        int maxToggleWidth = totalGridWidth - 4;
        if (totalToggleWidth > maxToggleWidth) {
            float scaleFactor = (float)maxToggleWidth / totalToggleWidth;
            labelWidth = (int)(labelWidth * scaleFactor);
            buttonWidth = (int)(buttonWidth * scaleFactor);
            totalToggleWidth = maxToggleWidth;
        }
        
        int toggleStartX = centerX - (totalToggleWidth / 2);
        
        // SEC label and button (left side)
        secLabel.setBounds(toggleStartX, toggleStartY, labelWidth, buttonHeight);
        secButton.setBounds(toggleStartX + labelWidth + labelSpacing, toggleStartY, buttonWidth, buttonHeight);
        
        // BEAT button and label (right side)
        int beatButtonX = toggleStartX + labelWidth + labelSpacing + buttonWidth + buttonSpacing;
        beatButton.setBounds(beatButtonX, toggleStartY, buttonWidth, buttonHeight);
        beatLabel.setBounds(beatButtonX + buttonWidth + labelSpacing, toggleStartY, labelWidth, buttonHeight);
        
        // Force automation visualizer repaint after resizing
        automationVisualizer.repaint();
    }
    
    // Knob value getters/setters
    void setDelayTime(double delay) { delayKnob.setValue(delay); }
    double getDelayTime() const { return delayKnob.getValue(); }
    
    void setAttackTime(double attack) { attackKnob.setValue(attack); }
    double getAttackTime() const { return attackKnob.getValue(); }
    
    void setReturnTime(double returnVal) { returnKnob.setValue(returnVal); }
    double getReturnTime() const { return returnKnob.getValue(); }
    
    void setCurveValue(double curve) { curveKnob.setValue(curve); }
    double getCurveValue() const { return curveKnob.getValue(); }
    
    // Target input methods
    void setTargetValue(double value) { targetLEDInput.setValidatedValue(value); }
    double getTargetValue() const { return targetLEDInput.getValidatedValue(); }
    void setTargetRange(double minVal, double maxVal) { targetLEDInput.setValidRange(minVal, maxVal); }
    
    // Time mode methods
    void setTimeMode(TimeMode mode)
    {
        if (currentTimeMode == mode) return;
        
        currentTimeMode = mode;
        
        // Update button toggle states
        secButton.setToggleState(mode == TimeMode::Seconds, juce::dontSendNotification);
        beatButton.setToggleState(mode == TimeMode::Beats, juce::dontSendNotification);
        
        // Update knobs with new time mode
        CustomKnob::TimeMode knobTimeMode = (mode == TimeMode::Seconds) ? 
            CustomKnob::TimeMode::Seconds : CustomKnob::TimeMode::Beats;
        
        delayKnob.setTimeMode(knobTimeMode);
        attackKnob.setTimeMode(knobTimeMode); 
        returnKnob.setTimeMode(knobTimeMode);
    }
    
    TimeMode getTimeMode() const { return currentTimeMode; }
    
    // Automation visualizer access
    AutomationVisualizer& getAutomationVisualizer() { return automationVisualizer; }
    
    // Callbacks
    std::function<void()> onGoButtonClicked;
    std::function<void(double)> onKnobValueChanged;
    
private:
    void setupKnobs()
    {
        // Attack knob
        addAndMakeVisible(attackKnob);
        attackKnob.setValue(1.0);
        attackKnob.onValueChanged = [this](double newValue) {
            updateVisualizerParameters();
            if (onKnobValueChanged) onKnobValueChanged(newValue);
        };
        
        // Delay knob
        addAndMakeVisible(delayKnob);
        delayKnob.setValue(0.0);
        delayKnob.onValueChanged = [this](double newValue) {
            updateVisualizerParameters();
            if (onKnobValueChanged) onKnobValueChanged(newValue);
        };
        
        // Return knob
        addAndMakeVisible(returnKnob);
        returnKnob.setValue(0.0);
        returnKnob.onValueChanged = [this](double newValue) {
            updateVisualizerParameters();
            if (onKnobValueChanged) onKnobValueChanged(newValue);
        };
        
        // Curve knob
        addAndMakeVisible(curveKnob);
        curveKnob.setValue(1.0);
        curveKnob.onValueChanged = [this](double newValue) {
            updateVisualizerParameters();
            if (onKnobValueChanged) onKnobValueChanged(newValue);
        };
    }
    
    void setupButtons()
    {
        // 3D GO button
        addAndMakeVisible(goButton3D);
        goButton3D.onClick = [this]() { 
            if (onGoButtonClicked) onGoButtonClicked();
        };
        
        // Time mode buttons
        addAndMakeVisible(secButton);
        addAndMakeVisible(beatButton);
        
        // Configure SEC button (default active)
        secButton.setToggleState(true, juce::dontSendNotification);
        secButton.setLookAndFeel(&buttonLookAndFeel);
        secButton.setRadioGroupId(1001);
        secButton.onClick = [this]() { 
            if (currentTimeMode != TimeMode::Seconds) {
                setTimeMode(TimeMode::Seconds); 
            } else {
                secButton.setToggleState(true, juce::dontSendNotification);
            }
        };
        
        // Configure BEAT button
        beatButton.setToggleState(false, juce::dontSendNotification);
        beatButton.setLookAndFeel(&buttonLookAndFeel);
        beatButton.setRadioGroupId(1001);
        beatButton.onClick = [this]() { 
            if (currentTimeMode != TimeMode::Beats) {
                setTimeMode(TimeMode::Beats); 
            } else {
                beatButton.setToggleState(true, juce::dontSendNotification);
            }
        };
    }
    
    void setupLabels()
    {
        addAndMakeVisible(secLabel);
        addAndMakeVisible(beatLabel);
        
        secLabel.setText("SEC", juce::dontSendNotification);
        secLabel.setJustificationType(juce::Justification::centred);
        secLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        secLabel.setFont(juce::FontOptions(9.0f));
        
        beatLabel.setText("BEAT", juce::dontSendNotification);
        beatLabel.setJustificationType(juce::Justification::centred);
        beatLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        beatLabel.setFont(juce::FontOptions(9.0f));
    }
    
    void setupVisualizer()
    {
        addAndMakeVisible(automationVisualizer);
        updateVisualizerParameters();
    }
    
    void setupTargetInput()
    {
        addAndMakeVisible(targetLEDInput);
        targetLEDInput.setValidatedValue(8192); // Default center value
    }
    
    void updateVisualizerParameters()
    {
        automationVisualizer.setParameters(delayKnob.getValue(), attackKnob.getValue(),
                                         returnKnob.getValue(), curveKnob.getValue());
    }
    
    // Components
    CustomKnob attackKnob, delayKnob, returnKnob, curveKnob;
    CustomLEDInput targetLEDInput;
    Custom3DButton goButton3D;
    AutomationVisualizer automationVisualizer;
    
    // Time mode controls
    TimeMode currentTimeMode;
    juce::ToggleButton secButton, beatButton;
    juce::Label secLabel, beatLabel;
    CustomButtonLookAndFeel buttonLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationControlPanel)
};