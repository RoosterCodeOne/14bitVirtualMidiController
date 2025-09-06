// DebugMidiController.h - Production Version with Preset System
#pragma once
#include <JuceHeader.h>
#include <algorithm>
#include <vector>
#include "CustomLookAndFeel.h"
#include "SimpleSliderControl.h"
#include "SettingsWindow.h"
#include "MidiLearnWindow.h"
#include "MidiMonitorWindow.h"
#include "Core/MidiManager.h"
#include "Core/KeyboardController.h"
#include "Core/BankManager.h"
#include "Core/Midi7BitController.h"
#include "Core/AutomationConfigManager.h"
#include "UI/AutomationConfigManagementWindow.h"
#include "UI/MainControllerLayout.h"
#include "UI/WindowManager.h"
#include "UI/BankButtonManager.h"
#include "Components/BankButtonLearnOverlay.h"
#include "Components/BankButtonLearnZone.h"
#include "Components/LearnZoneTypes.h"
#include "UI/GlobalUIScale.h"

//====================================================================================
class DebugMidiController : public juce::Component, 
                            public juce::Timer, 
                            public GlobalUIScale::ScaleChangeListener
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
                
                // Always use 14-bit output (compatible with both 7-bit and 14-bit receivers)
                midiManager.sendCC14BitWithSlider(sliderIndex + 1, midiChannel, ccNumber, value);
                
                // Trigger MIDI activity indicator AFTER successful MIDI send
                if (sliderIndex < sliderControls.size())
                    sliderControls[sliderIndex]->triggerMidiActivity();
            });
            
            // Always use 14-bit mode
            sliderControl->is14BitMode = [this](int sliderIndex) -> bool {
                return true;
            };
            
            // Add click handler for learn mode
            sliderControl->onSliderClick = [this, i]() {
                DBG("Slider " << i << " clicked. isLearningMode=" << (int)midi7BitController.isInLearnMode() << ", isInSettingsMode=" << (int)isInSettingsMode);
                
                if (midi7BitController.isInLearnMode())
                {
                    DBG("Setting up learn for slider " << i);
                    // Clear markers from all sliders
                    for (auto* slider : sliderControls)
                        slider->setShowLearnMarkers(false);
                        
                    // Set target and show markers (defaulting to SliderValue for backward compatibility)
                    midi7BitController.setLearnTarget(MidiTargetType::SliderValue, i);
                    sliderControls[i]->setShowLearnMarkers(true);
                    DBG("Set learn target to SliderValue for slider " << i << ", showing markers");
                    
                    learnButton.setButtonText("Move Controller");
                }
                else if (isInSettingsMode)
                {
                    // Settings mode: select slider for editing
                    DBG("Settings mode: selecting slider " << i << " for editing");
                    setSelectedSliderForEditing(i);
                    
                    // Update settings window to show this slider's configuration
                    settingsWindow.selectSlider(i);
                }
                else
                {
                    DBG("Slider clicked but not in learn or settings mode");
                }
            };
            
            sliderControls.add(sliderControl);
            addAndMakeVisible(sliderControl);
            
            // Set up automation config manager
            sliderControl->setConfigManager(&automationConfigManager);
            
            // Set up config management window callback
            sliderControl->onOpenConfigManagement = [this](int sliderIndex, AutomationConfigManagementWindow::Mode mode) {
                openConfigManagementWindow(sliderIndex, mode);
            };
            
            // Set up new learn zone callback
            sliderControl->onLearnZoneClicked = [this](const LearnZone& zone) {
                handleLearnZoneClicked(zone);
            };
        }
        
        // Bank buttons - setup through BankButtonManager
        addAndMakeVisible(&bankAButton);
        addAndMakeVisible(&bankBButton);
        addAndMakeVisible(&bankCButton);
        addAndMakeVisible(&bankDButton);
        
        bankButtonManager.setupBankButtons(bankAButton, bankBButton, bankCButton, bankDButton,
                                          customButtonLookAndFeel, 
                                          [this](int bankIndex) { bankManager.setActiveBank(bankIndex); });
        
        // Settings button - blueprint style with custom look and feel
        addAndMakeVisible(settingsButton);
        settingsButton.setButtonText("Settings");
        settingsButton.setLookAndFeel(&customButtonLookAndFeel);
        settingsButton.onClick = [this]() {
            toggleSettingsMode();
        };
        
        // Mode toggle button - blueprint style with custom look and feel
        addAndMakeVisible(&modeButton);
        modeButton.setButtonText(bankManager.isEightSliderMode() ? "8" : "4");
        modeButton.setLookAndFeel(&customButtonLookAndFeel);
        modeButton.onClick = [this]() {
            toggleSliderMode();
        };
        
        // Showing label for mode button - blueprint style
        addAndMakeVisible(&showingLabel);
        showingLabel.setText("Showing:", juce::dontSendNotification);
        showingLabel.setJustificationType(juce::Justification::centredRight);
        showingLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        showingLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f).boldened());
        
        // Learn button for MIDI mapping - blueprint style
        addAndMakeVisible(&learnButton);
        learnButton.setButtonText("Learn");
        learnButton.setLookAndFeel(&customButtonLookAndFeel);
        learnButton.onClick = [this]() {
            toggleLearnMode();
        };
        
        // MIDI Monitor button - blueprint style
        addAndMakeVisible(&monitorButton);
        monitorButton.setButtonText("MIDI Monitor");
        monitorButton.setLookAndFeel(&customButtonLookAndFeel);
        monitorButton.onClick = [this]() {
            toggleMidiMonitor();
        };
        
        // Settings window
        addChildComponent(&settingsWindow);
        settingsWindow.onSettingsChanged = [this]() { 
            updateSliderSettings(); 
        };
        settingsWindow.onPresetLoaded = [this](const ControllerPreset& preset) {
            applyPresetToSliders(preset);
        };
        settingsWindow.onSelectedSliderChanged = [this](int sliderIndex) {
            // Update visual highlighting when slider selection changes in settings
            setSelectedSliderForEditing(sliderIndex);
        };
        settingsWindow.onBankSelectionChanged = [this](int bankIndex) {
            // Update main window bank selection when changed from settings window
            updatingFromSettingsWindow = true;
            bankManager.setActiveBank(bankIndex);
            updatingFromSettingsWindow = false;
        };
        
        // MIDI Learn window
        addChildComponent(&midiLearnWindow);
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
        
        // MIDI Monitor window
        midiMonitorWindow = std::make_unique<MidiMonitorWindow>();
        
        // Set up visibility change callback
        midiMonitorWindow->onVisibilityChanged = [this](bool isVisible) {
            updateMonitorButtonText(isVisible);
        };
        
        // Movement speed tooltip - blueprint style
        addAndMakeVisible(&movementSpeedLabel);
        movementSpeedLabel.setJustificationType(juce::Justification::centredLeft);
        movementSpeedLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        movementSpeedLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        movementSpeedLabel.setFont(GlobalUIScale::getInstance().getScaledFont(10.0f));
        
        // MIDI tracking tooltip - blueprint style
        addAndMakeVisible(&windowSizeLabel);
        windowSizeLabel.setJustificationType(juce::Justification::centredRight);
        windowSizeLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        windowSizeLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        windowSizeLabel.setFont(GlobalUIScale::getInstance().getScaledFont(10.0f));
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
        
        // Initialize automation config manager
        setupAutomationConfigManager();
        
        // Setup bank button learn overlays
        setupBankButtonLearnOverlays();
        
        // Apply initial window constraints to prevent manual resizing
        updateWindowConstraints();
        
        // Register for scale change notifications
        GlobalUIScale::getInstance().addScaleChangeListener(this);
        
    }
    
    ~DebugMidiController()
    {
        // CRITICAL: Stop all timers before destruction
        stopTimer();
        midi7BitController.stopTimer();
        
        // Remove scale change listener
        GlobalUIScale::getInstance().removeScaleChangeListener(this);
        
        // Auto-save current state before destruction
        saveCurrentState();
        
        // Save automation configs before destruction
        automationConfigManager.saveToFile();
        
        // Clean up bank buttons through manager
        bankButtonManager.cleanupBankButtons(bankAButton, bankBButton, bankCButton, bankDButton, customButtonLookAndFeel);
        
        settingsButton.setLookAndFeel(nullptr);
        learnButton.setLookAndFeel(nullptr);
        monitorButton.setLookAndFeel(nullptr);
        modeButton.setLookAndFeel(nullptr);
        
        // MIDI cleanup is handled by MidiManager destructor
    }
    
    // Visual selection highlighting for settings editing
    void setSelectedSliderForEditing(int sliderIndex)
    {
        if (sliderIndex < -1 || sliderIndex >= 16)
            return;
            
        selectedSliderForEditing = sliderIndex;
        
        // If a valid slider is selected, ensure it's visible by auto-switching banks
        if (selectedSliderForEditing >= 0)
        {
            // Check if selected slider is currently visible
            bool isVisible = bankManager.isSliderVisible(selectedSliderForEditing);
            
            if (!isVisible)
            {
                // Auto-switch to the appropriate bank
                int requiredBank = selectedSliderForEditing / 4; // 0-3 for banks A-D
                
                if (bankManager.isEightSliderMode())
                {
                    // In 8-slider mode: switch to appropriate bank pair
                    // Banks A+B (0-7) or C+D (8-15)
                    if (selectedSliderForEditing < 8)
                        bankManager.setActiveBank(0); // Show A+B pair
                    else
                        bankManager.setActiveBank(2); // Show C+D pair
                }
                else
                {
                    // In 4-slider mode: switch to the specific bank
                    bankManager.setActiveBank(requiredBank);
                }
                
                // Update slider visibility and bank button appearance
                updateSliderVisibility();
                
                // Update bank button states with current colors
                auto currentColors = bankManager.getCurrentBankColors();
                bankButtonManager.updateBankButtonStates(bankAButton, bankBButton, 
                                                        bankCButton, bankDButton,
                                                        customButtonLookAndFeel,
                                                        bankManager.getActiveBank(),
                                                        currentColors.bankA, currentColors.bankB,
                                                        currentColors.bankC, currentColors.bankD);
            }
        }
        
        // Trigger repaint to show/hide highlighting
        repaint();
    }
    
    // Get currently selected slider for editing (for testing/debugging)
    int getSelectedSliderForEditing() const { return selectedSliderForEditing; }
    
    
    void paint(juce::Graphics& g) override
    {
        auto& scale = GlobalUIScale::getInstance();
        
        // Blueprint background - dark navy base
        g.fillAll(BlueprintColors::background);
        
        // Calculate layout bounds using MainControllerLayout
        auto layoutBounds = mainLayout.calculateLayoutBounds(getLocalBounds(), 
                                                            bankManager.isEightSliderMode(),
                                                            isInSettingsMode, isInLearnMode);
        
        // Draw blueprint grid overlay
        mainLayout.drawBlueprintGrid(g, layoutBounds.contentArea);
        
        
        
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
                
                // Draw the track with current slider value for progressive fill - WITH ORIENTATION SUPPORT
                SliderOrientation orientation = sliderControl->getOrientation();
                double bipolarCenter = sliderControl->getCenterValue();
                
                // Check if in snap zone for visual feedback
                bool isInSnapZone = false;
                if (orientation == SliderOrientation::Bipolar)
                {
                    double currentDisplayValue = sliderControl->getDisplayValue();
                    isInSnapZone = sliderControl->isInSnapZone(currentDisplayValue);
                }
                
                lookAndFeel.drawSliderTrack(g, trackBounds.toFloat(), sliderControl->getSliderColor(), 
                                          sliderControl->getValue(), 0.0, 16383.0, orientation, bipolarCenter, isInSnapZone);
                
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
        
        // Draw selection highlighting for settings editing (only when settings window is open AND not in learn mode)
        if (isInSettingsMode && selectedSliderForEditing >= 0 && selectedSliderForEditing < sliderControls.size() && !isInLearnMode)
        {
            // Check if the selected slider is currently visible
            if (bankManager.isSliderVisible(selectedSliderForEditing))
            {
                auto* selectedSliderControl = sliderControls[selectedSliderForEditing];
                
                // Get FULL PLATE BOUNDS (entire slider component bounds)
                auto fullPlateBounds = selectedSliderControl->getBounds().toFloat();
                
                // Use the slider's current color for the highlight border
                juce::Colour highlightColor = selectedSliderControl->getSliderColor();
                
                // Draw thick border around ENTIRE slider plate (3px thick border)
                g.setColour(highlightColor);
                g.drawRect(fullPlateBounds, scale.getScaled(3.0f));
                
                // Add subtle inner glow effect
                g.setColour(highlightColor.withAlpha(0.3f));
                g.drawRect(fullPlateBounds.reduced(scale.getScaled(3.0f)), scale.getScaledLineThickness());
            }
        }
        
        // Use layout bounds for top area positioning  
        juce::Rectangle<int> topAreaBounds = layoutBounds.topArea.withY(scale.getScaled(2)).withHeight(layoutBounds.topArea.getHeight() - scale.getScaled(2));
        
        // Draw blueprint-style outline around top area
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        g.drawRect(topAreaBounds.toFloat(), scale.getScaledLineThickness());
        
        // Draw MIDI input indicator next to Learn button - blueprint style
        int learnButtonX = topAreaBounds.getX() + scale.getScaled(10) + scale.getScaled(105);
        int indicatorY = topAreaBounds.getY() + scale.getScaled(26); // Adjusted to be relative to top area bounds
        midiInputIndicatorBounds = juce::Rectangle<float>(learnButtonX + scale.getScaled(55), indicatorY, scale.getScaled(12), scale.getScaled(12));
        
        juce::Colour inputIndicatorColor = BlueprintColors::warning;
        float inputAlpha = midiManager.getMidiInputActivity() ? 1.0f : 0.2f;
        g.setColour(inputIndicatorColor.withAlpha(inputAlpha));
        g.fillRect(midiInputIndicatorBounds);
        g.setColour(BlueprintColors::blueprintLines);
        g.drawRect(midiInputIndicatorBounds, scale.getScaledLineThickness());
        
        // Show MIDI status with blueprint styling
        g.setColour(BlueprintColors::textPrimary);
        g.setFont(scale.getScaledFont(12.0f));
        juce::String status = "MIDI: ";
        if (midiManager.isOutputConnected() && midiManager.isInputConnected())
            status += "IN/OUT Connected";
        else if (midiManager.isOutputConnected())
            status += "OUT Connected";
        else if (midiManager.isInputConnected())
            status += "IN Connected";
        else
            status += "Disconnected";
        g.drawText(status, topAreaBounds.getX() + scale.getScaled(10), topAreaBounds.getY() + scale.getScaled(3), scale.getScaled(200), scale.getScaled(20), juce::Justification::left);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        int visibleSliderCount = bankManager.getVisibleSliderCount();
        
        // Update MIDI tracking display
        updateMidiTrackingDisplay();
        
        // Calculate layout bounds using MainControllerLayout
        auto layoutBounds = mainLayout.calculateLayoutBounds(area, 
                                                            bankManager.isEightSliderMode(),
                                                            isInSettingsMode, isInLearnMode);
        
        // Position top area components using MainControllerLayout
        mainLayout.layoutTopAreaComponents(layoutBounds.topArea, settingsButton, learnButton, monitorButton,
                                          bankAButton, bankBButton, bankCButton, bankDButton,
                                          modeButton, showingLabel);
        
        // Position the active window (Settings OR Learn, never both)
        if (isInSettingsMode && settingsWindow.isVisible())
        {
            windowManager.positionSideWindow(settingsWindow, area, MainControllerLayout::Constants::getTopAreaHeight(), 
                                            MainControllerLayout::Constants::getSettingsPanelWidth());
        }
        else if (isInLearnMode && midiLearnWindow.isVisible())
        {
            windowManager.positionSideWindow(midiLearnWindow, area, MainControllerLayout::Constants::getTopAreaHeight(),
                                            MainControllerLayout::Constants::getSettingsPanelWidth());
        }
        
        // Layout sliders using MainControllerLayout
        mainLayout.layoutSliders(sliderControls, layoutBounds.contentArea, visibleSliderCount,
                                [this](int i) { return bankManager.getVisibleSliderIndex(i); });
        
        // Position tooltips using MainControllerLayout
        mainLayout.layoutTooltips(area, movementSpeedLabel, windowSizeLabel,
                                 isInSettingsMode, isInLearnMode, bankManager.isEightSliderMode());
        
        // Update bank button learn overlay positions
        updateBankButtonLearnOverlays();
    }
    
    bool keyPressed(const juce::KeyPress& key) override
    {
        // Handle arrow key navigation for bank switching (only when settings window is not visible)
        if (!settingsWindow.isVisible())
        {
            // Only handle plain arrow keys (no modifiers) to avoid interfering with system shortcuts
            if (key == juce::KeyPress::upKey && !key.getModifiers().isAnyModifierKeyDown())
            {
                cycleBankUp();
                return true;
            }
            else if (key == juce::KeyPress::downKey && !key.getModifiers().isAnyModifierKeyDown())
            {
                cycleBankDown();
                return true;
            }
        }
        
        // Pass other keys to keyboard controller for normal Q/A, W/S controls
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
            juce::String debugMsg = "MIDI IN: Ch=" + juce::String(channel) + " CC=" + juce::String(ccNumber) + " Val=" + juce::String(ccValue) +
                                   " (ourCh=" + juce::String(ourOutputChannel) + " learn=" + juce::String((int)midi7BitController.isInLearnMode()) +
                                   " target=" + midi7BitController.getCurrentLearnTarget().getDisplayName() + ")";
            DBG(debugMsg);
            
            // === AUTOMATION CONFIG MIDI LEARN PAIRING ===
            // Check if we're waiting to pair a config with incoming MIDI
            if (!midiLearnConfigId.isEmpty() && isInLearnMode && channel != ourOutputChannel)
            {
                DBG("MIDI Learn Pairing: Config " + midiLearnConfigId + " paired with Ch" + juce::String(channel) + " CC" + juce::String(ccNumber));
                
                // Store the MIDI assignment for this config
                handleConfigMidiPairing(midiLearnConfigId, channel, ccNumber);
                
                // Clear the pairing state
                midiLearnConfigId = {};
                
                // Update the config management window
                if (configManagementWindow)
                {
                    configManagementWindow->setConfigReadyForMidiLearn(-1, false);
                    configManagementWindow->refreshMidiAssignments();
                }
                
                // If this was one-shot learn mode, turn off learn mode now
                if (isOneShotLearnMode)
                {
                    DBG("One-shot MIDI learn complete - turning off learn mode");
                    isInLearnMode = false;
                    isOneShotLearnMode = false;
                    midi7BitController.stopLearnMode();
                    setBankButtonLearnMode(false);
                    setAllSlidersLearnMode(false);
                    clearAllActiveLearnZones();
                    learnButton.setButtonText("Learn");
                    
                    if (configManagementWindow)
                    {
                        configManagementWindow->setLearnModeActive(false);
                    }
                    
                    // Clear any learn markers
                    for (auto* slider : sliderControls)
                        slider->setShowLearnMarkers(false);
                }
                
                return; // Don't process further for pairing
            }
            
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
        
        // Set up MIDI monitor callbacks
        midiManager.onMidiSent = [this](int sliderNumber, int midiChannel, int ccNumber, int msbValue, int lsbValue, int combinedValue) {
            if (midiMonitorWindow)
                midiMonitorWindow->logOutgoingMessage(sliderNumber, midiChannel, ccNumber, msbValue, lsbValue, combinedValue);
        };
        
        midiManager.onMidiReceiveForMonitor = [this](int midiChannel, int ccNumber, int value, const juce::String& source, int targetSlider) {
            if (midiMonitorWindow)
                midiMonitorWindow->logIncomingMessage(midiChannel, ccNumber, value, source, targetSlider);
        };
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
        
        bankManager.onBankSelectionChanged = [this](int bankIndex) {
            // Notify settings window about bank selection change
            // But only if this change is not coming from the settings window
            if (settingsWindow.isVisible() && !updatingFromSettingsWindow)
            {
                settingsWindow.updateBankSelection(bankIndex);
            }
        };
        
        // Set initial bank button colors
        updateBankButtonStates();
        
        // Load config MIDI assignments from file
        loadConfigMidiAssignments();
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
        
        // Note: getSliderStepSize callback removed as part of 7-bit mode cleanup
        // KeyboardController now uses standard 1-unit steps for all movement
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
        
        // Set up mapping learned callback (updated signature for new target system)
        midi7BitController.onMappingLearned = [this](MidiTargetType targetType, int sliderIndex, int ccNumber, int channel) {
            // Clear markers for slider targets
            if (targetType == MidiTargetType::SliderValue && sliderIndex < sliderControls.size())
            {
                sliderControls[sliderIndex]->setShowLearnMarkers(false);
                DBG("Cleared learn markers for slider " << sliderIndex);
            }
            
            // Add mapping to learn window with target type information
            midiLearnWindow.addMapping(targetType, sliderIndex, channel, ccNumber);
            MidiTargetInfo tempTarget{targetType, sliderIndex, ccNumber, channel};
            DBG("Called midiLearnWindow.addMapping for " + tempTarget.getDisplayName());
            
            // Reset learn button state
            learnButton.setButtonText("Select Target");
            DBG("Reset learn state");
            
            // Show success feedback with target type
            MidiTargetInfo targetInfo{targetType, sliderIndex, ccNumber, channel};
            juce::String message = "Mapped CC " + juce::String(ccNumber) + " (Ch " + juce::String(channel) + ") to " + targetInfo.getDisplayName();
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "MIDI Learn", message);
            DBG("Showing success message: " << message);
        };
        
        // Set up MIDI tooltip update callback
        midi7BitController.onMidiTooltipUpdate = [this](int sliderIndex, int channel, int ccNumber, int ccValue) {
            updateMidiTooltip(sliderIndex, channel, ccNumber, ccValue);
            
            // Also log to MIDI monitor if it's a learn mode message
            if (midiMonitorWindow && midi7BitController.isInLearnMode())
            {
                juce::String source = "Learn Mode";
                midiMonitorWindow->logIncomingMessage(channel, ccNumber, ccValue, source, sliderIndex + 1);
            }
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
        
        // NEW: Set up bank cycle callback
        midi7BitController.onBankCycleRequested = [this]() {
            cycleBankUp(); // Use existing bank cycling logic
            DBG("MIDI-triggered bank cycle");
        };
        
        // NEW: Set up automation toggle callback (acts like GO button click)
        midi7BitController.onAutomationToggle = [this](int sliderIndex, bool ignored) {
            if (sliderIndex < sliderControls.size())
            {
                auto* slider = sliderControls[sliderIndex];
                if (slider)
                {
                    // Simulate GO button click - this handles start/stop toggle automatically
                    if (slider->automationControlPanel.onGoButtonClicked)
                        slider->automationControlPanel.onGoButtonClicked();
                    
                    DBG("MIDI-triggered automation GO button click for slider " + juce::String(sliderIndex));
                }
            }
        };
        
        // NEW: Set up automation knob control callback
        midi7BitController.onAutomationKnobChanged = [this](int sliderIndex, MidiTargetType knobType, double knobValue) {
            if (sliderIndex < sliderControls.size())
            {
                auto* slider = sliderControls[sliderIndex];
                if (slider)
                {
                    switch (knobType)
                    {
                        case MidiTargetType::AutomationDelay:
                            slider->setDelayTime(knobValue);
                            DBG("MIDI set slider " << sliderIndex << " delay to " << knobValue);
                            break;
                        case MidiTargetType::AutomationAttack:
                            slider->setAttackTime(knobValue);
                            DBG("MIDI set slider " << sliderIndex << " attack to " << knobValue);
                            break;
                        case MidiTargetType::AutomationReturn:
                            slider->setReturnTime(knobValue);
                            DBG("MIDI set slider " << sliderIndex << " return to " << knobValue);
                            break;
                        case MidiTargetType::AutomationCurve:
                            slider->setCurveValue(knobValue);
                            DBG("MIDI set slider " << sliderIndex << " curve to " << knobValue);
                            break;
                        default:
                            break;
                    }
                }
            }
        };
    }
    
    void setupAutomationConfigManager()
    {
        // New automation config manager is self-contained and doesn't need callbacks
        // Config operations are handled directly through the manager methods
        DBG("AutomationConfigManager: Setup complete - self-contained system initialized");
        
        // AutomationConfigManager is now self-contained and handles its own file management
        // It automatically creates configs in the application data directory
    }
    
    void setupBankButtonLearnOverlays()
    {
        // Legacy overlays - DISABLED to prevent duplicate overlays
        // addChildComponent(bankALearnOverlay);
        // addChildComponent(bankBLearnOverlay); 
        // addChildComponent(bankCLearnOverlay);
        // addChildComponent(bankDLearnOverlay);
        
        // Set up bank button learn zone system (single encompassing zone)
        bankButtonLearnZone = std::make_unique<BankButtonLearnZone>();
        addChildComponent(*bankButtonLearnZone);
        
        // Set up learn zone callback
        bankButtonLearnZone->onZoneClicked = [this](const LearnZone& zone) {
            handleLearnZoneClicked(zone);
        };
    }
    
    void updateBankButtonLearnOverlays()
    {
        // Legacy overlays - DISABLED to prevent duplicate overlays
        // bankALearnOverlay.setBounds(bankAButton.getBounds());
        // bankBLearnOverlay.setBounds(bankBButton.getBounds());
        // bankCLearnOverlay.setBounds(bankCButton.getBounds());
        // bankDLearnOverlay.setBounds(bankDButton.getBounds());
        
        // Position bank button learn zone over entire bank area
        if (bankButtonLearnZone)
        {
            auto bankAreaBounds = bankAButton.getBounds()
                .getUnion(bankBButton.getBounds())
                .getUnion(bankCButton.getBounds())
                .getUnion(bankDButton.getBounds());
            bankButtonLearnZone->setBounds(bankAreaBounds);
        }
    }
    
    void setBankButtonLearnMode(bool active)
    {
        // Legacy overlays - DISABLED to avoid duplicate overlays
        // bankALearnOverlay.setLearnModeActive(active);
        // bankBLearnOverlay.setLearnModeActive(active);
        // bankCLearnOverlay.setLearnModeActive(active);
        // bankDLearnOverlay.setLearnModeActive(active);
        
        // New learn zone system (single encompassing zone)
        if (bankButtonLearnZone)
            bankButtonLearnZone->setLearnModeActive(active);
    }
    
    void handleLearnZoneClicked(const LearnZone& zone)
    {
        // Handle learn zone clicks for the new learn system
        DBG("Learn zone clicked: " + zone.getDisplayName());
        
        // Set learn target based on zone type
        midi7BitController.setLearnTarget(zone.midiTargetType, zone.sliderIndex);
        
        // Update learn button text
        learnButton.setButtonText("Move Controller");
        
        // Show visual feedback for the clicked zone
        showOrangeBracketsForZone(zone);
        
        DBG("Learning target set: " + zone.getDisplayName());
    }
    
    void showOrangeBracketsForZone(const LearnZone& zone)
    {
        // This method will be called to show orange brackets around the clicked zone
        // The actual bracket drawing is handled by the individual learn zone components
        // This method can be used to coordinate visual feedback across the interface
        
        // Clear previous active zones
        clearAllActiveLearnZones();
        
        // The visual feedback is automatically handled by the learn zone components
        // through their paint methods and selection state
    }
    
    void clearAllActiveLearnZones()
    {
        // Clear active zones from all sliders
        for (auto* slider : sliderControls)
        {
            if (slider)
                slider->clearActiveLearnZone();
        }
    }
    
    void openConfigManagementWindow(int sliderIndex, AutomationConfigManagementWindow::Mode mode)
    {
        // Create window if it doesn't exist
        if (!configManagementWindow)
        {
            configManagementWindow = std::make_unique<AutomationConfigManagementWindow>(
                automationConfigManager, mode);
            
            // Set up callbacks for the management window
            configManagementWindow->onLoadConfig = [this](const AutomationConfig& config, int targetSlider) {
                if (targetSlider >= 0 && targetSlider < sliderControls.size())
                {
                    sliderControls[targetSlider]->applyAutomationConfig(config);
                    DBG("Loaded config '" + config.name + "' to slider " + juce::String(targetSlider + 1));
                }
            };
            
            configManagementWindow->onLoadAndSaveConfig = [this](const AutomationConfig& config, int targetSlider, bool alsoSave) {
                if (targetSlider >= 0 && targetSlider < sliderControls.size())
                {
                    // Load the config
                    sliderControls[targetSlider]->applyAutomationConfig(config);
                    DBG("Loaded config '" + config.name + "' to slider " + juce::String(targetSlider + 1));
                    
                    if (alsoSave)
                    {
                        // Switch to save mode and enable highlighting
                        configManagementWindow->setMode(AutomationConfigManagementWindow::Mode::Save, targetSlider);
                        sliderControls[targetSlider]->setAutomationHighlighted(true);
                    }
                }
            };
            
            configManagementWindow->onSaveNewConfig = [this](const juce::String& configName, int sourceSlider) {
                if (sourceSlider >= 0 && sourceSlider < sliderControls.size())
                {
                    auto config = sliderControls[sourceSlider]->getCurrentAutomationConfig();
                    config.name = configName;
                    
                    auto savedId = automationConfigManager.saveConfig(config);
                    if (!savedId.isEmpty())
                    {
                        DBG("Saved new config '" + configName + "' from slider " + juce::String(sourceSlider + 1));
                        configManagementWindow->refreshConfigList();
                        
                        // Clear highlighting after successful save
                        sliderControls[sourceSlider]->setAutomationHighlighted(false);
                    }
                    else
                    {
                        DBG("Failed to save config '" + configName + "'");
                    }
                }
            };
            
            configManagementWindow->onSourceHighlightChanged = [this](bool highlight, int sliderIndex) {
                if (sliderIndex >= 0 && sliderIndex < sliderControls.size())
                {
                    sliderControls[sliderIndex]->setAutomationHighlighted(highlight);
                }
            };
            
            configManagementWindow->onGetSliderCustomName = [this](int sliderIndex) -> juce::String {
                if (sliderIndex >= 0 && sliderIndex < sliderControls.size())
                {
                    // Get custom name from settings window
                    return settingsWindow.getSliderDisplayName(sliderIndex);
                }
                return {};
            };
            
            configManagementWindow->onStartMidiLearn = [this](const juce::String& configId) {
                // Start MIDI learn mode for the selected config
                DBG("Starting one-shot MIDI learn for config: " + configId);
                
                // Enter learn mode if not already active
                if (!isInLearnMode)
                {
                    isInLearnMode = true;
                    isOneShotLearnMode = true; // Flag this as one-shot mode
                    midi7BitController.startLearnMode();
                    setBankButtonLearnMode(true);
                    setAllSlidersLearnMode(true);
                    learnButton.setButtonText("Pairing...");
                    
                    // Show learn window if not visible
                    if (!midiLearnWindow.isVisible())
                    {
                        addAndMakeVisible(midiLearnWindow);
                        resized();
                    }
                }
                else
                {
                    // Already in learn mode - just set one-shot flag if not already set
                    if (!isOneShotLearnMode)
                    {
                        DBG("Already in normal learn mode, setting up config pairing without one-shot behavior");
                    }
                }
                
                // Notify config manager that learn mode is active
                configManagementWindow->setLearnModeActive(true);
                
                // Store the config ID that's ready for MIDI learn
                midiLearnConfigId = configId;
                
                DBG("Learn mode activated for config MIDI pairing: " + configId + " (one-shot: " + juce::String(isOneShotLearnMode ? "true" : "false") + ")");
            };
            
            configManagementWindow->onConfigSelectionChanged = [this](const juce::String& configId, int rowNumber) {
                // Handle config selection changes
                DBG("Config selected: " + configId + " at row " + juce::String(rowNumber));
            };
            
            // Provide access to MIDI assignment display strings
            configManagementWindow->onGetMidiAssignmentString = [this](const juce::String& configId) -> juce::String {
                return getConfigMidiAssignmentString(configId);
            };
        }
        
        // Set mode and target slider
        configManagementWindow->setMode(mode, sliderIndex);
        
        // Show the window
        configManagementWindow->setVisible(true);
        configManagementWindow->toFront(true);
        
        DBG("Opened config management window in mode " + juce::String((int)mode) + " for slider " + juce::String(sliderIndex + 1));
    }
    
    void setAllSlidersLearnMode(bool active)
    {
        // Activate/deactivate learn zones for ALL sliders (always-available system)
        for (auto* slider : sliderControls)
        {
            if (slider)
                slider->activateAllLearnZones(active);
        }
    }
    
    // Scale change notification implementation
    void scaleFactorChanged(float newScale) override
    {
        // Update font scaling for labels
        auto& scale = GlobalUIScale::getInstance();
        showingLabel.setFont(scale.getScaledFont(12.0f).boldened());
        movementSpeedLabel.setFont(scale.getScaledFont(10.0f));
        windowSizeLabel.setFont(scale.getScaledFont(10.0f));
        
        // Update window constraints for new scale
        updateWindowConstraints();
        
        // CRITICAL: Immediately resize the main window to new scaled dimensions
        if (auto* topLevel = getTopLevelComponent())
        {
            // Calculate new window dimensions based on current mode
            int contentWidth = bankManager.isEightSliderMode() ? scale.getScaled(970) : scale.getScaled(490);
            int settingsWidth = (isInSettingsMode || isInLearnMode) ? MainControllerLayout::Constants::getSettingsPanelWidth() : 0;
            int targetWidth = contentWidth + settingsWidth;
            
            // Calculate optimal height with scaling
            int optimalHeight = scale.getScaled(660); // Use scaled optimal height
            
            // Immediately resize to new scaled dimensions
            topLevel->setSize(targetWidth, optimalHeight);
        }
        windowSizeLabel.setFont(scale.getScaledFont(10.0f));
        
        // Trigger full layout update
        resized();
        repaint();
        
        // Notify child windows of scale change
        if (settingsWindow.isVisible())
        {
            settingsWindow.repaint();
        }
        
        if (midiLearnWindow.isVisible())
        {
            midiLearnWindow.repaint();
        }
        
        if (midiMonitorWindow && midiMonitorWindow->isVisible())
        {
            midiMonitorWindow->repaint();
        }
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
    
    void updateMonitorButtonText(bool isVisible)
    {
        monitorButton.setButtonText(isVisible ? "Hide Monitor" : "MIDI Monitor");
    }
    
    void updateBankButtonStates()
    {
        int activeBank = bankManager.getActiveBank();
        auto bankColors = bankManager.getCurrentBankColors();
        
        bankButtonManager.updateBankButtonStates(bankAButton, bankBButton, bankCButton, bankDButton,
                                                customButtonLookAndFeel, activeBank,
                                                bankColors.bankA, bankColors.bankB, 
                                                bankColors.bankC, bankColors.bankD);
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
            
            // Update display name
            juce::String displayName = settingsWindow.getSliderDisplayName(i);
            sliderControls[i]->setDisplayName(displayName);
            
            // Update step increment for quantization
            double increment = settingsWindow.getIncrement(i);
            sliderControls[i]->setStepIncrement(increment);
            
            // Update orientation - THIS IS THE KEY FIX FOR ORIENTATION PERSISTENCE
            SliderOrientation orientation = settingsWindow.getSliderOrientation(i);
            sliderControls[i]->setOrientation(orientation);
            
            // Update bipolar settings if in bipolar mode
            if (orientation == SliderOrientation::Bipolar)
            {
                BipolarSettings bipolarSettings = settingsWindow.getBipolarSettings(i);
                sliderControls[i]->setBipolarSettings(bipolarSettings);
            }
            
            // Update automation visibility
            bool showAutomation = settingsWindow.getShowAutomation(i);
            sliderControls[i]->setAutomationVisible(showAutomation);
        }
        
        // Trigger layout update when automation visibility changes
        resized();
        
        // Note: Using simple channel-based MIDI filtering
        
        // Auto-save whenever settings change
        saveCurrentState();
    }
    
    // Arrow key bank navigation methods
    void cycleBankUp()
    {
        int currentBank = bankManager.getActiveBank();
        bool isEightMode = bankManager.isEightSliderMode();
        
        if (isEightMode)
        {
            // In 8-slider mode, toggle between bank pairs: A+B (0) ↔ C+D (2)
            int newBank = (currentBank <= 1) ? 2 : 0;
            bankManager.setActiveBank(newBank);
        }
        else
        {
            // In 4-slider mode, cycle through individual banks: A→B→C→D→A
            int newBank = (currentBank + 1) % 4;
            bankManager.setActiveBank(newBank);
        }
        
        // Update visual button states
        updateBankButtonStates();
        repaint(); // Ensure visual updates
    }
    
    void cycleBankDown()
    {
        int currentBank = bankManager.getActiveBank();
        bool isEightMode = bankManager.isEightSliderMode();
        
        if (isEightMode)
        {
            // In 8-slider mode, toggle between bank pairs: A+B (0) ↔ C+D (2)
            int newBank = (currentBank <= 1) ? 2 : 0;
            bankManager.setActiveBank(newBank);
        }
        else
        {
            // In 4-slider mode, cycle through individual banks: D→C→B→A→D
            int newBank = (currentBank + 3) % 4; // +3 is equivalent to -1 in mod 4
            bankManager.setActiveBank(newBank);
        }
        
        // Update visual button states
        updateBankButtonStates();
        repaint(); // Ensure visual updates
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
                
                // Apply orientation from preset - CRITICAL FOR PRESET ORIENTATION PERSISTENCE
                SliderOrientation orientation = static_cast<SliderOrientation>(sliderPreset.orientation);
                sliderControls[i]->setOrientation(orientation);
                
                // Apply bipolar settings if in bipolar mode
                if (orientation == SliderOrientation::Bipolar)
                {
                    BipolarSettings bipolarSettings; // Center value now auto-calculated
                    sliderControls[i]->setBipolarSettings(bipolarSettings);
                }
            }
        }
        
        // Update settings to reflect the new configuration
        updateSliderSettings();
        
        // Note: Using simple channel-based MIDI filtering
    }
    
    void toggleSettingsMode()
    {
        auto onLearnModeExit = [this]() {
            isInLearnMode = false;
            midi7BitController.stopLearnMode();
            setBankButtonLearnMode(false);
            setAllSlidersLearnMode(false); // NEW: Deactivate all slider learn zones
            clearAllActiveLearnZones(); // NEW: Clear any active zone selections
            learnButton.setButtonText("Learn");
            midiLearnWindow.setVisible(false);
            
            // Clear MIDI learn config pairing state
            midiLearnConfigId = {};
            if (configManagementWindow)
            {
                configManagementWindow->setLearnModeActive(false);
            }
            
            // Clear any learn markers
            for (auto* slider : sliderControls)
                slider->setShowLearnMarkers(false);
        };
        
        windowManager.toggleSettingsWindow(getTopLevelComponent(), settingsWindow, &midiLearnWindow,
                                          isInSettingsMode, isInLearnMode, bankManager.isEightSliderMode(),
                                          MainControllerLayout::Constants::getSettingsPanelWidth(), onLearnModeExit, 
                                          [this]() { return bankManager.getActiveBank(); });
        
        if (isInSettingsMode)
        {
            addAndMakeVisible(settingsWindow);
            
            // Initialize selection with first visible slider for immediate highlighting
            int firstVisibleSlider = bankManager.getVisibleSliderIndex(0);
            if (firstVisibleSlider >= 0)
            {
                setSelectedSliderForEditing(firstVisibleSlider);
                settingsWindow.selectSlider(firstVisibleSlider);
            }
        }
        else
        {
            // Clear selection highlighting when settings window is closed
            setSelectedSliderForEditing(-1);
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
        windowManager.updateWindowConstraints(getTopLevelComponent(), bankManager.isEightSliderMode(),
                                            isInSettingsMode, isInLearnMode, 
                                            MainControllerLayout::Constants::getSettingsPanelWidth());
    }
    
    void saveCurrentState()
    {
        auto preset = getCurrentControllerState();
        settingsWindow.getPresetManager().autoSaveCurrentState(preset);
    }
    
    void loadAutoSavedState()
    {
        auto preset = settingsWindow.getPresetManager().loadAutoSavedState();
        
        // Apply UI scale factor first (before other UI operations)
        GlobalUIScale::getInstance().setScaleFactor(preset.uiScale);
        
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
        
        // Save current UI scale factor
        preset.uiScale = GlobalUIScale::getInstance().getScaleFactor();
        
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
                
                // Save orientation to preset - CRITICAL FOR PRESET PERSISTENCE
                preset.sliders.getReference(i).orientation = static_cast<int>(sliderControls[i]->getOrientation());
                // bipolarCenter removed - now automatically calculated from range
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
        auto onLearnModeEnter = [this]() {
            isInLearnMode = true;
            midi7BitController.startLearnMode();
            setBankButtonLearnMode(true);
            setAllSlidersLearnMode(true); // NEW: Activate all slider learn zones
            DBG("Entered learn mode: isLearningMode=" << (int)midi7BitController.isInLearnMode());
            learnButton.setButtonText("Select Target");
            
            // Notify config manager that learn mode is active
            if (configManagementWindow)
            {
                configManagementWindow->setLearnModeActive(true);
            }
        };
        
        auto onLearnModeExit = [this]() {
            isInLearnMode = false;
            midi7BitController.stopLearnMode();
            setBankButtonLearnMode(false);
            setAllSlidersLearnMode(false); // NEW: Deactivate all slider learn zones
            clearAllActiveLearnZones(); // NEW: Clear any active zone selections
            DBG("Exited learn mode: isLearningMode=" << (int)midi7BitController.isInLearnMode());
            learnButton.setButtonText("Learn");
            
            // Clear MIDI learn config pairing state
            midiLearnConfigId = {};
            if (configManagementWindow)
            {
                configManagementWindow->setLearnModeActive(false);
            }
            
            // Clear any learn markers
            for (auto* slider : sliderControls)
                slider->setShowLearnMarkers(false);
        };
        
        windowManager.toggleLearnWindow(getTopLevelComponent(), midiLearnWindow, settingsWindow,
                                       isInLearnMode, isInSettingsMode, bankManager.isEightSliderMode(),
                                       MainControllerLayout::Constants::getSettingsPanelWidth(),
                                       onLearnModeEnter, onLearnModeExit);
        
        if (isInLearnMode)
        {
            addAndMakeVisible(midiLearnWindow);
        }
        
        resized();
    }
    
    void toggleMidiMonitor()
    {
        if (midiMonitorWindow)
        {
            if (midiMonitorWindow->isVisible())
            {
                midiMonitorWindow->setVisible(false);
                // Callback will be automatically triggered by closeButtonPressed override
                // or we can trigger it manually here for programmatic closing
                if (midiMonitorWindow->onVisibilityChanged)
                    midiMonitorWindow->onVisibilityChanged(false);
            }
            else
            {
                midiMonitorWindow->setVisible(true);
                midiMonitorWindow->toFront(true);
                
                // Trigger callback for showing the window
                if (midiMonitorWindow->onVisibilityChanged)
                    midiMonitorWindow->onVisibilityChanged(true);
                
                // Position the window next to the main window
                if (auto* topLevel = getTopLevelComponent())
                {
                    auto mainBounds = topLevel->getBounds();
                    auto monitorBounds = juce::Rectangle<int>(600, 400);
                    monitorBounds.setPosition(mainBounds.getRight() + 10, mainBounds.getY());
                    midiMonitorWindow->setBounds(monitorBounds);
                }
            }
        }
    }
    
    // Note: Complex CC-based filtering removed in favor of simple channel-based approach
    
    
    // UI Components
    juce::OwnedArray<SimpleSliderControl> sliderControls;
    juce::TextButton settingsButton;
    juce::TextButton modeButton;
    juce::TextButton learnButton;
    juce::TextButton monitorButton;
    juce::ToggleButton bankAButton, bankBButton, bankCButton, bankDButton;
    CustomButtonLookAndFeel customButtonLookAndFeel;
    
    // Bank button learn overlays (legacy)
    BankButtonLearnOverlay bankALearnOverlay, bankBLearnOverlay, bankCLearnOverlay, bankDLearnOverlay;
    
    // New learn zone system
    std::unique_ptr<BankButtonLearnZone> bankButtonLearnZone;
    juce::Label showingLabel;
    SettingsWindow settingsWindow;
    MidiLearnWindow midiLearnWindow;
    std::unique_ptr<MidiMonitorWindow> midiMonitorWindow;
    std::unique_ptr<AutomationConfigManagementWindow> configManagementWindow;
    juce::Label movementSpeedLabel;
    juce::Label windowSizeLabel;
    
    // Core Systems
    MidiManager midiManager;
    KeyboardController keyboardController;
    BankManager bankManager;
    Midi7BitController midi7BitController;
    AutomationConfigManager automationConfigManager;
    
    // Layout and window managers
    MainControllerLayout mainLayout;
    WindowManager windowManager;
    BankButtonManager bankButtonManager;
    
    // State
    bool isInSettingsMode = false;
    bool isInLearnMode = false;
    int selectedSliderForEditing = -1; // -1 means no selection, 0-15 for slider index
    bool updatingFromSettingsWindow = false; // Flag to prevent circular callbacks
    juce::String midiLearnConfigId; // Config ID ready for MIDI learn pairing
    bool isOneShotLearnMode = false; // True when learn mode was initiated by config manager (turns off after pairing)
    
    // MIDI assignments for automation configs (configId -> channel/CC mapping)
    struct ConfigMidiAssignment
    {
        int channel;
        int ccNumber;
        juce::String configId;
        
        ConfigMidiAssignment() : channel(-1), ccNumber(-1) {}
        ConfigMidiAssignment(int ch, int cc, const juce::String& id) : channel(ch), ccNumber(cc), configId(id) {}
        
        bool isValid() const { return channel >= 0 && ccNumber >= 0 && configId.isNotEmpty(); }
    };
    
    std::vector<ConfigMidiAssignment> configMidiAssignments;
    
    // MIDI config assignment methods
    void handleConfigMidiPairing(const juce::String& configId, int channel, int ccNumber)
    {
        DBG("Pairing config " + configId + " with MIDI Ch" + juce::String(channel) + " CC" + juce::String(ccNumber));
        
        // Remove any existing assignment for this config
        configMidiAssignments.erase(
            std::remove_if(configMidiAssignments.begin(), configMidiAssignments.end(),
                [&configId](const ConfigMidiAssignment& assignment) {
                    return assignment.configId == configId;
                }),
            configMidiAssignments.end());
        
        // Add the new assignment
        configMidiAssignments.emplace_back(channel, ccNumber, configId);
        
        DBG("Config MIDI assignment added. Total assignments: " + juce::String(configMidiAssignments.size()));
        
        // Save assignments to file for persistence
        saveConfigMidiAssignments();
    }
    
    ConfigMidiAssignment getConfigMidiAssignment(const juce::String& configId) const
    {
        auto it = std::find_if(configMidiAssignments.begin(), configMidiAssignments.end(),
            [&configId](const ConfigMidiAssignment& assignment) {
                return assignment.configId == configId;
            });
        
        return (it != configMidiAssignments.end()) ? *it : ConfigMidiAssignment();
    }
    
    juce::String getConfigMidiAssignmentString(const juce::String& configId) const
    {
        auto assignment = getConfigMidiAssignment(configId);
        if (assignment.isValid())
        {
            return "Ch " + juce::String(assignment.channel) + " CC " + juce::String(assignment.ccNumber);
        }
        return "Not Assigned";
    }
    
    void saveConfigMidiAssignments()
    {
        auto file = automationConfigManager.getConfigFile().getSiblingFile("config_midi_assignments.json");
        
        juce::var assignmentsArray;
        for (const auto& assignment : configMidiAssignments)
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("configId", assignment.configId);
            obj->setProperty("channel", assignment.channel);
            obj->setProperty("ccNumber", assignment.ccNumber);
            assignmentsArray.append(obj);
        }
        
        auto json = juce::JSON::toString(assignmentsArray);
        file.replaceWithText(json);
        
        DBG("Saved " + juce::String(configMidiAssignments.size()) + " config MIDI assignments to " + file.getFullPathName());
    }
    
    void loadConfigMidiAssignments()
    {
        auto file = automationConfigManager.getConfigFile().getSiblingFile("config_midi_assignments.json");
        if (!file.existsAsFile()) return;
        
        auto json = file.loadFileAsString();
        auto result = juce::JSON::parse(json);
        
        if (result.isArray())
        {
            configMidiAssignments.clear();
            for (const auto& item : *result.getArray())
            {
                if (auto* obj = item.getDynamicObject())
                {
                    ConfigMidiAssignment assignment;
                    assignment.configId = obj->getProperty("configId").toString();
                    assignment.channel = obj->getProperty("channel");
                    assignment.ccNumber = obj->getProperty("ccNumber");
                    
                    if (assignment.isValid())
                        configMidiAssignments.push_back(assignment);
                }
            }
            
            DBG("Loaded " + juce::String(configMidiAssignments.size()) + " config MIDI assignments from " + file.getFullPathName());
        }
    }
    
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
