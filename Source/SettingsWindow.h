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
        midiChannelLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
        
        addAndMakeVisible(midiChannelCombo);
        for (int i = 1; i <= 16; ++i)
            midiChannelCombo.addItem("Channel " + juce::String(i), i);
        midiChannelCombo.setSelectedId(1);
        midiChannelCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF404040));
        midiChannelCombo.setColour(juce::ComboBox::textColourId, juce::Colour(0xFFEEEEEE));
        midiChannelCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF1A1A1A));
        
        // Preset controls
        addAndMakeVisible(presetLabel);
        presetLabel.setText("Presets:", juce::dontSendNotification);
        presetLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        presetLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
        
        addAndMakeVisible(presetCombo);
        presetCombo.setTextWhenNothingSelected("Select preset...");
        presetCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF404040));
        presetCombo.setColour(juce::ComboBox::textColourId, juce::Colour(0xFFEEEEEE));
        presetCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF1A1A1A));
        refreshPresetList();
        
        addAndMakeVisible(savePresetButton);
        savePresetButton.setButtonText("Save");
        savePresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404040));
        savePresetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFEEEEEE));
        savePresetButton.onClick = [this]() { showSavePresetDialog(); };
        
        addAndMakeVisible(loadPresetButton);
        loadPresetButton.setButtonText("Load");
        loadPresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404040));
        loadPresetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFEEEEEE));
        loadPresetButton.onClick = [this]() { loadSelectedPreset(); };
        
        addAndMakeVisible(deletePresetButton);
        deletePresetButton.setButtonText("Delete");
        deletePresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404040));
        deletePresetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFEEEEEE));
        deletePresetButton.onClick = [this]() { deleteSelectedPreset(); };
        
        addAndMakeVisible(presetFolderLabel);
        presetFolderLabel.setText("Preset Folder:", juce::dontSendNotification);
        presetFolderLabel.setFont(juce::FontOptions(14.0f));
        presetFolderLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));

        addAndMakeVisible(presetPathLabel);
        presetPathLabel.setText("", juce::dontSendNotification);
        presetPathLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFBBBBBB)); // Slightly dimmed
        presetPathLabel.setFont(juce::FontOptions(12.0f));
        presetPathLabel.setJustificationType(juce::Justification::centredLeft);
        
        addAndMakeVisible(openFolderButton);
        openFolderButton.setButtonText("Open Folder");
        openFolderButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404040));
        openFolderButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFEEEEEE));
        openFolderButton.onClick = [this]() { openPresetFolder(); };

        addAndMakeVisible(changeFolderButton);
        changeFolderButton.setButtonText("Change Folder");
        changeFolderButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404040));
        changeFolderButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFEEEEEE));
        changeFolderButton.onClick = [this]() { changePresetFolder(); };

        updatePresetFolderDisplay();
        
        addAndMakeVisible(resetToDefaultButton);
        resetToDefaultButton.setButtonText("Reset All");
        resetToDefaultButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404040));
        resetToDefaultButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFEEEEEE));
        resetToDefaultButton.onClick = [this]() { resetToDefaults(); };
        
        // Bank selector
        addAndMakeVisible(bankSelectorLabel);
        bankSelectorLabel.setText("Bank:", juce::dontSendNotification);
        bankSelectorLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        bankSelectorLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
        
        // Bank selector buttons - modern styling
        addAndMakeVisible(bankASelector);
        bankASelector.setText("A", juce::dontSendNotification);
        bankASelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        bankASelector.setJustificationType(juce::Justification::centred);
        bankASelector.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF0066CC)); // Modern blue
        bankASelector.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
        bankASelector.onClick = [this]() { setSelectedBank(0); };
        
        addAndMakeVisible(bankBSelector);
        bankBSelector.setText("B", juce::dontSendNotification);
        bankBSelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        bankBSelector.setJustificationType(juce::Justification::centred);
        bankBSelector.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF404040));
        bankBSelector.setColour(juce::Label::textColourId, juce::Colour(0xFFBBBBBB));
        bankBSelector.onClick = [this]() { setSelectedBank(1); };
        
        addAndMakeVisible(bankCSelector);
        bankCSelector.setText("C", juce::dontSendNotification);
        bankCSelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        bankCSelector.setJustificationType(juce::Justification::centred);
        bankCSelector.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF404040));
        bankCSelector.setColour(juce::Label::textColourId, juce::Colour(0xFFBBBBBB));
        bankCSelector.onClick = [this]() { setSelectedBank(2); };
        
        addAndMakeVisible(bankDSelector);
        bankDSelector.setText("D", juce::dontSendNotification);
        bankDSelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        bankDSelector.setJustificationType(juce::Justification::centred);
        bankDSelector.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF404040));
        bankDSelector.setColour(juce::Label::textColourId, juce::Colour(0xFFBBBBBB));
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
    
    void paint(juce::Graphics& g) override
    {
        // Modern dark interface background
        auto bounds = getLocalBounds().toFloat();
        
        // Dark background with rounded corners
        g.setColour(juce::Colour(0xFF2D2D2D)); // Dark charcoal background
        g.fillRoundedRectangle(bounds, 8.0f);
        
        // Subtle border
        g.setColour(juce::Colour(0xFF1A1A1A)); // Dark border
        g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
        
        // Settings title with clean modern styling
        g.setColour(juce::Colour(0xFFEEEEEE)); // Light gray text
        g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        auto titleArea = bounds.removeFromTop(40);
        g.drawText("Settings", titleArea, juce::Justification::centred);
        
        if (!controlsInitialized)
        {
            g.setColour(juce::Colour(0xFFEEEEEE));
            g.setFont(juce::FontOptions(14.0f));
            g.drawText("Loading controls...", bounds, juce::Justification::centred);
            return;
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(15); // Modern consistent padding
        
        bounds.removeFromTop(35); // Space for title
        
        // Preset controls section
        auto presetArea = bounds.removeFromTop(50); // Increased height for better spacing
        presetLabel.setBounds(presetArea.removeFromTop(20));
        
        bounds.removeFromTop(5); // Small gap after label
        
        auto presetButtonArea = bounds.removeFromTop(25);
        presetCombo.setBounds(presetButtonArea.removeFromLeft(180));
        presetButtonArea.removeFromLeft(10);
        savePresetButton.setBounds(presetButtonArea.removeFromLeft(50));
        presetButtonArea.removeFromLeft(8);
        loadPresetButton.setBounds(presetButtonArea.removeFromLeft(50));
        presetButtonArea.removeFromLeft(8);
        deletePresetButton.setBounds(presetButtonArea.removeFromLeft(55));
        presetButtonArea.removeFromLeft(10);
        resetToDefaultButton.setBounds(presetButtonArea);
        
        bounds.removeFromTop(20); // Section spacing
        
        // Folder controls section
        auto folderLabelArea = bounds.removeFromTop(20);
        presetFolderLabel.setBounds(folderLabelArea);
        
        bounds.removeFromTop(5);
        auto folderPathArea = bounds.removeFromTop(20);
        presetPathLabel.setBounds(folderPathArea);
        
        bounds.removeFromTop(8);
        auto folderButtonArea = bounds.removeFromTop(25);
        openFolderButton.setBounds(folderButtonArea.removeFromLeft(120));
        folderButtonArea.removeFromLeft(10);
        changeFolderButton.setBounds(folderButtonArea);
        
        bounds.removeFromTop(20); // Section spacing
        
        // MIDI Channel section
        auto channelArea = bounds.removeFromTop(25);
        midiChannelLabel.setBounds(channelArea.removeFromLeft(100));
        channelArea.removeFromLeft(10);
        midiChannelCombo.setBounds(channelArea);
        
        bounds.removeFromTop(20); // Section spacing
        
        if (!controlsInitialized)
            return; // Don't layout controls that don't exist yet
        
        // Bank selector section
        auto bankSelectorArea = bounds.removeFromTop(25);
        bankSelectorLabel.setBounds(bankSelectorArea.removeFromLeft(50));
        bankSelectorArea.removeFromLeft(10);
        
        // Bank selector buttons with consistent spacing
        bankASelector.setBounds(bankSelectorArea.removeFromLeft(28));
        bankSelectorArea.removeFromLeft(8);
        bankBSelector.setBounds(bankSelectorArea.removeFromLeft(28));
        bankSelectorArea.removeFromLeft(8);
        bankCSelector.setBounds(bankSelectorArea.removeFromLeft(28));
        bankSelectorArea.removeFromLeft(8);
        bankDSelector.setBounds(bankSelectorArea.removeFromLeft(28));
        
        bounds.removeFromTop(15); // Section spacing
        
        // Controls for current bank only (4 sliders)
        for (int i = 0; i < 4; ++i)
        {
            int sliderIndex = selectedBank * 4 + i;
            layoutSliderRow(bounds, sliderIndex);
        }
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

    
    juce::Label bankSelectorLabel;
    ClickableLabel bankASelector, bankBSelector, bankCSelector, bankDSelector;
    int selectedBank = 0;
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
        auto row = bounds.removeFromTop(30);
        
        // SLIDER X:
        sliderLabels[sliderIndex]->setBounds(row.removeFromLeft(120));
        
        // CC Value: [input]
        ccInputs[sliderIndex]->setBounds(row.removeFromLeft(80));
        
        // Range:
        rangeLabels[sliderIndex]->setBounds(row.removeFromLeft(70));
        
        // [min] - [max]
        minRangeInputs[sliderIndex]->setBounds(row.removeFromLeft(70));
        row.removeFromLeft(20); // Space for separator (-)
        maxRangeInputs[sliderIndex]->setBounds(row.removeFromLeft(70));
        
        row.removeFromLeft(10); // Spacing
        
        // Color: [combo]
        colorLabels[sliderIndex]->setBounds(row.removeFromLeft(50));
        colorCombos[sliderIndex]->setBounds(row.removeFromLeft(100));
        
        bounds.removeFromTop(5); // Row spacing
    }
    
    void initializeSliderControls()
    {
        // Create controls for all 16 sliders
        for (int i = 0; i < 16; ++i)
        {
            // SLIDER X: label
            auto* sliderLabel = new juce::Label();
            sliderLabels.add(sliderLabel);
            addAndMakeVisible(sliderLabel);
            sliderLabel->setText("Slider " + juce::String(i + 1) + ": CC Value:", juce::dontSendNotification);
            sliderLabel->setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
            
            // CC input
            auto* ccInput = new juce::TextEditor();
            ccInputs.add(ccInput);
            addAndMakeVisible(ccInput);
            ccInput->setText(juce::String(i), juce::dontSendNotification);
            ccInput->setInputRestrictions(3, "0123456789");
            ccInput->setTooltip("MIDI CC number (0-127)");
            ccInput->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF404040));
            ccInput->setColour(juce::TextEditor::textColourId, juce::Colour(0xFFEEEEEE));
            ccInput->setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF1A1A1A));
            ccInput->onReturnKey = [this, ccInput]() { validateCCInput(ccInput); };
            ccInput->onFocusLost = [this, ccInput]() { validateCCInput(ccInput); };
            
            // Range: label
            auto* rangeLabel = new juce::Label();
            rangeLabels.add(rangeLabel);
            addAndMakeVisible(rangeLabel);
            rangeLabel->setText("Range:", juce::dontSendNotification);
            rangeLabel->setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
            
            // Min range input
            auto* minInput = new juce::TextEditor();
            minRangeInputs.add(minInput);
            addAndMakeVisible(minInput);
            minInput->setText("0");
            minInput->setInputRestrictions(0, "-0123456789.");
            minInput->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF404040));
            minInput->setColour(juce::TextEditor::textColourId, juce::Colour(0xFFEEEEEE));
            minInput->setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF1A1A1A));
            minInput->onReturnKey = [this, minInput]() { validateRangeInput(minInput); };
            minInput->onFocusLost = [this, minInput]() { validateRangeInput(minInput); };
            
            // Max range input
            auto* maxInput = new juce::TextEditor();
            maxRangeInputs.add(maxInput);
            addAndMakeVisible(maxInput);
            maxInput->setText("16383");
            maxInput->setInputRestrictions(0, "-0123456789.");
            maxInput->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF404040));
            maxInput->setColour(juce::TextEditor::textColourId, juce::Colour(0xFFEEEEEE));
            maxInput->setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF1A1A1A));
            maxInput->onReturnKey = [this, maxInput]() { validateRangeInput(maxInput); };
            maxInput->onFocusLost = [this, maxInput]() { validateRangeInput(maxInput); };
            
            // Color: label
            auto* colorLabel = new juce::Label();
            colorLabels.add(colorLabel);
            addAndMakeVisible(colorLabel);
            colorLabel->setText("Color:", juce::dontSendNotification);
            colorLabel->setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
            
            // Color selector
            auto* colorCombo = new juce::ComboBox();
            colorCombos.add(colorCombo);
            addAndMakeVisible(colorCombo);
            colorCombo->setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF404040));
            colorCombo->setColour(juce::ComboBox::textColourId, juce::Colour(0xFFEEEEEE));
            colorCombo->setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF1A1A1A));
            
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
            for (int i = 0; i < 16; ++i)
            {
                bool shouldBeVisible = (i >= selectedBank * 4) && (i < (selectedBank + 1) * 4);
                
                if (i < sliderLabels.size()) sliderLabels[i]->setVisible(shouldBeVisible);
                if (i < ccInputs.size()) ccInputs[i]->setVisible(shouldBeVisible);
                if (i < rangeLabels.size()) rangeLabels[i]->setVisible(shouldBeVisible);
                if (i < minRangeInputs.size()) minRangeInputs[i]->setVisible(shouldBeVisible);
                if (i < maxRangeInputs.size()) maxRangeInputs[i]->setVisible(shouldBeVisible);
                if (i < colorLabels.size()) colorLabels[i]->setVisible(shouldBeVisible);
                if (i < colorCombos.size()) colorCombos[i]->setVisible(shouldBeVisible);
            }
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
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
};
