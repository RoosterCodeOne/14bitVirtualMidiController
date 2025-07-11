// DebugMidiController.h - Production Version with Preset System
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "SimpleSliderControl.h"
#include "SettingsWindow.h"

//==============================================================================
class DebugMidiController : public juce::Component, public juce::Timer
{
public:
    DebugMidiController()
    {
        // Create 8 slider controls with MIDI callback
        for (int i = 0; i < 8; ++i)
        {
            auto* sliderControl = new SimpleSliderControl(i, [this](int sliderIndex, int value) {
                sendMidiCC(sliderIndex, value);
            });
            sliderControls.add(sliderControl);
            addAndMakeVisible(sliderControl);
        }
        
        // Bank buttons
        addAndMakeVisible(bankAButton);
        bankAButton.setButtonText("A");
        bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
        bankAButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        bankAButton.onClick = [this]() { setBank(0); };
        
        addAndMakeVisible(bankBButton);
        bankBButton.setButtonText("B");
        bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        bankBButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        bankBButton.onClick = [this]() { setBank(1); };
        
        // Settings button
        addAndMakeVisible(settingsButton);
        settingsButton.setButtonText("Settings");
        settingsButton.onClick = [this]() {
            addAndMakeVisible(settingsWindow);
            settingsWindow.setBounds(getLocalBounds());
            settingsWindow.toFront(true);
        };
        
        // Settings window
        addChildComponent(settingsWindow);
        settingsWindow.onSettingsChanged = [this]() { updateSliderSettings(); };
        settingsWindow.onPresetLoaded = [this](const ControllerPreset& preset) {
            applyPresetToSliders(preset);
        };
        
        // Movement speed tooltip
        addAndMakeVisible(movementSpeedLabel);
        movementSpeedLabel.setJustificationType(juce::Justification::centred);
        movementSpeedLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        movementSpeedLabel.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);
        movementSpeedLabel.setFont(juce::FontOptions(12.0f));
        updateMovementSpeedDisplay();
        
        // Initialize MIDI output
        initializeMidiOutput();
        
        // Set initial bank
        setBank(0);
        
        // Load auto-saved state
        loadAutoSavedState();
        
        // Apply initial settings
        updateSliderSettings();
        
        // Initialize keyboard control system
        setWantsKeyboardFocus(true);
        initializeKeyboardControls();
    }
    
    ~DebugMidiController()
    {
        // Stop keyboard timer before destruction
        stopTimer();
        
        // Auto-save current state before destruction
        saveCurrentState();
        
        if (midiOutput)
            midiOutput->stopBackgroundThread();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(24.0f));
        g.drawText("14-Bit Virtual MIDI Controller", 10, 10, getWidth() - 20, 40, juce::Justification::centred);
        
        // Show MIDI status
        g.setFont(juce::FontOptions(14.0f));
        juce::String status = midiOutput ? "MIDI: Connected" : "MIDI: Disconnected";
        g.drawText(status, 10, 10, 200, 20, juce::Justification::left);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromTop(80); // Title + status space
        
        // Settings button - positioned on left under MIDI status
        settingsButton.setBounds(10, 35, 100, 25);
        
        // Bank buttons - positioned on top right
        int buttonWidth = 40;
        int buttonHeight = 25;
        int rightMargin = 10;
        bankBButton.setBounds(getWidth() - rightMargin - buttonWidth, 10, buttonWidth, buttonHeight);
        bankAButton.setBounds(getWidth() - rightMargin - (buttonWidth * 2) - 5, 10, buttonWidth, buttonHeight);
        
        // Reserve space for button area
        area.removeFromTop(40);
        
        // Reserve space for movement speed tooltip at bottom
        auto tooltipArea = area.removeFromBottom(25);
        movementSpeedLabel.setBounds(tooltipArea);
        
        // Divide remaining space between visible sliders (4 at a time)
        int sliderWidth = area.getWidth() / 4;
        for (int i = 0; i < 4; ++i)
        {
            int sliderIndex = currentBank * 4 + i;
            if (sliderIndex < sliderControls.size())
            {
                auto sliderBounds = area.removeFromLeft(sliderWidth);
                sliderBounds.reduce(10, 0); // Gap between sliders
                sliderControls[sliderIndex]->setBounds(sliderBounds);
            }
        }
        
        // Settings window
        if (settingsWindow.isVisible())
            settingsWindow.setBounds(getLocalBounds());
    }
    
    bool keyPressed(const juce::KeyPress& key) override
    {
        // Allow system shortcuts when modifier keys are held
        if (key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown() || 
            key.getModifiers().isAltDown())
        {
            return false; // Let the system handle Command+Q, Ctrl+C, etc.
        }
        
        // Don't interfere when any text editor has focus
        if (isTextEditorFocused())
        {
            return false;
        }
        
        auto keyChar = key.getKeyCode();
        
        // Handle movement rate adjustment (Z/X keys) - discrete rates
        if (keyChar == 'Z' || keyChar == 'z')
        {
            if (currentRateIndex > 0)
            {
                currentRateIndex--;
                keyboardMovementRate = movementRates[currentRateIndex];
                updateMovementSpeedDisplay();
            }
            return true;
        }
        if (keyChar == 'X' || keyChar == 'x')
        {
            if (currentRateIndex < movementRates.size() - 1)
            {
                currentRateIndex++;
                keyboardMovementRate = movementRates[currentRateIndex];
                updateMovementSpeedDisplay();
            }
            return true;
        }
        
        // Handle slider control keys
        for (auto& mapping : keyboardMappings)
        {
            if (keyChar == mapping.upKey || keyChar == mapping.downKey)
            {
                if (!mapping.isPressed)
                {
                    mapping.isPressed = true;
                    mapping.isUpDirection = (keyChar == mapping.upKey);
                    mapping.accumulatedMovement = 0.0; // Reset accumulator
                    
                    // Start timer for smooth movement if not already running
                    if (!isTimerRunning())
                        startTimer(16); // ~60fps
                }
                return true;
            }
        }
        
        return false;
    }
    
    bool keyStateChanged(bool isKeyDown) override
    {
        // Don't interfere when modifier keys are held or text editor has focus
        if (juce::ModifierKeys::getCurrentModifiers().isCommandDown() || 
            juce::ModifierKeys::getCurrentModifiers().isCtrlDown() ||
            juce::ModifierKeys::getCurrentModifiers().isAltDown() ||
            isTextEditorFocused())
        {
            return false;
        }
        
        if (!isKeyDown)
        {
            // Key released - check if any of our keys were released
            for (auto& mapping : keyboardMappings)
            {
                if (mapping.isPressed)
                {
                    // Check if this key is still pressed
                    bool upStillPressed = juce::KeyPress::isKeyCurrentlyDown(mapping.upKey);
                    bool downStillPressed = juce::KeyPress::isKeyCurrentlyDown(mapping.downKey);
                    
                    if (!upStillPressed && !downStillPressed)
                    {
                        mapping.isPressed = false;
                    }
                }
            }
            
            // Stop timer if no keys are pressed
            bool anyKeyPressed = false;
            for (const auto& mapping : keyboardMappings)
            {
                if (mapping.isPressed)
                {
                    anyKeyPressed = true;
                    break;
                }
            }
            
            if (!anyKeyPressed && isTimerRunning())
                stopTimer();
        }
        
        return false;
    }
    
    void timerCallback() override
    {
        for (auto& mapping : keyboardMappings)
        {
            if (mapping.isPressed && mapping.sliderIndex < sliderControls.size())
            {
                auto* slider = sliderControls[mapping.sliderIndex];
                
                // Check if slider is locked
                if (slider && !slider->isLocked())
                {
                    double currentValue = slider->getValue();
                    double newValue = currentValue;
                    
                    // Handle instant movement (100%)
                    if (keyboardMovementRate == -1)
                    {
                        if (mapping.isUpDirection)
                            newValue = 16383.0; // Go to max instantly
                        else
                            newValue = 0.0; // Go to min instantly
                    }
                    else
                    {
                        // Calculate movement delta based on rate (MIDI units per second)
                        double deltaTime = 1.0 / 60.0; // Assuming 60fps timer
                        double movementDelta = keyboardMovementRate * deltaTime;
                        
                        // Accumulate fractional movement
                        double direction = mapping.isUpDirection ? 1.0 : -1.0;
                        mapping.accumulatedMovement += movementDelta * direction;
                        
                        // Only move when we've accumulated at least 1 unit
                        if (std::abs(mapping.accumulatedMovement) >= 1.0)
                        {
                            double wholeUnitsToMove = std::floor(std::abs(mapping.accumulatedMovement));
                            
                            if (mapping.accumulatedMovement > 0)
                            {
                                newValue = juce::jmin(16383.0, currentValue + wholeUnitsToMove);
                                mapping.accumulatedMovement -= wholeUnitsToMove;
                            }
                            else
                            {
                                newValue = juce::jmax(0.0, currentValue - wholeUnitsToMove);
                                mapping.accumulatedMovement += wholeUnitsToMove;
                            }
                        }
                    }
                    
                    if (newValue != currentValue)
                    {
                        slider->setValueFromKeyboard(newValue);
                    }
                }
            }
        }
    }
    
private:
    struct KeyboardMapping
    {
        int sliderIndex;
        int upKey;
        int downKey;
        bool isPressed = false;
        bool isUpDirection = false;
        double accumulatedMovement = 0.0; // For fractional movement accumulation
    };
    
    void initializeKeyboardControls()
    {
        keyboardMappings.clear();
        
        // Q/A for slider 1, W/S for slider 2, E/D for slider 3, R/F for slider 4
        keyboardMappings.push_back({0, 'Q', 'A'});
        keyboardMappings.push_back({1, 'W', 'S'});
        keyboardMappings.push_back({2, 'E', 'D'});
        keyboardMappings.push_back({3, 'R', 'F'});
        
        // U/J, I/K, O/L, P/; for sliders 5-8
        keyboardMappings.push_back({4, 'U', 'J'});
        keyboardMappings.push_back({5, 'I', 'K'});
        keyboardMappings.push_back({6, 'O', 'L'});
        keyboardMappings.push_back({7, 'P', ';'});
        
        // Initialize discrete movement rates (last one is special: -1 = instant/100%)
        movementRates = {1, 5, 50, 100, 250, 500, 1000, 2500, 5000, 10000, -1};
        currentRateIndex = 2; // Start with 50 units/sec
        keyboardMovementRate = movementRates[currentRateIndex];
    }
    
    void updateMovementSpeedDisplay()
    {
        juce::String speedText;
        if (keyboardMovementRate == -1)
            speedText = "Keyboard Speed: 100% (instant) (Z/X to adjust)";
        else
            speedText = "Keyboard Speed: " + juce::String((int)keyboardMovementRate) + " units/sec (Z/X to adjust)";
        movementSpeedLabel.setText(speedText, juce::dontSendNotification);
    }
    
    bool isTextEditorFocused()
    {
        // Check if any text editor in the application currently has focus
        auto* focusedComponent = juce::Component::getCurrentlyFocusedComponent();
        return (focusedComponent != nullptr && 
                dynamic_cast<juce::TextEditor*>(focusedComponent) != nullptr);
    }
    
    void updateSliderSettings()
    {
        for (int i = 0; i < sliderControls.size(); ++i)
        {
            // Update display range
            auto range = settingsWindow.getCustomRange(i);
            sliderControls[i]->setDisplayRange(range.first, range.second);
            
            // Update color
            auto color = settingsWindow.getSliderColor(i);
            sliderControls[i]->setSliderColor(color);
        }
        
        // Auto-save whenever settings change
        saveCurrentState();
    }
    
    void applyPresetToSliders(const ControllerPreset& preset)
    {
        // Apply slider values and lock states from preset
        for (int i = 0; i < juce::jmin(sliderControls.size(), preset.sliders.size()); ++i)
        {
            const auto& sliderPreset = preset.sliders[i];
            
            // Apply slider value (convert to internal MIDI range if needed)
            if (sliderControls[i])
            {
                sliderControls[i]->setValue(sliderPreset.currentValue);
                
                // Apply lock state
                if (sliderPreset.isLocked)
                    sliderControls[i]->setLocked(true);
                else
                    sliderControls[i]->setLocked(false);
                    
                // NEW: Apply delay and attack times
                sliderControls[i]->setDelayTime(sliderPreset.delayTime);
                sliderControls[i]->setAttackTime(sliderPreset.attackTime);
            }
        }
        
        // Update settings to reflect the new configuration
        updateSliderSettings();
    }
    
    void saveCurrentState()
    {
        auto preset = getCurrentControllerState();
        settingsWindow.getPresetManager().autoSaveCurrentState(preset);
    }
    
    void loadAutoSavedState()
    {
        auto preset = settingsWindow.getPresetManager().loadAutoSavedState();
        
        // Force settings window to initialize its controls if not already done
        if (!settingsWindow.isVisible())
        {
            settingsWindow.setVisible(true);
            settingsWindow.setVisible(false);
        }
        
        // Apply the preset to settings window (CC numbers, ranges, colors)
        settingsWindow.applyPreset(preset);
        
        // Apply to sliders (values, lock states, delay/attack times)
        applyPresetToSliders(preset);
    }
    
    ControllerPreset getCurrentControllerState()
    {
        auto preset = settingsWindow.getCurrentPreset();
        
        // Add current slider values, lock states, and delay/attack times
        for (int i = 0; i < juce::jmin(sliderControls.size(), preset.sliders.size()); ++i)
        {
            if (sliderControls[i])
            {
                preset.sliders.getReference(i).currentValue = sliderControls[i]->getValue();
                preset.sliders.getReference(i).isLocked = sliderControls[i]->isLocked();
                preset.sliders.getReference(i).delayTime = sliderControls[i]->getDelayTime();    // NEW
                preset.sliders.getReference(i).attackTime = sliderControls[i]->getAttackTime();  // NEW
            }
        }
        
        return preset;
    }
    
    void setBank(int bank)
    {
        currentBank = bank;
        
        // Update button colors
        if (bank == 0)
        {
            bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        }
        else
        {
            bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
            bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
        }
        
        // Hide all sliders first
        for (auto* slider : sliderControls)
            slider->setVisible(false);
        
        // Show only the sliders for the current bank
        for (int i = 0; i < 4; ++i)
        {
            int sliderIndex = currentBank * 4 + i;
            if (sliderIndex < sliderControls.size())
                sliderControls[sliderIndex]->setVisible(true);
        }
        
        resized(); // Re-layout
    }
    
    void initializeMidiOutput()
    {
        auto midiDevices = juce::MidiOutput::getAvailableDevices();
        
        if (!midiDevices.isEmpty())
        {
            midiOutput = juce::MidiOutput::openDevice(midiDevices[0].identifier);
        }
        else
        {
            // Create virtual MIDI output
            midiOutput = juce::MidiOutput::createNewDevice("JUCE Virtual Controller");
        }
        
        if (midiOutput)
            midiOutput->startBackgroundThread();
    }
    
    void sendMidiCC(int sliderIndex, int value14bit)
    {
        if (!midiOutput) return;
        
        // Use settings from settings window
        int midiChannel = settingsWindow.getMidiChannel();
        int ccNumber = settingsWindow.getCCNumber(sliderIndex);
        
        // Convert 14-bit value to MSB and LSB
        int msb = (value14bit >> 7) & 0x7F;
        int lsb = value14bit & 0x7F;
        
        // Send MSB
        juce::MidiMessage msbMessage = juce::MidiMessage::controllerEvent(midiChannel, ccNumber, msb);
        midiOutput->sendMessageNow(msbMessage);
        
        // Send LSB
        if (ccNumber < 96)
        {
            juce::MidiMessage lsbMessage = juce::MidiMessage::controllerEvent(midiChannel, ccNumber + 32, lsb);
            midiOutput->sendMessageNow(lsbMessage);
        }
        
        // Trigger MIDI activity indicator AFTER successful MIDI send
        if (sliderIndex < sliderControls.size())
            sliderControls[sliderIndex]->triggerMidiActivity();
    }
    
    juce::OwnedArray<SimpleSliderControl> sliderControls;
    juce::TextButton settingsButton;
    juce::TextButton bankAButton, bankBButton;
    SettingsWindow settingsWindow;
    juce::Label movementSpeedLabel;
    std::unique_ptr<juce::MidiOutput> midiOutput;
    int currentBank = 0;
    
    // Keyboard control members
    std::vector<KeyboardMapping> keyboardMappings;
    std::vector<int> movementRates;
    int currentRateIndex = 2;
    double keyboardMovementRate = 50.0; // MIDI units per second
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugMidiController)
};
