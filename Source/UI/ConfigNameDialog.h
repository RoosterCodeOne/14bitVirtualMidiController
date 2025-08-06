#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"

//==============================================================================
/**
 * ConfigNameDialog - Simple dialog for entering automation config names
 */
class ConfigNameDialog : public juce::Component
{
public:
    ConfigNameDialog(const juce::String& defaultName = "New Config")
        : defaultName(defaultName)
    {
        // Title label
        addAndMakeVisible(titleLabel);
        titleLabel.setText("Save Automation Config", juce::dontSendNotification);
        titleLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        
        // Name input
        addAndMakeVisible(nameEditor);
        nameEditor.setText(defaultName);
        nameEditor.selectAll();
        nameEditor.setFont(juce::FontOptions(14.0f));
        nameEditor.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
        nameEditor.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
        nameEditor.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
        nameEditor.onReturnKey = [this]() { 
            if (onOkClicked) onOkClicked(nameEditor.getText()); 
        };
        nameEditor.onEscapeKey = [this]() { 
            if (onCancelClicked) onCancelClicked(); 
        };
        
        // Buttons
        addAndMakeVisible(okButton);
        okButton.setButtonText("Save");
        okButton.setLookAndFeel(&buttonLookAndFeel);
        okButton.onClick = [this]() { 
            if (onOkClicked) onOkClicked(nameEditor.getText()); 
        };
        
        addAndMakeVisible(cancelButton);
        cancelButton.setButtonText("Cancel");
        cancelButton.setLookAndFeel(&buttonLookAndFeel);
        cancelButton.onClick = [this]() { 
            if (onCancelClicked) onCancelClicked(); 
        };
        
        // Focus on name editor
        nameEditor.grabKeyboardFocus();
        
        setSize(300, 120);
    }
    
    ~ConfigNameDialog()
    {
        okButton.setLookAndFeel(nullptr);
        cancelButton.setLookAndFeel(nullptr);
    }
    
    void paint(juce::Graphics& g) override
    {
        // Background
        g.fillAll(BlueprintColors::windowBackground);
        
        // Border
        g.setColour(BlueprintColors::blueprintLines);
        g.drawRect(getLocalBounds(), 2);
    }
    
    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        
        // Title
        titleLabel.setBounds(area.removeFromTop(25));
        area.removeFromTop(10);
        
        // Name input
        nameEditor.setBounds(area.removeFromTop(25));
        area.removeFromTop(15);
        
        // Buttons
        auto buttonArea = area.removeFromTop(25);
        int buttonWidth = 70;
        int spacing = 10;
        int totalButtonWidth = (buttonWidth * 2) + spacing;
        int startX = (buttonArea.getWidth() - totalButtonWidth) / 2;
        
        okButton.setBounds(startX, buttonArea.getY(), buttonWidth, 25);
        cancelButton.setBounds(startX + buttonWidth + spacing, buttonArea.getY(), buttonWidth, 25);
    }
    
    juce::String getConfigName() const
    {
        return nameEditor.getText();
    }
    
    void setConfigName(const juce::String& name)
    {
        nameEditor.setText(name);
        nameEditor.selectAll();
    }
    
    // Show dialog and return result
    static void showDialog(juce::Component* parent, const juce::String& defaultName,
                          std::function<void(const juce::String&)> onSave)
    {
        auto* dialog = new ConfigNameDialog(defaultName);
        
        dialog->onOkClicked = [dialog, onSave](const juce::String& name) {
            if (!name.trim().isEmpty() && onSave)
                onSave(name.trim());
            delete dialog;
        };
        
        dialog->onCancelClicked = [dialog]() {
            delete dialog;
        };
        
        // Position dialog at center of parent
        if (parent)
        {
            auto parentBounds = parent->getScreenBounds();
            auto dialogBounds = dialog->getBounds();
            int x = parentBounds.getCentreX() - (dialogBounds.getWidth() / 2);
            int y = parentBounds.getCentreY() - (dialogBounds.getHeight() / 2);
            dialog->setTopLeftPosition(x, y);
        }
        
        dialog->addToDesktop(juce::ComponentPeer::windowIsTemporary | 
                           juce::ComponentPeer::windowHasDropShadow);
        dialog->grabKeyboardFocus();
        dialog->setVisible(true);
    }
    
    std::function<void(const juce::String&)> onOkClicked;
    std::function<void()> onCancelClicked;
    
private:
    juce::String defaultName;
    juce::Label titleLabel;
    juce::TextEditor nameEditor;
    juce::TextButton okButton;
    juce::TextButton cancelButton;
    CustomButtonLookAndFeel buttonLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigNameDialog)
};