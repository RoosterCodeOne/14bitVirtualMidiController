// SettingsWindow.h - Production Version with Preset System
#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"

//==============================================================================
class SettingsWindow : public juce::Component
{
public:
    SettingsWindow() : controlsInitialized(false), closeButton("X")
    {
        setSize(700, 750); // Slightly taller for preset controls
        
        // Only create the essential controls in constructor
        addAndMakeVisible(closeButton);
        closeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
        closeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        closeButton.onClick = [this]() { setVisible(false); };
        
        addAndMakeVisible(midiChannelLabel);
        midiChannelLabel.setText("MIDI Channel:", juce::dontSendNotification);
        addAndMakeVisible(midiChannelCombo);
        for (int i = 1; i <= 16; ++i)
            midiChannelCombo.addItem("Channel " + juce::String(i), i);
        midiChannelCombo.setSelectedId(1);
        
        // Preset controls
        addAndMakeVisible(presetLabel);
        presetLabel.setText("Presets:", juce::dontSendNotification);
        presetLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        
        addAndMakeVisible(presetCombo);
        presetCombo.setTextWhenNothingSelected("Select preset...");
        refreshPresetList();
        
        addAndMakeVisible(savePresetButton);
        savePresetButton.setButtonText("Save");
        savePresetButton.onClick = [this]() { showSavePresetDialog(); };
        
        addAndMakeVisible(loadPresetButton);
        loadPresetButton.setButtonText("Load");
        loadPresetButton.onClick = [this]() { loadSelectedPreset(); };
        
        addAndMakeVisible(deletePresetButton);
        deletePresetButton.setButtonText("Delete");
        deletePresetButton.onClick = [this]() { deleteSelectedPreset(); };
        
        addAndMakeVisible(presetFolderLabel);
        presetFolderLabel.setText("Preset Folder:", juce::dontSendNotification);
        presetFolderLabel.setFont(juce::FontOptions(14.0f));

        addAndMakeVisible(presetPathLabel);
        presetPathLabel.setText("", juce::dontSendNotification);
        presetPathLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        presetPathLabel.setFont(juce::FontOptions(12.0f));
        presetPathLabel.setJustificationType(juce::Justification::centredLeft);
        
        addAndMakeVisible(openFolderButton);
        openFolderButton.setButtonText("Open Folder");
        openFolderButton.onClick = [this]() { openPresetFolder(); };

        addAndMakeVisible(changeFolderButton);
        changeFolderButton.setButtonText("Change Folder");
        changeFolderButton.onClick = [this]() { changePresetFolder(); };

        updatePresetFolderDisplay();
        
        addAndMakeVisible(resetToDefaultButton);
        resetToDefaultButton.setButtonText("Reset All");
        resetToDefaultButton.onClick = [this]() { resetToDefaults(); };
        
        // Bank labels
        addAndMakeVisible(bankALabel);
        bankALabel.setText("Bank A", juce::dontSendNotification);
        bankALabel.setColour(juce::Label::textColourId, juce::Colours::red);
        bankALabel.setFont(juce::FontOptions(16.0f));
        
        addAndMakeVisible(bankBLabel);
        bankBLabel.setText("Bank B", juce::dontSendNotification);
        bankBLabel.setColour(juce::Label::textColourId, juce::Colours::blue);
        bankBLabel.setFont(juce::FontOptions(16.0f));
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
        // Semi-transparent background
        g.fillAll(juce::Colours::black.withAlpha(0.8f));
        
        // Settings panel
        auto bounds = getLocalBounds().reduced(50);
        g.setColour(juce::Colours::darkgrey);
        g.fillRoundedRectangle(bounds.toFloat(), 10.0f);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(18.0f));
        g.drawText("Settings", bounds.removeFromTop(40), juce::Justification::centred);
        
        if (!controlsInitialized)
        {
            g.setFont(juce::FontOptions(14.0f));
            g.drawText("Loading controls...", bounds, juce::Justification::centred);
            return;
        }
        
        // FIX 3: Update bounds calculation to match new layout order
        g.setColour(juce::Colours::lightgrey);
        g.setFont(juce::FontOptions(14.0f));
        bounds.removeFromTop(10);
        bounds.removeFromTop(40); // Preset area (moved to top)
        bounds.removeFromTop(15); // Spacing
        bounds.removeFromTop(20); // Folder label
        bounds.removeFromTop(25); // Folder path
        bounds.removeFromTop(30); // Folder buttons
        bounds.removeFromTop(15); // Spacing
        bounds.removeFromTop(30); // MIDI channel area (moved below presets)
        bounds.removeFromTop(15); // Spacing
        bounds.removeFromTop(25); // Bank A label
        bounds.removeFromTop(5);  // Small spacing
        
        // Draw separators for each slider row
        for (int i = 0; i < 8; ++i)
        {
            if (i == 4)
            {
                bounds.removeFromTop(10); // Bank spacing
                bounds.removeFromTop(25); // Bank B label
                bounds.removeFromTop(5);  // Small spacing
            }
            
            auto row = bounds.removeFromTop(30);
            
            // Calculate separator position (between min and max inputs)
            int separatorX = 50 + 120 + 80 + 70 + 5; // Label + CC + Range label + Min input + spacing
            g.drawText("-", separatorX, row.getY() + 8, 10, 14, juce::Justification::centred);
            
            bounds.removeFromTop(5); // Row spacing
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(50);
        
        // Close button
        closeButton.setBounds(bounds.getRight() - 30, bounds.getY() + 5, 25, 25);
        
        bounds.removeFromTop(50); // Title space
        
        // MOVED: Preset controls to the top
        auto presetArea = bounds.removeFromTop(40);
        presetLabel.setBounds(presetArea.removeFromTop(20));
        
        auto presetButtonArea = presetArea;
        presetCombo.setBounds(presetButtonArea.removeFromLeft(200));
        presetButtonArea.removeFromLeft(10); // spacing
        savePresetButton.setBounds(presetButtonArea.removeFromLeft(60));
        presetButtonArea.removeFromLeft(5);
        loadPresetButton.setBounds(presetButtonArea.removeFromLeft(60));
        presetButtonArea.removeFromLeft(5);
        deletePresetButton.setBounds(presetButtonArea.removeFromLeft(60));
        presetButtonArea.removeFromLeft(10);
        resetToDefaultButton.setBounds(presetButtonArea.removeFromLeft(80)); // NEW: Reset button
        
        bounds.removeFromTop(15); // Spacing
        
        // Preset folder controls
        auto folderLabelArea = bounds.removeFromTop(20);
        presetFolderLabel.setBounds(folderLabelArea);
        
        auto folderPathArea = bounds.removeFromTop(25);
        // FIX 2: Remove black background and make width fit content
        auto pathWidth = juce::jmin(400, bounds.getWidth() - 20); // Limit width
        presetPathLabel.setBounds(folderPathArea.removeFromLeft(pathWidth));
        
        auto folderButtonArea = bounds.removeFromTop(30);
        openFolderButton.setBounds(folderButtonArea.removeFromLeft(100));
        folderButtonArea.removeFromLeft(10);
        changeFolderButton.setBounds(folderButtonArea.removeFromLeft(100));
        
        bounds.removeFromTop(15); // Spacing
        
        // MIDI Channel (moved below presets)
        auto channelArea = bounds.removeFromTop(30);
        midiChannelLabel.setBounds(channelArea.removeFromLeft(100));
        midiChannelCombo.setBounds(channelArea.removeFromLeft(120));
        
        bounds.removeFromTop(15); // Spacing
        
        if (!controlsInitialized)
            return; // Don't layout controls that don't exist yet
        
        // Bank A label
        bankALabel.setBounds(bounds.removeFromTop(25));
        bounds.removeFromTop(5); // Small spacing
        
        // Controls for Bank A (sliders 0-3)
        for (int i = 0; i < 4; ++i)
        {
            layoutSliderRow(bounds, i);
        }
        
        bounds.removeFromTop(10); // Spacing between banks
        
        // Bank B label
        bankBLabel.setBounds(bounds.removeFromTop(25));
        bounds.removeFromTop(5);
        
        // Controls for Bank B (sliders 4-7)
        for (int i = 4; i < 8; ++i)
        {
            layoutSliderRow(bounds, i);
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
            return sliderIndex < 4 ? juce::Colours::red : juce::Colours::blue;
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
                    return sliderIndex < 4 ? juce::Colours::red : juce::Colours::blue;
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
            for (int i = 0; i < 8; ++i)
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
            for (int i = 0; i < 8; ++i)
            {
                if (i < preset.sliders.size())
                {
                    preset.sliders.getReference(i).ccNumber = i; // Default CC numbers
                    preset.sliders.getReference(i).minRange = 0.0;
                    preset.sliders.getReference(i).maxRange = 16383.0;
                    preset.sliders.getReference(i).colorId = 1; // Default color
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
        for (int i = 0; i < juce::jmin(8, preset.sliders.size()); ++i)
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
    juce::TextButton closeButton;
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

    
    juce::Label bankALabel, bankBLabel;
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
            for (int i = 0; i < 8; ++i)
            {
                if (i < ccInputs.size())
                    ccInputs[i]->setText(juce::String(i), juce::dontSendNotification);
                    
                if (i < minRangeInputs.size())
                    minRangeInputs[i]->setText("0", juce::dontSendNotification);
                    
                if (i < maxRangeInputs.size())
                    maxRangeInputs[i]->setText("16383", juce::dontSendNotification);
                    
                if (i < colorCombos.size())
                    colorCombos[i]->setSelectedId(1, juce::dontSendNotification); // Default color
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
        // Create controls for all 8 sliders
        for (int i = 0; i < 8; ++i)
        {
            // SLIDER X: label
            auto* sliderLabel = new juce::Label();
            sliderLabels.add(sliderLabel);
            addAndMakeVisible(sliderLabel);
            sliderLabel->setText("SLIDER " + juce::String(i + 1) + ": CC Value:", juce::dontSendNotification);
            
            // CC input
            auto* ccInput = new juce::TextEditor();
            ccInputs.add(ccInput);
            addAndMakeVisible(ccInput);
            ccInput->setText(juce::String(i), juce::dontSendNotification);
            ccInput->setInputRestrictions(3, "0123456789");
            ccInput->setTooltip("MIDI CC number (0-127)");
            ccInput->onReturnKey = [this, ccInput]() { validateCCInput(ccInput); };
            ccInput->onFocusLost = [this, ccInput]() { validateCCInput(ccInput); };
            
            // Range: label
            auto* rangeLabel = new juce::Label();
            rangeLabels.add(rangeLabel);
            addAndMakeVisible(rangeLabel);
            rangeLabel->setText("Range:", juce::dontSendNotification);
            
            // Min range input
            auto* minInput = new juce::TextEditor();
            minRangeInputs.add(minInput);
            addAndMakeVisible(minInput);
            minInput->setText("0");
            minInput->setInputRestrictions(0, "-0123456789.");
            minInput->onReturnKey = [this, minInput]() { validateRangeInput(minInput); };
            minInput->onFocusLost = [this, minInput]() { validateRangeInput(minInput); };
            
            // Max range input
            auto* maxInput = new juce::TextEditor();
            maxRangeInputs.add(maxInput);
            addAndMakeVisible(maxInput);
            maxInput->setText("16383");
            maxInput->setInputRestrictions(0, "-0123456789.");
            maxInput->onReturnKey = [this, maxInput]() { validateRangeInput(maxInput); };
            maxInput->onFocusLost = [this, maxInput]() { validateRangeInput(maxInput); };
            
            // Color: label
            auto* colorLabel = new juce::Label();
            colorLabels.add(colorLabel);
            addAndMakeVisible(colorLabel);
            colorLabel->setText("Color:", juce::dontSendNotification);
            
            // Color selector
            auto* colorCombo = new juce::ComboBox();
            colorCombos.add(colorCombo);
            addAndMakeVisible(colorCombo);
            
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
        resized();
        repaint();
        
        if (onSettingsChanged)
            onSettingsChanged();
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
