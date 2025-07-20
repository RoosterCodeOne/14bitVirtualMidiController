// SettingsWindow.h - Production Version with Preset System
#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"
#include "CustomLookAndFeel.h"

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
        midiChannelLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        
        addAndMakeVisible(midiChannelCombo);
        for (int i = 1; i <= 16; ++i)
            midiChannelCombo.addItem("Channel " + juce::String(i), i);
        midiChannelCombo.setSelectedId(1);
        midiChannelCombo.setColour(juce::ComboBox::backgroundColourId, BlueprintColors::background);
        midiChannelCombo.setColour(juce::ComboBox::textColourId, BlueprintColors::textPrimary);
        midiChannelCombo.setColour(juce::ComboBox::outlineColourId, BlueprintColors::blueprintLines);
        midiChannelCombo.onChange = [this]() {
            if (onSettingsChanged)
                onSettingsChanged();
        };
        
        // Preset controls
        addAndMakeVisible(presetLabel);
        presetLabel.setText("Presets:", juce::dontSendNotification);
        presetLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        presetLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        
        addAndMakeVisible(presetCombo);
        presetCombo.setTextWhenNothingSelected("Select preset...");
        presetCombo.setColour(juce::ComboBox::backgroundColourId, BlueprintColors::background);
        presetCombo.setColour(juce::ComboBox::textColourId, BlueprintColors::textPrimary);
        presetCombo.setColour(juce::ComboBox::outlineColourId, BlueprintColors::blueprintLines);
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
        presetFolderLabel.setFont(juce::FontOptions(14.0f));
        presetFolderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);

        addAndMakeVisible(presetPathLabel);
        presetPathLabel.setText("", juce::dontSendNotification);
        presetPathLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary); // Slightly dimmed
        presetPathLabel.setFont(juce::FontOptions(12.0f));
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
        
        // Column headers for slider controls
        addAndMakeVisible(ccValueHeaderLabel);
        ccValueHeaderLabel.setText("CC Value", juce::dontSendNotification);
        ccValueHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        ccValueHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        ccValueHeaderLabel.setJustificationType(juce::Justification::centred);
        
        addAndMakeVisible(rangeHeaderLabel);
        rangeHeaderLabel.setText("Range", juce::dontSendNotification);
        rangeHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        rangeHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        rangeHeaderLabel.setJustificationType(juce::Justification::centred);
        
        addAndMakeVisible(colorHeaderLabel);
        colorHeaderLabel.setText("Color", juce::dontSendNotification);
        colorHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        colorHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        colorHeaderLabel.setJustificationType(juce::Justification::centred);
        
        addAndMakeVisible(resetToDefaultButton);
        resetToDefaultButton.setButtonText("Reset");
        resetToDefaultButton.setLookAndFeel(&customButtonLookAndFeel);
        resetToDefaultButton.onClick = [this]() { resetToDefaults(); };
        
        // Bank selector
        addAndMakeVisible(bankSelectorLabel);
        bankSelectorLabel.setText("Bank:", juce::dontSendNotification);
        bankSelectorLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        bankSelectorLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        
        // Bank selector buttons - modern styling
        addAndMakeVisible(bankASelector);
        bankASelector.setText("A", juce::dontSendNotification);
        bankASelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        bankASelector.setJustificationType(juce::Justification::centred);
        bankASelector.setColour(juce::Label::backgroundColourId, BlueprintColors::active); // Blueprint active color
        bankASelector.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        bankASelector.onClick = [this]() { setSelectedBank(0); };
        
        addAndMakeVisible(bankBSelector);
        bankBSelector.setText("B", juce::dontSendNotification);
        bankBSelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        bankBSelector.setJustificationType(juce::Justification::centred);
        bankBSelector.setColour(juce::Label::backgroundColourId, BlueprintColors::inactive);
        bankBSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        bankBSelector.onClick = [this]() { setSelectedBank(1); };
        
        addAndMakeVisible(bankCSelector);
        bankCSelector.setText("C", juce::dontSendNotification);
        bankCSelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        bankCSelector.setJustificationType(juce::Justification::centred);
        bankCSelector.setColour(juce::Label::backgroundColourId, BlueprintColors::inactive);
        bankCSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        bankCSelector.onClick = [this]() { setSelectedBank(2); };
        
        addAndMakeVisible(bankDSelector);
        bankDSelector.setText("D", juce::dontSendNotification);
        bankDSelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        bankDSelector.setJustificationType(juce::Justification::centred);
        bankDSelector.setColour(juce::Label::backgroundColourId, BlueprintColors::inactive);
        bankDSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        bankDSelector.onClick = [this]() { setSelectedBank(3); };
    }
    
    void setVisible(bool shouldBeVisible) override
    {
        if (shouldBeVisible && !controlsInitialized)
        {
            initializeSliderControls();
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
        g.setColour(BlueprintColors::windowBackground);
        g.fillAll();
        
        // Draw complete window outline - blueprint style
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        g.drawRect(bounds, 1.0f);
        
        // Draw section background rectangles
        drawSectionBackgrounds(g);
        
        // Title removed to create more space for controls
        
        if (!controlsInitialized)
        {
            g.setColour(BlueprintColors::textPrimary);
            g.setFont(juce::FontOptions(14.0f));
            g.drawText("Loading controls...", bounds, juce::Justification::centred);
            return;
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(15); // Modern consistent padding
        
        // Calculate available height for dynamic spacing (no title space needed)
        int availableHeight = bounds.getHeight();
        int fixedHeight = 10 + 16 + 6 + 22 + 6 + 20 + 10 + 16 + 5 + 16 + 7 + 20 + 10 + 22 + 8 + 22 + 8; // Updated padding values
        if (controlsInitialized)
            fixedHeight += 18 + 4 + (4 * 26) + (4 * 3) + 8; // Headers + gap + 4 slider rows at 26px each + row spacing + 8px bottom padding
        
        int flexibleSpacing = juce::jmax(3, (availableHeight - fixedHeight) / 8); // Distribute remaining space
        
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
        
        bounds.removeFromTop(8); // Reduced spacing
        
        if (!controlsInitialized)
            return; // Don't layout controls that don't exist yet
        
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
        
        // Column headers for slider controls - positioned above the slider rows
        if (controlsInitialized)
        {
            auto headerArea = bounds.removeFromTop(18); // Height for column headers
            int availableWidth = headerArea.getWidth();
            
            // Calculate column positions to match layoutSliderRow
            int sliderLabelWidth = juce::jmin(100, availableWidth / 5);
            int ccInputWidth = juce::jmin(60, availableWidth / 8);
            int rangeColumnWidth = juce::jmin(120, availableWidth / 3);
            int colorWidth = juce::jmin(80, availableWidth / 5);
            
            // Skip slider label area
            headerArea.removeFromLeft(sliderLabelWidth + 6);
            
            // CC Value header
            ccValueHeaderLabel.setBounds(headerArea.removeFromLeft(ccInputWidth));
            headerArea.removeFromLeft(8);
            
            // Range header
            rangeHeaderLabel.setBounds(headerArea.removeFromLeft(rangeColumnWidth));
            headerArea.removeFromLeft(8);
            
            // Color header
            colorHeaderLabel.setBounds(headerArea.removeFromLeft(colorWidth));
            
            bounds.removeFromTop(4); // Small gap between headers and first row
        }
        
        // Controls for current bank only (4 sliders) - more space now available
        for (int i = 0; i < 4; ++i)
        {
            int sliderIndex = selectedBank * 4 + i;
            layoutSliderRow(bounds, sliderIndex);
        }
        
        // Ensure 8px bottom padding for the last section
        bounds.removeFromTop(8);
    }

    int getMidiChannel() const { return midiChannelCombo.getSelectedId(); }
    
    int getCCNumber(int sliderIndex) const
    {
        if (!controlsInitialized) return sliderIndex;
        
        if (sliderIndex < ccInputs.size())
        {
            auto text = ccInputs[sliderIndex]->getText();
            int ccNumber = text.getIntValue();
            return juce::jlimit(0, 127, ccNumber);
        }
        return sliderIndex;
    }
    
    std::pair<double, double> getCustomRange(int sliderIndex) const
    {
        if (!controlsInitialized) return {0.0, 16383.0};
        
        if (sliderIndex < minRangeInputs.size() && sliderIndex < maxRangeInputs.size())
        {
            double minVal = minRangeInputs[sliderIndex]->getText().getDoubleValue();
            double maxVal = maxRangeInputs[sliderIndex]->getText().getDoubleValue();
            return {minVal, maxVal};
        }
        return {0.0, 16383.0};
    }
    
    juce::Colour getSliderColor(int sliderIndex) const
    {
        if (!controlsInitialized)
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
        
        if (sliderIndex < colorCombos.size())
        {
            switch (colorCombos[sliderIndex]->getSelectedId())
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
        
        // IMPORTANT: Only read settings if controls are initialized
        if (controlsInitialized)
        {
            for (int i = 0; i < 16; ++i)
            {
                if (i < preset.sliders.size())
                {
                    preset.sliders.getReference(i).ccNumber = getCCNumber(i);
                    auto range = getCustomRange(i);
                    preset.sliders.getReference(i).minRange = range.first;
                    preset.sliders.getReference(i).maxRange = range.second;
                    
                    // Get color ID from combo
                    if (i < colorCombos.size())
                        preset.sliders.getReference(i).colorId = colorCombos[i]->getSelectedId();
                    
                    // Note: currentValue, isLocked, delayTime, attackTime need to be set by caller from actual sliders
                }
            }
        }
        else
        {
            // If controls aren't initialized, provide defaults
            for (int i = 0; i < 16; ++i)
            {
                if (i < preset.sliders.size())
                {
                    preset.sliders.getReference(i).ccNumber = i; // Default CC numbers
                    preset.sliders.getReference(i).minRange = 0.0;
                    preset.sliders.getReference(i).maxRange = 16383.0;
                    
                    // Set default colors based on bank
                    int bankIndex = i / 4;
                    switch (bankIndex)
                    {
                        case 0: preset.sliders.getReference(i).colorId = 2; break; // Red
                        case 1: preset.sliders.getReference(i).colorId = 3; break; // Blue
                        case 2: preset.sliders.getReference(i).colorId = 4; break; // Green
                        case 3: preset.sliders.getReference(i).colorId = 5; break; // Yellow
                        default: preset.sliders.getReference(i).colorId = 1; break; // Default
                    }
                }
            }
        }
        
        return preset;
    }
    void applyPreset(const ControllerPreset& preset)
    {
        if (!controlsInitialized)
            return;
            
        // Apply MIDI channel
        midiChannelCombo.setSelectedId(preset.midiChannel, juce::dontSendNotification);
        
        // Apply slider settings
        for (int i = 0; i < juce::jmin(16, preset.sliders.size()); ++i)
        {
            const auto& sliderPreset = preset.sliders[i];
            
            if (i < ccInputs.size())
                ccInputs[i]->setText(juce::String(sliderPreset.ccNumber), juce::dontSendNotification);
                
            if (i < minRangeInputs.size())
                minRangeInputs[i]->setText(juce::String(sliderPreset.minRange), juce::dontSendNotification);
                
            if (i < maxRangeInputs.size())
                maxRangeInputs[i]->setText(juce::String(sliderPreset.maxRange), juce::dontSendNotification);
                
            if (i < colorCombos.size())
                colorCombos[i]->setSelectedId(sliderPreset.colorId, juce::dontSendNotification);
        }
        
        // Notify that settings changed
        if (onSettingsChanged)
            onSettingsChanged();
    }
    
    PresetManager& getPresetManager() { return presetManager; }
    
    std::function<void()> onSettingsChanged;
    std::function<void(const ControllerPreset&)> onPresetLoaded; // For slider values and lock states
    
private:
    bool controlsInitialized;
    // Close button removed - closing handled by main Settings button toggle
    juce::Label midiChannelLabel;
    juce::ComboBox midiChannelCombo;
    
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
    
    // Column headers for slider controls
    juce::Label ccValueHeaderLabel;
    juce::Label rangeHeaderLabel;
    juce::Label colorHeaderLabel;
    
    juce::OwnedArray<juce::Label> sliderLabels;
    juce::OwnedArray<juce::TextEditor> ccInputs;
    juce::OwnedArray<juce::Label> rangeLabels;
    juce::OwnedArray<juce::TextEditor> minRangeInputs;
    juce::OwnedArray<juce::TextEditor> maxRangeInputs;
    juce::OwnedArray<juce::Label> colorLabels;
    juce::OwnedArray<juce::ComboBox> colorCombos;
    
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
        
        // Reset all slider settings to defaults
        if (controlsInitialized)
        {
            for (int i = 0; i < 16; ++i)
            {
                if (i < ccInputs.size())
                    ccInputs[i]->setText(juce::String(i), juce::dontSendNotification);
                    
                if (i < minRangeInputs.size())
                    minRangeInputs[i]->setText("0", juce::dontSendNotification);
                    
                if (i < maxRangeInputs.size())
                    maxRangeInputs[i]->setText("16383", juce::dontSendNotification);
                    
                if (i < colorCombos.size())
                {
                    // Set default colors based on bank
                    int bankIndex = i / 4;
                    switch (bankIndex)
                    {
                        case 0: colorCombos[i]->setSelectedId(2, juce::dontSendNotification); break; // Red
                        case 1: colorCombos[i]->setSelectedId(3, juce::dontSendNotification); break; // Blue
                        case 2: colorCombos[i]->setSelectedId(4, juce::dontSendNotification); break; // Green
                        case 3: colorCombos[i]->setSelectedId(5, juce::dontSendNotification); break; // Yellow
                        default: colorCombos[i]->setSelectedId(1, juce::dontSendNotification); break; // Default
                    }
                }
            }
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

    void layoutSliderRow(juce::Rectangle<int>& bounds, int sliderIndex)
    {
        auto row = bounds.removeFromTop(26); // Reduced height since no per-row labels
        int availableWidth = row.getWidth();
        
        // Calculate column widths to align with headers
        int sliderLabelWidth = juce::jmin(100, availableWidth / 5);
        int ccInputWidth = juce::jmin(60, availableWidth / 8);
        int rangeColumnWidth = juce::jmin(120, availableWidth / 3); // Space for both min and max inputs
        int colorWidth = juce::jmin(80, availableWidth / 5);
        
        // Slider X: (just the slider number label)
        sliderLabels[sliderIndex]->setBounds(row.removeFromLeft(sliderLabelWidth));
        row.removeFromLeft(6); // Gap
        
        // CC Value input (aligned with CC Value header)
        ccInputs[sliderIndex]->setBounds(row.removeFromLeft(ccInputWidth));
        row.removeFromLeft(8); // Gap
        
        // Range inputs (aligned with Range header)
        auto rangeArea = row.removeFromLeft(rangeColumnWidth);
        int rangeInputWidth = (rangeColumnWidth - 8) / 2; // Split range area in half with 8px gap
        minRangeInputs[sliderIndex]->setBounds(rangeArea.removeFromLeft(rangeInputWidth));
        rangeArea.removeFromLeft(8); // Gap between min and max
        maxRangeInputs[sliderIndex]->setBounds(rangeArea);
        
        row.removeFromLeft(8); // Gap
        
        // Color combo (aligned with Color header)
        colorCombos[sliderIndex]->setBounds(row.removeFromLeft(colorWidth));
        
        bounds.removeFromTop(3); // Reduced row spacing
    }
    
    void initializeSliderControls()
    {
        // Create controls for all 16 sliders
        for (int i = 0; i < 16; ++i)
        {
            // SLIDER X: label (simplified - no "CC Value:" text)
            auto* sliderLabel = new juce::Label();
            sliderLabels.add(sliderLabel);
            addAndMakeVisible(sliderLabel);
            sliderLabel->setText("Slider " + juce::String(i + 1) + ":", juce::dontSendNotification);
            sliderLabel->setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
            
            // CC input
            auto* ccInput = new juce::TextEditor();
            ccInputs.add(ccInput);
            addAndMakeVisible(ccInput);
            ccInput->setText(juce::String(i), juce::dontSendNotification);
            ccInput->setInputRestrictions(3, "0123456789");
            ccInput->setTooltip("MIDI CC number (0-127)");
            ccInput->setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
            ccInput->setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
            ccInput->setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
            ccInput->onReturnKey = [this, ccInput]() { validateCCInput(ccInput); };
            ccInput->onFocusLost = [this, ccInput]() { validateCCInput(ccInput); };
            
            // Range: label (kept for compatibility but will be hidden)
            auto* rangeLabel = new juce::Label();
            rangeLabels.add(rangeLabel);
            addAndMakeVisible(rangeLabel);
            rangeLabel->setText("", juce::dontSendNotification); // Empty text since header replaces it
            rangeLabel->setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
            
            // Min range input
            auto* minInput = new juce::TextEditor();
            minRangeInputs.add(minInput);
            addAndMakeVisible(minInput);
            minInput->setText("0");
            minInput->setInputRestrictions(0, "-0123456789.");
            minInput->setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
            minInput->setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
            minInput->setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
            minInput->onReturnKey = [this, minInput]() { validateRangeInput(minInput); };
            minInput->onFocusLost = [this, minInput]() { validateRangeInput(minInput); };
            
            // Max range input
            auto* maxInput = new juce::TextEditor();
            maxRangeInputs.add(maxInput);
            addAndMakeVisible(maxInput);
            maxInput->setText("16383");
            maxInput->setInputRestrictions(0, "-0123456789.");
            maxInput->setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
            maxInput->setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
            maxInput->setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
            maxInput->onReturnKey = [this, maxInput]() { validateRangeInput(maxInput); };
            maxInput->onFocusLost = [this, maxInput]() { validateRangeInput(maxInput); };
            
            // Color: label (kept for compatibility but will be hidden)
            auto* colorLabel = new juce::Label();
            colorLabels.add(colorLabel);
            addAndMakeVisible(colorLabel);
            colorLabel->setText("", juce::dontSendNotification); // Empty text since header replaces it
            colorLabel->setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
            
            // Color selector
            auto* colorCombo = new juce::ComboBox();
            colorCombos.add(colorCombo);
            addAndMakeVisible(colorCombo);
            colorCombo->setColour(juce::ComboBox::backgroundColourId, BlueprintColors::background);
            colorCombo->setColour(juce::ComboBox::textColourId, BlueprintColors::textPrimary);
            colorCombo->setColour(juce::ComboBox::outlineColourId, BlueprintColors::blueprintLines);
            
            colorCombo->addItem("Default", 1);
            colorCombo->addItem("Red", 2);
            colorCombo->addItem("Blue", 3);
            colorCombo->addItem("Green", 4);
            colorCombo->addItem("Yellow", 5);
            colorCombo->addItem("Purple", 6);
            colorCombo->addItem("Orange", 7);
            colorCombo->addItem("Cyan", 8);
            colorCombo->addItem("White", 9);
            
            colorCombo->setSelectedId(1);
            
            // Add callback to notify when color changes
            colorCombo->onChange = [this]() {
                if (onSettingsChanged)
                    onSettingsChanged();
            };
        }
        
        controlsInitialized = true;
        
        // Set initial bank visibility
        setSelectedBank(0);
        
        resized();
        repaint();
        
        if (onSettingsChanged)
            onSettingsChanged();
    }
    
    void setSelectedBank(int bank)
    {
        selectedBank = bank;
        
        // Update bank selector button appearances
        bankASelector.setColour(juce::Label::backgroundColourId, bank == 0 ? juce::Colours::red : juce::Colours::darkgrey);
        bankASelector.setColour(juce::Label::textColourId, bank == 0 ? juce::Colours::white : juce::Colours::lightgrey);
        
        bankBSelector.setColour(juce::Label::backgroundColourId, bank == 1 ? juce::Colours::blue : juce::Colours::darkgrey);
        bankBSelector.setColour(juce::Label::textColourId, bank == 1 ? juce::Colours::white : juce::Colours::lightgrey);
        
        bankCSelector.setColour(juce::Label::backgroundColourId, bank == 2 ? juce::Colours::green : juce::Colours::darkgrey);
        bankCSelector.setColour(juce::Label::textColourId, bank == 2 ? juce::Colours::white : juce::Colours::lightgrey);
        
        bankDSelector.setColour(juce::Label::backgroundColourId, bank == 3 ? juce::Colours::yellow : juce::Colours::darkgrey);
        bankDSelector.setColour(juce::Label::textColourId, bank == 3 ? juce::Colours::black : juce::Colours::lightgrey);
        
        // Show/hide appropriate slider controls
        if (controlsInitialized)
        {
            // Show column headers when any slider is visible
            bool hasVisibleSliders = false;
            
            for (int i = 0; i < 16; ++i)
            {
                bool shouldBeVisible = (i >= selectedBank * 4) && (i < (selectedBank + 1) * 4);
                
                if (shouldBeVisible)
                    hasVisibleSliders = true;
                
                if (i < sliderLabels.size()) sliderLabels[i]->setVisible(shouldBeVisible);
                if (i < ccInputs.size()) ccInputs[i]->setVisible(shouldBeVisible);
                if (i < rangeLabels.size()) rangeLabels[i]->setVisible(false); // Always hidden - replaced by header
                if (i < minRangeInputs.size()) minRangeInputs[i]->setVisible(shouldBeVisible);
                if (i < maxRangeInputs.size()) maxRangeInputs[i]->setVisible(shouldBeVisible);
                if (i < colorLabels.size()) colorLabels[i]->setVisible(false); // Always hidden - replaced by header
                if (i < colorCombos.size()) colorCombos[i]->setVisible(shouldBeVisible);
            }
            
            // Show/hide column headers based on whether any sliders are visible
            ccValueHeaderLabel.setVisible(hasVisibleSliders);
            rangeHeaderLabel.setVisible(hasVisibleSliders);
            colorHeaderLabel.setVisible(hasVisibleSliders);
        }
        
        resized();
        repaint();
    }
    
    void validateCCInput(juce::TextEditor* input)
    {
        auto text = input->getText();
        if (text.isEmpty())
        {
            input->setText("0", juce::dontSendNotification);
            return;
        }
        
        int ccNumber = text.getIntValue();
        ccNumber = juce::jlimit(0, 127, ccNumber);
        input->setText(juce::String(ccNumber), juce::dontSendNotification);
        
        if (onSettingsChanged)
            onSettingsChanged();
    }
    
    void validateRangeInput(juce::TextEditor* input)
    {
        auto text = input->getText();
        if (text.isEmpty())
        {
            input->setText("0", juce::dontSendNotification);
            return;
        }
        
        double value = text.getDoubleValue();
        value = juce::jlimit(-999999.0, 999999.0, value);
        input->setText(juce::String(value, 2), juce::dontSendNotification);
        
        if (onSettingsChanged)
            onSettingsChanged();
    }
    
    void drawSectionBackgrounds(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().reduced(15); // Same padding as resized()
        
        // Calculate section bounds using same logic as resized()
        int availableHeight = bounds.getHeight();
        int fixedHeight = 10 + 16 + 6 + 22 + 6 + 20 + 10 + 16 + 5 + 16 + 7 + 20 + 10 + 22 + 8 + 22 + 8; // Updated padding values
        if (controlsInitialized)
            fixedHeight += 18 + 4 + (4 * 26) + (4 * 3) + 8;
        
        int flexibleSpacing = juce::jmax(3, (availableHeight - fixedHeight) / 8);
        
        // Top section (Preset controls) - blueprint background and outline style
        auto topSectionBounds = bounds.removeFromTop(10 + 16 + 6 + 22 + 6 + 20); // Updated padding values
        topSectionBounds = topSectionBounds.expanded(5, 0).withTrimmedBottom(1).withBottom(topSectionBounds.getBottom() + 4); // 5px left/right margin, 1px bottom gap, extend bottom by 4px
        
        g.setColour(BlueprintColors::sectionBackground);
        g.fillRect(topSectionBounds.toFloat());
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        g.drawRect(topSectionBounds.toFloat(), 1.0f);
        
        // Skip flexible spacing
        bounds.removeFromTop(flexibleSpacing);
        
        // Middle section (Preset folder controls) - blueprint background and outline style
        auto middleSectionBounds = bounds.removeFromTop(10 + 16 + 5 + 16 + 7 + 20); // Updated folder section height
        middleSectionBounds = middleSectionBounds.expanded(5, 0).withTrimmedTop(1).withTrimmedBottom(1); // 5px left/right margin, 1px top/bottom gaps
        middleSectionBounds = middleSectionBounds.withTop(middleSectionBounds.getY() - 2).withBottom(middleSectionBounds.getBottom() + 6); // Raise top by 2px, extend bottom by 6px
        
        g.setColour(BlueprintColors::sectionBackground);
        g.fillRect(middleSectionBounds.toFloat());
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        g.drawRect(middleSectionBounds.toFloat(), 1.0f);
        
        // Skip flexible spacing
        bounds.removeFromTop(flexibleSpacing);
        
        // Bottom section (MIDI Channel + Bank + Slider controls) - blueprint background and outline style
        auto bottomSectionHeight = 10 + 22 + 8 + 22 + 8; // Updated MIDI + spacing + Bank
        if (controlsInitialized)
            bottomSectionHeight += 18 + 4 + (4 * 26) + (4 * 3) + 8; // Headers + sliders
        
        auto bottomSectionBounds = bounds.removeFromTop(bottomSectionHeight);
        bottomSectionBounds = bottomSectionBounds.expanded(5, 0).withTrimmedTop(1).withBottom(bottomSectionBounds.getBottom() + 5); // 5px left/right margin, 1px top gap, extend bottom by 5px
        
        g.setColour(BlueprintColors::sectionBackground);
        g.fillRect(bottomSectionBounds.toFloat());
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        g.drawRect(bottomSectionBounds.toFloat(), 1.0f);
    }
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
};
