// ControllerSettingsTab.h - MIDI/BPM/Bank/Slider Configuration Tab
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"
#include "../SimpleSliderControl.h"

// Forward declaration
class SettingsWindow;

//==============================================================================
class ControllerSettingsTab : public juce::Component
{
public:
    ControllerSettingsTab(SettingsWindow* parentWindow);
    ~ControllerSettingsTab();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Public interface for main window coordination
    void updateControlsForSelectedSlider(int sliderIndex);
    void updateBankSelectorAppearance(int selectedBank);
    void applyPreset(const ControllerPreset& preset);
    void setSliderSettings(int ccNumber, bool is14Bit, double rangeMin, double rangeMax, 
                          const juce::String& displayUnit, double increment, bool useDeadzone, int colorId);
    
    // Access methods for main window
    int getMidiChannel() const { return midiChannelCombo.getSelectedId(); }
    double getBPM() const { return bpmSlider.getValue(); }
    void setBPM(double bpm) { bpmSlider.setValue(bpm, juce::dontSendNotification); }
    void setSyncStatus(bool isExternal, double externalBPM = 0.0);
    
    // Getter methods for current slider settings (for parent coordination)
    int getCurrentCCNumber() const { return ccNumberInput.getText().getIntValue(); }
    bool getCurrentIs14Bit() const { return output14BitButton.getToggleState(); }
    double getCurrentRangeMin() const { return rangeMinInput.getText().getDoubleValue(); }
    double getCurrentRangeMax() const { return rangeMaxInput.getText().getDoubleValue(); }
    juce::String getCurrentDisplayUnit() const { return displayUnitInput.getText(); }
    double getCurrentIncrement() const { return incrementsInput.getText().getDoubleValue(); }
    bool getCurrentUseDeadzone() const { return deadzoneButton.getToggleState(); }
    int getCurrentColorId() const { return currentColorId; }
    
    // Callback functions for communication with parent
    std::function<void()> onSettingsChanged;
    std::function<void(double)> onBPMChanged;
    std::function<void(int)> onBankSelected;
    std::function<void(int)> onSliderSettingChanged;
    
private:
    SettingsWindow* parentWindow;
    CustomButtonLookAndFeel customButtonLookAndFeel;
    int currentColorId = 1; // Track current color selection
    
    // MIDI Channel controls
    juce::Label midiChannelLabel;
    juce::ComboBox midiChannelCombo;
    
    // BPM controls
    juce::Label bpmLabel;
    juce::Slider bpmSlider;
    juce::Label syncStatusLabel;
    
    // Bank selector
    juce::Label bankSelectorLabel;
    ClickableLabel bankASelector, bankBSelector, bankCSelector, bankDSelector;
    int selectedBank = 0;
    int selectedSlider = 0;
    
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
    
    // Private methods
    void setupControllerControls();
    void setupBankSelector();
    void setupPerSliderControls();
    void layoutPerSliderSections(juce::Rectangle<int>& bounds);
    void cycleSliderInBank(int bankIndex);
    void updateBreadcrumbLabel();
    void updateColorButtonSelection();
    
    // Validation and application methods
    void validateAndApplyCCNumber();
    void applyOutputMode();
    void validateAndApplyRange();
    void applyDisplayUnit();
    void applyIncrements();
    void applyInputMode();
    void selectColor(int colorId);
    void resetCurrentSlider();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControllerSettingsTab)
};

//==============================================================================
// Implementation
inline ControllerSettingsTab::ControllerSettingsTab(SettingsWindow* parent)
    : parentWindow(parent)
{
    setupControllerControls();
    setupBankSelector();
    setupPerSliderControls();
}

inline ControllerSettingsTab::~ControllerSettingsTab()
{
    // Clean up custom look and feel
    resetSliderButton.setLookAndFeel(nullptr);
}

inline void ControllerSettingsTab::paint(juce::Graphics& g)
{
    // Blueprint aesthetic background
    g.setColour(BlueprintColors::windowBackground);
    g.fillAll();
    
    // Draw section backgrounds similar to original
    auto bounds = getLocalBounds().reduced(15);
    
    // Controller section background
    auto controllerSectionBounds = bounds.removeFromTop(10 + 22 + 6 + 22 + 8 + 20 + 6 + 22 + 8);
    controllerSectionBounds = controllerSectionBounds.expanded(5, 0).withTrimmedBottom(1);
    
    g.setColour(BlueprintColors::sectionBackground);
    g.fillRect(controllerSectionBounds.toFloat());
    g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
    g.drawRect(controllerSectionBounds.toFloat(), 1.0f);
}

inline void ControllerSettingsTab::resized()
{
    auto bounds = getLocalBounds().reduced(15);
    
    // MIDI Channel section
    bounds.removeFromTop(10);
    auto channelArea = bounds.removeFromTop(22);
    midiChannelLabel.setBounds(channelArea.removeFromLeft(100));
    channelArea.removeFromLeft(8);
    midiChannelCombo.setBounds(channelArea);
    
    bounds.removeFromTop(6);
    
    // BPM section
    auto bpmArea = bounds.removeFromTop(22);
    bpmLabel.setBounds(bpmArea.removeFromLeft(40));
    bpmArea.removeFromLeft(8);
    auto sliderArea = bpmArea.removeFromLeft(120);
    bpmSlider.setBounds(sliderArea);
    bpmArea.removeFromLeft(8);
    syncStatusLabel.setBounds(bpmArea);
    
    bounds.removeFromTop(8);
    
    // Breadcrumb section
    auto breadcrumbArea = bounds.removeFromTop(20);
    breadcrumbLabel.setBounds(breadcrumbArea);
    
    bounds.removeFromTop(6);
    
    // Bank selector section
    auto bankSelectorArea = bounds.removeFromTop(22);
    bankSelectorLabel.setBounds(bankSelectorArea.removeFromLeft(40));
    bankSelectorArea.removeFromLeft(8);
    
    int bankButtonWidth = (bankSelectorArea.getWidth() - 21) / 4;
    bankASelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
    bankSelectorArea.removeFromLeft(7);
    bankBSelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
    bankSelectorArea.removeFromLeft(7);
    bankCSelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
    bankSelectorArea.removeFromLeft(7);
    bankDSelector.setBounds(bankSelectorArea.removeFromLeft(bankButtonWidth));
    
    bounds.removeFromTop(8);
    
    // Layout per-slider controls in 4 sections
    layoutPerSliderSections(bounds);
}

inline void ControllerSettingsTab::setupControllerControls()
{
    // MIDI Channel controls
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
    
    // BPM controls
    addAndMakeVisible(bpmLabel);
    bpmLabel.setText("BPM:", juce::dontSendNotification);
    bpmLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(bpmSlider);
    bpmSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bpmSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    bpmSlider.setRange(60.0, 200.0, 1.0);
    bpmSlider.setValue(120.0);
    bpmSlider.setColour(juce::Slider::backgroundColourId, BlueprintColors::background);
    bpmSlider.setColour(juce::Slider::trackColourId, BlueprintColors::blueprintLines);
    bpmSlider.setColour(juce::Slider::thumbColourId, BlueprintColors::active);
    bpmSlider.setColour(juce::Slider::textBoxTextColourId, BlueprintColors::textPrimary);
    bpmSlider.setColour(juce::Slider::textBoxBackgroundColourId, BlueprintColors::background);
    bpmSlider.setColour(juce::Slider::textBoxOutlineColourId, BlueprintColors::blueprintLines);
    bpmSlider.onValueChange = [this]() {
        if (onBPMChanged)
            onBPMChanged(bpmSlider.getValue());
    };
    
    addAndMakeVisible(syncStatusLabel);
    syncStatusLabel.setText("Internal Sync", juce::dontSendNotification);
    syncStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
    syncStatusLabel.setFont(juce::FontOptions(10.0f));
    syncStatusLabel.setJustificationType(juce::Justification::centredRight);
    
    // Breadcrumb label
    addAndMakeVisible(breadcrumbLabel);
    breadcrumbLabel.setText("Bank A > Slider 1", juce::dontSendNotification);
    breadcrumbLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    breadcrumbLabel.setColour(juce::Label::textColourId, BlueprintColors::active);
    breadcrumbLabel.setJustificationType(juce::Justification::centredLeft);
}

inline void ControllerSettingsTab::setupBankSelector()
{
    addAndMakeVisible(bankSelectorLabel);
    bankSelectorLabel.setText("Bank:", juce::dontSendNotification);
    bankSelectorLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    bankSelectorLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    // Bank selector buttons
    addAndMakeVisible(bankASelector);
    bankASelector.setText("A", juce::dontSendNotification);
    bankASelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    bankASelector.setJustificationType(juce::Justification::centred);
    bankASelector.setColour(juce::Label::backgroundColourId, BlueprintColors::active);
    bankASelector.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    bankASelector.onClick = [this]() { cycleSliderInBank(0); };
    
    addAndMakeVisible(bankBSelector);
    bankBSelector.setText("B", juce::dontSendNotification);
    bankBSelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    bankBSelector.setJustificationType(juce::Justification::centred);
    bankBSelector.setColour(juce::Label::backgroundColourId, BlueprintColors::inactive);
    bankBSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
    bankBSelector.onClick = [this]() { cycleSliderInBank(1); };
    
    addAndMakeVisible(bankCSelector);
    bankCSelector.setText("C", juce::dontSendNotification);
    bankCSelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    bankCSelector.setJustificationType(juce::Justification::centred);
    bankCSelector.setColour(juce::Label::backgroundColourId, BlueprintColors::inactive);
    bankCSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
    bankCSelector.onClick = [this]() { cycleSliderInBank(2); };
    
    addAndMakeVisible(bankDSelector);
    bankDSelector.setText("D", juce::dontSendNotification);
    bankDSelector.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    bankDSelector.setJustificationType(juce::Justification::centred);
    bankDSelector.setColour(juce::Label::backgroundColourId, BlueprintColors::inactive);
    bankDSelector.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
    bankDSelector.onClick = [this]() { cycleSliderInBank(3); };
}

inline void ControllerSettingsTab::setupPerSliderControls()
{
    // Section 1 - Core MIDI
    addAndMakeVisible(section1Header);
    section1Header.setText("Core MIDI", juce::dontSendNotification);
    section1Header.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    section1Header.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(ccNumberLabel);
    ccNumberLabel.setText("MIDI CC Number:", juce::dontSendNotification);
    ccNumberLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(ccNumberInput);
    ccNumberInput.setInputRestrictions(3, "0123456789");
    ccNumberInput.setTooltip("MIDI CC number (0-127)");
    ccNumberInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
    ccNumberInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
    ccNumberInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
    ccNumberInput.onReturnKey = [this]() { ccNumberInput.moveKeyboardFocusToSibling(true); };
    ccNumberInput.onFocusLost = [this]() { validateAndApplyCCNumber(); };
    
    addAndMakeVisible(outputModeLabel);
    outputModeLabel.setText("Output Mode:", juce::dontSendNotification);
    outputModeLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
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
    section2Header.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    section2Header.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(rangeLabel);
    rangeLabel.setText("Range:", juce::dontSendNotification);
    rangeLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(rangeMinInput);
    rangeMinInput.setInputRestrictions(0, "-0123456789.");
    rangeMinInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
    rangeMinInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
    rangeMinInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
    rangeMinInput.onReturnKey = [this]() { rangeMinInput.moveKeyboardFocusToSibling(true); };
    rangeMinInput.onFocusLost = [this]() { validateAndApplyRange(); };
    
    addAndMakeVisible(rangeDashLabel);
    rangeDashLabel.setText("-", juce::dontSendNotification);
    rangeDashLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    rangeDashLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(rangeMaxInput);
    rangeMaxInput.setInputRestrictions(0, "-0123456789.");
    rangeMaxInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
    rangeMaxInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
    rangeMaxInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
    rangeMaxInput.onReturnKey = [this]() { rangeMaxInput.moveKeyboardFocusToSibling(true); };
    rangeMaxInput.onFocusLost = [this]() { validateAndApplyRange(); };
    
    addAndMakeVisible(displayUnitLabel);
    displayUnitLabel.setText("Display Unit:", juce::dontSendNotification);
    displayUnitLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(displayUnitInput);
    displayUnitInput.setInputRestrictions(4);
    displayUnitInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
    displayUnitInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
    displayUnitInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
    displayUnitInput.onReturnKey = [this]() { displayUnitInput.moveKeyboardFocusToSibling(true); };
    displayUnitInput.onFocusLost = [this]() { applyDisplayUnit(); };
    
    addAndMakeVisible(incrementsLabel);
    incrementsLabel.setText("Custom Steps:", juce::dontSendNotification);
    incrementsLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(incrementsInput);
    incrementsInput.setInputRestrictions(0, "0123456789.");
    incrementsInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
    incrementsInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
    incrementsInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines);
    incrementsInput.onReturnKey = [this]() { incrementsInput.moveKeyboardFocusToSibling(true); };
    incrementsInput.onFocusLost = [this]() { applyIncrements(); };
    
    // Section 3 - Input Behavior
    addAndMakeVisible(section3Header);
    section3Header.setText("Input Behavior", juce::dontSendNotification);
    section3Header.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    section3Header.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(inputModeLabel);
    inputModeLabel.setText("MIDI Input Mode:", juce::dontSendNotification);
    inputModeLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
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
    section4Header.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    section4Header.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(colorPickerLabel);
    colorPickerLabel.setText("Color:", juce::dontSendNotification);
    colorPickerLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
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
        colorButton->onClick = [this, i]() { selectColor(i + 2); };
    }
    
    addAndMakeVisible(resetSliderButton);
    resetSliderButton.setButtonText("Reset Slider");
    resetSliderButton.setLookAndFeel(&customButtonLookAndFeel);
    resetSliderButton.onClick = [this]() { resetCurrentSlider(); };
}

inline void ControllerSettingsTab::layoutPerSliderSections(juce::Rectangle<int>& bounds)
{
    const int sectionSpacing = 3;
    const int controlSpacing = 2;
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
    
    // Range row
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
    auto section4Bounds = bounds.removeFromTop(headerHeight + labelHeight + 60 + inputHeight + controlSpacing);
    
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

// Additional inline method implementations would continue here...
// Due to length constraints, I'll create this as a header-only implementation for now
// In a production environment, you'd split the implementation into a .cpp file

inline void ControllerSettingsTab::setSyncStatus(bool isExternal, double externalBPM)
{
    if (isExternal && externalBPM > 0.0)
    {
        syncStatusLabel.setText("DAW Sync: " + juce::String(externalBPM, 1) + " BPM", 
                              juce::dontSendNotification);
        syncStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::active);
    }
    else
    {
        syncStatusLabel.setText("Internal Sync", juce::dontSendNotification);
        syncStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
    }
}

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

inline void ControllerSettingsTab::applyOutputMode()
{
    // Get the 14-bit state and notify parent
    bool is14Bit = output14BitButton.getToggleState();
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

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
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::applyDisplayUnit()
{
    // Get the display unit text and notify parent
    juce::String unit = displayUnitInput.getText();
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::applyIncrements()
{
    auto text = incrementsInput.getText();
    if (text.isEmpty()) { incrementsInput.setText("1", juce::dontSendNotification); text = "1"; }
    
    double increment = juce::jmax(0.001, text.getDoubleValue());
    incrementsInput.setText(juce::String(increment, 3), juce::dontSendNotification);
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::applyInputMode()
{
    // Get the deadzone state and notify parent
    bool useDeadzone = deadzoneButton.getToggleState();
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::selectColor(int colorId)
{
    // Store the selected color and update visual selection
    currentColorId = colorId;
    updateColorButtonSelection();
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::resetCurrentSlider()
{
    // Reset all controls to defaults for the current slider
    ccNumberInput.setText(juce::String(selectedSlider), juce::dontSendNotification);
    output14BitButton.setToggleState(true, juce::dontSendNotification);
    output7BitButton.setToggleState(false, juce::dontSendNotification);
    rangeMinInput.setText("0", juce::dontSendNotification);
    rangeMaxInput.setText("16383", juce::dontSendNotification);
    displayUnitInput.setText("", juce::dontSendNotification);
    incrementsInput.setText("1", juce::dontSendNotification);
    deadzoneButton.setToggleState(true, juce::dontSendNotification);
    directButton.setToggleState(false, juce::dontSendNotification);
    
    // Set default color based on bank
    int bankIndex = selectedSlider / 4;
    int defaultColorId = 2; // Red for bank A
    switch (bankIndex)
    {
        case 0: defaultColorId = 2; break; // Red
        case 1: defaultColorId = 3; break; // Blue  
        case 2: defaultColorId = 4; break; // Green
        case 3: defaultColorId = 5; break; // Yellow
    }
    
    selectColor(defaultColorId);
    
    if (onSliderSettingChanged)
        onSliderSettingChanged(selectedSlider);
}

inline void ControllerSettingsTab::cycleSliderInBank(int bankIndex)
{
    selectedBank = bankIndex;
    updateBreadcrumbLabel();
    updateBankSelectorAppearance(bankIndex);
    
    if (onBankSelected)
        onBankSelected(bankIndex);
}

inline void ControllerSettingsTab::updateBreadcrumbLabel()
{
    char bankLetter = 'A' + selectedBank;
    int sliderInBank = (selectedSlider % 4) + 1;
    breadcrumbLabel.setText("Bank " + juce::String(bankLetter) + " > Slider " + juce::String(sliderInBank),
                           juce::dontSendNotification);
}

inline void ControllerSettingsTab::updateBankSelectorAppearance(int selectedBankIndex)
{
    selectedBank = selectedBankIndex;
    
    bankASelector.setColour(juce::Label::backgroundColourId, selectedBank == 0 ? BlueprintColors::active : BlueprintColors::inactive);
    bankASelector.setColour(juce::Label::textColourId, selectedBank == 0 ? BlueprintColors::textPrimary : BlueprintColors::textSecondary);
    
    bankBSelector.setColour(juce::Label::backgroundColourId, selectedBank == 1 ? BlueprintColors::active : BlueprintColors::inactive);
    bankBSelector.setColour(juce::Label::textColourId, selectedBank == 1 ? BlueprintColors::textPrimary : BlueprintColors::textSecondary);
    
    bankCSelector.setColour(juce::Label::backgroundColourId, selectedBank == 2 ? BlueprintColors::active : BlueprintColors::inactive);
    bankCSelector.setColour(juce::Label::textColourId, selectedBank == 2 ? BlueprintColors::textPrimary : BlueprintColors::textSecondary);
    
    bankDSelector.setColour(juce::Label::backgroundColourId, selectedBank == 3 ? BlueprintColors::active : BlueprintColors::inactive);
    bankDSelector.setColour(juce::Label::textColourId, selectedBank == 3 ? BlueprintColors::textPrimary : BlueprintColors::textSecondary);
}

inline void ControllerSettingsTab::updateControlsForSelectedSlider(int sliderIndex)
{
    selectedSlider = sliderIndex;
    selectedBank = sliderIndex / 4;
    updateBreadcrumbLabel();
    updateBankSelectorAppearance(selectedBank);
    
    // Update all controls with the current slider's settings from parent window
    // These will be called by the parent to populate the controls
    updateColorButtonSelection();
}

inline void ControllerSettingsTab::setSliderSettings(int ccNumber, bool is14Bit, double rangeMin, double rangeMax, 
                                                    const juce::String& displayUnit, double increment, 
                                                    bool useDeadzone, int colorId)
{
    // Update all controls with the provided settings (without triggering callbacks)
    ccNumberInput.setText(juce::String(ccNumber), juce::dontSendNotification);
    output14BitButton.setToggleState(is14Bit, juce::dontSendNotification);
    output7BitButton.setToggleState(!is14Bit, juce::dontSendNotification);
    rangeMinInput.setText(juce::String(rangeMin, 2), juce::dontSendNotification);
    rangeMaxInput.setText(juce::String(rangeMax, 2), juce::dontSendNotification);
    displayUnitInput.setText(displayUnit, juce::dontSendNotification);
    incrementsInput.setText(juce::String(increment, 3), juce::dontSendNotification);
    deadzoneButton.setToggleState(useDeadzone, juce::dontSendNotification);
    directButton.setToggleState(!useDeadzone, juce::dontSendNotification);
    
    // Update color selection
    currentColorId = colorId;
    updateColorButtonSelection();
}

inline void ControllerSettingsTab::updateColorButtonSelection()
{
    // Reset all color button appearances
    for (int i = 0; i < colorButtons.size(); ++i)
    {
        auto* button = colorButtons[i];
        bool isSelected = (i + 2) == currentColorId; // +2 because colorId starts at 2
        
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
    // Apply MIDI channel
    midiChannelCombo.setSelectedId(preset.midiChannel, juce::dontSendNotification);
    
    // Update controls for currently selected slider would be handled by parent coordination
}