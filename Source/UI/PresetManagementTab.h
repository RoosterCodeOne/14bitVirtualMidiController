// PresetManagementTab.h - Preset Management Tab
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"
#include "../PresetManager.h"

// Forward declaration
class SettingsWindow;

//==============================================================================
class PresetManagementTab : public juce::Component
{
public:
    PresetManagementTab(SettingsWindow* parentWindow, PresetManager& presetManager);
    ~PresetManagementTab();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Public interface for main window coordination
    void refreshPresetList();
    void updatePresetFolderDisplay();
    
    // Callback functions for communication with parent
    std::function<void(const ControllerPreset&)> onPresetLoaded;
    std::function<void()> onPresetSaved;
    std::function<void()> onPresetDeleted;
    std::function<void()> onResetToDefaults;
    
private:
    SettingsWindow* parentWindow;
    PresetManager& presetManager;
    CustomButtonLookAndFeel customButtonLookAndFeel;
    
    // Preset controls
    juce::Label presetLabel;
    juce::ComboBox presetCombo;
    juce::TextButton savePresetButton, loadPresetButton, deletePresetButton;
    juce::TextButton resetToDefaultButton;
    
    // Preset folder controls
    juce::Label presetFolderLabel;
    juce::Label presetPathLabel;
    juce::TextButton openFolderButton, changeFolderButton;
    
    // Private methods
    void setupPresetControls();
    void setupFolderControls();
    void showSavePresetDialog();
    void loadSelectedPreset();
    void deleteSelectedPreset();
    void resetToDefaults();
    void openPresetFolder();
    void changePresetFolder();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManagementTab)
};

//==============================================================================
// Implementation
inline PresetManagementTab::PresetManagementTab(SettingsWindow* parent, PresetManager& manager)
    : parentWindow(parent), presetManager(manager)
{
    setupPresetControls();
    setupFolderControls();
    refreshPresetList();
    updatePresetFolderDisplay();
}

inline PresetManagementTab::~PresetManagementTab()
{
    // Clean up custom look and feel
    savePresetButton.setLookAndFeel(nullptr);
    loadPresetButton.setLookAndFeel(nullptr);
    deletePresetButton.setLookAndFeel(nullptr);
    resetToDefaultButton.setLookAndFeel(nullptr);
    openFolderButton.setLookAndFeel(nullptr);
    changeFolderButton.setLookAndFeel(nullptr);
}

inline void PresetManagementTab::paint(juce::Graphics& g)
{
    auto& scale = GlobalUIScale::getInstance();
    
    // Blueprint aesthetic background
    g.setColour(BlueprintColors::windowBackground);
    g.fillAll();
    
    // Draw section backgrounds
    auto bounds = getLocalBounds().reduced(scale.getScaled(15));
    
    // Preset controls section background
    auto presetSectionBounds = bounds.removeFromTop(scale.getScaled(10 + 16 + 6 + 22 + 6 + 20));
    presetSectionBounds = presetSectionBounds.expanded(scale.getScaled(5), 0).withTrimmedBottom(scale.getScaled(1));
    
    g.setColour(BlueprintColors::sectionBackground);
    g.fillRect(presetSectionBounds.toFloat());
    g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
    g.drawRect(presetSectionBounds.toFloat(), scale.getScaledLineThickness());
    
    // Skip spacing
    bounds.removeFromTop(scale.getScaled(20));
    
    // Folder controls section background
    auto folderSectionBounds = bounds.removeFromTop(scale.getScaled(10 + 16 + 5 + 16 + 7 + 20));
    folderSectionBounds = folderSectionBounds.expanded(scale.getScaled(5), 0);
    
    g.setColour(BlueprintColors::sectionBackground);
    g.fillRect(folderSectionBounds.toFloat());
    g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
    g.drawRect(folderSectionBounds.toFloat(), scale.getScaledLineThickness());
}

inline void PresetManagementTab::resized()
{
    auto& scale = GlobalUIScale::getInstance();
    auto bounds = getLocalBounds().reduced(scale.getScaled(15));
    
    // Preset controls section
    bounds.removeFromTop(scale.getScaled(10));
    presetLabel.setBounds(bounds.removeFromTop(scale.getScaled(16)));
    bounds.removeFromTop(scale.getScaled(6));
    
    // Preset combo box and buttons on same row
    auto presetRowArea = bounds.removeFromTop(scale.getScaled(22));
    presetCombo.setBounds(presetRowArea.removeFromLeft(scale.getScaled(160)));
    presetRowArea.removeFromLeft(scale.getScaled(8));
    
    // 2x2 grid for preset buttons
    const int buttonWidth = scale.getScaled(40);
    const int buttonHeight = scale.getScaled(20);
    const int buttonSpacing = scale.getScaled(6);
    
    // Top row: Save, Load
    savePresetButton.setBounds(presetRowArea.removeFromLeft(buttonWidth));
    presetRowArea.removeFromLeft(buttonSpacing);
    loadPresetButton.setBounds(presetRowArea.removeFromLeft(buttonWidth));
    
    bounds.removeFromTop(scale.getScaled(6));
    
    // Bottom row: Delete, Reset
    auto bottomRowArea = bounds.removeFromTop(buttonHeight);
    bottomRowArea.removeFromLeft(scale.getScaled(160 + 8));
    deletePresetButton.setBounds(bottomRowArea.removeFromLeft(buttonWidth));
    bottomRowArea.removeFromLeft(buttonSpacing);
    resetToDefaultButton.setBounds(bottomRowArea.removeFromLeft(buttonWidth));
    
    bounds.removeFromTop(scale.getScaled(20)); // Flexible spacing
    
    // Folder controls section
    bounds.removeFromTop(scale.getScaled(10));
    presetFolderLabel.setBounds(bounds.removeFromTop(scale.getScaled(16)));
    bounds.removeFromTop(scale.getScaled(5));
    
    auto folderPathArea = bounds.removeFromTop(scale.getScaled(16));
    presetPathLabel.setBounds(folderPathArea);
    
    bounds.removeFromTop(scale.getScaled(7));
    auto folderButtonArea = bounds.removeFromTop(scale.getScaled(20));
    int folderButtonWidth = (folderButtonArea.getWidth() - scale.getScaled(8)) / 2;
    openFolderButton.setBounds(folderButtonArea.removeFromLeft(folderButtonWidth));
    folderButtonArea.removeFromLeft(scale.getScaled(8));
    changeFolderButton.setBounds(folderButtonArea);
}

inline void PresetManagementTab::setupPresetControls()
{
    // Preset controls
    addAndMakeVisible(presetLabel);
    presetLabel.setText("Presets:", juce::dontSendNotification);
    presetLabel.setFont(GlobalUIScale::getInstance().getScaledFont(16.0f).boldened());
    presetLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(presetCombo);
    presetCombo.setTextWhenNothingSelected("Select preset...");
    presetCombo.setColour(juce::ComboBox::backgroundColourId, BlueprintColors::background);
    presetCombo.setColour(juce::ComboBox::textColourId, BlueprintColors::textPrimary);
    presetCombo.setColour(juce::ComboBox::outlineColourId, BlueprintColors::blueprintLines);
    
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
    
    addAndMakeVisible(resetToDefaultButton);
    resetToDefaultButton.setButtonText("Reset");
    resetToDefaultButton.setLookAndFeel(&customButtonLookAndFeel);
    resetToDefaultButton.onClick = [this]() { resetToDefaults(); };
}

inline void PresetManagementTab::setupFolderControls()
{
    addAndMakeVisible(presetFolderLabel);
    presetFolderLabel.setText("Preset Folder:", juce::dontSendNotification);
    presetFolderLabel.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f));
    presetFolderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);

    addAndMakeVisible(presetPathLabel);
    presetPathLabel.setText("", juce::dontSendNotification);
    presetPathLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
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
}

inline void PresetManagementTab::refreshPresetList()
{
    presetCombo.clear();
    auto presetNames = presetManager.getPresetNames();
    for (int i = 0; i < presetNames.size(); ++i)
    {
        presetCombo.addItem(presetNames[i], i + 1);
    }
}

inline void PresetManagementTab::updatePresetFolderDisplay()
{
    auto path = presetManager.getPresetDirectory().getFullPathName();
    presetPathLabel.setText(path, juce::dontSendNotification);
}

inline void PresetManagementTab::showSavePresetDialog()
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
                // Get current preset from parent window through callback
                if (onPresetSaved)
                {
                    onPresetSaved();
                    refreshPresetList();
                    presetCombo.setText(*textHolder, juce::dontSendNotification);
                }
            }
        }), true);
}

inline void PresetManagementTab::loadSelectedPreset()
{
    auto selectedText = presetCombo.getText();
    if (selectedText.isNotEmpty())
    {
        auto preset = presetManager.loadPreset(selectedText);
        
        if (onPresetLoaded)
            onPresetLoaded(preset);
    }
}

inline void PresetManagementTab::deleteSelectedPreset()
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
                        
                        if (onPresetDeleted)
                            onPresetDeleted();
                    }
                }
            });
    }
}

inline void PresetManagementTab::resetToDefaults()
{
    if (onResetToDefaults)
        onResetToDefaults();
}

inline void PresetManagementTab::openPresetFolder()
{
    auto presetDir = presetManager.getPresetDirectory();
    if (presetDir.exists())
    {
        presetDir.revealToUser();
    }
}

inline void PresetManagementTab::changePresetFolder()
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