// AutomationControlPanel.h - Automation controls (knobs, buttons, target input)
#pragma once
#include <JuceHeader.h>
#include <cmath>
#include "../CustomKnob.h"
#include "../CustomLEDInput.h"
#include "../Custom3DButton.h"
#include "../AutomationVisualizer.h"
#include "../CustomLookAndFeel.h"
#include "LearnModeOverlay.h"

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
        setupLearnOverlays();
        
        // Set default time mode
        setTimeMode(TimeMode::Seconds);
        
        // Enable mouse event interception to ensure right-clicks reach this component
        setInterceptsMouseClicks(true, true);
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
        
        // Update learn overlay positions
        updateOverlayBounds();
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
    
    // Automation highlighting for config management
    void setHighlighted(bool highlighted) 
    { 
        if (isHighlighted != highlighted)
        {
            isHighlighted = highlighted;
            repaint();
        }
    }
    
    bool isComponentHighlighted() const { return isHighlighted; }
    
    void paint(juce::Graphics& g) override
    {
        if (isHighlighted)
        {
            // Draw green highlighting border around entire automation area
            g.setColour(juce::Colours::lime.withAlpha(0.4f));
            g.fillAll();
            
            // Draw bright green border
            g.setColour(juce::Colours::lime);
            g.drawRect(getLocalBounds(), 2);
        }
    }
    
    // Automation visualizer access
    AutomationVisualizer& getAutomationVisualizer() { return automationVisualizer; }
    
    // Config extraction and application methods for automation configuration management
    // These methods are used by AutomationConfigManager to save/load configurations
    
    // Extract current automation state as AutomationConfig
    // This captures all current knob values, target value, and time mode
    void extractCurrentConfig(double& outTargetValue, double& outDelayTime, double& outAttackTime,
                             double& outReturnTime, double& outCurveValue, TimeMode& outTimeMode) const
    {
        outTargetValue = getTargetValue();
        outDelayTime = getDelayTime();
        outAttackTime = getAttackTime();
        outReturnTime = getReturnTime();
        outCurveValue = getCurveValue();
        outTimeMode = getTimeMode();
    }
    
    // Apply automation configuration to panel controls
    // This sets all knob values, target value, and time mode from a saved config
    void applyConfig(double targetValue, double delayTime, double attackTime,
                    double returnTime, double curveValue, TimeMode timeMode)
    {
        // Apply values to controls
        setTargetValue(targetValue);
        setDelayTime(delayTime);
        setAttackTime(attackTime);
        setReturnTime(returnTime);
        setCurveValue(curveValue);
        setTimeMode(timeMode);
        
        // Update automation visualizer with new parameters
        updateVisualizerParameters();
        
        // Trigger callbacks to notify of changes
        if (onKnobValueChanged)
        {
            // Notify about the configuration change (using delay time as representative value)
            onKnobValueChanged(delayTime);
        }
    }
    
    // Check if current config matches given values (for detecting changes)
    bool configMatches(double targetValue, double delayTime, double attackTime,
                      double returnTime, double curveValue, TimeMode timeMode) const
    {
        const double tolerance = 0.001;
        
        return std::abs(getTargetValue() - targetValue) < tolerance &&
               std::abs(getDelayTime() - delayTime) < tolerance &&
               std::abs(getAttackTime() - attackTime) < tolerance &&
               std::abs(getReturnTime() - returnTime) < tolerance &&
               std::abs(getCurveValue() - curveValue) < tolerance &&
               getTimeMode() == timeMode;
    }
    
    // Reset automation parameters to their default values
    void resetToDefaults()
    {
        // Reset all parameters to defaults
        setTargetValue(0.0);        // Target: 0.0 (or current slider value)
        setDelayTime(0.0);          // Delay: 0.0 seconds
        setAttackTime(1.0);         // Attack: 1.0 seconds
        setReturnTime(0.0);         // Return: 0.0 seconds
        setCurveValue(1.0);         // Curve: 1.0 (linear)
        setTimeMode(TimeMode::Seconds);  // Time Mode: Seconds (default)
        
        // Update automation visualizer with new parameters
        updateVisualizerParameters();
        
        // Trigger callbacks to notify of changes
        if (onKnobValueChanged)
        {
            // Notify about the reset (using delay time as representative value)
            onKnobValueChanged(0.0);
        }
        
        // Trigger repaint to update the UI
        repaint();
        
        DBG("Automation parameters reset to defaults");
    }
    
    // Ensure the entire automation panel area can receive mouse events (including empty spaces)
    bool hitTest(int x, int y) override
    {
        // Accept mouse events anywhere within the automation panel bounds
        return getLocalBounds().contains(x, y);
    }
    
    // Mouse handling for context menu and learn mode
    void mouseDown(const juce::MouseEvent& event) override
    {
        // RIGHT-CLICK TAKES PRIORITY - handle before any child components
        if (event.mods.isRightButtonDown() && !isInLearnMode)
        {
            // Debug logging
            DBG("AutomationControlPanel: Right-click detected at " + juce::String(event.getPosition().x) + ", " + juce::String(event.getPosition().y));
            
            // Show context menu for automation area
            if (onContextMenuRequested)
            {
                onContextMenuRequested(event.getPosition());
                DBG("AutomationControlPanel: Context menu request sent");
                return; // Don't pass to children
            }
            else
            {
                DBG("AutomationControlPanel: onContextMenuRequested callback is null!");
            }
        }
        
        // Handle learn mode clicks for individual components
        if (isInLearnMode)
        {
            handleLearnModeClick(event);
            return; // Don't pass to children in learn mode
        }
        
        // Only pass left-clicks to children for normal component interaction
        if (event.mods.isLeftButtonDown())
        {
            Component::mouseDown(event);
        }
        // For any other clicks, don't pass to children
    }
    
    // Learn mode management
    void setLearnModeActive(bool active, int sliderIndex = -1)
    {
        isInLearnMode = active;
        currentSliderIndex = sliderIndex;
        
        // Show/hide learn overlays
        goButtonOverlay.setLearnModeActive(active);
        delayKnobOverlay.setLearnModeActive(active);
        attackKnobOverlay.setLearnModeActive(active);
        returnKnobOverlay.setLearnModeActive(active);
        curveKnobOverlay.setLearnModeActive(active);
        
        if (active && sliderIndex >= 0)
        {
            // Set up target info for each overlay
            goButtonOverlay.setTargetInfo(MidiTargetType::AutomationGO, sliderIndex);
            delayKnobOverlay.setTargetInfo(MidiTargetType::AutomationDelay, sliderIndex);
            attackKnobOverlay.setTargetInfo(MidiTargetType::AutomationAttack, sliderIndex);
            returnKnobOverlay.setTargetInfo(MidiTargetType::AutomationReturn, sliderIndex);
            curveKnobOverlay.setTargetInfo(MidiTargetType::AutomationCurve, sliderIndex);
        }
        
        repaint();
    }
    
    bool getLearnModeActive() const { return isInLearnMode; }
    
    // Get bounds for specific automation components
    juce::Rectangle<int> getGoButtonBounds() const { return goButton3D.getBounds(); }
    juce::Rectangle<int> getDelayKnobBounds() const { return delayKnob.getBounds(); }
    juce::Rectangle<int> getAttackKnobBounds() const { return attackKnob.getBounds(); }
    juce::Rectangle<int> getReturnKnobBounds() const { return returnKnob.getBounds(); }
    juce::Rectangle<int> getCurveKnobBounds() const { return curveKnob.getBounds(); }
    
    // Callbacks
    std::function<void()> onGoButtonClicked;
    std::function<void(double)> onKnobValueChanged;
    std::function<void(juce::Point<int>)> onContextMenuRequested;
    std::function<void(MidiTargetType, int)> onLearnModeTargetClicked;
    
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
    
    // Config management highlighting
    bool isHighlighted = false;
    
    // Learn mode overlays
    LearnModeOverlay goButtonOverlay;
    LearnModeOverlay delayKnobOverlay;
    LearnModeOverlay attackKnobOverlay;
    LearnModeOverlay returnKnobOverlay;
    LearnModeOverlay curveKnobOverlay;
    
    // Learn mode state
    bool isInLearnMode = false;
    int currentSliderIndex = -1;
    
    void handleLearnModeClick(const juce::MouseEvent& event)
    {
        // Determine which component was clicked
        auto pos = event.getPosition();
        MidiTargetType targetType = MidiTargetType::SliderValue;
        
        if (goButton3D.getBounds().contains(pos))
            targetType = MidiTargetType::AutomationGO;
        else if (delayKnob.getBounds().contains(pos))
            targetType = MidiTargetType::AutomationDelay;
        else if (attackKnob.getBounds().contains(pos))
            targetType = MidiTargetType::AutomationAttack;
        else if (returnKnob.getBounds().contains(pos))
            targetType = MidiTargetType::AutomationReturn;
        else if (curveKnob.getBounds().contains(pos))
            targetType = MidiTargetType::AutomationCurve;
        else
            return; // Click wasn't on a learnable component
        
        if (onLearnModeTargetClicked)
            onLearnModeTargetClicked(targetType, currentSliderIndex);
    }
    
    void setupLearnOverlays()
    {
        // Add overlays as children
        addChildComponent(goButtonOverlay);
        addChildComponent(delayKnobOverlay);
        addChildComponent(attackKnobOverlay);
        addChildComponent(returnKnobOverlay);
        addChildComponent(curveKnobOverlay);
        
        // Set up overlay callbacks
        auto setupOverlayCallback = [this](LearnModeOverlay& overlay) {
            overlay.onTargetClicked = [this](MidiTargetType targetType, int sliderIndex) {
                if (onLearnModeTargetClicked)
                    onLearnModeTargetClicked(targetType, sliderIndex);
            };
        };
        
        setupOverlayCallback(goButtonOverlay);
        setupOverlayCallback(delayKnobOverlay);
        setupOverlayCallback(attackKnobOverlay);
        setupOverlayCallback(returnKnobOverlay);
        setupOverlayCallback(curveKnobOverlay);
    }
    
    void updateOverlayBounds()
    {
        // Position overlays exactly over their target components
        goButtonOverlay.setBounds(goButton3D.getBounds());
        delayKnobOverlay.setBounds(delayKnob.getBounds());
        attackKnobOverlay.setBounds(attackKnob.getBounds());
        returnKnobOverlay.setBounds(returnKnob.getBounds());
        curveKnobOverlay.setBounds(curveKnob.getBounds());
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationControlPanel)
};
