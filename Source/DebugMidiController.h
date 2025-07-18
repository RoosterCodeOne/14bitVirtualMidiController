// DebugMidiController.h - Production Version with Preset System
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "SimpleSliderControl.h"
#include "SettingsWindow.h"
#include "MidiLearnWindow.h"
#include "Core/MidiManager.h"
#include "Core/KeyboardController.h"
#include "Core/BankManager.h"
#include "Core/Midi7BitController.h"

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
                int midiChannel = settingsWindow.getMidiChannel();
                int ccNumber = settingsWindow.getCCNumber(sliderIndex);
                midiManager.sendCC14Bit(midiChannel, ccNumber, value);
                
                // Trigger MIDI activity indicator AFTER successful MIDI send
                if (sliderIndex < sliderControls.size())
                    sliderControls[sliderIndex]->triggerMidiActivity();
            });
            
            // Add click handler for learn mode
            sliderControl->onSliderClick = [this, i]() {
                DBG("Slider " << i << " clicked. isLearningMode=" << (int)midi7BitController.isInLearnMode());
                if (midi7BitController.isInLearnMode())
                {
                    DBG("Setting up learn for slider " << i);
                    // Clear markers from all sliders
                    for (auto* slider : sliderControls)
                        slider->setShowLearnMarkers(false);
                        
                    // Set target and show markers
                    midi7BitController.setLearnTarget(i);
                    sliderControls[i]->setShowLearnMarkers(true);
                    DBG("Set learnTargetSlider=" << i << ", showing markers");
                    
                    learnButton.setButtonText("Move Controller");
                    learnButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::warning);
                }
                else
                {
                    DBG("Slider clicked but not in learn mode");
                }
            };
            
            sliderControls.add(sliderControl);
            addAndMakeVisible(sliderControl);
        }
        
        // Bank buttons - blueprint style
        addAndMakeVisible(bankAButton);
        bankAButton.setButtonText("A");
        bankAButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::active);
        bankAButton.setColour(juce::TextButton::textColourOffId, BlueprintColors::textPrimary);
        bankAButton.onClick = [this]() { bankManager.setActiveBank(0); };
        
        addAndMakeVisible(bankBButton);
        bankBButton.setButtonText("B");
        bankBButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::inactive);
        bankBButton.setColour(juce::TextButton::textColourOffId, BlueprintColors::textPrimary);
        bankBButton.onClick = [this]() { bankManager.setActiveBank(1); };
        
        addAndMakeVisible(bankCButton);
        bankCButton.setButtonText("C");
        bankCButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::inactive);
        bankCButton.setColour(juce::TextButton::textColourOffId, BlueprintColors::textPrimary);
        bankCButton.onClick = [this]() { bankManager.setActiveBank(2); };
        
        addAndMakeVisible(bankDButton);
        bankDButton.setButtonText("D");
        bankDButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::inactive);
        bankDButton.setColour(juce::TextButton::textColourOffId, BlueprintColors::textPrimary);
        bankDButton.onClick = [this]() { bankManager.setActiveBank(3); };
        
        // Settings button - blueprint style
        addAndMakeVisible(settingsButton);
        settingsButton.setButtonText("Settings");
        settingsButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::panel);
        settingsButton.setColour(juce::TextButton::textColourOffId, BlueprintColors::textPrimary);
        settingsButton.onClick = [this]() {
            toggleSettingsMode();
        };
        
        // Mode toggle button - blueprint style
        addAndMakeVisible(modeButton);
        modeButton.setButtonText(bankManager.isEightSliderMode() ? "8" : "4");
        modeButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::panel);
        modeButton.setColour(juce::TextButton::textColourOffId, BlueprintColors::textPrimary);
        modeButton.onClick = [this]() {
            toggleSliderMode();
        };
        
        // Showing label for mode button - blueprint style
        addAndMakeVisible(showingLabel);
        showingLabel.setText("Showing:", juce::dontSendNotification);
        showingLabel.setJustificationType(juce::Justification::centredRight);
        showingLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        showingLabel.setFont(juce::FontOptions(11.0f));
        
        // Learn button for MIDI mapping - blueprint style
        addAndMakeVisible(learnButton);
        learnButton.setButtonText("Learn");
        learnButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::panel);
        learnButton.setColour(juce::TextButton::textColourOffId, BlueprintColors::textPrimary);
        learnButton.onClick = [this]() {
            toggleLearnMode();
        };
        
        // Settings window
        addChildComponent(settingsWindow);
        settingsWindow.onSettingsChanged = [this]() { 
            updateSliderSettings(); 
        };
        settingsWindow.onPresetLoaded = [this](const ControllerPreset& preset) {
            applyPresetToSliders(preset);
        };
        
        // MIDI Learn window
        addChildComponent(midiLearnWindow);
        midiLearnWindow.onMappingAdded = [this](int sliderIndex, int midiChannel, int ccNumber) {
            // The mapping is already handled by Midi7BitController
            // Update tooltip to show current mapping
            updateMidiTooltip(sliderIndex, midiChannel, ccNumber, -1);
        };
        midiLearnWindow.onMappingCleared = [this](int sliderIndex) {
            midi7BitController.clearMapping(sliderIndex);
        };
        midiLearnWindow.onAllMappingsCleared = [this]() {
            midi7BitController.clearAllMappings();
        };
        
        // MIDI device selection callbacks will be set up in setupMidiManager()
        
        // Movement speed tooltip - blueprint style
        addAndMakeVisible(movementSpeedLabel);
        movementSpeedLabel.setJustificationType(juce::Justification::centredLeft);
        movementSpeedLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        movementSpeedLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        movementSpeedLabel.setFont(juce::FontOptions(10.0f));
        
        // MIDI tracking tooltip - blueprint style
        addAndMakeVisible(windowSizeLabel);
        windowSizeLabel.setJustificationType(juce::Justification::centredRight);
        windowSizeLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        windowSizeLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        windowSizeLabel.setFont(juce::FontOptions(10.0f));
        updateMidiTrackingDisplay();
        
        // Initialize MIDI Manager and set up callbacks
        setupMidiManager();
        
        // Initialize bank manager and set up callbacks
        setupBankManager();
        
        // Set initial bank
        bankManager.setActiveBank(0);
        
        // Load auto-saved state
        loadAutoSavedState();
        
        // Apply initial settings
        updateSliderSettings();
        
        // Note: Using simple channel-based MIDI filtering
        
        // Initialize keyboard control system
        setWantsKeyboardFocus(true);
        setupKeyboardController();
        
        // Initialize 7-bit MIDI controller
        setupMidi7BitController();
        
        // Apply initial window constraints to prevent manual resizing
        updateWindowConstraints();
        
    }
    
    ~DebugMidiController()
    {
        // CRITICAL: Stop all timers before destruction
        stopTimer();
        midi7BitController.stopTimer();
        
        // Auto-save current state before destruction
        saveCurrentState();
        
        // MIDI cleanup is handled by MidiManager destructor
    }
    
    void paint(juce::Graphics& g) override
    {
        // Blueprint background - dark navy base
        g.fillAll(BlueprintColors::background);
        
        // Calculate content area bounds using same logic as resized()
        const int topAreaHeight = 50;
        const int tooltipHeight = 25;
        const int verticalGap = 10;
        const int contentAreaWidth = bankManager.isEightSliderMode() ? 970 : 490;
        
        auto area = getLocalBounds();
        int contentX = (isInSettingsMode || isInLearnMode) ? SETTINGS_PANEL_WIDTH : (area.getWidth() - contentAreaWidth) / 2;
        int contentY = topAreaHeight + verticalGap;
        int contentHeight = area.getHeight() - topAreaHeight - tooltipHeight - (2 * verticalGap) + 8; // Allow 8px overlap into tooltip area
        
        juce::Rectangle<int> contentAreaBounds(contentX, contentY, contentAreaWidth, contentHeight);
        
        // Draw blueprint grid overlay
        CustomSliderLookAndFeel lookAndFeelGrid;
        lookAndFeelGrid.drawBlueprintGrid(g, contentAreaBounds);
        
        
        // Draw settings panel separator if needed
        if ((isInSettingsMode && settingsWindow.isVisible()) || (isInLearnMode && midiLearnWindow.isVisible()))
        {
            drawSettingsPanelSeparator(g, contentAreaBounds);
        }
        
        CustomSliderLookAndFeel lookAndFeel;
        
        // Draw plates and visual tracks for each visible slider
        int visibleSliderCount = bankManager.getVisibleSliderCount();
        for (int i = 0; i < visibleSliderCount; ++i)
        {
            int sliderIndex = bankManager.getVisibleSliderIndex(i);
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
                
                // Draw the track with current slider value for progressive fill
                lookAndFeel.drawSliderTrack(g, trackBounds.toFloat(), sliderControl->getSliderColor(), 
                                          sliderControl->getValue(), 0.0, 16383.0);
                
                // Draw tick marks
                lookAndFeel.drawTickMarks(g, trackBounds.toFloat());
                
                // Get thumb position relative to this component
                auto thumbPos = sliderControl->getThumbPosition();
                thumbPos.x += sliderBounds.getX();
                thumbPos.y += sliderBounds.getY();
                
                // Draw the thumb
                lookAndFeel.drawSliderThumb(g, thumbPos.x, thumbPos.y, sliderControl->getSliderColor());
            }
        }
        
        // Calculate top area bounds for text positioning
        juce::Rectangle<int> topAreaBounds;
        if ((isInSettingsMode && settingsWindow.isVisible()) || (isInLearnMode && midiLearnWindow.isVisible()))
        {
            topAreaBounds = juce::Rectangle<int>(0, 0, getWidth(), topAreaHeight);
        }
        else
        {
            topAreaBounds = juce::Rectangle<int>(contentX, 0, contentAreaWidth, topAreaHeight);
        }
        
        
        // Draw MIDI input indicator next to Learn button - blueprint style
        int learnButtonX = topAreaBounds.getX() + 10 + 105;
        midiInputIndicatorBounds = juce::Rectangle<float>(learnButtonX + 55, 28, 12, 12);
        
        juce::Colour inputIndicatorColor = BlueprintColors::warning;
        float inputAlpha = midiManager.getMidiInputActivity() ? 1.0f : 0.2f;
        g.setColour(inputIndicatorColor.withAlpha(inputAlpha));
        g.fillRect(midiInputIndicatorBounds);
        g.setColour(BlueprintColors::blueprintLines);
        g.drawRect(midiInputIndicatorBounds, 1.0f);
        
        // Show MIDI status with blueprint styling
        g.setColour(BlueprintColors::textPrimary);
        g.setFont(juce::FontOptions(12.0f));
        juce::String status = "MIDI: ";
        if (midiManager.isOutputConnected() && midiManager.isInputConnected())
            status += "IN/OUT Connected";
        else if (midiManager.isOutputConnected())
            status += "OUT Connected";
        else if (midiManager.isInputConnected())
            status += "IN Connected";
        else
            status += "Disconnected";
        g.drawText(status, topAreaBounds.getX() + 10, 5, 200, 20, juce::Justification::left);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        
        // Use current slider mode (controlled by mode button)
        int visibleSliderCount = bankManager.getVisibleSliderCount();
        
        // Update MIDI tracking display
        updateMidiTrackingDisplay();
        
        // Calculate layout dimensions once
        const int topAreaHeight = 50;
        const int tooltipHeight = 25;
        const int verticalGap = 10;
        const int contentAreaWidth = bankManager.isEightSliderMode() ? 970 : 490;
        
        // Calculate content area bounds - simplified logic
        int contentX = (isInSettingsMode || isInLearnMode) ? SETTINGS_PANEL_WIDTH : (area.getWidth() - contentAreaWidth) / 2;
        int contentY = topAreaHeight + verticalGap;
        int contentHeight = area.getHeight() - topAreaHeight - tooltipHeight - (2 * verticalGap) + 8; // Allow 8px overlap into tooltip area
        
        juce::Rectangle<int> contentAreaBounds(contentX, contentY, contentAreaWidth, contentHeight);
        
        // Calculate top area bounds based on settings or learn state
        juce::Rectangle<int> topAreaBounds;
        if ((isInSettingsMode && settingsWindow.isVisible()) || (isInLearnMode && midiLearnWindow.isVisible()))
        {
            topAreaBounds = juce::Rectangle<int>(0, 0, getWidth(), topAreaHeight);
        }
        else
        {
            topAreaBounds = juce::Rectangle<int>(contentX, 0, contentAreaWidth, topAreaHeight);
        }
        
        // Position top area components
        layoutTopAreaComponents(topAreaBounds);
        
        // Position the active window (Settings OR Learn, never both)
        if (isInSettingsMode && settingsWindow.isVisible())
        {
            int windowY = topAreaHeight;
            int windowHeight = area.getHeight() - windowY;
            settingsWindow.setBounds(0, windowY, SETTINGS_PANEL_WIDTH, windowHeight);
        }
        else if (isInLearnMode && midiLearnWindow.isVisible())
        {
            int windowY = topAreaHeight;
            int windowHeight = area.getHeight() - windowY;
            midiLearnWindow.setBounds(0, windowY, SETTINGS_PANEL_WIDTH, windowHeight);
        }
        
        // Layout sliders within content area
        layoutSlidersFixed(contentAreaBounds, visibleSliderCount);
        
        // Position tooltips at bottom
        layoutTooltips(area, contentAreaWidth, tooltipHeight);
    }
    
    bool keyPressed(const juce::KeyPress& key) override
    {
        return keyboardController.handleKeyPressed(key);
    }
    
    bool keyStateChanged(bool isKeyDown) override
    {
        return keyboardController.handleKeyStateChanged(isKeyDown);
    }
    
    void timerCallback() override
    {
        double currentTime = juce::Time::getMillisecondCounterHiRes();
        
        // Handle MIDI input activity timeout
        if (midiManager.getMidiInputActivity() && (currentTime - midiManager.getLastMidiInputTime()) > MIDI_INPUT_ACTIVITY_DURATION)
        {
            midiManager.resetMidiInputActivity();
            repaint();
        }
    }
    
    
    void layoutSlidersFixed(juce::Rectangle<int> area, int visibleSliderCount)
    {
        // Calculate total width needed for the slider rack
        int totalSliderWidth = (visibleSliderCount * SLIDER_PLATE_WIDTH) + ((visibleSliderCount - 1) * SLIDER_GAP);
        
        // Center sliders within the provided content area
        int startX = area.getX() + (area.getWidth() - totalSliderWidth) / 2;

        
        // Set bounds for each visible slider - complete recalculation
        for (int i = 0; i < visibleSliderCount; ++i)
        {
            int sliderIndex = bankManager.getVisibleSliderIndex(i);
            if (sliderIndex < sliderControls.size())
            {
                int xPos = startX + (i * (SLIDER_PLATE_WIDTH + SLIDER_GAP));
                auto sliderBounds = juce::Rectangle<int>(xPos, area.getY(), SLIDER_PLATE_WIDTH, area.getHeight());
                sliderControls[sliderIndex]->setBounds(sliderBounds);
                
                // Force slider to repaint with fresh bounds
                sliderControls[sliderIndex]->repaint();
            }
        }
    }
    
    
    
    void layoutTopAreaComponents(const juce::Rectangle<int>& topAreaBounds)
    {
        // Settings button - positioned on left within top area
        int settingsButtonX = topAreaBounds.getX() + 10;
        settingsButton.setBounds(settingsButtonX, 25, 100, 20);
        
        // Learn button - positioned closer to settings button
        int learnButtonX = settingsButtonX + 105; // 100px settings button + 5px gap (reduced from 10px)
        learnButton.setBounds(learnButtonX, 25, 50, 20);
        
        // Bank buttons - positioned as 2x2 grid in top right of top area
        const int buttonWidth = 35;
        const int buttonHeight = 20;
        const int buttonSpacing = 5;
        const int rightMargin = 10;
        
        int gridWidth = (2 * buttonWidth) + buttonSpacing;
        int gridStartX = topAreaBounds.getRight() - rightMargin - gridWidth;
        int gridStartY = 5;
        
        // Top row: A and B buttons
        bankAButton.setBounds(gridStartX, gridStartY, buttonWidth, buttonHeight);
        bankBButton.setBounds(gridStartX + buttonWidth + buttonSpacing, gridStartY, buttonWidth, buttonHeight);
        
        // Bottom row: C and D buttons
        bankCButton.setBounds(gridStartX, gridStartY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
        bankDButton.setBounds(gridStartX + buttonWidth + buttonSpacing, gridStartY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
        
        // Mode button - positioned to the left of C bank button with "Showing:" label
        int showingLabelX = gridStartX - 85; // Position for "Showing:" label
        int modeButtonX = gridStartX - 40; // Position for mode button (left of C button)
        showingLabel.setBounds(showingLabelX, gridStartY + buttonHeight + buttonSpacing, 40, 20);
        modeButton.setBounds(modeButtonX, gridStartY + buttonHeight + buttonSpacing, 30, 20);
    }
    
    void layoutTooltips(const juce::Rectangle<int>& area, int contentAreaWidth, int tooltipHeight)
    {
        auto tooltipArea = area.withHeight(tooltipHeight).withBottomY(area.getBottom());
        
        if ((isInSettingsMode && settingsWindow.isVisible()) || (isInLearnMode && midiLearnWindow.isVisible()))
        {
            // Settings or learn open: tooltips span remaining width after panel
            auto adjustedTooltipArea = tooltipArea.withTrimmedLeft(SETTINGS_PANEL_WIDTH);
            auto leftTooltip = adjustedTooltipArea.removeFromLeft(adjustedTooltipArea.getWidth() / 2);
            movementSpeedLabel.setBounds(leftTooltip);
            windowSizeLabel.setBounds(adjustedTooltipArea);
        }
        else
        {
            // Settings closed: tooltips positioned within content area bounds
            int contentAreaX = (area.getWidth() - contentAreaWidth) / 2;
            auto contentTooltipArea = tooltipArea.withX(contentAreaX).withWidth(contentAreaWidth);
            auto leftTooltip = contentTooltipArea.removeFromLeft(contentTooltipArea.getWidth() / 2);
            movementSpeedLabel.setBounds(leftTooltip);
            windowSizeLabel.setBounds(contentTooltipArea);
        }
    }
    
    void drawSettingsPanelSeparator(juce::Graphics& g, juce::Rectangle<int> contentAreaBounds)
    {
        // Blueprint-style technical divider line
        const int dividerX = SETTINGS_PANEL_WIDTH;
        g.setColour(BlueprintColors::blueprintLines);
        g.drawLine(dividerX, 50, dividerX, getHeight() - 25, 2.0f);
        
        // No shadow - clean technical appearance
    }
    
    void setupMidiManager()
    {
        // Initialize MIDI devices
        midiManager.initializeDevices();
        
        // Set up MIDI device selection callbacks
        midiLearnWindow.onMidiDeviceSelected = [this](const juce::String& deviceName) {
            midiManager.selectInputDevice(deviceName);
        };
        
        midiLearnWindow.onMidiDevicesRefreshed = [this]() {
            // Restore previously selected device after refresh
            if (!midiManager.getSelectedDeviceName().isEmpty())
            {
                midiLearnWindow.setSelectedDevice(midiManager.getSelectedDeviceName());
                midiLearnWindow.setConnectionStatus(midiManager.getSelectedDeviceName(), midiManager.isInputConnected());
            }
        };
        
        // Set up MIDI input callback
        midiManager.onMidiReceived = [this](int channel, int ccNumber, int ccValue) {
            int ourOutputChannel = settingsWindow.getMidiChannel();
            
            // DEBUG: Log all incoming MIDI messages
            DBG("MIDI IN: Ch=" << channel << " CC=" << ccNumber << " Val=" << ccValue 
                << " (ourCh=" << ourOutputChannel << " learn=" << (int)midi7BitController.isInLearnMode() 
                << " target=" << midi7BitController.getLearnTarget() << ")");
            
            // === MIDI FEEDBACK PREVENTION ===
            // The app outputs MIDI on 'ourOutputChannel', so we must prevent processing
            // incoming MIDI on the same channel to avoid feedback loops where:
            // Slider A sends CC -> MIDI loopback -> Slider B receives same CC -> moves Slider B
            
            // 1. Activity Indicator: Only flash for external channels (not our output channel)
            if (channel != ourOutputChannel)
            {
                if (!isTimerRunning())
                    startTimer(16);
                repaint();
                
                // 2. External Channel Processing: Process MIDI from external channels normally
                midi7BitController.processIncomingCC(ccNumber, ccValue, channel);
            }
            
            // 3. Our Channel: Block all normal slider control to prevent feedback loops
        };
        
        // Set up connection status callback
        midiManager.onConnectionStatusChanged = [this](const juce::String& deviceName, bool connected) {
            midiLearnWindow.setConnectionStatus(deviceName, connected);
        };
        
        // Set up preset directory callback
        midiManager.getPresetDirectory = [this]() -> juce::File {
            return settingsWindow.getPresetManager().getPresetDirectory();
        };
        
        // Load saved MIDI device preference
        midiManager.loadDevicePreference();
        
        // Update UI with current device selection
        if (!midiManager.getSelectedDeviceName().isEmpty())
        {
            midiLearnWindow.setSelectedDevice(midiManager.getSelectedDeviceName());
            midiLearnWindow.setConnectionStatus(midiManager.getSelectedDeviceName(), midiManager.isInputConnected());
        }
    }
    
    void setupBankManager()
    {
        // Set up bank manager callbacks
        bankManager.onBankChanged = [this]() {
            updateSliderVisibility();
            keyboardController.setCurrentBank(bankManager.getActiveBank());
            resized(); // Re-layout
        };
        
        bankManager.onModeChanged = [this]() {
            updateSliderVisibility();
            keyboardController.setSliderMode(bankManager.isEightSliderMode());
            
            // Update window size for mode change
            int contentWidth = bankManager.isEightSliderMode() ? 970 : 490;
            int settingsWidth = (isInSettingsMode || isInLearnMode) ? 350 : 0;
            int targetWidth = contentWidth + settingsWidth;
            
            updateWindowConstraints();
            
            if (auto* topLevel = getTopLevelComponent())
            {
                topLevel->setSize(targetWidth, topLevel->getHeight());
            }
            
            resized();
        };
        
        bankManager.onBankColorsChanged = [this](const BankManager::BankColors& colors) {
            updateBankButtonStates();
        };
    }
    
    void setupKeyboardController()
    {
        // Initialize the keyboard controller
        keyboardController.initialize();
        
        // Set initial configuration
        keyboardController.setSliderMode(bankManager.isEightSliderMode());
        keyboardController.setCurrentBank(bankManager.getActiveBank());
        
        // Set up callbacks
        keyboardController.onSliderValueChanged = [this](int sliderIndex, double newValue) {
            if (sliderIndex < sliderControls.size())
            {
                auto* slider = sliderControls[sliderIndex];
                if (slider)
                {
                    slider->setValueFromKeyboard(newValue);
                }
            }
        };
        
        keyboardController.onSpeedDisplayChanged = [this](const juce::String& speedText) {
            movementSpeedLabel.setText(speedText, juce::dontSendNotification);
        };
        
        keyboardController.isSliderLocked = [this](int sliderIndex) -> bool {
            if (sliderIndex < sliderControls.size())
            {
                auto* slider = sliderControls[sliderIndex];
                return slider ? slider->isLocked() : false;
            }
            return false;
        };
        
        keyboardController.getSliderValue = [this](int sliderIndex) -> double {
            if (sliderIndex < sliderControls.size())
            {
                auto* slider = sliderControls[sliderIndex];
                return slider ? slider->getValue() : 0.0;
            }
            return 0.0;
        };
        
        keyboardController.getVisibleSliderIndex = [this](int keyboardPosition) -> int {
            return bankManager.getVisibleSliderIndex(keyboardPosition);
        };
    }
    
    void setupMidi7BitController()
    {
        // Set up slider value update callback
        midi7BitController.onSliderValueChanged = [this](int sliderIndex, double newValue) {
            if (sliderIndex < sliderControls.size())
            {
                auto* slider = sliderControls[sliderIndex];
                if (slider)
                {
                    slider->setValueFromKeyboard(newValue);
                }
            }
        };
        
        // Set up mapping learned callback
        midi7BitController.onMappingLearned = [this](int sliderIndex, int ccNumber, int channel) {
            // Clear markers
            if (sliderIndex < sliderControls.size())
            {
                sliderControls[sliderIndex]->setShowLearnMarkers(false);
                DBG("Cleared learn markers for slider " << sliderIndex);
            }
            
            // Add mapping to learn window
            midiLearnWindow.addMapping(sliderIndex, channel, ccNumber);
            DBG("Called midiLearnWindow.addMapping(" << sliderIndex << ", " << channel << ", " << ccNumber << ")");
            
            // Reset learn button state
            learnButton.setButtonText("Select Slider");
            learnButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::active);
            DBG("Reset learn state");
            
            // Show success feedback
            juce::String message = "Mapped CC " + juce::String(ccNumber) + " (Ch " + juce::String(channel) + ") to Slider " + juce::String(sliderIndex + 1);
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "MIDI Learn", message);
            DBG("Showing success message: " << message);
        };
        
        // Set up MIDI tooltip update callback
        midi7BitController.onMidiTooltipUpdate = [this](int sliderIndex, int channel, int ccNumber, int ccValue) {
            updateMidiTooltip(sliderIndex, channel, ccNumber, ccValue);
        };
        
        // Set up slider activity trigger callback
        midi7BitController.onSliderActivityTrigger = [this](int sliderIndex) {
            if (sliderIndex < sliderControls.size())
                sliderControls[sliderIndex]->triggerMidiActivity();
        };
        
        // Set up slider locked check callback
        midi7BitController.isSliderLocked = [this](int sliderIndex) -> bool {
            if (sliderIndex < sliderControls.size())
            {
                auto* slider = sliderControls[sliderIndex];
                return slider ? slider->isLocked() : false;
            }
            return false;
        };
        
        // Set up get slider value callback
        midi7BitController.getSliderValue = [this](int sliderIndex) -> double {
            if (sliderIndex < sliderControls.size())
            {
                auto* slider = sliderControls[sliderIndex];
                return slider ? slider->getValue() : 0.0;
            }
            return 0.0;
        };
    }
    
private:
    
    
    void updateSliderVisibility()
    {
        // Hide all sliders first
        for (auto* slider : sliderControls)
            slider->setVisible(false);
        
        // Show appropriate sliders based on mode
        int visibleCount = bankManager.getVisibleSliderCount();
        for (int i = 0; i < visibleCount; ++i)
        {
            int sliderIndex = bankManager.getVisibleSliderIndex(i);
            if (sliderIndex < sliderControls.size())
                sliderControls[sliderIndex]->setVisible(true);
        }
    }
    
    void updateBankButtonStates()
    {
        int activeBank = bankManager.getActiveBank();
        
        // Update colors based on blueprint style
        bankAButton.setColour(juce::TextButton::buttonColourId, activeBank == 0 ? BlueprintColors::active : BlueprintColors::inactive);
        bankBButton.setColour(juce::TextButton::buttonColourId, activeBank == 1 ? BlueprintColors::active : BlueprintColors::inactive);
        bankCButton.setColour(juce::TextButton::buttonColourId, activeBank == 2 ? BlueprintColors::active : BlueprintColors::inactive);
        bankDButton.setColour(juce::TextButton::buttonColourId, activeBank == 3 ? BlueprintColors::active : BlueprintColors::inactive);
    }
    
    
    void updateMidiTrackingDisplay()
    {
        juce::String midiText;
        if (lastMidiSliderIndex >= 0 && lastMidiCC >= 0)
        {
            midiText = "Slider " + juce::String(lastMidiSliderIndex + 1) + 
                      ": Channel " + juce::String(lastMidiChannel) +
                      ", CC " + juce::String(lastMidiCC);
            if (lastMidiValue >= 0)
                midiText += ", Value " + juce::String(lastMidiValue);
        }
        else
        {
            midiText = "No MIDI input detected";
        }
        
        windowSizeLabel.setText(midiText, juce::dontSendNotification);
    }
    
    void updateMidiTooltip(int sliderIndex, int channel, int cc, int value)
    {
        lastMidiSliderIndex = sliderIndex;
        lastMidiChannel = channel;  
        lastMidiCC = cc;
        lastMidiValue = value;
        updateMidiTrackingDisplay();
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
        
        // Note: Using simple channel-based MIDI filtering
        
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
                    
                // Apply delay and attack times
                sliderControls[i]->setDelayTime(sliderPreset.delayTime);
                sliderControls[i]->setAttackTime(sliderPreset.attackTime);
                sliderControls[i]->setReturnTime(sliderPreset.returnTime);
                sliderControls[i]->setCurveValue(sliderPreset.curveValue);
            }
        }
        
        // Update settings to reflect the new configuration
        updateSliderSettings();
        
        // Note: Using simple channel-based MIDI filtering
    }
    
    void toggleSettingsMode()
    {
        // Close learn mode if it's open
        if (isInLearnMode)
        {
            isInLearnMode = false;
            midi7BitController.stopLearnMode();
            learnButton.setButtonText("Learn");
            learnButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::panel);
            midiLearnWindow.setVisible(false);
            
            // Clear any learn markers
            for (auto* slider : sliderControls)
                slider->setShowLearnMarkers(false);
        }
        
        isInSettingsMode = !isInSettingsMode;
        
        // Update button appearance
        settingsButton.setColour(juce::TextButton::buttonColourId, 
                               isInSettingsMode ? BlueprintColors::active : BlueprintColors::panel);
        
        // Update constraints BEFORE resizing to prevent constraint violations
        updateWindowConstraints();
        
        if (auto* topLevel = getTopLevelComponent())
        {
            // Calculate target window width: content area + settings panel (if open)
            int contentAreaWidth = bankManager.isEightSliderMode() ? 970 : 490;
            int targetWidth = isInSettingsMode ? (contentAreaWidth + SETTINGS_PANEL_WIDTH) : contentAreaWidth;
            
            // Resize window instantly
            topLevel->setSize(targetWidth, topLevel->getHeight());
            
            if (isInSettingsMode)
            {
                // Show settings window
                addAndMakeVisible(settingsWindow);
                settingsWindow.toFront(true);
            }
            else
            {
                // Hide settings window
                settingsWindow.setVisible(false);
            }
        }
        
        resized(); // Re-layout components
    }
    
    void toggleSliderMode()
    {
        // Toggle mode state through bank manager
        bankManager.setSliderMode(!bankManager.isEightSliderMode());
        
        // Update button text
        modeButton.setButtonText(bankManager.isEightSliderMode() ? "8" : "4");
    }
    
    
    void updateWindowConstraints()
    {
        if (auto* topLevel = getTopLevelComponent())
        {
            if (auto* documentWindow = dynamic_cast<juce::DocumentWindow*>(topLevel))
            {
                if (auto* constrainer = documentWindow->getConstrainer())
                {
                    // Fixed window widths based on mode
                    int fixedWidth = bankManager.isEightSliderMode() ? 970 : 490;
                    
                    if (isInSettingsMode || isInLearnMode)
                    {
                        // Add panel width to fixed width (same width for both settings and learn)
                        fixedWidth += SETTINGS_PANEL_WIDTH;
                    }
                    
                    // Set both min and max to the same value to prevent resizing
                    constrainer->setMinimumWidth(fixedWidth);
                    constrainer->setMaximumWidth(fixedWidth);
                }
            }
        }
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
        
        // Note: Using simple channel-based MIDI filtering
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
                preset.sliders.getReference(i).delayTime = sliderControls[i]->getDelayTime();
                preset.sliders.getReference(i).attackTime = sliderControls[i]->getAttackTime();
                preset.sliders.getReference(i).returnTime = sliderControls[i]->getReturnTime();
                preset.sliders.getReference(i).curveValue = sliderControls[i]->getCurveValue();
            }
        }
        
        return preset;
    }
    
    
    
    
    void processMidiMSB(int sliderIndex, int ccNumber, int msbValue)
    {
        // NOTE: This method processes 14-bit MIDI MSB values from external sources only
        // It should only be called from process7BitControl() which already has feedback prevention
        // Store MSB value and timestamp
        auto& state = midiCCStates[sliderIndex];
        state.msbValue = msbValue;
        state.msbTimestamp = juce::Time::getMillisecondCounterHiRes();
        state.hasMSB = true;
        
        // If we have a recent LSB value, combine them
        if (state.hasLSB && (state.msbTimestamp - state.lsbTimestamp) < MIDI_PAIR_TIMEOUT)
        {
            int combined14bit = (msbValue << 7) | state.lsbValue;
            updateSliderFromMidiInput(sliderIndex, combined14bit);
            
            // Clear the state after using
            state.hasMSB = false;
            state.hasLSB = false;
        }
        else
        {
            // Use just MSB value (7-bit to 14-bit conversion)
            int expanded14bit = msbValue << 7;
            updateSliderFromMidiInput(sliderIndex, expanded14bit);
        }
    }
    
    void processMidiLSB(int sliderIndex, int baseCCNumber, int lsbValue)
    {
        // NOTE: This method processes 14-bit MIDI LSB values from external sources only
        // It should only be called from process7BitControl() which already has feedback prevention
        // Only process LSB if the base CC is < 96 (supports 14-bit)
        if (baseCCNumber >= 96) return;
        
        auto& state = midiCCStates[sliderIndex];
        state.lsbValue = lsbValue;
        state.lsbTimestamp = juce::Time::getMillisecondCounterHiRes();
        state.hasLSB = true;
        
        // If we have a recent MSB value, combine them
        if (state.hasMSB && (state.lsbTimestamp - state.msbTimestamp) < MIDI_PAIR_TIMEOUT)
        {
            int combined14bit = (state.msbValue << 7) | lsbValue;
            updateSliderFromMidiInput(sliderIndex, combined14bit);
            
            // Clear the state after using
            state.hasMSB = false;
            state.hasLSB = false;
        }
        // If no recent MSB, we wait for the next MSB message
    }
    
    void updateSliderFromMidiInput(int sliderIndex, int value14bit)
    {
        // CRITICAL: This method moves sliders based on MIDI input
        // It should ONLY be called for external MIDI (never from our own output)
        // Feedback prevention is handled at handleIncomingMidiMessage() level
        if (sliderIndex < sliderControls.size())
        {
            // Clamp value to valid range
            int clampedValue = juce::jlimit(0, 16383, value14bit);
            
            // Thread-safe update to UI thread - check lock status on message thread
            juce::MessageManager::callAsync([this, sliderIndex, clampedValue]() {
                if (sliderIndex < sliderControls.size())
                {
                    auto* slider = sliderControls[sliderIndex];
                    
                    // Check if slider is locked (now on message thread)
                    if (slider->isLocked())
                        return;
                        
                    slider->setValueFromMIDI(clampedValue);
                }
            });
        }
    }
    
    
    
    
    void toggleLearnMode()
    {
        // Close settings mode if it's open
        if (isInSettingsMode)
        {
            isInSettingsMode = false;
            settingsButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
            settingsWindow.setVisible(false);
        }
        
        isInLearnMode = !isInLearnMode;
        
        if (isInLearnMode)
        {
            // Enter learn mode - show MIDI learn window
            midi7BitController.startLearnMode();
            DBG("Entered learn mode: isLearningMode=" << (int)midi7BitController.isInLearnMode());
            learnButton.setButtonText("Exit Learn");
            learnButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::active);
            
            // Show learn window
            updateWindowConstraints();
            
            if (auto* topLevel = getTopLevelComponent())
            {
                int contentAreaWidth = bankManager.isEightSliderMode() ? 970 : 490;
                int targetWidth = contentAreaWidth + SETTINGS_PANEL_WIDTH;
                topLevel->setSize(targetWidth, topLevel->getHeight());
                
                addAndMakeVisible(midiLearnWindow);
                midiLearnWindow.toFront(true);
            }
        }
        else
        {
            // Exit learn mode
            midi7BitController.stopLearnMode();
            DBG("Exited learn mode: isLearningMode=" << (int)midi7BitController.isInLearnMode());
            learnButton.setButtonText("Learn");
            learnButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::panel);
            
            midiLearnWindow.setVisible(false);
            
            // Clear any learn markers
            for (auto* slider : sliderControls)
                slider->setShowLearnMarkers(false);
            
            // Resize window back
            updateWindowConstraints();
            if (auto* topLevel = getTopLevelComponent())
            {
                int contentAreaWidth = bankManager.isEightSliderMode() ? 970 : 490;
                topLevel->setSize(contentAreaWidth, topLevel->getHeight());
            }
        }
        
        resized();
    }
    
    // Note: Complex CC-based filtering removed in favor of simple channel-based approach
    
    
    juce::OwnedArray<SimpleSliderControl> sliderControls;
    juce::TextButton settingsButton;
    juce::TextButton modeButton;
    juce::TextButton learnButton;
    juce::TextButton bankAButton, bankBButton, bankCButton, bankDButton;
    juce::Label showingLabel;
    SettingsWindow settingsWindow;
    MidiLearnWindow midiLearnWindow;
    juce::Label movementSpeedLabel;
    juce::Label windowSizeLabel;
    MidiManager midiManager;
    KeyboardController keyboardController;
    BankManager bankManager;
    Midi7BitController midi7BitController;
    bool isInSettingsMode = false;
    bool isInLearnMode = false;
    
    static constexpr int SLIDER_PLATE_WIDTH = 110; // Fixed slider plate width
    static constexpr int SLIDER_GAP = 10; // Gap between sliders
    static constexpr int SETTINGS_PANEL_WIDTH = 350; // Width of settings panel
    
    
    // MIDI Input handling
    struct MidiCCState
    {
        int msbValue = 0;
        int lsbValue = 0;
        double msbTimestamp = 0.0;
        double lsbTimestamp = 0.0;
        bool hasMSB = false;
        bool hasLSB = false;
    };
    
    std::array<MidiCCState, 16> midiCCStates; // State for each slider
    static constexpr double MIDI_PAIR_TIMEOUT = 50.0; // milliseconds
    
    
    // MIDI input activity indicator
    juce::Rectangle<float> midiInputIndicatorBounds;
    static constexpr double MIDI_INPUT_ACTIVITY_DURATION = 150.0; // milliseconds
    
    // MIDI input tracking for tooltip display
    int lastMidiSliderIndex = -1;
    int lastMidiChannel = -1;
    int lastMidiCC = -1;
    int lastMidiValue = -1;
    
    
    // Note: Simple channel-based MIDI filtering - no complex tracking needed
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugMidiController)
};
