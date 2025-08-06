#pragma once
#include <JuceHeader.h>
#include "../Core/AutomationConfigManager.h"
#include "../CustomLookAndFeel.h"
#include "ConfigNameDialog.h"

//==============================================================================
/**
 * ConfigManagementDialog - Dialog for browsing, editing, and managing automation configs
 */
class ConfigManagementDialog : public juce::Component
{
public:
    ConfigManagementDialog(AutomationConfigManager& configManager)
        : configManager(configManager)
    {
        // Title
        addAndMakeVisible(titleLabel);
        titleLabel.setText("Automation Config Manager", juce::dontSendNotification);
        titleLabel.setFont(juce::FontOptions(18.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        
        // Config list
        addAndMakeVisible(configListBox);
        configListBox.setModel(this);
        configListBox.setColour(juce::ListBox::backgroundColourId, BlueprintColors::background);
        configListBox.setColour(juce::ListBox::outlineColourId, BlueprintColors::blueprintLines);
        configListBox.setOutlineThickness(1);
        
        // Buttons
        addAndMakeVisible(renameButton);
        renameButton.setButtonText("Rename");
        renameButton.setLookAndFeel(&buttonLookAndFeel);
        renameButton.setEnabled(false);
        renameButton.onClick = [this]() { renameSelectedConfig(); };
        
        addAndMakeVisible(deleteButton);
        deleteButton.setButtonText("Delete");
        deleteButton.setLookAndFeel(&buttonLookAndFeel);
        deleteButton.setEnabled(false);
        deleteButton.onClick = [this]() { deleteSelectedConfig(); };
        
        addAndMakeVisible(duplicateButton);
        duplicateButton.setButtonText("Duplicate");
        duplicateButton.setLookAndFeel(&buttonLookAndFeel);
        duplicateButton.setEnabled(false);
        duplicateButton.onClick = [this]() { duplicateSelectedConfig(); };
        
        addAndMakeVisible(assignMidiButton);
        assignMidiButton.setButtonText("Assign MIDI");
        assignMidiButton.setLookAndFeel(&buttonLookAndFeel);
        assignMidiButton.setEnabled(false);
        assignMidiButton.onClick = [this]() { assignMidiToSelected(); };
        
        addAndMakeVisible(closeButton);
        closeButton.setButtonText("Close");
        closeButton.setLookAndFeel(&buttonLookAndFeel);
        closeButton.onClick = [this]() { 
            if (onCloseRequested) onCloseRequested(); 
        };
        
        // Config details
        addAndMakeVisible(detailsLabel);
        detailsLabel.setText("Select a config to view details", juce::dontSendNotification);
        detailsLabel.setFont(juce::FontOptions(12.0f));
        detailsLabel.setJustificationType(juce::Justification::topLeft);
        detailsLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        
        refreshConfigList();
        setSize(600, 450);
    }
    
    ~ConfigManagementDialog()
    {
        renameButton.setLookAndFeel(nullptr);
        deleteButton.setLookAndFeel(nullptr);
        duplicateButton.setLookAndFeel(nullptr);
        assignMidiButton.setLookAndFeel(nullptr);
        closeButton.setLookAndFeel(nullptr);
    }
    
    void paint(juce::Graphics& g) override
    {
        // Background
        g.fillAll(BlueprintColors::windowBackground);
        
        // Border
        g.setColour(BlueprintColors::blueprintLines);
        g.drawRect(getLocalBounds(), 2);
        
        // Details section background
        auto detailsArea = getDetailsArea();
        g.setColour(BlueprintColors::sectionBackground);
        g.fillRect(detailsArea);
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        g.drawRect(detailsArea, 1);
    }
    
    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        
        // Title
        titleLabel.setBounds(area.removeFromTop(25));
        area.removeFromTop(10);
        
        // Main content - split into left and right
        auto leftArea = area.removeFromLeft(300);
        area.removeFromLeft(10); // Spacing
        auto rightArea = area;
        
        // Left side - config list and buttons
        configListBox.setBounds(leftArea.removeFromTop(leftArea.getHeight() - 50));
        leftArea.removeFromTop(10);
        
        // Buttons in left area
        auto buttonArea = leftArea;
        int buttonWidth = 70;
        int spacing = 5;
        
        renameButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
        buttonArea.removeFromLeft(spacing);
        deleteButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
        buttonArea.removeFromLeft(spacing);
        duplicateButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
        buttonArea.removeFromLeft(spacing);
        assignMidiButton.setBounds(buttonArea.removeFromLeft(80));
        
        // Right side - details and close
        auto detailsArea = rightArea.removeFromTop(rightArea.getHeight() - 40);
        detailsLabel.setBounds(detailsArea.reduced(5));
        
        rightArea.removeFromTop(10);
        closeButton.setBounds(rightArea.removeFromRight(80).removeFromTop(25));
    }
    
    // ListBoxModel implementation
    int getNumRows() override
    {
        return static_cast<int>(allConfigs.size());
    }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowNumber >= 0 && rowNumber < static_cast<int>(allConfigs.size()))
        {
            const auto& config = allConfigs[rowNumber];
            
            if (rowIsSelected)
                g.fillAll(BlueprintColors::active.withAlpha(0.3f));
            
            g.setColour(rowIsSelected ? BlueprintColors::textPrimary : BlueprintColors::textSecondary);
            g.setFont(12.0f);
            
            // Config name
            g.drawText(config.name, 5, 0, width - 100, height, juce::Justification::centredLeft);
            
            // Show MIDI assignment indicator
            auto midiAssignments = configManager.getMidiAssignmentsForConfig(config.id);
            if (!midiAssignments.empty())
            {
                g.setColour(BlueprintColors::active);
                g.drawText("[MIDI]", width - 95, 0, 50, height, juce::Justification::centredLeft);
            }
            
            // Time mode indicator
            juce::String timeMode = (config.timeMode == AutomationControlPanel::TimeMode::Beats) ? "♪" : "s";
            g.setColour(BlueprintColors::textSecondary);
            g.drawText(timeMode, width - 45, 0, 20, height, juce::Justification::centred);
            
            // Original slider
            g.drawText(juce::String(config.originalSliderIndex + 1), width - 25, 0, 20, height, juce::Justification::centred);
        }
    }
    
    void selectedRowsChanged(int lastRowSelected) override
    {
        bool hasSelection = lastRowSelected >= 0 && lastRowSelected < static_cast<int>(allConfigs.size());
        
        renameButton.setEnabled(hasSelection);
        deleteButton.setEnabled(hasSelection);
        duplicateButton.setEnabled(hasSelection);
        assignMidiButton.setEnabled(hasSelection);
        
        if (hasSelection)
        {
            updateDetailsDisplay(allConfigs[lastRowSelected]);
        }
        else
        {
            detailsLabel.setText("Select a config to view details", juce::dontSendNotification);
        }
    }
    
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override
    {
        if (row >= 0 && row < static_cast<int>(allConfigs.size()))
        {
            renameSelectedConfig();
        }
    }
    
    void refreshConfigList()
    {
        allConfigs = configManager.getAllConfigs();
        configListBox.updateContent();
        selectedRowsChanged(-1); // Clear selection
    }
    
    std::function<void()> onCloseRequested;
    std::function<void()> onMidiLearnRequested;
    
private:
    AutomationConfigManager& configManager;
    std::vector<AutomationConfig> allConfigs;
    CustomButtonLookAndFeel buttonLookAndFeel;
    
    // UI Components
    juce::Label titleLabel;
    juce::ListBox configListBox;
    juce::TextButton renameButton;
    juce::TextButton deleteButton;
    juce::TextButton duplicateButton;
    juce::TextButton assignMidiButton;
    juce::TextButton closeButton;
    juce::Label detailsLabel;
    
    juce::Rectangle<int> getDetailsArea() const
    {
        auto area = getLocalBounds().reduced(10);
        area.removeFromTop(35); // Title + spacing
        area.removeFromLeft(310); // Left area + spacing
        return area.removeFromTop(area.getHeight() - 50);
    }
    
    void updateDetailsDisplay(const AutomationConfig& config)
    {
        juce::String details;
        details << "Name: " << config.name << "\n\n";
        details << "Target Value: " << juce::String(config.targetValue, 2) << "\n";
        details << "Delay: " << juce::String(config.delayTime, 2) << " ";
        details << (config.timeMode == AutomationControlPanel::TimeMode::Beats ? "beats" : "sec") << "\n";
        details << "Attack: " << juce::String(config.attackTime, 2) << " ";
        details << (config.timeMode == AutomationControlPanel::TimeMode::Beats ? "beats" : "sec") << "\n";
        details << "Return: " << juce::String(config.returnTime, 2) << " ";
        details << (config.timeMode == AutomationControlPanel::TimeMode::Beats ? "beats" : "sec") << "\n";
        details << "Curve: " << juce::String(config.curveValue, 2) << "\n\n";
        details << "Original Slider: " << juce::String(config.originalSliderIndex + 1) << "\n";
        
        auto midiAssignments = configManager.getMidiAssignmentsForConfig(config.id);
        if (!midiAssignments.empty())
        {
            details << "\nMIDI Assignments:\n";
            for (const auto& assignment : midiAssignments)
            {
                details << "CC " << assignment.first << " Ch " << assignment.second << "\n";
            }
        }
        else
        {
            details << "\nNo MIDI assignments";
        }
        
        detailsLabel.setText(details, juce::dontSendNotification);
    }
    
    void renameSelectedConfig()
    {
        int selectedRow = configListBox.getSelectedRow();
        if (selectedRow >= 0 && selectedRow < static_cast<int>(allConfigs.size()))
        {
            const auto& config = allConfigs[selectedRow];
            
            ConfigNameDialog::showDialog(this, config.name, [this, configId = config.id](const juce::String& newName) {
                auto config = configManager.loadConfig(configId);
                config.name = newName;
                configManager.saveConfig(config);
                refreshConfigList();
            });
        }
    }
    
    void deleteSelectedConfig()
    {
        int selectedRow = configListBox.getSelectedRow();
        if (selectedRow >= 0 && selectedRow < static_cast<int>(allConfigs.size()))
        {
            const auto& config = allConfigs[selectedRow];
            
            auto result = juce::AlertWindow::showYesNoCancelBox(
                juce::AlertWindow::QuestionIcon,
                "Delete Config",
                "Are you sure you want to delete '" + config.name + "'?\n\nThis action cannot be undone.",
                "Delete", "Cancel", juce::String(),
                this
            );
            
            if (result == 1) // Yes
            {
                configManager.deleteConfig(config.id);
                refreshConfigList();
            }
        }
    }
    
    void duplicateSelectedConfig()
    {
        int selectedRow = configListBox.getSelectedRow();
        if (selectedRow >= 0 && selectedRow < static_cast<int>(allConfigs.size()))
        {
            auto config = allConfigs[selectedRow];
            config.name += " (Copy)";
            config.id = juce::String(); // Will generate new ID
            
            ConfigNameDialog::showDialog(this, config.name, [this, config](const juce::String& newName) mutable {
                config.name = newName;
                configManager.saveConfig(config);
                refreshConfigList();
            });
        }
    }
    
    void assignMidiToSelected()
    {
        int selectedRow = configListBox.getSelectedRow();
        if (selectedRow >= 0 && selectedRow < static_cast<int>(allConfigs.size()))
        {
            if (onMidiLearnRequested)
                onMidiLearnRequested();
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigManagementDialog)
};