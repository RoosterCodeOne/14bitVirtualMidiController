// AutomationSaveDialog.h - Modal dialog for naming automation configurations
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"

//==============================================================================
class AutomationSaveDialog : public juce::Component
{
public:
    AutomationSaveDialog(const juce::String& initialName = "")
        : configName(initialName)
    {
        setupComponents();
        setupLayout();
        
        // Set initial focus to text editor
        nameEditor.grabKeyboardFocus();
        nameEditor.selectAll();
    }
    
    ~AutomationSaveDialog() = default;
    
    // Get/Set config name
    juce::String getConfigName() const { return nameEditor.getText(); }
    void setConfigName(const juce::String& name) 
    { 
        configName = name;
        nameEditor.setText(name);
        nameEditor.selectAll();
    }
    
    // Dialog result callbacks
    std::function<void(const juce::String&)> onSave;
    std::function<void()> onCancel;
    
    // Component overrides
    void resized() override
    {
        auto area = getLocalBounds().reduced(20);
        
        // Title area
        auto titleArea = area.removeFromTop(30);
        titleLabel.setBounds(titleArea);
        
        area.removeFromTop(10); // spacing
        
        // Input area
        auto inputArea = area.removeFromTop(25);
        auto labelArea = inputArea.removeFromLeft(80);
        nameLabel.setBounds(labelArea);
        inputArea.removeFromLeft(10); // spacing
        nameEditor.setBounds(inputArea);
        
        area.removeFromTop(15); // spacing
        
        // Button area
        auto buttonArea = area.removeFromBottom(30);
        int buttonWidth = 80;
        int buttonSpacing = 10;
        
        cancelButton.setBounds(buttonArea.removeFromRight(buttonWidth));
        buttonArea.removeFromRight(buttonSpacing);
        saveButton.setBounds(buttonArea.removeFromRight(buttonWidth));
    }
    
    void paint(juce::Graphics& g) override
    {
        // Draw background with blueprint style
        g.fillAll(BlueprintColors::background);
        
        // Draw border
        g.setColour(BlueprintColors::active);
        g.drawRect(getLocalBounds(), 2);
        
        // Draw title underline
        auto titleBounds = titleLabel.getBounds();
        g.drawHorizontalLine(titleBounds.getBottom() + 5, 
                           static_cast<float>(titleBounds.getX()), 
                           static_cast<float>(titleBounds.getRight()));
    }
    
    bool keyPressed(const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::returnKey)
        {
            handleSave();
            return true;
        }
        else if (key == juce::KeyPress::escapeKey)
        {
            handleCancel();
            return true;
        }
        
        return Component::keyPressed(key);
    }
    
    // Validation
    bool isValidName() const
    {
        juce::String name = nameEditor.getText().trim();
        return !name.isEmpty() && name.length() <= 50; // Max 50 characters
    }
    
    juce::String getValidationError() const
    {
        juce::String name = nameEditor.getText().trim();
        
        if (name.isEmpty())
            return "Config name cannot be empty";
        
        if (name.length() > 50)
            return "Config name too long (max 50 characters)";
        
        // Check for invalid characters (optional)
        if (name.containsAnyOf("/<>:\"|?*\\"))
            return "Config name contains invalid characters";
        
        return juce::String(); // No error
    }
    
private:
    juce::String configName;
    
    // UI Components
    juce::Label titleLabel;
    juce::Label nameLabel;
    juce::TextEditor nameEditor;
    juce::TextButton saveButton;
    juce::TextButton cancelButton;
    juce::Label errorLabel;
    
    CustomButtonLookAndFeel buttonLookAndFeel;
    
    void setupComponents()
    {
        // Title label
        addAndMakeVisible(titleLabel);
        titleLabel.setText("Save Automation Config", juce::dontSendNotification);
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        titleLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        
        // Name label
        addAndMakeVisible(nameLabel);
        nameLabel.setText("Name:", juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centredRight);
        nameLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        nameLabel.setFont(juce::FontOptions(12.0f));
        
        // Name editor
        addAndMakeVisible(nameEditor);
        nameEditor.setText(configName);
        nameEditor.setMultiLine(false);
        nameEditor.setReturnKeyStartsNewLine(false);
        nameEditor.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::panel);
        nameEditor.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
        nameEditor.setColour(juce::TextEditor::highlightColourId, BlueprintColors::active.withAlpha(0.3f));
        nameEditor.setColour(juce::TextEditor::outlineColourId, BlueprintColors::active);
        nameEditor.setColour(juce::TextEditor::focusedOutlineColourId, BlueprintColors::active);
        nameEditor.setFont(juce::FontOptions(12.0f));
        
        // Text change listener for real-time validation
        nameEditor.onTextChange = [this]() {
            updateValidationState();
        };
        
        // Save button
        addAndMakeVisible(saveButton);
        saveButton.setButtonText("Save");
        saveButton.setLookAndFeel(&buttonLookAndFeel);
        saveButton.onClick = [this]() { handleSave(); };
        
        // Cancel button
        addAndMakeVisible(cancelButton);
        cancelButton.setButtonText("Cancel");
        cancelButton.setLookAndFeel(&buttonLookAndFeel);
        cancelButton.onClick = [this]() { handleCancel(); };
        
        // Error label (initially hidden)
        addAndMakeVisible(errorLabel);
        errorLabel.setJustificationType(juce::Justification::centred);
        errorLabel.setColour(juce::Label::textColourId, juce::Colours::red);
        errorLabel.setFont(juce::FontOptions(10.0f));
        errorLabel.setVisible(false);
    }
    
    void setupLayout()
    {
        // Set preferred size
        setSize(350, 150);
    }
    
    void handleSave()
    {
        DBG("AutomationSaveDialog::handleSave() called");
        
        if (isValidName())
        {
            juce::String name = nameEditor.getText().trim();
            DBG("Name is valid: '" + name + "', checking onSave callback...");
            
            if (onSave)
            {
                DBG("onSave callback exists, calling it...");
                onSave(name);
                DBG("onSave callback completed successfully");
            }
            else
            {
                DBG("ERROR: onSave callback is null!");
            }
        }
        else
        {
            DBG("Name validation failed: " + getValidationError());
            // Show validation error
            showError(getValidationError());
        }
    }
    
    void handleCancel()
    {
        DBG("AutomationSaveDialog::handleCancel() called");
        if (onCancel)
        {
            DBG("onCancel callback exists, calling it...");
            onCancel();
            DBG("onCancel callback completed successfully");
        }
        else
        {
            DBG("ERROR: onCancel callback is null!");
        }
    }
    
    void updateValidationState()
    {
        if (isValidName())
        {
            hideError();
            saveButton.setEnabled(true);
        }
        else
        {
            saveButton.setEnabled(false);
            // Don't show error immediately while typing, only on attempt to save
        }
    }
    
    void showError(const juce::String& message)
    {
        errorLabel.setText(message, juce::dontSendNotification);
        errorLabel.setVisible(true);
        
        // Expand dialog to show error
        setSize(getWidth(), 170);
        
        // Position error label
        auto bounds = getLocalBounds().reduced(20);
        bounds.removeFromTop(95); // Skip existing content
        errorLabel.setBounds(bounds.removeFromTop(20));
        
        repaint();
    }
    
    void hideError()
    {
        if (errorLabel.isVisible())
        {
            errorLabel.setVisible(false);
            setSize(getWidth(), 150); // Shrink back to original size
            repaint();
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationSaveDialog)
};

//==============================================================================
// Modal dialog wrapper for easier integration
class AutomationSaveDialogWindow : public juce::DocumentWindow
{
public:
    AutomationSaveDialogWindow(const juce::String& initialName = "")
        : juce::DocumentWindow("Save Automation Config", 
                              BlueprintColors::background, 
                              juce::DocumentWindow::closeButton)
        , dialog(initialName)
    {
        setUsingNativeTitleBar(true);
        setResizable(false, false);
        
        // Set up dialog callbacks
        dialog.onSave = [this](const juce::String& name) {
            configName = name;
            userClickedSave = true;
            setVisible(false);
        };
        
        dialog.onCancel = [this]() {
            userClickedSave = false;
            setVisible(false);
        };
        
        setContentNonOwned(&dialog, true);
        centreWithSize(dialog.getWidth(), dialog.getHeight());
        
        setAlwaysOnTop(true);
    }
    
    ~AutomationSaveDialogWindow() override = default;
    
    void closeButtonPressed() override
    {
        userClickedSave = false;
        setVisible(false);
    }
    
    // Show dialog and return result
    static std::pair<bool, juce::String> showDialog(const juce::String& initialName = "")
    {
        AutomationSaveDialogWindow window(initialName);
        window.setVisible(true);
        
        // Run modal loop
        window.enterModalState(true);
        
        return { window.userClickedSave, window.configName };
    }
    
private:
    AutomationSaveDialog dialog;
    bool userClickedSave = false;
    juce::String configName;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationSaveDialogWindow)
};