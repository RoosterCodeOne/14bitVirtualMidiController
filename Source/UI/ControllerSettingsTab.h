// ControllerSettingsTab.h - MIDI/BPM/Bank/Slider Configuration Tab
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"
#include "../SimpleSliderControl.h"
#include "../Core/SliderDisplayManager.h"
#include "GlobalUIScale.h"

// Forward declaration
class SettingsWindow;

//==============================================================================
class ControllerSettingsTab : public juce::Component, 
                               public GlobalUIScale::ScaleChangeListener
{
public:
    ControllerSettingsTab(SettingsWindow* parentWindow);
    ~ControllerSettingsTab();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;
    void mouseDown(const juce::MouseEvent& event) override;
    
    // Scale change notification implementation
    void scaleFactorChanged(float newScale) override;
    
    // Public interface for main window coordination
    void updateControlsForSelectedSlider(int sliderIndex);
    void updateBankSelectorAppearance(int selectedBank);
    void applyPreset(const ControllerPreset& preset);
    void setSliderSettings(int ccNumber, double rangeMin, double rangeMax, 
                          double increment, bool isCustomStep, bool useDeadzone, int colorId,
                          SliderOrientation orientation = SliderOrientation::Normal,
                          const juce::String& customName = "", SnapThreshold snapThreshold = SnapThreshold::Medium,
                          bool showAutomation = true);
    
    // Access methods for main window (MIDI Channel and BPM moved to GlobalSettingsTab)
    
    // Getter methods for current slider settings (for parent coordination)
    int getCurrentCCNumber() const { return ccNumberInput.getText().getIntValue(); }
    double getCurrentRangeMin() const { return rangeMinInput.getText().getDoubleValue(); }
    double getCurrentRangeMax() const { return rangeMaxInput.getText().getDoubleValue(); }
    double getCurrentIncrement() const { return incrementsInput.getText().getDoubleValue(); }
    bool getCurrentIsCustomStep() const { return isCustomStepFlag; }
    bool getCurrentUseDeadzone() const { return deadzoneButton.getToggleState(); }
    int getCurrentColorId() const { return currentColorId; }
    SliderOrientation getCurrentOrientation() const { return static_cast<SliderOrientation>(orientationCombo.getSelectedId() - 1); }
    juce::String getCurrentCustomName() const { return nameInput.getText(); }
    SnapThreshold getCurrentSnapThreshold() const 
    {
        if (snapSmallButton.getToggleState()) return SnapThreshold::Small;
        if (snapLargeButton.getToggleState()) return SnapThreshold::Large;
        return SnapThreshold::Medium; // Default
    }
    bool getCurrentShowAutomation() const { return showAutomationButton.getToggleState(); }
    
    // Callback functions for communication with parent
    std::function<void()> onSettingsChanged;
    std::function<void(int)> onBankSelected;
    std::function<void(int)> onSliderSettingChanged;
    std::function<void(int)> onSliderSelectionChanged; // For cycling without saving settings
    std::function<void(int)> onSliderReset; // For reset button action tracking
    std::function<void()> onRequestFocus; // Callback to request focus restoration
    
private:
    SettingsWindow* parentWindow;
    CustomButtonLookAndFeel customButtonLookAndFeel;
    int currentColorId = 1; // Track current color selection
    bool isCustomStepFlag = false; // Track whether step is custom or auto
    
    // MIDI Channel and BPM controls moved to GlobalSettingsTab
    
    // Bank selector
    juce::Label bankSelectorLabel;
    ClickableLabel bankASelector, bankBSelector, bankCSelector, bankDSelector;
    int selectedBank = 0;
    int selectedSlider = 0;
    
    // Breadcrumb navigation
    juce::Label breadcrumbLabel;
    
    // Name input controls
    juce::Label nameLabel;
    juce::TextEditor nameInput;
    
    // Section headers
    juce::Label section1Header, section2Header, section3Header;
    
    // Section 1 - Core MIDI
    juce::Label ccNumberLabel;
    juce::TextEditor ccNumberInput;
    // outputModeLabel removed - no longer needed without 7-bit/14-bit toggle
    
    // Section 2 - Display & Range
    juce::Label rangeLabel;
    juce::TextEditor rangeMinInput, rangeMaxInput;
    juce::Label rangeDashLabel;
    juce::Label incrementsLabel;
    juce::TextEditor incrementsInput;
    juce::TextButton autoStepButton;
    juce::Label orientationLabel;
    juce::ComboBox orientationCombo;
    juce::Label snapLabel;
    juce::ToggleButton snapSmallButton, snapMediumButton, snapLargeButton;
    juce::Label automationVisibilityLabel;
    juce::ToggleButton showAutomationButton;
    
    // Section 3 - Input Behavior
    juce::Label inputModeLabel;
    juce::ToggleButton deadzoneButton, directButton;
    
    // Section 4 - Visual
    juce::Label colorPickerLabel;
    juce::OwnedArray<juce::TextButton> colorButtons; // 4x2 grid
    juce::TextButton resetSliderButton;
    
    // Current color box and grid system
    class ColorBox : public juce::Component
    {
    public:
        ColorBox() = default;
        
        void paint(juce::Graphics& g) override
        {
            // Draw filled rectangle with current color
            g.setColour(currentColor);
            g.fillRect(getLocalBounds().reduced(1));
            
            // Draw border
            g.setColour(BlueprintColors::blueprintLines);
            g.drawRect(getLocalBounds(), 1);
        }
        
        void mouseDown(const juce::MouseEvent& event) override
        {
            if (onClicked)
                onClicked();
        }
        
        void setCurrentColor(juce::Colour color)
        {
            currentColor = color;
            repaint();
        }
        
        juce::Colour getCurrentColor() const { return currentColor; }
        
        std::function<void()> onClicked;
        
    private:
        juce::Colour currentColor = juce::Colours::cyan;
    };
    
    ColorBox currentColorBox;
    bool colorGridVisible = false;
    juce::Rectangle<int> colorGridBounds;
    
    // Private methods
    void setupBankSelector();
    
    // Color grid management
    void showColorGrid();
    void hideColorGrid();
    void paintColorGrid(juce::Graphics& g);
    void handleColorGridClick(const juce::MouseEvent& event);
    int calculateColorIndexFromPosition(const juce::Point<int>& position);
    juce::Colour getColorById(int colorId);
    void setupNameControls();
    void setupPerSliderControls();
    void layoutPerSliderSections(juce::Rectangle<int>& bounds);
    void cycleSliderInBank(int bankIndex);
    void updateBreadcrumbLabel();
    void updateColorButtonSelection();
    
    // Validation and application methods
    void validateAndApplyCCNumber();
    void validateAndApplyRange();
    void applyIncrements();
    void setAutoStepMode();
    void updateStepIndicationVisuals();
    void applyInputMode();
    void applyOrientation();
    void applySnapThreshold();
    void applyCustomName();
    void applyAutomationVisibility();
    void selectColor(int colorId);
    void resetCurrentSlider();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllerSettingsTab)
};

//==============================================================================
// Implementation
inline ControllerSettingsTab::ControllerSettingsTab(SettingsWindow* parent)
    : parentWindow(parent)
{
    setupBankSelector();
    setupNameControls();
    setupPerSliderControls();
    
    // Enable keyboard focus for tab
    setWantsKeyboardFocus(true);
    
    // Register for scale change notifications
    GlobalUIScale::getInstance().addScaleChangeListener(this);
}

inline ControllerSettingsTab::~ControllerSettingsTab()
{
    // Remove scale change listener
    GlobalUIScale::getInstance().removeScaleChangeListener(this);
    
    // Clean up custom look and feel
    resetSliderButton.setLookAndFeel(nullptr);
    autoStepButton.setLookAndFeel(nullptr);
}

inline void ControllerSettingsTab::paint(juce::Graphics& g)
{
    auto& scale = GlobalUIScale::getInstance();
    
    // Blueprint aesthetic background
    g.setColour(BlueprintColors::windowBackground);
    g.fillAll();
    
    // Draw section backgrounds for the 3-section structure
    auto bounds = getLocalBounds().reduced(scale.getScaled(15));
    
    // Calculate section positions based on new layout
    const int sectionSpacing = scale.getScaled(8);
    const int controlSpacing = scale.getScaled(4);
    const int labelHeight = scale.getScaled(18);
    const int headerHeight = scale.getScaled(22);
    
    // Skip Breadcrumb (no background box)
    bounds.removeFromTop(scale.getScaled(20 + 6));
    
    // Skip Bank selector (no background box)
    bounds.removeFromTop(scale.getScaled(22) + sectionSpacing);
    
    // Section 1 - Slider Configuration Box
    auto section2Height = headerHeight + (labelHeight + controlSpacing) * 4 + controlSpacing;
    auto section2Bounds = bounds.removeFromTop(section2Height);
    section2Bounds = section2Bounds.expanded(scale.getScaled(8), scale.getScaled(4));
    
    g.setColour(BlueprintColors::sectionBackground);
    g.fillRoundedRectangle(section2Bounds.toFloat(), scale.getScaled(4.0f));
    g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
    g.drawRoundedRectangle(section2Bounds.toFloat(), scale.getScaled(4.0f), scale.getScaledLineThickness());
    
    bounds.removeFromTop(sectionSpacing);
    
    // Section 2 - Display & Range Box (expanded)
    auto section3Height = headerHeight + (labelHeight + controlSpacing) * 6 + scale.getScaled(60) + controlSpacing * 2;
    auto section3Bounds = bounds.removeFromTop(section3Height);
    section3Bounds = section3Bounds.expanded(scale.getScaled(8), scale.getScaled(4));
    
    g.setColour(BlueprintColors::sectionBackground);
    g.fillRoundedRectangle(section3Bounds.toFloat(), scale.getScaled(4.0f));
    g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
    g.drawRoundedRectangle(section3Bounds.toFloat(), scale.getScaled(4.0f), scale.getScaledLineThickness());
    
    // Paint color grid if visible (appears on top of everything)
    paintColorGrid(g);
}

inline void ControllerSettingsTab::resized()
{
    auto& scale = GlobalUIScale::getInstance();
    auto bounds = getLocalBounds().reduced(scale.getScaled(15));
    
    const int sectionSpacing = scale.getScaled(8);
    const int controlSpacing = scale.getScaled(4);
    const int labelHeight = scale.getScaled(18);
    const int headerHeight = scale.getScaled(22);
    
    // Breadcrumb (no section box) - at the top
    auto breadcrumbArea = bounds.removeFromTop(scale.getScaled(20));
    breadcrumbLabel.setBounds(breadcrumbArea);
    
    bounds.removeFromTop(scale.getScaled(6));
    
    // Bank selector (no section box)
    auto bankSelectorArea = bounds.removeFromTop(scale.getScaled(22));
    bankSelectorLabel.setBounds(bankSelectorArea.removeFromLeft(scale.getScaled(40)));
    bankSelectorArea.removeFromLeft(scale.getScaled(8));
    
    int bankButtonWidth = (bankSelectorArea.getWidth() - scale.getScaled(21)) / 4;
    bankASelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
    bankSelectorArea.removeFromLeft(scale.getScaled(7));
    bankBSelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
    bankSelectorArea.removeFromLeft(scale.getScaled(7));
    bankCSelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
    bankSelectorArea.removeFromLeft(scale.getScaled(7));
    bankDSelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
    
    bounds.removeFromTop(sectionSpacing);
    
    // Layout the remaining 2 sections (Slider Configuration and Display & Range)
    layoutPerSliderSections(bounds);
}

inline bool ControllerSettingsTab::keyPressed(const juce::KeyPress& key)
{
    // Since we can't call parentWindow->keyPressed() due to incomplete type,
    // we need to handle common navigation keys here and let others bubble up
    if (key == juce::KeyPress::escapeKey || 
        key == juce::KeyPress::upKey || 
        key == juce::KeyPress::downKey ||
        key == juce::KeyPress::leftKey ||
        key == juce::KeyPress::rightKey)
    {
        // Let parent handle these navigation keys
        return false; // Allow parent to handle
    }
    return Component::keyPressed(key);
}

inline void ControllerSettingsTab::mouseDown(const juce::MouseEvent& event)
{
    // Handle color grid interaction first
    if (colorGridVisible)
    {
        if (colorGridBounds.expanded(8).contains(event.getPosition()))
        {
            handleColorGridClick(event);
            return; // Don't process other mouse handling
        }
        else
        {
            hideColorGrid(); // Click outside grid hides it
            return;
        }
    }
    
    // Handle mouse event normally
    Component::mouseDown(event);
    
    // Restore focus to parent SettingsWindow after mouse click
    // This ensures keyboard navigation continues to work
    if (onRequestFocus)
        onRequestFocus();
}

inline void ControllerSettingsTab::handleColorGridClick(const juce::MouseEvent& event)
{
    // Calculate which color was clicked based on grid position
    auto relativePos = event.getPosition() - colorGridBounds.getTopLeft();
    int gridIndex = calculateColorIndexFromPosition(relativePos);
    
    DBG("Grid click - calculated grid index: " << gridIndex);
    
    if (gridIndex >= 0 && gridIndex < 8)
    {
        // Grid uses 0-7, but we need to store the actual colorId (which is gridIndex for direct mapping)
        currentColorId = gridIndex;
        juce::Colour selectedColor = getColorById(gridIndex);
        
        DBG("Grid click - setting color ID: " << currentColorId);
        DBG("Grid click - color is: " << selectedColor.toString());
        
        currentColorBox.setCurrentColor(selectedColor);
        
        // Hide grid
        hideColorGrid();
        
        // Apply color change
        if (onSliderSettingChanged)
            onSliderSettingChanged(selectedSlider);
    }
}


inline void ControllerSettingsTab::setupBankSelector()
{
    // Breadcrumb label
    addAndMakeVisible(breadcrumbLabel);
    breadcrumbLabel.setText("Bank A > Slider 1", juce::dontSendNotification);
    breadcrumbLabel.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    breadcrumbLabel.setColour(juce::Label::textColourId, BlueprintColors::active);
    breadcrumbLabel.setJustificationType(juce::Justification::centredLeft);
    
    addAndMakeVisible(bankSelectorLabel);
    bankSelectorLabel.setText("Bank:", juce::dontSendNotification);
    bankSelectorLabel.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    bankSelectorLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    // Bank selector buttons with proper colors
    addAndMakeVisible(bankASelector);
    bankASelector.setText("A", juce::dontSendNotification);
    bankASelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    bankASelector.setJustificationType(juce::Justification::centred);
    bankASelector.setColour(juce::Label::backgroundColourId, juce::Colours::red); // Start with A selected
    bankASelector.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    bankASelector.onClick = [this]() { 
        cycleSliderInBank(0);
        // Restore focus to parent after bank selection
        if (onRequestFocus) onRequestFocus();
    };
    
    addAndMakeVisible(bankBSelector);
    bankBSelector.setText("B", juce::dontSendNotification);
    bankBSelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    bankBSelector.setJustificationType(juce::Justification::centred);
    bankBSelector.setColour(juce::Label::backgroundColourId, juce::Colours::blue.withAlpha(0.3f)); // Inactive blue
    bankBSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
    bankBSelector.onClick = [this]() { 
        cycleSliderInBank(1);
        // Restore focus to parent after bank selection
        if (onRequestFocus) onRequestFocus();
    };
    
    addAndMakeVisible(bankCSelector);
    bankCSelector.setText("C", juce::dontSendNotification);
    bankCSelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    bankCSelector.setJustificationType(juce::Justification::centred);
    bankCSelector.setColour(juce::Label::backgroundColourId, juce::Colours::green.withAlpha(0.3f)); // Inactive green
    bankCSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
    bankCSelector.onClick = [this]() { 
        cycleSliderInBank(2);
        // Restore focus to parent after bank selection
        if (onRequestFocus) onRequestFocus();
    };
    
    addAndMakeVisible(bankDSelector);
    bankDSelector.setText("D", juce::dontSendNotification);
    bankDSelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    bankDSelector.setJustificationType(juce::Justification::centred);
    bankDSelector.setColour(juce::Label::backgroundColourId, juce::Colours::yellow.withAlpha(0.3f)); // Inactive yellow
    bankDSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
    bankDSelector.onClick = [this]() { 
        cycleSliderInBank(3);
        // Restore focus to parent after bank selection
        if (onRequestFocus) onRequestFocus();
    };
}

inline void ControllerSettingsTab::setupNameControls()
{
    // Name label
    addAndMakeVisible(nameLabel);
    nameLabel.setText("Name:", juce::dontSendNotification);
    nameLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    nameLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    // Name input
    addAndMakeVisible(nameInput);
    nameInput.setInputRestrictions(20); // 20 character limit
    nameInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
    nameInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
    nameInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
    nameInput.onReturnKey = [this]() { nameInput.moveKeyboardFocusToSibling(true); };
    nameInput.onFocusLost = [this]() { applyCustomName(); };
    nameInput.onTextChange = [this]() { applyCustomName(); };
    // Set font after all other properties are configured
    nameInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
}

inline void ControllerSettingsTab::setupPerSliderControls()
{
    // Section 1 - Slider Configuration
    addAndMakeVisible(section1Header);
    section1Header.setText("Slider Configuration", juce::dontSendNotification);
    section1Header.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    section1Header.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(ccNumberLabel);
    ccNumberLabel.setText("MIDI CC Number:", juce::dontSendNotification);
    ccNumberLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    ccNumberLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(ccNumberInput);
    ccNumberInput.setInputRestrictions(3, "0123456789");
    ccNumberInput.setTooltip("MIDI CC number (0-127)");
    ccNumberInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
    ccNumberInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
    ccNumberInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
    ccNumberInput.onReturnKey = [this]() { ccNumberInput.moveKeyboardFocusToSibling(true); };
    ccNumberInput.onFocusLost = [this]() { validateAndApplyCCNumber(); };
    // Set font after all other properties are configured
    ccNumberInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    
    // outputModeLabel setup removed - no longer needed
    
    
    // Input Behavior controls (moved to Slider Configuration section)
    addAndMakeVisible(inputModeLabel);
    inputModeLabel.setText("Input Behavior:", juce::dontSendNotification);
    inputModeLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    inputModeLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(deadzoneButton);
    deadzoneButton.setButtonText("Deadzone");
    deadzoneButton.setRadioGroupId(2);
    deadzoneButton.setToggleState(true, juce::dontSendNotification);
    deadzoneButton.onClick = [this]() { 
        applyInputMode();
        if (onRequestFocus) onRequestFocus();
    };
    
    addAndMakeVisible(directButton);
    directButton.setButtonText("Direct");
    directButton.setRadioGroupId(2);
    directButton.onClick = [this]() { 
        applyInputMode();
        if (onRequestFocus) onRequestFocus();
    };
    
    // Section 2 - Display & Range
    addAndMakeVisible(section2Header);
    section2Header.setText("Display & Range", juce::dontSendNotification);
    section2Header.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    section2Header.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(rangeLabel);
    rangeLabel.setText("Range:", juce::dontSendNotification);
    rangeLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    rangeLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(rangeMinInput);
    rangeMinInput.setInputRestrictions(0, "-0123456789.");
    rangeMinInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
    rangeMinInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
    rangeMinInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
    rangeMinInput.onReturnKey = [this]() { rangeMinInput.moveKeyboardFocusToSibling(true); };
    rangeMinInput.onFocusLost = [this]() { validateAndApplyRange(); };
    // Set font after all other properties are configured
    rangeMinInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    
    addAndMakeVisible(rangeDashLabel);
    rangeDashLabel.setText("-", juce::dontSendNotification);
    rangeDashLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    rangeDashLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    rangeDashLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(rangeMaxInput);
    rangeMaxInput.setInputRestrictions(0, "-0123456789.");
    rangeMaxInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
    rangeMaxInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
    rangeMaxInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
    rangeMaxInput.onReturnKey = [this]() { rangeMaxInput.moveKeyboardFocusToSibling(true); };
    rangeMaxInput.onFocusLost = [this]() { validateAndApplyRange(); };
    // Set font after all other properties are configured
    rangeMaxInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    
    
    addAndMakeVisible(incrementsLabel);
    incrementsLabel.setText("Custom Steps:", juce::dontSendNotification);
    incrementsLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    incrementsLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(incrementsInput);
    incrementsInput.setInputRestrictions(0, "0123456789.");
    incrementsInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
    incrementsInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
    incrementsInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
    incrementsInput.onReturnKey = [this]() { incrementsInput.moveKeyboardFocusToSibling(true); };
    incrementsInput.onFocusLost = [this]() { applyIncrements(); };
    incrementsInput.onTextChange = [this]() { 
        // Mark as custom step when user manually changes the value
        isCustomStepFlag = true;
    };
    // Set font after all other properties are configured
    incrementsInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    
    // Auto step button
    addAndMakeVisible(autoStepButton);
    autoStepButton.setButtonText("Auto");
    autoStepButton.setLookAndFeel(&customButtonLookAndFeel);
    autoStepButton.onClick = [this]() { 
        setAutoStepMode();
    };
    
    // Orientation controls
    addAndMakeVisible(orientationLabel);
    orientationLabel.setText("Orientation:", juce::dontSendNotification);
    orientationLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    orientationLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(orientationCombo);
    orientationCombo.addItem("Normal", static_cast<int>(SliderOrientation::Normal) + 1);
    orientationCombo.addItem("Inverted", static_cast<int>(SliderOrientation::Inverted) + 1);
    orientationCombo.addItem("Bipolar", static_cast<int>(SliderOrientation::Bipolar) + 1);
    orientationCombo.setSelectedId(static_cast<int>(SliderOrientation::Normal) + 1);
    orientationCombo.setColour(juce::ComboBox::backgroundColourId, BlueprintColors::background);
    orientationCombo.setColour(juce::ComboBox::textColourId, BlueprintColors::textPrimary);
    orientationCombo.setColour(juce::ComboBox::outlineColourId, BlueprintColors::blueprintLines);
    orientationCombo.onChange = [this]() { 
        applyOrientation();
        if (onRequestFocus) onRequestFocus();
    };
    
    // Center value controls removed - center is now automatically calculated
    
    // Snap threshold controls (small radio buttons)
    addAndMakeVisible(snapLabel);
    snapLabel.setText("Snap:", juce::dontSendNotification);
    snapLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    snapLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    snapLabel.setVisible(false); // Initially hidden
    
    addAndMakeVisible(snapSmallButton);
    snapSmallButton.setButtonText("S");
    snapSmallButton.setLookAndFeel(&customButtonLookAndFeel);
    snapSmallButton.setRadioGroupId(100); // Snap group
    snapSmallButton.onClick = [this]() { applySnapThreshold(); if (onRequestFocus) onRequestFocus(); };
    snapSmallButton.setVisible(false);
    
    addAndMakeVisible(snapMediumButton);
    snapMediumButton.setButtonText("M");
    snapMediumButton.setLookAndFeel(&customButtonLookAndFeel);
    snapMediumButton.setRadioGroupId(100); // Snap group
    snapMediumButton.setToggleState(true, juce::dontSendNotification); // Default
    snapMediumButton.onClick = [this]() { applySnapThreshold(); if (onRequestFocus) onRequestFocus(); };
    snapMediumButton.setVisible(false);
    
    addAndMakeVisible(snapLargeButton);
    snapLargeButton.setButtonText("L");
    snapLargeButton.setLookAndFeel(&customButtonLookAndFeel);
    snapLargeButton.setRadioGroupId(100); // Snap group
    snapLargeButton.onClick = [this]() { applySnapThreshold(); if (onRequestFocus) onRequestFocus(); };
    snapLargeButton.setVisible(false);
    
    // Automation Visibility controls (moved to Display & Range section)
    addAndMakeVisible(automationVisibilityLabel);
    automationVisibilityLabel.setText("Show Automation:", juce::dontSendNotification);
    automationVisibilityLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    automationVisibilityLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(showAutomationButton);
    showAutomationButton.setButtonText("Show Controls");
    showAutomationButton.setToggleState(true, juce::dontSendNotification); // Default to showing automation
    showAutomationButton.onClick = [this]() { 
        applyAutomationVisibility();
        if (onRequestFocus) onRequestFocus();
    };
    
    // Color controls (moved to Display & Range section)
    addAndMakeVisible(colorPickerLabel);
    colorPickerLabel.setText("Color:", juce::dontSendNotification);
    colorPickerLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    colorPickerLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    // Add current color box
    addAndMakeVisible(currentColorBox);
    currentColorBox.onClicked = [this]() { 
        if (colorGridVisible)
            hideColorGrid();
        else
            showColorGrid();
    };
    
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
        colorButton->onClick = [this, i]() { 
            selectColor(i); // Direct mapping - no +2 offset needed
            // Restore focus to parent after color selection
            if (onRequestFocus) onRequestFocus();
        };
    }
    
    // Reset button (moved to Display & Range section)
    addAndMakeVisible(resetSliderButton);
    resetSliderButton.setButtonText("Reset Slider");
    resetSliderButton.setLookAndFeel(&customButtonLookAndFeel);
    resetSliderButton.onClick = [this]() { 
        resetCurrentSlider();
        // Notify about reset action
        if (onSliderReset) onSliderReset(selectedSlider);
        // Restore focus to parent after reset
        if (onRequestFocus) onRequestFocus();
    };
}

inline void ControllerSettingsTab::layoutPerSliderSections(juce::Rectangle<int>& bounds)
{
    auto& scale = GlobalUIScale::getInstance();
    const int sectionSpacing = scale.getScaled(8);
    const int controlSpacing = scale.getScaled(4);
    const int labelHeight = scale.getScaled(18);
    const int inputHeight = scale.getScaled(22);
    const int headerHeight = scale.getScaled(22);
    
    // Section 2 - Slider Configuration (Name, CC Number, Input Behavior)
    auto section2Bounds = bounds.removeFromTop(headerHeight + (labelHeight + controlSpacing) * 4 + controlSpacing);
    
    section2Header.setBounds(section2Bounds.removeFromTop(headerHeight));
    section2Bounds.removeFromTop(controlSpacing);
    
    // Name row
    auto nameRow = section2Bounds.removeFromTop(labelHeight);
    nameLabel.setBounds(nameRow.removeFromLeft(scale.getScaled(60)));
    nameRow.removeFromLeft(scale.getScaled(8));
    nameInput.setBounds(nameRow.removeFromLeft(scale.getScaled(200)));
    
    section2Bounds.removeFromTop(controlSpacing);
    
    // CC Number row
    auto ccRow = section2Bounds.removeFromTop(labelHeight);
    ccNumberLabel.setBounds(ccRow.removeFromLeft(scale.getScaled(120)));
    ccRow.removeFromLeft(scale.getScaled(8));
    ccNumberInput.setBounds(ccRow.removeFromLeft(scale.getScaled(80)));
    
    section2Bounds.removeFromTop(controlSpacing);
    
    // Output mode row
    auto outputRow = section2Bounds.removeFromTop(labelHeight);
    // outputModeLabel bounds removed
    // Output mode buttons removed - always 14-bit
    
    section2Bounds.removeFromTop(controlSpacing);
    
    // Input Behavior row (moved from separate section)
    auto inputModeRow = section2Bounds.removeFromTop(labelHeight);
    inputModeLabel.setBounds(inputModeRow.removeFromLeft(scale.getScaled(120)));
    inputModeRow.removeFromLeft(scale.getScaled(8));
    deadzoneButton.setBounds(inputModeRow.removeFromLeft(scale.getScaled(80)));
    inputModeRow.removeFromLeft(scale.getScaled(8));
    directButton.setBounds(inputModeRow.removeFromLeft(scale.getScaled(60)));
    
    bounds.removeFromTop(sectionSpacing);
    
    // Reserve space for reset button at bottom with spacing above it
    auto resetButtonArea = bounds.removeFromBottom(inputHeight); // Button height
    bounds.removeFromBottom(scale.getScaled(20)); // Blank space above reset button
    
    // Section 3 - Display & Range (expanded to include color, automation visibility)
    auto section3Bounds = bounds.removeFromTop(headerHeight + (labelHeight + controlSpacing) * 6 + controlSpacing * 2);
    
    section3Header.setBounds(section3Bounds.removeFromTop(headerHeight));
    section3Bounds.removeFromTop(controlSpacing);
    
    // Range row
    auto rangeRow = section3Bounds.removeFromTop(labelHeight);
    rangeLabel.setBounds(rangeRow.removeFromLeft(scale.getScaled(50)));
    rangeRow.removeFromLeft(scale.getScaled(4));
    rangeMinInput.setBounds(rangeRow.removeFromLeft(scale.getScaled(80)));
    rangeRow.removeFromLeft(scale.getScaled(2));
    rangeDashLabel.setBounds(rangeRow.removeFromLeft(scale.getScaled(10)));
    rangeRow.removeFromLeft(scale.getScaled(2));
    rangeMaxInput.setBounds(rangeRow.removeFromLeft(scale.getScaled(80)));
    
    section3Bounds.removeFromTop(controlSpacing);
    
    // Increments row
    auto incrementRow = section3Bounds.removeFromTop(labelHeight);
    incrementsLabel.setBounds(incrementRow.removeFromLeft(scale.getScaled(120)));
    incrementRow.removeFromLeft(scale.getScaled(8));
    incrementsInput.setBounds(incrementRow.removeFromLeft(scale.getScaled(70)));
    incrementRow.removeFromLeft(scale.getScaled(4));
    autoStepButton.setBounds(incrementRow.removeFromLeft(scale.getScaled(40)));
    
    section3Bounds.removeFromTop(controlSpacing);
    
    // Orientation row
    auto orientationRow = section3Bounds.removeFromTop(labelHeight);
    orientationLabel.setBounds(orientationRow.removeFromLeft(scale.getScaled(120)));
    orientationRow.removeFromLeft(scale.getScaled(8));
    orientationCombo.setBounds(orientationRow.removeFromLeft(scale.getScaled(80)));
    
    section3Bounds.removeFromTop(controlSpacing);
    
    // Snap controls row (only visible for bipolar mode)
    auto snapRow = section3Bounds.removeFromTop(labelHeight);
    snapLabel.setBounds(snapRow.removeFromLeft(scale.getScaled(40)));
    snapRow.removeFromLeft(scale.getScaled(4));
    snapSmallButton.setBounds(snapRow.removeFromLeft(scale.getScaled(20)));
    snapRow.removeFromLeft(scale.getScaled(2));
    snapMediumButton.setBounds(snapRow.removeFromLeft(scale.getScaled(20)));
    snapRow.removeFromLeft(scale.getScaled(2));
    snapLargeButton.setBounds(snapRow.removeFromLeft(scale.getScaled(20)));
    
    section3Bounds.removeFromTop(controlSpacing);
    
    // Automation Visibility row (moved above color picker)
    auto automationRow = section3Bounds.removeFromTop(labelHeight);
    automationVisibilityLabel.setBounds(automationRow.removeFromLeft(scale.getScaled(120)));
    automationRow.removeFromLeft(scale.getScaled(8));
    showAutomationButton.setBounds(automationRow.removeFromLeft(scale.getScaled(100)));
    
    section3Bounds.removeFromTop(controlSpacing);
    
    // Color section
    auto colorRow = section3Bounds.removeFromTop(labelHeight);
    colorPickerLabel.setBounds(colorRow.removeFromLeft(scale.getScaled(50)));
    colorRow.removeFromLeft(scale.getScaled(8));
    currentColorBox.setBounds(colorRow.removeFromLeft(scale.getScaled(24))); // Small square box
    
    // Reset button at bottom with distinguishing space above it
    resetSliderButton.setBounds(resetButtonArea.reduced(scale.getScaled(20), scale.getScaled(2))); // Center with padding
}

// Color grid management methods
inline void ControllerSettingsTab::showColorGrid()
{
    auto& scale = GlobalUIScale::getInstance();
    colorGridVisible = true;
    
    // Position grid relative to the current color box
    const int buttonSize = scale.getScaled(18); // Base size 18 scaled
    const int buttonGap = scale.getScaled(4);   // Base gap 4 scaled
    const int colorsPerRow = 4;
    const int colorRows = 2;
    const int spacing = scale.getScaled(8);     // Space between color box and grid
    
    int gridWidth = buttonSize * colorsPerRow + (colorsPerRow - 1) * buttonGap;
    int gridHeight = buttonSize * colorRows + (colorRows - 1) * buttonGap;
    
    // Position grid so its top-left aligns with top-right of color box (with spacing)
    auto colorBoxBounds = currentColorBox.getBounds();
    
    colorGridBounds = juce::Rectangle<int>(
        colorBoxBounds.getRight() + spacing + scale.getScaled(15),  // Right of color box with spacing + 15px
        colorBoxBounds.getY() + scale.getScaled(15),                // Align with top of color box + 15px down
        gridWidth,
        gridHeight
    );
    
    repaint();
}

inline void ControllerSettingsTab::hideColorGrid()
{
    colorGridVisible = false;
    repaint();
}

inline void ControllerSettingsTab::paintColorGrid(juce::Graphics& g)
{
    if (!colorGridVisible) return;
    
    auto& scale = GlobalUIScale::getInstance();
    
    // Draw background
    g.setColour(BlueprintColors::sectionBackground);
    g.fillRoundedRectangle(colorGridBounds.expanded(scale.getScaled(8)).toFloat(), scale.getScaled(4.0f));
    g.setColour(BlueprintColors::blueprintLines.withAlpha(0.8f));
    g.drawRoundedRectangle(colorGridBounds.expanded(scale.getScaled(8)).toFloat(), scale.getScaled(4.0f), scale.getScaledLineThickness(2.0f));
    
    // Draw color grid - use same color array as getColorById for consistency
    const juce::Colour colors[] = {
        juce::Colours::red, juce::Colours::blue, juce::Colours::green, juce::Colours::yellow,
        juce::Colours::purple, juce::Colours::orange, juce::Colours::cyan, juce::Colours::white
    };
    
    const int buttonSize = scale.getScaled(18); // Match showColorGrid scaled size
    const int buttonGap = scale.getScaled(4);   // Match showColorGrid scaled gap
    
    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            int index = row * 4 + col;
            if (index < 8)
            {
                int x = colorGridBounds.getX() + col * (buttonSize + buttonGap);
                int y = colorGridBounds.getY() + row * (buttonSize + buttonGap);
                
                juce::Rectangle<int> colorRect(x, y, buttonSize, buttonSize);
                
                // Fill with color
                g.setColour(colors[index]);
                g.fillRect(colorRect);
                
                // Draw border
                g.setColour(BlueprintColors::blueprintLines);
                g.drawRect(colorRect, scale.getScaledLineThickness());
                
                // Highlight current color - match the grid index to currentColorId
                if (index == currentColorId)
                {
                    g.setColour(juce::Colours::white.withAlpha(0.8f));
                    g.drawRect(colorRect, scale.getScaledLineThickness(2.0f));
                }
            }
        }
    }
}

inline int ControllerSettingsTab::calculateColorIndexFromPosition(const juce::Point<int>& position)
{
    auto& scale = GlobalUIScale::getInstance();
    const int buttonSize = scale.getScaled(18); // Match showColorGrid scaled size  
    const int buttonGap = scale.getScaled(4);   // Match showColorGrid scaled gap
    
    int col = position.x / (buttonSize + buttonGap);
    int row = position.y / (buttonSize + buttonGap);
    
    DBG("Position: " << position.toString() << " -> Col: " << col << " Row: " << row);
    
    if (col >= 0 && col < 4 && row >= 0 && row < 2)
    {
        int index = row * 4 + col;
        DBG("Calculated index: " << index);
        return index;
    }
    
    return -1;
}

inline juce::Colour ControllerSettingsTab::getColorById(int colorId)
{
    const juce::Colour colors[] = {
        juce::Colours::red, juce::Colours::blue, juce::Colours::green, juce::Colours::yellow,
        juce::Colours::purple, juce::Colours::orange, juce::Colours::cyan, juce::Colours::white
    };
    
    if (colorId >= 0 && colorId < 8)
        return colors[colorId];
    
    return juce::Colours::cyan; // Default
}

// Additional inline method implementations would continue here...
// Due to length constraints, I'll create this as a header-only implementation for now
// In a production environment, you'd split the implementation into a .cpp file

// setSyncStatus method moved to GlobalSettingsTab

// Complete implementations for validation methods with proper data handling
inline void ControllerSettingsTab::validateAndApplyCCNumber()
{
    auto text = ccNumberInput.getText();
    if (text.isEmpty())
    {
        ccNumberInput.setText("0", juce::dontSendNotification);
        text = "0";
    }
    
    int ccNumber = juce::jlimit(0, 127, text.getIntValue());
    ccNumberInput.setText(juce::String(ccNumber), juce::dontSendNotification);
    
    // Notify parent window of the change with the validated value
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

// applyOutputMode method removed - no longer needed as system always uses 14-bit output

inline void ControllerSettingsTab::validateAndApplyRange()
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
    
    // Bipolar center automatically calculated - no manual update needed
    
    // Auto-recalculate step if in auto mode
    if (!isCustomStepFlag)
    {
        setAutoStepMode();
        return; // setAutoStepMode() already calls onSliderSettingChanged
    }
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}


inline void ControllerSettingsTab::applyIncrements()
{
    auto text = incrementsInput.getText();
    if (text.isEmpty()) { incrementsInput.setText("1", juce::dontSendNotification); text = "1"; }
    
    double increment = juce::jmax(0.001, text.getDoubleValue());
    incrementsInput.setText(juce::String(increment, 3), juce::dontSendNotification);
    
    // Mark as custom step since user manually set the value
    isCustomStepFlag = true;
    
    // Update visual indication
    updateStepIndicationVisuals();
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::setAutoStepMode()
{
    // Calculate auto step based on current range (always 14-bit mode)
    double rangeMin = rangeMinInput.getText().getDoubleValue();
    double rangeMax = rangeMaxInput.getText().getDoubleValue();
    
    int numSteps = 16384; // Always use 14-bit resolution
    double range = std::abs(rangeMax - rangeMin);
    double autoStep = range / (numSteps - 1);
    
    // Set the calculated step
    incrementsInput.setText(juce::String(autoStep, 6), juce::dontSendNotification);
    isCustomStepFlag = false; // Mark as auto-calculated
    
    // Update visual indication
    updateStepIndicationVisuals();
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::updateStepIndicationVisuals()
{
    if (isCustomStepFlag)
    {
        // Custom step mode - normal appearance
        incrementsInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
        incrementsInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
        autoStepButton.setButtonText("Auto");
    }
    else
    {
        // Auto step mode - slightly different appearance to indicate auto-calculated
        incrementsInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background.brighter(0.1f));
        incrementsInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textSecondary);
        autoStepButton.setButtonText("AUTO");
    }
    
    incrementsInput.repaint();
    autoStepButton.repaint();
}

inline void ControllerSettingsTab::applyInputMode()
{
    // Get the deadzone state and notify parent
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::selectColor(int colorId)
{
    DBG("selectColor called with ID: " << colorId);
    juce::Colour selectedColor = getColorById(colorId);
    DBG("selectColor - color is: " << selectedColor.toString());
    
    // Store the selected color and update visual selection
    currentColorId = colorId;
    currentColorBox.setCurrentColor(selectedColor);
    updateColorButtonSelection();
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::applyOrientation()
{
    SliderOrientation newOrientation = getCurrentOrientation();
    
    // Show/hide snap controls based on orientation (only for bipolar mode)
    bool showSnapControls = (newOrientation == SliderOrientation::Bipolar);
    snapLabel.setVisible(showSnapControls);
    snapSmallButton.setVisible(showSnapControls);
    snapMediumButton.setVisible(showSnapControls);
    snapLargeButton.setVisible(showSnapControls);
    
    // Center value is automatically calculated - no manual setting needed
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

// validateAndApplyCenterValue method removed - center value is now automatically calculated

inline void ControllerSettingsTab::applySnapThreshold()
{
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::applyCustomName()
{
    // Get the custom name and notify parent
    juce::String customName = nameInput.getText();
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::resetCurrentSlider()
{
    // Reset all controls to defaults for the current slider
    ccNumberInput.setText(juce::String(selectedSlider), juce::dontSendNotification);
    // Always 14-bit mode - no button to reset
    rangeMinInput.setText("0", juce::dontSendNotification);
    rangeMaxInput.setText("16383", juce::dontSendNotification);
    incrementsInput.setText("1", juce::dontSendNotification);
    deadzoneButton.setToggleState(true, juce::dontSendNotification);
    directButton.setToggleState(false, juce::dontSendNotification);
    nameInput.setText("", juce::dontSendNotification);
    
    // Reset orientation to normal
    orientationCombo.setSelectedId(static_cast<int>(SliderOrientation::Normal) + 1, juce::dontSendNotification);
    snapLabel.setVisible(false);
    snapSmallButton.setVisible(false);
    snapMediumButton.setVisible(false);
    snapLargeButton.setVisible(false);
    
    // Set default color based on bank
    int bankIndex = selectedSlider / 4;
    int defaultColorId = 0; // Red for bank A (direct mapping)
    switch (bankIndex)
    {
        case 0: defaultColorId = 0; break; // Red
        case 1: defaultColorId = 1; break; // Blue  
        case 2: defaultColorId = 2; break; // Green
        case 3: defaultColorId = 3; break; // Yellow
    }
    
    selectColor(defaultColorId);
    
    // Reset Show Automation to default (true)
    showAutomationButton.setToggleState(true, juce::dontSendNotification);
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::cycleSliderInBank(int bankIndex)
{
    int currentBank = selectedSlider / 4;
    
    if (currentBank == bankIndex)
    {
        // Same bank clicked - cycle to next slider in bank (A1->A2->A3->A4->A1)
        int sliderInBank = selectedSlider % 4;
        int nextSliderInBank = (sliderInBank + 1) % 4; // Cycle within bank
        selectedSlider = bankIndex * 4 + nextSliderInBank;
    }
    else
    {
        // Different bank clicked - select first slider in that bank
        selectedSlider = bankIndex * 4; // First slider in bank (0, 4, 8, or 12)
    }
    
    selectedBank = bankIndex;
    updateBreadcrumbLabel();
    updateBankSelectorAppearance(bankIndex);
    
    // Notify parent that slider selection has changed (without saving current settings)
    if (onSliderSelectionChanged)
        onSliderSelectionChanged(selectedSlider);
    
    if (onBankSelected)
        onBankSelected(bankIndex);
}

inline void ControllerSettingsTab::updateBreadcrumbLabel()
{
    char bankLetter = 'A' + selectedBank;
    juce::String sliderName = "Slider " + juce::String(selectedSlider + 1); // Default name
    
    // Use custom name if available from current UI input
    if (!nameInput.getText().isEmpty())
        sliderName = nameInput.getText();
    
    breadcrumbLabel.setText("Bank " + juce::String::charToString(bankLetter) + " > " + sliderName,
                           juce::dontSendNotification);
    breadcrumbLabel.repaint();
}

inline void ControllerSettingsTab::updateBankSelectorAppearance(int selectedBankIndex)
{
    selectedBank = selectedBankIndex;
    
    // Bank A - Red
    auto bankAColor = selectedBank == 0 ? juce::Colours::red : juce::Colours::red.withAlpha(0.3f);
    bankASelector.setColour(juce::Label::backgroundColourId, bankAColor);
    bankASelector.setColour(juce::Label::textColourId, selectedBank == 0 ? BlueprintColors::textPrimary : BlueprintColors::textSecondary);
    bankASelector.repaint();
    
    // Bank B - Blue
    auto bankBColor = selectedBank == 1 ? juce::Colours::blue : juce::Colours::blue.withAlpha(0.3f);
    bankBSelector.setColour(juce::Label::backgroundColourId, bankBColor);
    bankBSelector.setColour(juce::Label::textColourId, selectedBank == 1 ? BlueprintColors::textPrimary : BlueprintColors::textSecondary);
    bankBSelector.repaint();
    
    // Bank C - Green
    auto bankCColor = selectedBank == 2 ? juce::Colours::green : juce::Colours::green.withAlpha(0.3f);
    bankCSelector.setColour(juce::Label::backgroundColourId, bankCColor);
    bankCSelector.setColour(juce::Label::textColourId, selectedBank == 2 ? BlueprintColors::textPrimary : BlueprintColors::textSecondary);
    bankCSelector.repaint();
    
    // Bank D - Yellow
    auto bankDColor = selectedBank == 3 ? juce::Colours::yellow : juce::Colours::yellow.withAlpha(0.3f);
    bankDSelector.setColour(juce::Label::backgroundColourId, bankDColor);
    bankDSelector.setColour(juce::Label::textColourId, selectedBank == 3 ? BlueprintColors::textPrimary : BlueprintColors::textSecondary);
    bankDSelector.repaint();
}

inline void ControllerSettingsTab::updateControlsForSelectedSlider(int sliderIndex)
{
    selectedSlider = sliderIndex;
    selectedBank = sliderIndex / 4;
    updateBreadcrumbLabel();
    updateBankSelectorAppearance(selectedBank);
    
    // Hide color grid when switching sliders
    hideColorGrid();
    
    // Update all controls with the current slider's settings from parent window
    // These will be called by the parent to populate the controls
    updateColorButtonSelection();
}

inline void ControllerSettingsTab::setSliderSettings(int ccNumber, double rangeMin, double rangeMax, 
                                                    double increment, bool isCustomStep,
                                                    bool useDeadzone, int colorId,
                                                    SliderOrientation orientation,
                                                    const juce::String& customName, SnapThreshold snapThreshold,
                                                    bool showAutomation)
{
    // Update all controls with the provided settings (without triggering callbacks)
    ccNumberInput.setText(juce::String(ccNumber), juce::dontSendNotification);
    // Always 14-bit mode - is14Bit parameter ignored
    rangeMinInput.setText(juce::String(rangeMin, 2), juce::dontSendNotification);
    rangeMaxInput.setText(juce::String(rangeMax, 2), juce::dontSendNotification);
    incrementsInput.setText(juce::String(increment, 3), juce::dontSendNotification);
    isCustomStepFlag = isCustomStep; // Update custom step flag
    
    // Update visual indication based on custom/auto mode
    updateStepIndicationVisuals();
    deadzoneButton.setToggleState(useDeadzone, juce::dontSendNotification);
    directButton.setToggleState(!useDeadzone, juce::dontSendNotification);
    
    // Update orientation settings
    orientationCombo.setSelectedId(static_cast<int>(orientation) + 1, juce::dontSendNotification);
    
    // Show/hide snap controls based on orientation (only for bipolar mode)
    bool showSnapControls = (orientation == SliderOrientation::Bipolar);
    snapLabel.setVisible(showSnapControls);
    snapSmallButton.setVisible(showSnapControls);
    snapMediumButton.setVisible(showSnapControls);
    snapLargeButton.setVisible(showSnapControls);
    
    // Set snap threshold radio buttons
    snapSmallButton.setToggleState(snapThreshold == SnapThreshold::Small, juce::dontSendNotification);
    snapMediumButton.setToggleState(snapThreshold == SnapThreshold::Medium, juce::dontSendNotification);
    snapLargeButton.setToggleState(snapThreshold == SnapThreshold::Large, juce::dontSendNotification);
    
    // Update custom name
    nameInput.setText(customName, juce::dontSendNotification);
    
    // Update automation visibility setting
    showAutomationButton.setToggleState(showAutomation, juce::dontSendNotification);
    
    // Update color selection
    currentColorId = colorId;
    updateColorButtonSelection();
    
    // Update current color box
    juce::Colour sliderColor = getColorById(colorId);
    currentColorBox.setCurrentColor(sliderColor);
}

inline void ControllerSettingsTab::updateColorButtonSelection()
{
    // Reset all color button appearances - now with direct mapping
    for (int i = 0; i < colorButtons.size(); ++i)
    {
        auto* button = colorButtons[i];
        bool isSelected = i == currentColorId; // Direct comparison - no offset
        
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

inline void ControllerSettingsTab::applyPreset(const ControllerPreset& preset)
{
    // MIDI Channel handling moved to GlobalSettingsTab
    // Update controls for currently selected slider would be handled by parent coordination
}
inline void ControllerSettingsTab::applyAutomationVisibility()
{
    // Notify parent window of the automation visibility change
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::scaleFactorChanged(float newScale)
{
    // Update fonts for all labels and buttons
    breadcrumbLabel.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    bankSelectorLabel.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    bankASelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    bankBSelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    bankCSelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    bankDSelector.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    nameLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    section1Header.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    ccNumberLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    inputModeLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    section2Header.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    rangeLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    rangeDashLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    incrementsLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    orientationLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    snapLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    automationVisibilityLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    colorPickerLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    
    // Update fonts for input boxes
    nameInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    ccNumberInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    rangeMinInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    rangeMaxInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    incrementsInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    
    // Force TextEditor components to refresh and show new fonts immediately
    // Store current text, clear, and reset to force font refresh
    auto nameText = nameInput.getText();
    auto ccText = ccNumberInput.getText();
    auto minText = rangeMinInput.getText();
    auto maxText = rangeMaxInput.getText();
    auto incText = incrementsInput.getText();
    
    nameInput.clear();
    ccNumberInput.clear();
    rangeMinInput.clear();
    rangeMaxInput.clear();
    incrementsInput.clear();
    
    nameInput.setText(nameText, false);
    ccNumberInput.setText(ccText, false);
    rangeMinInput.setText(minText, false);
    rangeMaxInput.setText(maxText, false);
    incrementsInput.setText(incText, false);
    
    // Trigger layout and repaint
    resized();
    repaint();
}
