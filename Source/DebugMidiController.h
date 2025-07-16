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
            toggleSettingsMode();
        };
        
        // Mode toggle button
        addAndMakeVisible(modeButton);
        modeButton.setButtonText(isEightSliderMode ? "8" : "4");
        modeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
        modeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        modeButton.onClick = [this]() {
            toggleSliderMode();
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
        movementSpeedLabel.setFont(juce::FontOptions(10.0f));
        updateMovementSpeedDisplay();
        
        // Window size tooltip
        addAndMakeVisible(windowSizeLabel);
        windowSizeLabel.setJustificationType(juce::Justification::centredRight);
        windowSizeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        windowSizeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);
        windowSizeLabel.setFont(juce::FontOptions(10.0f));
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
        // App background - solid color
        g.fillAll(juce::Colour(0xFF5E5E5E));
        
        // Calculate content area bounds using same logic as resized()
        const int topAreaHeight = 65;
        const int tooltipHeight = 38;
        const int verticalGap = 15;
        const int contentAreaWidth = isEightSliderMode ? 970 : 490;
        
        auto area = getLocalBounds();
        int contentX = isInSettingsMode ? SETTINGS_PANEL_WIDTH : (area.getWidth() - contentAreaWidth) / 2;
        int contentY = topAreaHeight + verticalGap;
        int contentHeight = area.getHeight() - topAreaHeight - tooltipHeight - (2 * verticalGap);
        
        juce::Rectangle<int> contentAreaBounds(contentX, contentY, contentAreaWidth, contentHeight);
        
        // Draw eurorack-style background before slider plates
        drawEurorackBackground(g, contentAreaBounds);
        
        // Draw tooltip box above eurorack background
        drawTooltipBox(g, contentAreaBounds);
        
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
            }
        }
        
        // Calculate top area bounds for text positioning
        juce::Rectangle<int> topAreaBounds;
        if (isInSettingsMode && settingsWindow.isVisible())
        {
            topAreaBounds = juce::Rectangle<int>(0, 0, getWidth(), topAreaHeight);
        }
        else
        {
            topAreaBounds = juce::Rectangle<int>(contentX, 0, contentAreaWidth, topAreaHeight);
        }
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(20.0f));
        // Title centered in top area
        g.drawText("VMC14",
                  topAreaBounds.getX() + 10, 10,
                  topAreaBounds.getWidth() - 20, 35, 
                  juce::Justification::centred);
        
        // Show MIDI status left-aligned in top area
        g.setFont(juce::FontOptions(12.0f));
        juce::String status = midiOutput ? "MIDI: Connected" : "MIDI: Disconnected";
        g.drawText(status, topAreaBounds.getX() + 10, 10, 200, 20, juce::Justification::left);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        
        // Use current slider mode (controlled by mode button)
        int visibleSliderCount = isEightSliderMode ? 8 : 4;
        
        // Update window size display
        updateWindowSizeDisplay();
        
        // Calculate layout dimensions once
        const int topAreaHeight = 65;
        const int tooltipHeight = 38;
        const int verticalGap = 15;
        const int contentAreaWidth = isEightSliderMode ? 970 : 490;
        
        // Calculate content area bounds - simplified logic
        int contentX = isInSettingsMode ? SETTINGS_PANEL_WIDTH : (area.getWidth() - contentAreaWidth) / 2;
        int contentY = topAreaHeight + verticalGap;
        int contentHeight = area.getHeight() - topAreaHeight - tooltipHeight - (2 * verticalGap);
        
        juce::Rectangle<int> contentAreaBounds(contentX, contentY, contentAreaWidth, contentHeight);
        
        // Calculate top area bounds based on settings state
        juce::Rectangle<int> topAreaBounds;
        if (isInSettingsMode && settingsWindow.isVisible())
        {
            topAreaBounds = juce::Rectangle<int>(0, 0, getWidth(), topAreaHeight);
        }
        else
        {
            topAreaBounds = juce::Rectangle<int>(contentX, 0, contentAreaWidth, topAreaHeight);
        }
        
        // Position top area components
        layoutTopAreaComponents(topAreaBounds);
        
        // Position settings window if visible
        if (isInSettingsMode && settingsWindow.isVisible())
        {
            int settingsY = topAreaHeight;
            int settingsHeight = area.getHeight() - settingsY;
            settingsWindow.setBounds(0, settingsY, SETTINGS_PANEL_WIDTH, settingsHeight);
        }
        
        // Layout sliders within content area
        layoutSlidersFixed(contentAreaBounds, visibleSliderCount);
        
        // Position tooltips at bottom
        layoutTooltips(area, contentAreaWidth, tooltipHeight);
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
        // Handle keyboard controls
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
    
    
    void layoutSlidersFixed(juce::Rectangle<int> area, int visibleSliderCount)
    {
        // Calculate total width needed for the slider rack
        int totalSliderWidth = (visibleSliderCount * SLIDER_PLATE_WIDTH) + ((visibleSliderCount - 1) * SLIDER_GAP);
        
        // Center sliders within the provided content area
        int startX = area.getX() + (area.getWidth() - totalSliderWidth) / 2;

        
        // Set bounds for each visible slider - complete recalculation
        for (int i = 0; i < visibleSliderCount; ++i)
        {
            int sliderIndex = getVisibleSliderIndex(i);
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
    
    void drawEurorackBackground(juce::Graphics& g, juce::Rectangle<int> contentAreaBounds)
    {
        const int railHeight = 15;
        
        // Use passed content area bounds strictly - no recalculation
        auto rackArea = contentAreaBounds;
        
        // Top rail within content area
        auto topRail = rackArea.removeFromTop(railHeight);
        drawMetallicRail(g, topRail);
        
        // Bottom rail within content area
        auto bottomRail = rackArea.removeFromBottom(railHeight);
        drawMetallicRail(g, bottomRail);
        
        // Central recessed area (rack body depth) within content area
        drawRackBodyDepth(g, rackArea);
        
        // In settings mode, draw visual separation with improved positioning
        if (isInSettingsMode && settingsWindow.isVisible())
        {
            drawSettingsPanelSeparator(g, contentAreaBounds, railHeight);
        }
    }
    
    void drawMetallicRail(juce::Graphics& g, juce::Rectangle<int> railBounds)
    {
        auto bounds = railBounds.toFloat();
        
        // Calculate track dimensions (8px total height with gap in middle)
        float trackHeight = 8.0f;
        float lineHeight = 2.0f;
        float gapHeight = trackHeight - (2 * lineHeight); // 4px gap
        
        // Center the track within the rail bounds
        float trackY = bounds.getY() + (bounds.getHeight() - trackHeight) / 2.0f;
        
        // Upper mounting track line (lighter metallic)
        auto upperLine = juce::Rectangle<float>(bounds.getX(), trackY, bounds.getWidth(), lineHeight);
        g.setColour(juce::Colour(0xFFE0E0E0)); // Lighter metallic color
        g.fillRect(upperLine);
        
        // Gap in middle (background color or slightly darker)
        auto gapArea = juce::Rectangle<float>(bounds.getX(), trackY + lineHeight, bounds.getWidth(), gapHeight);
        g.setColour(juce::Colour(0xFF303030)); // Slightly darker background
        g.fillRect(gapArea);
        
        // Lower mounting track line (darker metallic)
        auto lowerLine = juce::Rectangle<float>(bounds.getX(), trackY + lineHeight + gapHeight, bounds.getWidth(), lineHeight);
        g.setColour(juce::Colour(0xFFA0A0A0)); // Darker metallic color
        g.fillRect(lowerLine);
    }
    
    void drawRackBodyDepth(juce::Graphics& g, juce::Rectangle<int> bodyBounds)
    {
        auto bounds = bodyBounds.toFloat();
        
        // Central recessed area gradient (darker in center, lighter at edges)
        juce::ColourGradient depthGradient(
            juce::Colour(0xFF2A2A2A), bounds.getCentre(), // Dark center
            juce::Colour(0xFF404040), bounds.getTopLeft(), // Lighter edges
            true // Radial gradient
        );
        depthGradient.addColour(0.7f, juce::Colour(0xFF353535));
        
        g.setGradientFill(depthGradient);
        g.fillRect(bounds);
        
        // Subtle inner shadow at top
        juce::ColourGradient topShadow(
            juce::Colour(0x60000000), bounds.getTopLeft(),
            juce::Colour(0x00000000), bounds.getTopLeft() + juce::Point<float>(0, 20),
            false
        );
        g.setGradientFill(topShadow);
        g.fillRect(bounds.removeFromTop(20));
        
        // Subtle highlight at bottom
        juce::ColourGradient bottomHighlight(
            juce::Colour(0x00000000), bounds.getBottomLeft() - juce::Point<float>(0, 20),
            juce::Colour(0x30FFFFFF), bounds.getBottomLeft(),
            false
        );
        g.setGradientFill(bottomHighlight);
        g.fillRect(bounds.removeFromBottom(20));
    }
    
    void drawTooltipBox(juce::Graphics& g, juce::Rectangle<int> contentAreaBounds)
    {
        // Tooltip box bounds are calculated but no background is drawn
        // Content positioning remains the same but no visual background
        auto tooltipBounds = contentAreaBounds;
        tooltipBounds.setY(contentAreaBounds.getBottom() + 15); // Position below content area with gap
        tooltipBounds.setHeight(38); // Fixed tooltip height
        
        // No background drawing - tooltip content uses app background
    }
    
    void layoutTopAreaComponents(const juce::Rectangle<int>& topAreaBounds)
    {
        // Settings button - positioned on left within top area
        int settingsButtonX = topAreaBounds.getX() + 10;
        settingsButton.setBounds(settingsButtonX, 35, 100, 20);
        
        // Mode button - positioned next to settings button
        int modeButtonX = settingsButtonX + 110; // 100px settings button + 10px gap
        modeButton.setBounds(modeButtonX, 35, 30, 20);
        
        // Bank buttons - positioned as 2x2 grid in top right of top area
        const int buttonWidth = 35;
        const int buttonHeight = 20;
        const int buttonSpacing = 5;
        const int rightMargin = 10;
        
        int gridWidth = (2 * buttonWidth) + buttonSpacing;
        int gridStartX = topAreaBounds.getRight() - rightMargin - gridWidth;
        int gridStartY = 10;
        
        // Top row: A and B buttons
        bankAButton.setBounds(gridStartX, gridStartY, buttonWidth, buttonHeight);
        bankBButton.setBounds(gridStartX + buttonWidth + buttonSpacing, gridStartY, buttonWidth, buttonHeight);
        
        // Bottom row: C and D buttons
        bankCButton.setBounds(gridStartX, gridStartY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
        bankDButton.setBounds(gridStartX + buttonWidth + buttonSpacing, gridStartY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
    }
    
    void layoutTooltips(const juce::Rectangle<int>& area, int contentAreaWidth, int tooltipHeight)
    {
        auto tooltipArea = area.withHeight(tooltipHeight).withBottomY(area.getBottom());
        
        if (isInSettingsMode && settingsWindow.isVisible())
        {
            // Settings open: tooltips span remaining width after settings panel
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
    
    void drawSettingsPanelSeparator(juce::Graphics& g)
    {
        const int dividerX = SETTINGS_PANEL_WIDTH;
        const int topAreaHeight = 65;
        const int tooltipHeight = 38;
        const int railHeight = 15;
        
        // Draw vertical divider line between settings and main content
        g.setColour(juce::Colour(0xFF606060));
        g.drawLine(dividerX, topAreaHeight, dividerX, getHeight() - tooltipHeight, 1.0f);
        
        // Calculate shadow bounds to avoid overlapping with rails
        int shadowStartY = topAreaHeight + railHeight;
        int shadowHeight = getHeight() - topAreaHeight - tooltipHeight - (2 * railHeight);
        
        // Only draw shadow if there's space between the rails
        if (shadowHeight > 0)
        {
            // Add subtle shadow effect on the right side of settings panel
            // but only in the area between the top and bottom rails
            juce::ColourGradient shadow(
                juce::Colour(0x40000000), dividerX, shadowStartY,
                juce::Colour(0x00000000), dividerX + 10, shadowStartY,
                false
            );
            g.setGradientFill(shadow);
            g.fillRect(dividerX, shadowStartY, 10, shadowHeight);
        }
    }
    
    void drawSettingsPanelSeparator(juce::Graphics& g, juce::Rectangle<int> contentAreaBounds, int railHeight)
    {
        // Draw subtle vertical divider line between settings and main content
        const int dividerX = SETTINGS_PANEL_WIDTH;
        g.setColour(juce::Colour(0xFF606060));
        g.drawLine(dividerX, 65, dividerX, getHeight() - 35, 1.0f);
        
        // Add subtle shadow effect on the right side of settings panel
        // Only draw shadow in the central area between the rails (not above or below)
        int shadowStartY = contentAreaBounds.getY() + railHeight; // Below top rail
        int shadowEndY = contentAreaBounds.getBottom() - railHeight; // Above bottom rail
        int shadowHeight = shadowEndY - shadowStartY;
        
        if (shadowHeight > 0)
        {
            juce::ColourGradient shadow(
                juce::Colour(0x30000000), dividerX, 0,
                juce::Colour(0x00000000), dividerX + 8, 0,
                false
            );
            g.setGradientFill(shadow);
            g.fillRect(dividerX, shadowStartY, 8, shadowHeight);
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
    
    void toggleSettingsMode()
    {
        isInSettingsMode = !isInSettingsMode;
        
        // Update button appearance
        settingsButton.setColour(juce::TextButton::buttonColourId, 
                               isInSettingsMode ? juce::Colours::orange : juce::Colours::grey);
        
        // Update constraints BEFORE resizing to prevent constraint violations
        updateWindowConstraints();
        
        if (auto* topLevel = getTopLevelComponent())
        {
            // Calculate target window width: content area + settings panel (if open)
            int contentAreaWidth = isEightSliderMode ? 970 : 490;
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
        // Toggle mode state first
        isEightSliderMode = !isEightSliderMode;
        
        // Update button text
        modeButton.setButtonText(isEightSliderMode ? "8" : "4");
        
        // Calculate target window width BEFORE updating layout
        int contentWidth = isEightSliderMode ? 970 : 490;
        int settingsWidth = isInSettingsMode ? 350 : 0;
        int targetWidth = contentWidth + settingsWidth;
        
        // Update constrainer limits BEFORE resizing window
        updateWindowConstraints();
        
        // Force window resize immediately
        if (auto* topLevel = getTopLevelComponent())
        {
            topLevel->setSize(targetWidth, topLevel->getHeight());
        }
        
        // Update slider visibility and layout after window resize
        updateSliderVisibility();
        updateBankButtonStates();
        resized();
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
                    int fixedWidth = isEightSliderMode ? 970 : 490;
                    
                    if (isInSettingsMode)
                    {
                        // Add settings panel width to fixed width
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
    juce::TextButton modeButton;
    juce::TextButton bankAButton, bankBButton, bankCButton, bankDButton;
    SettingsWindow settingsWindow;
    juce::Label movementSpeedLabel;
    juce::Label windowSizeLabel;
    std::unique_ptr<juce::MidiOutput> midiOutput;
    int currentBank = 0;
    bool isEightSliderMode = false;
    bool isInSettingsMode = false;
    
    static constexpr int SLIDER_PLATE_WIDTH = 110; // Fixed slider plate width
    static constexpr int SLIDER_GAP = 10; // Gap between sliders
    static constexpr int SETTINGS_PANEL_WIDTH = 350; // Width of settings panel
    
    // Keyboard control members
    std::vector<KeyboardMapping> keyboardMappings;
    std::vector<int> movementRates;
    int currentRateIndex = 2;
    double keyboardMovementRate = 50.0; // MIDI units per second
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugMidiController)
};
