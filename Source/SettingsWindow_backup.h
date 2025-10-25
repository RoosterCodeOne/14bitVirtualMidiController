// SettingsWindow.h - Production Version with Preset System
#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"
#include "CustomLookAndFeel.h"
#include "UI/GlobalUIScale.h"

//==============================================================================
class SettingsWindow : public juce::Component
{
public:
    SettingsWindow() : controlsInitialized(false)
    {
        // Size will be controlled by parent component to match slider rack area
        // Closing is handled by the main Settings button toggle
        
        addAndMakeVisible(midiChannelLabel);
        midiChannelLabel.setText("MIDI Channel:", juce::dontSendNotification);
        midiChannelLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(midiChannelCombo);
        for (int i = 1; i <= 16; ++i)
            midiChannelCombo.addItem("Channel " + juce::String(i), i);
        midiChannelCombo.setSelectedId(1);
        midiChannelCombo.setColour(juce::ComboBox::backgroundColourId, BlueprintColors::background());
        midiChannelCombo.setColour(juce::ComboBox::textColourId, BlueprintColors::textPrimary());
        midiChannelCombo.setColour(juce::ComboBox::outlineColourId, BlueprintColors::blueprintLines());
        midiChannelCombo.onChange = [this]() {
            if (onSettingsChanged)
                onSettingsChanged();
        };
        
        // BPM controls
        addAndMakeVisible(bpmLabel);
        bpmLabel.setText("BPM:", juce::dontSendNotification);
        bpmLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(bpmSlider);
        bpmSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        bpmSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
        bpmSlider.setRange(60.0, 200.0, 1.0);
        bpmSlider.setValue(120.0); // Default BPM
        bpmSlider.setColour(juce::Slider::backgroundColourId, BlueprintColors::background());
        bpmSlider.setColour(juce::Slider::trackColourId, BlueprintColors::blueprintLines());
        bpmSlider.setColour(juce::Slider::thumbColourId, BlueprintColors::active());
        bpmSlider.setColour(juce::Slider::textBoxTextColourId, BlueprintColors::textPrimary());
        bpmSlider.setColour(juce::Slider::textBoxBackgroundColourId, BlueprintColors::background());
        bpmSlider.setColour(juce::Slider::textBoxOutlineColourId, BlueprintColors::blueprintLines());
        bpmSlider.onValueChange = [this]() {
            if (onBPMChanged)
                onBPMChanged(bpmSlider.getValue());
        };
        
        addAndMakeVisible(syncStatusLabel);
        syncStatusLabel.setText("Internal Sync", juce::dontSendNotification);
        syncStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary());
        syncStatusLabel.setFont(GlobalUIScale::getInstance().getScaledFont(10.0f));
        syncStatusLabel.setJustificationType(juce::Justification::centredRight);
        
        // Preset controls
        addAndMakeVisible(presetLabel);
        presetLabel.setText("Presets:", juce::dontSendNotification);
        presetLabel.setFont(GlobalUIScale::getInstance().getScaledFont(16.0f).boldened());
        presetLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(presetCombo);
        presetCombo.setTextWhenNothingSelected("Select preset...");
        presetCombo.setColour(juce::ComboBox::backgroundColourId, BlueprintColors::background());
        presetCombo.setColour(juce::ComboBox::textColourId, BlueprintColors::textPrimary());
        presetCombo.setColour(juce::ComboBox::outlineColourId, BlueprintColors::blueprintLines());
        refreshPresetList();
        
        addAndMakeVisible(savePresetButton);
        savePresetButton.setButtonText("Save");
        savePresetButton.setLookAndFeel(&customButtonLookAndFeel);
        savePresetButton.onClick = [this]() { showSavePresetDialog(); };
        
        addAndMakeVisible(loadPresetButton);
        loadPresetButton.setButtonText("Load");
        loadPresetButton.setLookAndFeel(&customButtonLookAndFeel);
        loadPresetButton.onClick = [this]() { loadSelectedPreset(); };
        
        addAndMakeVisible(deletePresetButton);
        deletePresetButton.setButtonText("Del");
        deletePresetButton.setLookAndFeel(&customButtonLookAndFeel);
        deletePresetButton.onClick = [this]() { deleteSelectedPreset(); };
        
        addAndMakeVisible(presetFolderLabel);
        presetFolderLabel.setText("Preset Folder:", juce::dontSendNotification);
        presetFolderLabel.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f));
        presetFolderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());

        addAndMakeVisible(presetPathLabel);
        presetPathLabel.setText("", juce::dontSendNotification);
        presetPathLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary()); // Slightly dimmed
        presetPathLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
        presetPathLabel.setJustificationType(juce::Justification::centredLeft);
        
        addAndMakeVisible(openFolderButton);
        openFolderButton.setButtonText("Open Folder");
        openFolderButton.setLookAndFeel(&customButtonLookAndFeel);
        openFolderButton.onClick = [this]() { openPresetFolder(); };

        addAndMakeVisible(changeFolderButton);
        changeFolderButton.setButtonText("Change Folder");
        changeFolderButton.setLookAndFeel(&customButtonLookAndFeel);
        changeFolderButton.onClick = [this]() { changePresetFolder(); };

        updatePresetFolderDisplay();
        
        
        addAndMakeVisible(resetToDefaultButton);
        resetToDefaultButton.setButtonText("Reset");
        resetToDefaultButton.setLookAndFeel(&customButtonLookAndFeel);
        resetToDefaultButton.onClick = [this]() { resetToDefaults(); };
        
        // Bank selector
        addAndMakeVisible(bankSelectorLabel);
        bankSelectorLabel.setText("Bank:", juce::dontSendNotification);
        bankSelectorLabel.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
        bankSelectorLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        // Bank selector buttons - modern styling
        addAndMakeVisible(bankASelector);
        bankASelector.setText("A", juce::dontSendNotification);
        bankASelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
        bankASelector.setJustificationType(juce::Justification::centred);
        bankASelector.setColour(juce::Label::backgroundColourId, BlueprintColors::active()); // Blueprint active color
        bankASelector.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        bankASelector.onClick = [this]() { cycleSliderInBank(0); };
        
        addAndMakeVisible(bankBSelector);
        bankBSelector.setText("B", juce::dontSendNotification);
        bankBSelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
        bankBSelector.setJustificationType(juce::Justification::centred);
        bankBSelector.setColour(juce::Label::backgroundColourId, BlueprintColors::inactive());
        bankBSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary());
        bankBSelector.onClick = [this]() { cycleSliderInBank(1); };
        
        addAndMakeVisible(bankCSelector);
        bankCSelector.setText("C", juce::dontSendNotification);
        bankCSelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
        bankCSelector.setJustificationType(juce::Justification::centred);
        bankCSelector.setColour(juce::Label::backgroundColourId, BlueprintColors::inactive());
        bankCSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary());
        bankCSelector.onClick = [this]() { cycleSliderInBank(2); };
        
        addAndMakeVisible(bankDSelector);
        bankDSelector.setText("D", juce::dontSendNotification);
        bankDSelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
        bankDSelector.setJustificationType(juce::Justification::centred);
        bankDSelector.setColour(juce::Label::backgroundColourId, BlueprintColors::inactive());
        bankDSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary());
        bankDSelector.onClick = [this]() { cycleSliderInBank(3); };
        
        // Breadcrumb label
        addAndMakeVisible(breadcrumbLabel);
        breadcrumbLabel.setText("Bank A > Slider 1", juce::dontSendNotification);
        breadcrumbLabel.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
        breadcrumbLabel.setColour(juce::Label::textColourId, BlueprintColors::active());
        breadcrumbLabel.setJustificationType(juce::Justification::centredLeft);
        
        // Enable keyboard focus for arrow key handling
        setWantsKeyboardFocus(true);
        
        // Initialize slider settings data with defaults
        for (int i = 0; i < 16; ++i)
        {
            // Use the constructor to ensure proper initialization
            sliderSettingsData[i] = SliderSettings();
            sliderSettingsData[i].ccNumber = i;
            
            // Set default colors based on bank
            int bankIndex = i / 4;
            switch (bankIndex)
            {
                case 0: sliderSettingsData[i].colorId = 2; break; // Red
                case 1: sliderSettingsData[i].colorId = 3; break; // Blue
                case 2: sliderSettingsData[i].colorId = 4; break; // Green
                case 3: sliderSettingsData[i].colorId = 5; break; // Yellow
                default: sliderSettingsData[i].colorId = 1; break; // Default
            }
        }
        
        setupPerSliderControls();
    }
    
    void setVisible(bool shouldBeVisible) override
    {
        if (shouldBeVisible && !controlsInitialized)
        {
            controlsInitialized = true;
        }
        
        if (shouldBeVisible)
        {
            refreshPresetList();
        }
        
        Component::setVisible(shouldBeVisible);
    }
    
    ~SettingsWindow()
    {
        // Clean up custom look and feel
        savePresetButton.setLookAndFeel(nullptr);
        loadPresetButton.setLookAndFeel(nullptr);
        deletePresetButton.setLookAndFeel(nullptr);
        resetToDefaultButton.setLookAndFeel(nullptr);
        openFolderButton.setLookAndFeel(nullptr);
        changeFolderButton.setLookAndFeel(nullptr);
    }
    
    void paint(juce::Graphics& g) override
    {
        // Blueprint aesthetic background
        auto bounds = getLocalBounds().toFloat();
        
        // Blueprint window background (slightly lighter than main background)
        g.setColour(BlueprintColors::windowBackground());
        g.fillAll();
        
        // Draw complete window outline - blueprint style
        g.setColour(BlueprintColors::blueprintLines().withAlpha(0.6f));
        g.drawRect(bounds, 1.0f);
        
        // Draw section background rectangles
        drawSectionBackgrounds(g);
        
        // Title removed to create more space for controls
        
        if (!controlsInitialized)
        {
            g.setColour(BlueprintColors::textPrimary());
            g.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f));
            g.drawText("Loading controls...", bounds, juce::Justification::centred);
            return;
        }
    }
    
    void resized() override
    {
        auto& scale = GlobalUIScale::getInstance();
        auto bounds = getLocalBounds().reduced(scale.getScaled(15)); // Scale-aware consistent padding
        
        // Calculate available height for dynamic spacing (no title space needed)
        int availableHeight = bounds.getHeight();
        int fixedHeight = scale.getScaled(10 + 16 + 6 + 22 + 6 + 20 + 10 + 16 + 5 + 16 + 7 + 20 + 10 + 20 + 6 + 22 + 8); // Scale-aware padding values + breadcrumb + bank
        if (controlsInitialized)
        {
            // Calculate height for 4 sections with proper spacing - all scaled
            int section1Height = scale.getScaled(20 + 16 + 22 + 16 + 22 + 8); // Header + CC label + input + mode label + buttons + minimal spacing
            int section2Height = scale.getScaled(20 + (16 + 2) * 3 + 8); // Header + 3 rows of controls (range combined) + minimal spacing
            int section3Height = scale.getScaled(20 + 16 + 22 + 8); // Header + mode label + buttons + minimal spacing
            int section4Height = scale.getScaled(20 + 16 + 60 + 22 + 8); // Header + color label + grid + reset button + minimal spacing
            int sectionSpacing = scale.getScaled(3 * 3); // 3 gaps between sections (further reduced) - scaled
            
            fixedHeight += section1Height + section2Height + section3Height + section4Height + sectionSpacing;
        }
        
        int flexibleSpacing = juce::jmax(scale.getScaled(3), (availableHeight - fixedHeight) / 8); // Distribute remaining space
        
        // Preset controls section with better padding alignment
        bounds.removeFromTop(10); // Increased top padding for preset section
        presetLabel.setBounds(bounds.removeFromTop(16));
        bounds.removeFromTop(6); // Increased gap between label and combo
        
        // Preset combo box and buttons on same row
        auto presetRowArea = bounds.removeFromTop(22);
        presetCombo.setBounds(presetRowArea.removeFromLeft(160));
        presetRowArea.removeFromLeft(8); // Gap between combo and buttons
        
        // 2x2 grid for preset buttons - positioned to the right of combo box
        const int buttonWidth = 40;
        const int buttonHeight = 20;
        const int buttonSpacing = 6;
        
        // Top row: Save, Load (aligned with preset combo box)
        savePresetButton.setBounds(presetRowArea.removeFromLeft(buttonWidth));
        presetRowArea.removeFromLeft(buttonSpacing);
        loadPresetButton.setBounds(presetRowArea.removeFromLeft(buttonWidth));
        
        bounds.removeFromTop(6); // Increased gap between rows
        
        // Bottom row: Delete, Reset (positioned below top row)
        auto bottomRowArea = bounds.removeFromTop(buttonHeight);
        bottomRowArea.removeFromLeft(160 + 8); // Skip combo box width + gap to align with buttons above
        deletePresetButton.setBounds(bottomRowArea.removeFromLeft(buttonWidth));
        bottomRowArea.removeFromLeft(buttonSpacing);
        resetToDefaultButton.setBounds(bottomRowArea.removeFromLeft(buttonWidth));
        
        bounds.removeFromTop(flexibleSpacing);
        
        // Folder controls section with better padding alignment
        bounds.removeFromTop(10); // Increased top padding for folder section
        presetFolderLabel.setBounds(bounds.removeFromTop(16));
        bounds.removeFromTop(5); // Increased gap
        
        auto folderPathArea = bounds.removeFromTop(16);
        presetPathLabel.setBounds(folderPathArea);
        
        bounds.removeFromTop(7); // Increased gap
        auto folderButtonArea = bounds.removeFromTop(20);
        int folderButtonWidth = (folderButtonArea.getWidth() - 8) / 2;
        openFolderButton.setBounds(folderButtonArea.removeFromLeft(folderButtonWidth));
        folderButtonArea.removeFromLeft(8);
        changeFolderButton.setBounds(folderButtonArea);
        
        bounds.removeFromTop(flexibleSpacing);
        
        // MIDI Channel section with better padding alignment
        bounds.removeFromTop(10); // Increased top padding for MIDI/Bank/Slider section
        auto channelArea = bounds.removeFromTop(22);
        midiChannelLabel.setBounds(channelArea.removeFromLeft(100));
        channelArea.removeFromLeft(8);
        midiChannelCombo.setBounds(channelArea);
        
        bounds.removeFromTop(6); // Spacing before BPM
        
        // BPM section
        auto bpmArea = bounds.removeFromTop(22);
        bpmLabel.setBounds(bpmArea.removeFromLeft(40));
        bpmArea.removeFromLeft(8);
        auto sliderArea = bpmArea.removeFromLeft(120);
        bpmSlider.setBounds(sliderArea);
        bpmArea.removeFromLeft(8);
        syncStatusLabel.setBounds(bpmArea);
        
        bounds.removeFromTop(8); // Reduced spacing
        
        if (!controlsInitialized)
            return; // Don't layout controls that don't exist yet
        
        // Breadcrumb label section
        auto breadcrumbArea = bounds.removeFromTop(20);
        breadcrumbLabel.setBounds(breadcrumbArea);
        
        bounds.removeFromTop(6); // Spacing after breadcrumb
        
        // Bank selector section
        auto bankSelectorArea = bounds.removeFromTop(22);
        bankSelectorLabel.setBounds(bankSelectorArea.removeFromLeft(40));
        bankSelectorArea.removeFromLeft(8);
        
        // Bank selector buttons with flexible spacing
        int bankButtonWidth = (bankSelectorArea.getWidth() - 21) / 4; // 3 gaps of 7px each
        bankASelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
        bankSelectorArea.removeFromLeft(7);
        bankBSelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
        bankSelectorArea.removeFromLeft(7);
        bankCSelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
        bankSelectorArea.removeFromLeft(7);
        bankDSelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
        
        bounds.removeFromTop(8); // Reduced spacing
        
        // Layout per-slider controls in 4 sections
        if (controlsInitialized)
        {
            layoutPerSliderSections(bounds);
        }
        
        // Ensure 8px bottom padding for the last section
        bounds.removeFromTop(8);
    }

    int getMidiChannel() const { return midiChannelCombo.getSelectedId(); }
    
    int getCCNumber(int sliderIndex) const
    {
        if (sliderIndex >= 0 && sliderIndex < 16)
            return sliderSettingsData[sliderIndex].ccNumber;
        return sliderIndex;
    }
    
    std::pair<double, double> getCustomRange(int sliderIndex) const
    {
        if (sliderIndex >= 0 && sliderIndex < 16)
        {
            const auto& settings = sliderSettingsData[sliderIndex];
            return {settings.rangeMin, settings.rangeMax};
        }
        return {0.0, 16383.0};
    }
    
    juce::Colour getSliderColor(int sliderIndex) const
    {
        if (sliderIndex >= 0 && sliderIndex < 16)
        {
            int colorId = sliderSettingsData[sliderIndex].colorId;
            switch (colorId)
            {
                case 2: return juce::Colours::red;
                case 3: return juce::Colours::blue;
                case 4: return juce::Colours::green;
                case 5: return juce::Colours::yellow;
                case 6: return juce::Colours::purple;
                case 7: return juce::Colours::orange;
                case 8: return juce::Colours::cyan;
                case 9: return juce::Colours::white;
                default:
                {
                    // Return default bank colors
                    int bankIndex = sliderIndex / 4;
                    switch (bankIndex)
                    {
                        case 0: return juce::Colours::red;
                        case 1: return juce::Colours::blue;
                        case 2: return juce::Colours::green;
                        case 3: return juce::Colours::yellow;
                        default: return juce::Colours::cyan;
                    }
                }
            }
        }
        return juce::Colours::cyan;
    }
    
    // Preset system interface
    // Replace the getCurrentPreset method in SettingsWindow.h:

    ControllerPreset getCurrentPreset() const
    {
        ControllerPreset preset;
        preset.name = "Current State";
        preset.midiChannel = getMidiChannel();
        
        // Read from internal slider settings data
        for (int i = 0; i < 16; ++i)
        {
            if (i < preset.sliders.size())
            {
                const auto& settings = sliderSettingsData[i];
                preset.sliders.getReference(i).ccNumber = settings.ccNumber;
                preset.sliders.getReference(i).minRange = settings.rangeMin;
                preset.sliders.getReference(i).maxRange = settings.rangeMax;
                preset.sliders.getReference(i).colorId = settings.colorId;
                
                // Note: currentValue, isLocked, delayTime, attackTime need to be set by caller from actual sliders
            }
        }
        
        return preset;
    }
    void applyPreset(const ControllerPreset& preset)
    {
        // Apply MIDI channel
        midiChannelCombo.setSelectedId(preset.midiChannel, juce::dontSendNotification);
        
        // Apply slider settings to internal data
        for (int i = 0; i < juce::jmin(16, preset.sliders.size()); ++i)
        {
            const auto& sliderPreset = preset.sliders[i];
            auto& settings = sliderSettingsData[i];
            
            settings.ccNumber = sliderPreset.ccNumber;
            settings.rangeMin = sliderPreset.minRange;
            settings.rangeMax = sliderPreset.maxRange;
            settings.colorId = sliderPreset.colorId;
        }
        
        // Update controls for currently selected slider
        if (controlsInitialized)
        {
            updateControlsForSelectedSlider();
        }
        
        // Notify that settings changed
        if (onSettingsChanged)
            onSettingsChanged();
    }
    
    PresetManager& getPresetManager() { return presetManager; }
    
    // Per-slider selection methods
    int getSelectedSlider() const { return selectedSlider; }
    int getSelectedBank() const { return selectedBank; }
    void selectSlider(int sliderIndex) { setSelectedSlider(sliderIndex); }
    
    // New per-slider settings access methods
    bool is14BitOutput(int sliderIndex) const
    {
        if (sliderIndex >= 0 && sliderIndex < 16)
            return sliderSettingsData[sliderIndex].is14Bit;
        return true;
    }
    
    
    double getIncrement(int sliderIndex) const
    {
        if (sliderIndex >= 0 && sliderIndex < 16)
            return sliderSettingsData[sliderIndex].increment;
        return 1.0;
    }
    
    bool useDeadzone(int sliderIndex) const
    {
        if (sliderIndex >= 0 && sliderIndex < 16)
            return sliderSettingsData[sliderIndex].useDeadzone;
        return true;
    }
    
    juce::String getDisplayUnit(int sliderIndex) const
    {
        if (sliderIndex >= 0 && sliderIndex < 16)
            return sliderSettingsData[sliderIndex].displayUnit;
        return juce::String();
    }
    
    // BPM management methods
    void setBPM(double bpm)
    {
        bpmSlider.setValue(bpm, juce::dontSendNotification);
    }
    
    double getBPM() const
    {
        return bpmSlider.getValue();
    }
    
    void setSyncStatus(bool isExternal, double externalBPM = 0.0)
    {
        if (isExternal && externalBPM > 0.0)
        {
            syncStatusLabel.setText("DAW Sync: " + juce::String(externalBPM, 1) + " BPM", 
                                  juce::dontSendNotification);
            syncStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::active());
        }
        else
        {
            syncStatusLabel.setText("Internal Sync", juce::dontSendNotification);
            syncStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary());
        }
    }
    
    std::function<void()> onSettingsChanged;
    std::function<void(const ControllerPreset&)> onPresetLoaded; // For slider values and lock states
    std::function<void(double)> onBPMChanged; // For BPM changes
    std::function<void(int)> onSelectedSliderChanged; // For per-slider selection notifications
    
private:
    bool controlsInitialized;
    // Close button removed - closing handled by main Settings button toggle
    juce::Label midiChannelLabel;
    juce::ComboBox midiChannelCombo;
    
    // BPM controls
    juce::Label bpmLabel;
    juce::Slider bpmSlider;
    juce::Label syncStatusLabel;
    
    // Preset controls
    juce::Label presetLabel;
    juce::ComboBox presetCombo;
    juce::TextButton savePresetButton, loadPresetButton, deletePresetButton;
    PresetManager presetManager;
    juce::Label presetFolderLabel;
    juce::Label presetPathLabel;
    juce::TextButton openFolderButton, changeFolderButton;
    juce::TextButton resetToDefaultButton;
    CustomButtonLookAndFeel customButtonLookAndFeel;

    
    juce::Label bankSelectorLabel;
    ClickableLabel bankASelector, bankBSelector, bankCSelector, bankDSelector;
    int selectedBank = 0;
    int selectedSlider = 0; // Currently selected slider (0-15)
    
    // Breadcrumb navigation
    juce::Label breadcrumbLabel;
    
    // Section headers
    juce::Label section1Header, section2Header, section3Header, section4Header;
    
    // Section 1 - Core MIDI
    juce::Label ccNumberLabel;
    juce::TextEditor ccNumberInput;
    juce::Label outputModeLabel;
    juce::ToggleButton output7BitButton, output14BitButton;
    
    // Section 2 - Display & Range
    juce::Label rangeLabel;
    juce::TextEditor rangeMinInput, rangeMaxInput;
    juce::Label rangeDashLabel;
    juce::Label displayUnitLabel;
    juce::TextEditor displayUnitInput;
    juce::Label incrementsLabel;
    juce::TextEditor incrementsInput;
    
    // Section 3 - Input Behavior
    juce::Label inputModeLabel;
    juce::ToggleButton deadzoneButton, directButton;
    
    // Section 4 - Visual
    juce::Label colorPickerLabel;
    juce::OwnedArray<juce::TextButton> colorButtons; // 4x2 grid
    juce::TextButton resetSliderButton;
    
    // Data storage for all 16 sliders
    struct SliderSettings {
        int ccNumber = 0;
        bool is14Bit = true;
        double rangeMin = 0.0;
        double rangeMax = 16383.0;
        juce::String displayUnit;
        double increment = 1.0;
        bool useDeadzone = true;
        int colorId = 1;
        
        SliderSettings()
        {
            ccNumber = 0;
            is14Bit = true;
            rangeMin = 0.0;
            rangeMax = 16383.0;
            displayUnit = juce::String();
            increment = 1.0;
            useDeadzone = true;
            colorId = 1;
        }
    };
    SliderSettings sliderSettingsData[16];
    
    void refreshPresetList()
    {
        presetCombo.clear();
        auto presetNames = presetManager.getPresetNames();
        for (int i = 0; i < presetNames.size(); ++i)
        {
            presetCombo.addItem(presetNames[i], i + 1);
        }
    }
    void showSavePresetDialog()
    {
        auto textHolder = std::make_shared<juce::String>();
        
        auto* alertWindow = new juce::AlertWindow("Save Preset",
                                                 "Enter preset name:",
                                                 juce::MessageBoxIconType::QuestionIcon);
        
        alertWindow->addTextEditor("presetName", "", "Preset Name:");
        alertWindow->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
        alertWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        alertWindow->setEscapeKeyCancels(true);
        
        alertWindow->enterModalState(true,
            juce::ModalCallbackFunction::create([this, alertWindow, textHolder](int result)
            {
                *textHolder = alertWindow->getTextEditorContents("presetName");
                
                if (result == 1 && textHolder->isNotEmpty())
                {
                    auto preset = getCurrentPreset();
                    preset.name = *textHolder;
                    
                    if (presetManager.savePreset(preset, *textHolder))
                    {
                        refreshPresetList();
                        presetCombo.setText(*textHolder, juce::dontSendNotification);
                    }
                }
            }), true);
    }

    void loadSelectedPreset()
    {
        auto selectedText = presetCombo.getText();
        if (selectedText.isNotEmpty())
        {
            auto preset = presetManager.loadPreset(selectedText);
            applyPreset(preset);
            
            if (onPresetLoaded)
                onPresetLoaded(preset);
        }
    }

    void deleteSelectedPreset()
    {
        auto selectedText = presetCombo.getText();
        if (selectedText.isNotEmpty())
        {
            juce::AlertWindow::showAsync(
                juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Delete Preset")
                    .withMessage("Are you sure you want to delete preset '" + selectedText + "'?")
                    .withButton("Delete")
                    .withButton("Cancel"),
                [this, selectedText](int result)
                {
                    if (result == 1)
                    {
                        if (presetManager.deletePreset(selectedText))
                        {
                            refreshPresetList();
                            presetCombo.clear();
                        }
                    }
                });
        }
    }
    
    void updatePresetFolderDisplay()
    {
        auto path = presetManager.getPresetDirectory().getFullPathName();
        presetPathLabel.setText(path, juce::dontSendNotification);
    }

    void openPresetFolder()
    {
        auto presetDir = presetManager.getPresetDirectory();
        if (presetDir.exists())
        {
            presetDir.revealToUser();
        }
    }


    void changePresetFolder()
    {
        auto chooser = std::make_shared<juce::FileChooser>("Choose preset folder",
                                                           presetManager.getPresetDirectory());
        
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                            [this, chooser](const juce::FileChooser&)
                            {
                                auto result = chooser->getResult();
                                if (result.exists() && result.isDirectory())
                                {
                                    presetManager.setPresetDirectory(result);
                                    updatePresetFolderDisplay();
                                    refreshPresetList();
                                }
                            });
    }
    
    void resetToDefaults()
    {
        // Reset MIDI channel
        midiChannelCombo.setSelectedId(1, juce::dontSendNotification);
        
        // Reset all slider settings to defaults in internal data
        for (int i = 0; i < 16; ++i)
        {
            auto& settings = sliderSettingsData[i];
            settings.ccNumber = i;
            settings.is14Bit = true;
            settings.rangeMin = 0.0;
            settings.rangeMax = 16383.0;
            settings.displayUnit = juce::String();
            settings.increment = 1.0;
            settings.useDeadzone = true;
            
            // Set default colors based on bank
            int bankIndex = i / 4;
            switch (bankIndex)
            {
                case 0: settings.colorId = 2; break; // Red
                case 1: settings.colorId = 3; break; // Blue
                case 2: settings.colorId = 4; break; // Green
                case 3: settings.colorId = 5; break; // Yellow
                default: settings.colorId = 1; break; // Default
            }
        }
        
        // Update controls for currently selected slider
        if (controlsInitialized)
        {
            updateControlsForSelectedSlider();
        }
        
        // Notify that settings changed
        if (onSettingsChanged)
            onSettingsChanged();
        
        // Also notify parent to reset sliders to default values/states
        if (onPresetLoaded)
        {
            ControllerPreset defaultPreset; // Creates preset with default values
            onPresetLoaded(defaultPreset);
        }
    }

    
    
    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::escapeKey)
        {
            setVisible(false);
            return true;
        }
        else if (key == juce::KeyPress::upKey)
        {
            // Up: Switch between banks A→B→C→D→A, reset to first slider in bank
            int newBank = (selectedSlider / 4 + 3) % 4; // Previous bank
            setSelectedSlider(newBank * 4);
            return true;
        }
        else if (key == juce::KeyPress::downKey)
        {
            // Down: Switch between banks A→B→C→D→A, reset to first slider in bank
            int newBank = (selectedSlider / 4 + 1) % 4; // Next bank
            setSelectedSlider(newBank * 4);
            return true;
        }
        else if (key == juce::KeyPress::leftKey)
        {
            // Left: Previous slider (wrap around)
            int newSlider = (selectedSlider + 15) % 16;
            setSelectedSlider(newSlider);
            return true;
        }
        else if (key == juce::KeyPress::rightKey)
        {
            // Right: Next slider (wrap around)
            int newSlider = (selectedSlider + 1) % 16;
            setSelectedSlider(newSlider);
            return true;
        }
        
        return Component::keyPressed(key);
    }
    
    void cycleSliderInBank(int bankIndex)
    {
        int bankStart = bankIndex * 4;
        int bankEnd = bankStart + 3;
        
        // If currently selected slider is in this bank, cycle to next slider in bank
        if (selectedSlider >= bankStart && selectedSlider <= bankEnd)
        {
            int nextSlider = selectedSlider + 1;
            if (nextSlider > bankEnd)
                nextSlider = bankStart; // Wrap to start of bank
            setSelectedSlider(nextSlider);
        }
        else
        {
            // Jump to first slider in this bank
            setSelectedSlider(bankStart);
        }
    }
    
    void setSelectedSlider(int sliderIndex)
    {
        if (sliderIndex < 0 || sliderIndex >= 16)
            return;
            
        selectedSlider = sliderIndex;
        selectedBank = selectedSlider / 4;
        
        updateBreadcrumbLabel();
        updateBankButtonAppearance();
        updateSliderVisibility();
        
        // Update controls for the newly selected slider
        if (controlsInitialized)
        {
            updateControlsForSelectedSlider();
        }
        
        if (onSelectedSliderChanged)
            onSelectedSliderChanged(selectedSlider);
    }
    
    void updateBreadcrumbLabel()
    {
        char bankLetter = 'A' + selectedBank;
        int sliderInBank = (selectedSlider % 4) + 1;
        juce::String breadcrumbText = juce::String("Bank ") + juce::String::charToString(bankLetter) + 
                                     juce::String(" > Slider ") + juce::String(sliderInBank);
        breadcrumbLabel.setText(breadcrumbText, juce::dontSendNotification);
    }
    
    void updateBankButtonAppearance()
    {
        // Update bank selector button appearances to show which bank contains selected slider
        bankASelector.setColour(juce::Label::backgroundColourId, selectedBank == 0 ? BlueprintColors::active() : BlueprintColors::inactive());
        bankASelector.setColour(juce::Label::textColourId, selectedBank == 0 ? BlueprintColors::textPrimary() : BlueprintColors::textSecondary());
        
        bankBSelector.setColour(juce::Label::backgroundColourId, selectedBank == 1 ? BlueprintColors::active() : BlueprintColors::inactive());
        bankBSelector.setColour(juce::Label::textColourId, selectedBank == 1 ? BlueprintColors::textPrimary() : BlueprintColors::textSecondary());
        
        bankCSelector.setColour(juce::Label::backgroundColourId, selectedBank == 2 ? BlueprintColors::active() : BlueprintColors::inactive());
        bankCSelector.setColour(juce::Label::textColourId, selectedBank == 2 ? BlueprintColors::textPrimary() : BlueprintColors::textSecondary());
        
        bankDSelector.setColour(juce::Label::backgroundColourId, selectedBank == 3 ? BlueprintColors::active() : BlueprintColors::inactive());
        bankDSelector.setColour(juce::Label::textColourId, selectedBank == 3 ? BlueprintColors::textPrimary() : BlueprintColors::textSecondary());
        
        repaint();
    }
    
    void setupPerSliderControls()
    {
        // Section 1 - Core MIDI
        addAndMakeVisible(section1Header);
        section1Header.setText("Core MIDI", juce::dontSendNotification);
        section1Header.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
        section1Header.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(ccNumberLabel);
        ccNumberLabel.setText("MIDI CC Number:", juce::dontSendNotification);
        ccNumberLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(ccNumberInput);
        ccNumberInput.setInputRestrictions(3, "0123456789");
        ccNumberInput.setTooltip("MIDI CC number (0-127)");
        ccNumberInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background());
        ccNumberInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary());
        ccNumberInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines());
        ccNumberInput.onReturnKey = [this]() { ccNumberInput.moveKeyboardFocusToSibling(true); };
        ccNumberInput.onFocusLost = [this]() { validateAndApplyCCNumber(); };
        
        addAndMakeVisible(outputModeLabel);
        outputModeLabel.setText("Output Mode:", juce::dontSendNotification);
        outputModeLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(output7BitButton);
        output7BitButton.setButtonText("7-bit");
        output7BitButton.setRadioGroupId(1);
        output7BitButton.onClick = [this]() { applyOutputMode(); };
        
        addAndMakeVisible(output14BitButton);
        output14BitButton.setButtonText("14-bit");
        output14BitButton.setRadioGroupId(1);
        output14BitButton.setToggleState(true, juce::dontSendNotification);
        output14BitButton.onClick = [this]() { applyOutputMode(); };
        
        // Section 2 - Display & Range
        addAndMakeVisible(section2Header);
        section2Header.setText("Display & Range", juce::dontSendNotification);
        section2Header.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
        section2Header.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(rangeLabel);
        rangeLabel.setText("Range:", juce::dontSendNotification);
        rangeLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(rangeMinInput);
        rangeMinInput.setInputRestrictions(0, "-0123456789.");
        rangeMinInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background());
        rangeMinInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary());
        rangeMinInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines());
        rangeMinInput.onReturnKey = [this]() { rangeMinInput.moveKeyboardFocusToSibling(true); };
        rangeMinInput.onFocusLost = [this]() { validateAndApplyRange(); };
        
        addAndMakeVisible(rangeDashLabel);
        rangeDashLabel.setText("-", juce::dontSendNotification);
        rangeDashLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        rangeDashLabel.setJustificationType(juce::Justification::centred);
        
        addAndMakeVisible(rangeMaxInput);
        rangeMaxInput.setInputRestrictions(0, "-0123456789.");
        rangeMaxInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background());
        rangeMaxInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary());
        rangeMaxInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines());
        rangeMaxInput.onReturnKey = [this]() { rangeMaxInput.moveKeyboardFocusToSibling(true); };
        rangeMaxInput.onFocusLost = [this]() { validateAndApplyRange(); };
        
        addAndMakeVisible(displayUnitLabel);
        displayUnitLabel.setText("Display Unit:", juce::dontSendNotification);
        displayUnitLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(displayUnitInput);
        displayUnitInput.setInputRestrictions(4); // 4 character limit
        displayUnitInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background());
        displayUnitInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary());
        displayUnitInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines());
        displayUnitInput.onReturnKey = [this]() { displayUnitInput.moveKeyboardFocusToSibling(true); };
        displayUnitInput.onFocusLost = [this]() { applyDisplayUnit(); };
        
        
        addAndMakeVisible(incrementsLabel);
        incrementsLabel.setText("Custom Steps:", juce::dontSendNotification);
        incrementsLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(incrementsInput);
        incrementsInput.setInputRestrictions(0, "0123456789.");
        incrementsInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background());
        incrementsInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary());
        incrementsInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines());
        incrementsInput.onReturnKey = [this]() { incrementsInput.moveKeyboardFocusToSibling(true); };
        incrementsInput.onFocusLost = [this]() { applyIncrements(); };
        
        // Section 3 - Input Behavior
        addAndMakeVisible(section3Header);
        section3Header.setText("Input Behavior", juce::dontSendNotification);
        section3Header.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
        section3Header.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(inputModeLabel);
        inputModeLabel.setText("MIDI Input Mode:", juce::dontSendNotification);
        inputModeLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(deadzoneButton);
        deadzoneButton.setButtonText("Deadzone");
        deadzoneButton.setRadioGroupId(2);
        deadzoneButton.setToggleState(true, juce::dontSendNotification);
        deadzoneButton.onClick = [this]() { applyInputMode(); };
        
        addAndMakeVisible(directButton);
        directButton.setButtonText("Direct");
        directButton.setRadioGroupId(2);
        directButton.onClick = [this]() { applyInputMode(); };
        
        // Section 4 - Visual
        addAndMakeVisible(section4Header);
        section4Header.setText("Visual", juce::dontSendNotification);
        section4Header.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
        section4Header.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        addAndMakeVisible(colorPickerLabel);
        colorPickerLabel.setText("Color:", juce::dontSendNotification);
        colorPickerLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
        
        // Create 4x2 color picker grid
        const juce::Colour colors[] = {
            juce::Colours::red, juce::Colours::blue, juce::Colours::green, juce::Colours::yellow,
            juce::Colours::purple, juce::Colours::orange, juce::Colours::cyan, juce::Colours::white
        };
        
        for (int i = 0; i < 8; ++i)
        {
            auto* colorButton = new juce::TextButton();
            colorButtons.add(colorButton);
            addAndMakeVisible(colorButton);
            colorButton->setColour(juce::TextButton::buttonColourId, colors[i]);
            colorButton->onClick = [this, i]() { selectColor(i + 2); }; // +2 because colorId starts at 2
        }
        
        addAndMakeVisible(resetSliderButton);
        resetSliderButton.setButtonText("Reset Slider");
        resetSliderButton.setLookAndFeel(&customButtonLookAndFeel);
        resetSliderButton.onClick = [this]() { resetCurrentSlider(); };
        
        // Only update controls if they're properly initialized
        if (controlsInitialized)
        {
            updateControlsForSelectedSlider();
        }
    }
    
    void validateAndApplyCCNumber()
    {
        auto text = ccNumberInput.getText();
        if (text.isEmpty())
        {
            ccNumberInput.setText("0", juce::dontSendNotification);
            text = "0";
        }
        
        int ccNumber = juce::jlimit(0, 127, text.getIntValue());
        ccNumberInput.setText(juce::String(ccNumber), juce::dontSendNotification);
        
        if (selectedSlider >= 0 && selectedSlider < 16)
        {
            sliderSettingsData[selectedSlider].ccNumber = ccNumber;
            if (onSettingsChanged) onSettingsChanged();
        }
    }
    
    void applyOutputMode()
    {
        if (selectedSlider >= 0 && selectedSlider < 16)
        {
            sliderSettingsData[selectedSlider].is14Bit = output14BitButton.getToggleState();
            if (onSettingsChanged) onSettingsChanged();
        }
    }
    
    void validateAndApplyRange()
    {
        auto minText = rangeMinInput.getText();
        auto maxText = rangeMaxInput.getText();
        
        if (minText.isEmpty()) { rangeMinInput.setText("0", juce::dontSendNotification); minText = "0"; }
        if (maxText.isEmpty()) { rangeMaxInput.setText("16383", juce::dontSendNotification); maxText = "16383"; }
        
        double minVal = minText.getDoubleValue();
        double maxVal = maxText.getDoubleValue();
        
        // Ensure min < max
        if (minVal >= maxVal) 
        {
            maxVal = minVal + 1;
            rangeMaxInput.setText(juce::String(maxVal, 2), juce::dontSendNotification);
        }
        
        if (selectedSlider >= 0 && selectedSlider < 16)
        {
            sliderSettingsData[selectedSlider].rangeMin = minVal;
            sliderSettingsData[selectedSlider].rangeMax = maxVal;
            if (onSettingsChanged) onSettingsChanged();
        }
    }
    
    
    void applyIncrements()
    {
        auto text = incrementsInput.getText();
        if (text.isEmpty()) { incrementsInput.setText("1", juce::dontSendNotification); text = "1"; }
        
        double increment = juce::jmax(0.001, text.getDoubleValue());
        incrementsInput.setText(juce::String(increment, 3), juce::dontSendNotification);
        
        if (selectedSlider >= 0 && selectedSlider < 16)
        {
            sliderSettingsData[selectedSlider].increment = increment;
            if (onSettingsChanged) onSettingsChanged();
        }
    }
    
    void applyInputMode()
    {
        if (selectedSlider >= 0 && selectedSlider < 16)
        {
            sliderSettingsData[selectedSlider].useDeadzone = deadzoneButton.getToggleState();
            if (onSettingsChanged) onSettingsChanged();
        }
    }
    
    void selectColor(int colorId)
    {
        if (selectedSlider >= 0 && selectedSlider < 16)
        {
            sliderSettingsData[selectedSlider].colorId = colorId;
            updateColorButtonSelection();
            if (onSettingsChanged) onSettingsChanged();
        }
    }
    
    void resetCurrentSlider()
    {
        if (selectedSlider >= 0 && selectedSlider < 16)
        {
            auto& settings = sliderSettingsData[selectedSlider];
            settings.ccNumber = selectedSlider;
            settings.is14Bit = true;
            settings.rangeMin = 0.0;
            settings.rangeMax = 16383.0;
            settings.displayUnit = juce::String();
            settings.increment = 1.0;
            settings.useDeadzone = true;
            
            // Set default color based on bank
            int bankIndex = selectedSlider / 4;
            switch (bankIndex)
            {
                case 0: settings.colorId = 2; break; // Red
                case 1: settings.colorId = 3; break; // Blue
                case 2: settings.colorId = 4; break; // Green
                case 3: settings.colorId = 5; break; // Yellow
                default: settings.colorId = 1; break; // Default
            }
            
            updateControlsForSelectedSlider();
            if (onSettingsChanged) onSettingsChanged();
        }
    }
    
    void updateControlsForSelectedSlider()
    {
        if (selectedSlider < 0 || selectedSlider >= 16) return;
        
        const auto& settings = sliderSettingsData[selectedSlider];
        
        // Update all controls with current slider's settings - with safety checks
        ccNumberInput.setText(juce::String(settings.ccNumber), juce::dontSendNotification);
        output14BitButton.setToggleState(settings.is14Bit, juce::dontSendNotification);
        output7BitButton.setToggleState(!settings.is14Bit, juce::dontSendNotification);
        
        rangeMinInput.setText(juce::String(settings.rangeMin, 2), juce::dontSendNotification);
        rangeMaxInput.setText(juce::String(settings.rangeMax, 2), juce::dontSendNotification);
        displayUnitInput.setText(settings.displayUnit, juce::dontSendNotification);
        incrementsInput.setText(juce::String(settings.increment, 3), juce::dontSendNotification);
        
        deadzoneButton.setToggleState(settings.useDeadzone, juce::dontSendNotification);
        directButton.setToggleState(!settings.useDeadzone, juce::dontSendNotification);
        
        updateColorButtonSelection();
    }
    
    void applyDisplayUnit()
    {
        if (selectedSlider >= 0 && selectedSlider < 16)
        {
            sliderSettingsData[selectedSlider].displayUnit = displayUnitInput.getText();
            if (onSettingsChanged) onSettingsChanged();
        }
    }
    
    void updateColorButtonSelection()
    {
        if (selectedSlider < 0 || selectedSlider >= 16) return;
        
        int selectedColorId = sliderSettingsData[selectedSlider].colorId;
        
        // Reset all color button appearances
        for (int i = 0; i < colorButtons.size(); ++i)
        {
            auto* button = colorButtons[i];
            bool isSelected = (i + 2) == selectedColorId; // +2 because colorId starts at 2
            
            if (isSelected)
            {
                // Add selection indication by darkening the button
                const juce::Colour colors[] = {
                    juce::Colours::red, juce::Colours::blue, juce::Colours::green, juce::Colours::yellow,
                    juce::Colours::purple, juce::Colours::orange, juce::Colours::cyan, juce::Colours::white
                };
                button->setColour(juce::TextButton::buttonColourId, colors[i].darker(0.3f));
                button->setButtonText("X"); // Add X for selection
            }
            else
            {
                // Reset to original color
                const juce::Colour colors[] = {
                    juce::Colours::red, juce::Colours::blue, juce::Colours::green, juce::Colours::yellow,
                    juce::Colours::purple, juce::Colours::orange, juce::Colours::cyan, juce::Colours::white
                };
                button->setColour(juce::TextButton::buttonColourId, colors[i]);
                button->setButtonText(""); // Remove checkmark
            }
        }
    }
    
    void layoutPerSliderSections(juce::Rectangle<int>& bounds)
    {
        const int sectionSpacing = 3; // Further reduced to prevent overflow
        const int controlSpacing = 2; // Further reduced spacing
        const int labelHeight = 16;
        const int inputHeight = 22;
        const int headerHeight = 20;
        
        // Section 1 - Core MIDI
        auto section1Bounds = bounds.removeFromTop(headerHeight + labelHeight + inputHeight + labelHeight + inputHeight + controlSpacing * 2);
        
        section1Header.setBounds(section1Bounds.removeFromTop(headerHeight));
        section1Bounds.removeFromTop(controlSpacing);
        
        // CC Number row
        auto ccRow = section1Bounds.removeFromTop(labelHeight);
        ccNumberLabel.setBounds(ccRow.removeFromLeft(120));
        ccRow.removeFromLeft(8);
        ccNumberInput.setBounds(ccRow.removeFromLeft(80));
        
        section1Bounds.removeFromTop(controlSpacing);
        
        // Output mode row
        auto outputRow = section1Bounds.removeFromTop(inputHeight);
        outputModeLabel.setBounds(outputRow.removeFromLeft(120));
        outputRow.removeFromLeft(8);
        output7BitButton.setBounds(outputRow.removeFromLeft(60));
        outputRow.removeFromLeft(8);
        output14BitButton.setBounds(outputRow.removeFromLeft(60));
        
        bounds.removeFromTop(sectionSpacing);
        
        // Section 2 - Display & Range
        auto section2Bounds = bounds.removeFromTop(headerHeight + (labelHeight + controlSpacing) * 3 + controlSpacing);
        
        section2Header.setBounds(section2Bounds.removeFromTop(headerHeight));
        section2Bounds.removeFromTop(controlSpacing);
        
        // Range row (combined min - max on one line)
        auto rangeRow = section2Bounds.removeFromTop(labelHeight);
        rangeLabel.setBounds(rangeRow.removeFromLeft(50));
        rangeRow.removeFromLeft(4);
        rangeMinInput.setBounds(rangeRow.removeFromLeft(80));
        rangeRow.removeFromLeft(2);
        rangeDashLabel.setBounds(rangeRow.removeFromLeft(10));
        rangeRow.removeFromLeft(2);
        rangeMaxInput.setBounds(rangeRow.removeFromLeft(80));
        
        section2Bounds.removeFromTop(controlSpacing);
        
        // Display Unit row
        auto unitRow = section2Bounds.removeFromTop(labelHeight);
        displayUnitLabel.setBounds(unitRow.removeFromLeft(120));
        unitRow.removeFromLeft(8);
        displayUnitInput.setBounds(unitRow.removeFromLeft(60));
        
        section2Bounds.removeFromTop(controlSpacing);
        
        // Increments row
        auto incrementRow = section2Bounds.removeFromTop(labelHeight);
        incrementsLabel.setBounds(incrementRow.removeFromLeft(120));
        incrementRow.removeFromLeft(8);
        incrementsInput.setBounds(incrementRow.removeFromLeft(100));
        
        bounds.removeFromTop(sectionSpacing);
        
        // Section 3 - Input Behavior
        auto section3Bounds = bounds.removeFromTop(headerHeight + labelHeight + inputHeight + controlSpacing);
        
        section3Header.setBounds(section3Bounds.removeFromTop(headerHeight));
        section3Bounds.removeFromTop(controlSpacing);
        
        auto inputModeRow = section3Bounds.removeFromTop(labelHeight);
        inputModeLabel.setBounds(inputModeRow.removeFromLeft(120));
        inputModeRow.removeFromLeft(8);
        deadzoneButton.setBounds(inputModeRow.removeFromLeft(80));
        inputModeRow.removeFromLeft(8);
        directButton.setBounds(inputModeRow.removeFromLeft(60));
        
        bounds.removeFromTop(sectionSpacing);
        
        // Section 4 - Visual
        auto section4Bounds = bounds.removeFromTop(headerHeight + labelHeight + 60 + inputHeight + controlSpacing); // 60px for color grid, minimal spacing
        
        section4Header.setBounds(section4Bounds.removeFromTop(headerHeight));
        section4Bounds.removeFromTop(controlSpacing);
        
        colorPickerLabel.setBounds(section4Bounds.removeFromTop(labelHeight));
        section4Bounds.removeFromTop(controlSpacing);
        
        // Color picker grid (4x2)
        auto colorArea = section4Bounds.removeFromTop(60);
        const int buttonSize = 25;
        const int buttonGap = 8;
        
        for (int row = 0; row < 2; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                int index = row * 4 + col;
                if (index < colorButtons.size())
                {
                    int x = col * (buttonSize + buttonGap);
                    int y = row * (buttonSize + buttonGap);
                    colorButtons[index]->setBounds(x, y + colorArea.getY(), buttonSize, buttonSize);
                }
            }
        }
        
        section4Bounds.removeFromTop(controlSpacing);
        resetSliderButton.setBounds(section4Bounds.removeFromTop(inputHeight).removeFromLeft(120));
    }
    
    void updateSliderVisibility()
    {
        if (!controlsInitialized)
            return;
            
        // The new per-slider controls are always visible when controlsInitialized is true
        // Individual controls are updated via updateControlsForSelectedSlider()
        
        resized();
        repaint();
    }
    
    
    
    void drawSectionBackgrounds(juce::Graphics& g)
    {
        auto& scale = GlobalUIScale::getInstance();
        auto bounds = getLocalBounds().reduced(scale.getScaled(15)); // Same scaled padding as resized()
        
        // Calculate section bounds using same logic as resized() - all scaled
        int availableHeight = bounds.getHeight();
        int fixedHeight = scale.getScaled(10 + 16 + 6 + 22 + 6 + 20 + 10 + 16 + 5 + 16 + 7 + 20 + 10 + 20 + 6 + 22 + 8); // Scaled padding values
        if (controlsInitialized)
            fixedHeight += scale.getScaled(18 + 4 + 26 + 8);
        
        int flexibleSpacing = juce::jmax(scale.getScaled(3), (availableHeight - fixedHeight) / 8);
        
        // Top section (Preset controls) - blueprint background and outline style
        auto topSectionBounds = bounds.removeFromTop(scale.getScaled(10 + 16 + 6 + 22 + 6 + 20)); // Scaled padding values
        topSectionBounds = topSectionBounds.expanded(scale.getScaled(5), 0).withTrimmedBottom(scale.getScaled(1)).withBottom(topSectionBounds.getBottom() + scale.getScaled(4)); // Scaled margins and gaps
        
        g.setColour(BlueprintColors::sectionBackground());
        g.fillRect(topSectionBounds.toFloat());
        g.setColour(BlueprintColors::blueprintLines().withAlpha(0.6f));
        g.drawRect(topSectionBounds.toFloat(), scale.getScaled(1.0f));
        
        // Skip flexible spacing
        bounds.removeFromTop(flexibleSpacing);
        
        // Middle section (Preset folder controls) - blueprint background and outline style
        auto middleSectionBounds = bounds.removeFromTop(scale.getScaled(10 + 16 + 5 + 16 + 7 + 20)); // Scaled folder section height
        middleSectionBounds = middleSectionBounds.expanded(scale.getScaled(5), 0).withTrimmedTop(scale.getScaled(1)).withTrimmedBottom(scale.getScaled(1)); // Scaled margins and gaps
        middleSectionBounds = middleSectionBounds.withTop(middleSectionBounds.getY() - scale.getScaled(2)).withBottom(middleSectionBounds.getBottom() + scale.getScaled(6)); // Scaled positioning
        
        g.setColour(BlueprintColors::sectionBackground());
        g.fillRect(middleSectionBounds.toFloat());
        g.setColour(BlueprintColors::blueprintLines().withAlpha(0.6f));
        g.drawRect(middleSectionBounds.toFloat(), scale.getScaled(1.0f));
        
        // Skip flexible spacing
        bounds.removeFromTop(flexibleSpacing);
        
        // Bottom section (MIDI Channel + Breadcrumb + Bank + Slider controls) - blueprint background and outline style
        auto bottomSectionHeight = scale.getScaled(10 + 22 + 8 + 20 + 6 + 22 + 8); // Scaled MIDI + spacing + breadcrumb + bank
        if (controlsInitialized)
        {
            // Calculate height for 4 sections with proper spacing - all scaled
            int section1Height = scale.getScaled(20 + 16 + 22 + 16 + 22 + 8); // Header + CC label + input + mode label + buttons + minimal spacing
            int section2Height = scale.getScaled(20 + (16 + 2) * 3 + 8); // Header + 3 rows of controls (range combined) + minimal spacing
            int section3Height = scale.getScaled(20 + 16 + 22 + 8); // Header + mode label + buttons + minimal spacing
            int section4Height = scale.getScaled(20 + 16 + 60 + 22 + 8); // Header + color label + grid + reset button + minimal spacing
            int sectionSpacing = scale.getScaled(3 * 3); // 3 gaps between sections (further reduced) - scaled
            
            bottomSectionHeight += section1Height + section2Height + section3Height + section4Height + sectionSpacing;
        }
        
        auto bottomSectionBounds = bounds.removeFromTop(bottomSectionHeight);
        bottomSectionBounds = bottomSectionBounds.expanded(scale.getScaled(5), 0).withTrimmedTop(scale.getScaled(1)).withBottom(bottomSectionBounds.getBottom() + scale.getScaled(5)); // Scaled margins and positioning
        
        g.setColour(BlueprintColors::sectionBackground());
        g.fillRect(bottomSectionBounds.toFloat());
        g.setColour(BlueprintColors::blueprintLines().withAlpha(0.6f));
        g.drawRect(bottomSectionBounds.toFloat(), scale.getScaled(1.0f));
    }
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
};
