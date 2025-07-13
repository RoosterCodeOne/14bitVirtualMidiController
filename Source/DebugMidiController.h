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
        // Create 16 slider controls with MIDI callback
        for (int i = 0; i < 16; ++i)
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
        
        addAndMakeVisible(bankCButton);
        bankCButton.setButtonText("C");
        bankCButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        bankCButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        bankCButton.onClick = [this]() { setBank(2); };
        
        addAndMakeVisible(bankDButton);
        bankDButton.setButtonText("D");
        bankDButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        bankDButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        bankDButton.onClick = [this]() { setBank(3); };
        
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
        movementSpeedLabel.setJustificationType(juce::Justification::centredLeft);
        movementSpeedLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        movementSpeedLabel.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);
        movementSpeedLabel.setFont(juce::FontOptions(12.0f));
        updateMovementSpeedDisplay();
        
        // Window size tooltip
        addAndMakeVisible(windowSizeLabel);
        windowSizeLabel.setJustificationType(juce::Justification::centredRight);
        windowSizeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        windowSizeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);
        windowSizeLabel.setFont(juce::FontOptions(12.0f));
        updateWindowSizeDisplay();
        
        // Initialize MIDI output
        initializeMidiOutput();
        
        // Set initial bank and slider visibility
        setBank(0);
        updateSliderVisibility();
        updateBankButtonStates();
        
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
        
        CustomSliderLookAndFeel lookAndFeel;
        
        // Draw plates and visual tracks for each visible slider
        int visibleSliderCount = isEightSliderMode ? 8 : 4;
        for (int i = 0; i < visibleSliderCount; ++i)
        {
            int sliderIndex = getVisibleSliderIndex(i);
            if (sliderIndex < sliderControls.size())
            {
                auto* sliderControl = sliderControls[sliderIndex];
                auto sliderBounds = sliderControl->getBounds();
                
                // Draw the plate background
                lookAndFeel.drawExtendedModulePlate(g, sliderBounds.toFloat());
                
                // Get visual track bounds relative to this component
                auto trackBounds = sliderControl->getVisualTrackBounds();
                trackBounds.setX(trackBounds.getX() + sliderBounds.getX());
                trackBounds.setY(trackBounds.getY() + sliderBounds.getY());
                
                // Set slider color
                lookAndFeel.setSliderColor(sliderControl->getSliderColor());
                
                // Draw the track
                lookAndFeel.drawSliderTrack(g, trackBounds.toFloat(), sliderControl->getSliderColor());
                
                // Draw tick marks
                lookAndFeel.drawTickMarks(g, trackBounds.toFloat());
                
                // Get thumb position relative to this component
                auto thumbPos = sliderControl->getThumbPosition();
                thumbPos.x += sliderBounds.getX();
                thumbPos.y += sliderBounds.getY();
                
                // Draw the thumb
                lookAndFeel.drawSliderThumb(g, thumbPos.x, thumbPos.y, sliderControl->getSliderColor());
                
                // DEBUG: Draw mainSlider bounds overlay
                auto mainSliderBounds = sliderControl->getMainSliderBounds();
                mainSliderBounds.setX(mainSliderBounds.getX() + sliderBounds.getX());
                mainSliderBounds.setY(mainSliderBounds.getY() + sliderBounds.getY());
                
                g.setColour(juce::Colours::yellow.withAlpha(0.3f));
                g.fillRect(mainSliderBounds);
                g.setColour(juce::Colours::yellow.withAlpha(0.8f));
                g.drawRect(mainSliderBounds, 1);
            }
        }
        
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
        
        // Calculate how many sliders can fit with fixed 175px width + gaps
        int visibleSliderCount = calculateVisibleSliderCount();
        bool shouldBeEightSliderMode = (visibleSliderCount == 8);
        
        if (shouldBeEightSliderMode != isEightSliderMode)
        {
            isEightSliderMode = shouldBeEightSliderMode;
            updateSliderVisibility();
            updateBankButtonStates();
        }
        
        // Update window size display
        updateWindowSizeDisplay();
        
        area.removeFromTop(80); // Title + status space
        
        // Settings button - positioned on left under MIDI status
        settingsButton.setBounds(10, 35, 100, 25);
        
        // Bank buttons - positioned on top right
        int buttonWidth = 40;
        int buttonHeight = 25;
        int rightMargin = 10;
        bankDButton.setBounds(getWidth() - rightMargin - buttonWidth, 10, buttonWidth, buttonHeight);
        bankCButton.setBounds(getWidth() - rightMargin - (buttonWidth * 2) - 5, 10, buttonWidth, buttonHeight);
        bankBButton.setBounds(getWidth() - rightMargin - (buttonWidth * 3) - 10, 10, buttonWidth, buttonHeight);
        bankAButton.setBounds(getWidth() - rightMargin - (buttonWidth * 4) - 15, 10, buttonWidth, buttonHeight);
        
        // Reserve space for button area
        area.removeFromTop(40);
        
        // Reserve space for tooltips at bottom
        auto tooltipArea = area.removeFromBottom(25);
        auto leftTooltip = tooltipArea.removeFromLeft(tooltipArea.getWidth() / 2);
        movementSpeedLabel.setBounds(leftTooltip);
        windowSizeLabel.setBounds(tooltipArea);
        
        // Layout sliders with fixed 175px width and proper centering
        layoutSlidersFixed(area, visibleSliderCount);
        
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
        
        // Handle slider control keys - map to currently visible sliders
        int maxMappings = isEightSliderMode ? 8 : 4;
        for (int i = 0; i < keyboardMappings.size() && i < maxMappings; ++i)
        {
            auto& mapping = keyboardMappings[i];
            if (keyChar == mapping.upKey || keyChar == mapping.downKey)
            {
                if (!mapping.isPressed)
                {
                    mapping.isPressed = true;
                    mapping.isUpDirection = (keyChar == mapping.upKey);
                    mapping.accumulatedMovement = 0.0; // Reset accumulator
                    mapping.currentSliderIndex = getVisibleSliderIndex(i); // Map to visible slider
                    
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
            if (mapping.isPressed && mapping.currentSliderIndex < sliderControls.size())
            {
                auto* slider = sliderControls[mapping.currentSliderIndex];
                
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
    
    int calculateVisibleSliderCount()
    {
        // Calculate available width for sliders (excluding margins/gaps)
        int availableWidth = getWidth() - 20; // 10px margin on each side
        
        // Calculate how many 8-slider setup would need
        int eightSliderTotalWidth = (8 * SLIDER_PLATE_WIDTH) + (7 * SLIDER_GAP);
        
        if (availableWidth >= eightSliderTotalWidth)
            return 8;
        else
            return 4;
    }
    
    void layoutSlidersFixed(juce::Rectangle<int> area, int visibleSliderCount)
    {
        // Calculate total width needed for the slider rack
        int totalSliderWidth = (visibleSliderCount * SLIDER_PLATE_WIDTH) + ((visibleSliderCount - 1) * SLIDER_GAP);
        
        // Center the slider rack horizontally
        int startX = (area.getWidth() - totalSliderWidth) / 2;
        
        for (int i = 0; i < visibleSliderCount; ++i)
        {
            int sliderIndex = getVisibleSliderIndex(i);
            if (sliderIndex < sliderControls.size())
            {
                int xPos = startX + (i * (SLIDER_PLATE_WIDTH + SLIDER_GAP));
                auto sliderBounds = juce::Rectangle<int>(xPos, area.getY(), SLIDER_PLATE_WIDTH, area.getHeight());
                sliderControls[sliderIndex]->setBounds(sliderBounds);
            }
        }
    }
    
private:
    struct KeyboardMapping
    {
        int upKey;
        int downKey;
        bool isPressed = false;
        bool isUpDirection = false;
        double accumulatedMovement = 0.0; // For fractional movement accumulation
        int currentSliderIndex = 0; // Maps to currently visible slider
    };
    
    void initializeKeyboardControls()
    {
        keyboardMappings.clear();
        
        // Q/A for visible slider 1, W/S for visible slider 2, E/D for visible slider 3, R/F for visible slider 4
        keyboardMappings.push_back({'Q', 'A'});
        keyboardMappings.push_back({'W', 'S'});
        keyboardMappings.push_back({'E', 'D'});
        keyboardMappings.push_back({'R', 'F'});
        
        // Keep the other mappings for future use when 8 sliders are visible
        keyboardMappings.push_back({'U', 'J'});
        keyboardMappings.push_back({'I', 'K'});
        keyboardMappings.push_back({'O', 'L'});
        keyboardMappings.push_back({'P', ';'});
        
        // Initialize discrete movement rates (last one is special: -1 = instant/100%)
        movementRates = {1, 5, 50, 100, 250, 500, 1000, 2500, 5000, 10000, -1};
        currentRateIndex = 2; // Start with 50 units/sec
        keyboardMovementRate = movementRates[currentRateIndex];
    }
    
    int getVisibleSliderIndex(int visiblePosition) const
    {
        if (isEightSliderMode)
        {
            // In 8-slider mode, show bank pairs: A+B (0-7) or C+D (8-15)
            int bankPair = currentBank >= 2 ? 1 : 0; // 0 for A+B, 1 for C+D
            return (bankPair * 8) + visiblePosition;
        }
        else
        {
            // In 4-slider mode, show single bank
            return (currentBank * 4) + visiblePosition;
        }
    }
    
    void updateSliderVisibility()
    {
        // Hide all sliders first
        for (auto* slider : sliderControls)
            slider->setVisible(false);
        
        // Show appropriate sliders based on mode
        int visibleCount = isEightSliderMode ? 8 : 4;
        for (int i = 0; i < visibleCount; ++i)
        {
            int sliderIndex = getVisibleSliderIndex(i);
            if (sliderIndex < sliderControls.size())
                sliderControls[sliderIndex]->setVisible(true);
        }
    }
    
    void updateBankButtonStates()
    {
        // Reset all buttons to dark grey first
        bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        bankCButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        bankDButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        
        if (isEightSliderMode)
        {
            // In 8-slider mode, light up both banks in the pair
            if (currentBank <= 1) // A+B pair
            {
                bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
                bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
            }
            else // C+D pair
            {
                bankCButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
                bankDButton.setColour(juce::TextButton::buttonColourId, juce::Colours::yellow);
            }
        }
        else
        {
            // In 4-slider mode, light up only the active bank
            switch (currentBank)
            {
                case 0: bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red); break;
                case 1: bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue); break;
                case 2: bankCButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green); break;
                case 3: bankDButton.setColour(juce::TextButton::buttonColourId, juce::Colours::yellow); break;
            }
        }
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
    
    void updateWindowSizeDisplay()
    {
        int eightSliderThreshold = (8 * SLIDER_PLATE_WIDTH) + (7 * SLIDER_GAP) + 20; // +20 for margins
        juce::String sizeText = "Window: " + juce::String(getWidth()) + "x" + juce::String(getHeight()) + 
                               " | Mode: " + (isEightSliderMode ? "8-slider" : "4-slider") + 
                               " | 8-Slider Threshold: " + juce::String(eightSliderThreshold);
        windowSizeLabel.setText(sizeText, juce::dontSendNotification);
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
        if (isEightSliderMode)
        {
            // In 8-slider mode, clicking a bank switches to its pair
            if (bank <= 1)
                currentBank = 0; // Clicking A or B shows A+B pair
            else
                currentBank = 2; // Clicking C or D shows C+D pair
        }
        else
        {
            // In 4-slider mode, show the individual bank
            currentBank = bank;
        }
        
        updateSliderVisibility();
        updateBankButtonStates();
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
    juce::TextButton bankAButton, bankBButton, bankCButton, bankDButton;
    SettingsWindow settingsWindow;
    juce::Label movementSpeedLabel;
    juce::Label windowSizeLabel;
    std::unique_ptr<juce::MidiOutput> midiOutput;
    int currentBank = 0;
    bool isEightSliderMode = false;
    static constexpr int SLIDER_PLATE_WIDTH = 175; // Fixed slider plate width
    static constexpr int SLIDER_GAP = 10; // Gap between sliders
    
    // Keyboard control members
    std::vector<KeyboardMapping> keyboardMappings;
    std::vector<int> movementRates;
    int currentRateIndex = 2;
    double keyboardMovementRate = 50.0; // MIDI units per second
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugMidiController)
};
