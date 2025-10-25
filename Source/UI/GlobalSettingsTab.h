// GlobalSettingsTab.h - Global MIDI Channel and BPM Settings Tab
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"
#include "../SimpleSliderControl.h"
#include "GlobalUIScale.h"
#include "ThemeManager.h"

// Forward declaration
class SettingsWindow;

//==============================================================================
class GlobalSettingsTab : public juce::Component,
                           public GlobalUIScale::ScaleChangeListener,
                           public ThemeManager::ThemeChangeListener
{
public:
    GlobalSettingsTab(SettingsWindow* parentWindow);
    ~GlobalSettingsTab();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;
    void mouseDown(const juce::MouseEvent& event) override;
    
    // Scale change notification implementation
    void scaleFactorChanged(float newScale) override;

    // Theme change notification implementation
    void themeChanged(ThemeManager::ThemeType newTheme, const ThemeManager::ThemePalette& palette) override;

    // Public interface for main window coordination
    void applyPreset(const ControllerPreset& preset);
    
    // Access methods for main window
    int getMidiChannel() const { return midiChannelCombo.getSelectedId(); }
    double getBPM() const { return bpmSlider.getValue(); }
    void setBPM(double bpm) { 
        bpmSlider.setValue(bpm, juce::dontSendNotification); 
        bpmInput.setText(juce::String(static_cast<int>(bpm)), false);
    }
    void setSyncStatus(bool isExternal, double externalBPM = 0.0);
    
    // UI Scale methods
    float getUIScale() const;
    void setUIScale(float scale);
    
    // Always On Top methods
    bool getAlwaysOnTop() const { return alwaysOnTopToggle.getToggleState(); }
    void setAlwaysOnTop(bool alwaysOnTop) {
        alwaysOnTopToggle.setToggleState(alwaysOnTop, juce::dontSendNotification);
        // Apply to main window immediately
        if (auto* topLevel = getTopLevelComponent())
        {
            topLevel->setAlwaysOnTop(alwaysOnTop);
        }
    }

    // Theme methods
    juce::String getThemeName() const;
    void setTheme(const juce::String& themeName);

    
    // Callback functions for communication with parent
    std::function<void()> onSettingsChanged;
    std::function<void(double)> onBPMChanged;
    std::function<void()> onRequestFocus; // Callback to request focus restoration
    
private:
    SettingsWindow* parentWindow;
    
    // Section header
    juce::Label globalHeader;
    
    // MIDI Channel controls
    juce::Label midiChannelLabel;
    juce::ComboBox midiChannelCombo;
    
    // BPM controls
    juce::Label bpmLabel;
    juce::Slider bpmSlider;
    juce::TextEditor bpmInput;
    juce::Label syncStatusLabel;
    
    // UI Scale controls
    juce::Label uiScaleLabel;
    juce::ComboBox uiScaleCombo;
    std::vector<float> validScaleOptions;
    
    // Always On Top controls
    juce::Label alwaysOnTopLabel;
    juce::ToggleButton alwaysOnTopToggle;

    // Theme controls
    juce::Label themeLabel;
    juce::ComboBox themeCombo;

    // Private methods
    void setupGlobalControls();
    void updateScaleComboOptions();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlobalSettingsTab)
};

//==============================================================================
// Implementation
inline GlobalSettingsTab::GlobalSettingsTab(SettingsWindow* parent)
    : parentWindow(parent)
{
    setupGlobalControls();

    // Enable keyboard focus for tab
    setWantsKeyboardFocus(true);

    // Register for scale change notifications
    GlobalUIScale::getInstance().addScaleChangeListener(this);

    // Register for theme change notifications
    ThemeManager::getInstance().addThemeChangeListener(this);
}

inline GlobalSettingsTab::~GlobalSettingsTab()
{
    // Remove scale change listener
    GlobalUIScale::getInstance().removeScaleChangeListener(this);

    // Remove theme change listener
    ThemeManager::getInstance().removeThemeChangeListener(this);
}

inline void GlobalSettingsTab::paint(juce::Graphics& g)
{
    auto& scale = GlobalUIScale::getInstance();

    // Blueprint aesthetic background
    g.setColour(BlueprintColors::windowBackground());
    g.fillAll();

    // Draw section backgrounds
    auto bounds = getLocalBounds().reduced(scale.getScaled(15));

    const int sectionSpacing = scale.getScaled(8);
    const int controlSpacing = scale.getScaled(4);
    const int labelHeight = scale.getScaled(18);
    const int headerHeight = scale.getScaled(22);

    // Global Settings section box
    const auto section1Height = headerHeight + (labelHeight + controlSpacing) * 2 + controlSpacing;
    auto section1Bounds = bounds.removeFromTop(section1Height);
    section1Bounds = section1Bounds.expanded(scale.getScaled(8), scale.getScaled(4));

    g.setColour(BlueprintColors::sectionBackground());
    g.fillRoundedRectangle(section1Bounds.toFloat(), scale.getScaled(4.0f));
    g.setColour(BlueprintColors::blueprintLines().withAlpha(0.6f));
    g.drawRoundedRectangle(section1Bounds.toFloat(), scale.getScaled(4.0f), scale.getScaledLineThickness());

    bounds.removeFromTop(sectionSpacing);

    // Appearance section box (Theme + UI Scale + Always On Top)
    const auto section2Height = headerHeight + (labelHeight + controlSpacing) * 3 + controlSpacing;
    auto section2Bounds = bounds.removeFromTop(section2Height);
    section2Bounds = section2Bounds.expanded(scale.getScaled(8), scale.getScaled(4));

    g.setColour(BlueprintColors::sectionBackground());
    g.fillRoundedRectangle(section2Bounds.toFloat(), scale.getScaled(4.0f));
    g.setColour(BlueprintColors::blueprintLines().withAlpha(0.6f));
    g.drawRoundedRectangle(section2Bounds.toFloat(), scale.getScaled(4.0f), scale.getScaledLineThickness());
}

inline void GlobalSettingsTab::resized()
{
    auto& scale = GlobalUIScale::getInstance();
    auto bounds = getLocalBounds().reduced(scale.getScaled(15));
    
    const int sectionSpacing = scale.getScaled(8);
    const int controlSpacing = scale.getScaled(4);
    const int labelHeight = scale.getScaled(18);
    const int headerHeight = scale.getScaled(22);
    
    // Global Settings section
    auto globalBounds = bounds.removeFromTop(headerHeight + (labelHeight + controlSpacing) * 2 + controlSpacing);

    globalHeader.setBounds(globalBounds.removeFromTop(headerHeight));
    globalBounds.removeFromTop(controlSpacing);

    // MIDI Channel row
    auto channelRow = globalBounds.removeFromTop(labelHeight);
    midiChannelLabel.setBounds(channelRow.removeFromLeft(scale.getScaled(100)));
    channelRow.removeFromLeft(scale.getScaled(8));
    midiChannelCombo.setBounds(channelRow.removeFromLeft(scale.getScaled(120)));

    globalBounds.removeFromTop(controlSpacing);

    // BPM row
    auto bpmRow = globalBounds.removeFromTop(labelHeight);
    bpmLabel.setBounds(bpmRow.removeFromLeft(scale.getScaled(40)));
    bpmRow.removeFromLeft(scale.getScaled(8));
    bpmSlider.setBounds(bpmRow.removeFromLeft(scale.getScaled(80)));
    bpmRow.removeFromLeft(scale.getScaled(4));
    bpmInput.setBounds(bpmRow.removeFromLeft(scale.getScaled(50)));
    bpmRow.removeFromLeft(scale.getScaled(8));
    syncStatusLabel.setBounds(bpmRow);


    bounds.removeFromTop(sectionSpacing);

    // Appearance section (Theme + UI Scale + Always On Top)
    auto appearanceBounds = bounds.removeFromTop((labelHeight + controlSpacing) * 3);

    // Theme row
    auto themeRow = appearanceBounds.removeFromTop(labelHeight);
    themeLabel.setBounds(themeRow.removeFromLeft(scale.getScaled(80)));
    themeRow.removeFromLeft(scale.getScaled(8));
    themeCombo.setBounds(themeRow.removeFromLeft(scale.getScaled(100)));

    appearanceBounds.removeFromTop(controlSpacing);

    // UI Scale row
    auto scaleRow = appearanceBounds.removeFromTop(labelHeight);
    uiScaleLabel.setBounds(scaleRow.removeFromLeft(scale.getScaled(80)));
    scaleRow.removeFromLeft(scale.getScaled(8));
    uiScaleCombo.setBounds(scaleRow.removeFromLeft(scale.getScaled(100)));

    appearanceBounds.removeFromTop(controlSpacing);

    // Always On Top row
    auto alwaysOnTopRow = appearanceBounds.removeFromTop(labelHeight);
    alwaysOnTopLabel.setBounds(alwaysOnTopRow.removeFromLeft(scale.getScaled(100)));
    alwaysOnTopRow.removeFromLeft(scale.getScaled(8));
    alwaysOnTopToggle.setBounds(alwaysOnTopRow.removeFromLeft(scale.getScaled(80)));
}

inline bool GlobalSettingsTab::keyPressed(const juce::KeyPress& key)
{
    // Let parent handle navigation keys
    if (key == juce::KeyPress::escapeKey || 
        key == juce::KeyPress::upKey || 
        key == juce::KeyPress::downKey ||
        key == juce::KeyPress::leftKey ||
        key == juce::KeyPress::rightKey)
    {
        return false; // Allow parent to handle
    }
    return Component::keyPressed(key);
}

inline void GlobalSettingsTab::mouseDown(const juce::MouseEvent& event)
{
    // Handle mouse event normally
    Component::mouseDown(event);
    
    // Restore focus to parent SettingsWindow after mouse click
    if (onRequestFocus)
        onRequestFocus();
}

inline void GlobalSettingsTab::setupGlobalControls()
{
    // Section header
    addAndMakeVisible(globalHeader);
    globalHeader.setText("Global Settings", juce::dontSendNotification);
    globalHeader.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    globalHeader.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
    
    // MIDI Channel controls
    addAndMakeVisible(midiChannelLabel);
    midiChannelLabel.setText("MIDI Channel:", juce::dontSendNotification);
    midiChannelLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    midiChannelLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
    
    addAndMakeVisible(midiChannelCombo);
    for (int i = 1; i <= 16; ++i)
        midiChannelCombo.addItem("Channel " + juce::String(i), i);
    midiChannelCombo.setSelectedId(11); // Default to channel 11
    midiChannelCombo.setColour(juce::ComboBox::backgroundColourId, BlueprintColors::background());
    midiChannelCombo.setColour(juce::ComboBox::textColourId, BlueprintColors::textPrimary());
    midiChannelCombo.setColour(juce::ComboBox::outlineColourId, BlueprintColors::blueprintLines());
    midiChannelCombo.onChange = [this]() {
        if (onSettingsChanged)
            onSettingsChanged();
        // Restore focus to parent after combo selection
        if (onRequestFocus) onRequestFocus();
    };
    
    // BPM controls
    addAndMakeVisible(bpmLabel);
    bpmLabel.setText("BPM:", juce::dontSendNotification);
    bpmLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    bpmLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
    
    addAndMakeVisible(bpmSlider);
    bpmSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bpmSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);  // No text box on slider
    bpmSlider.setRange(60.0, 200.0, 1.0);
    bpmSlider.setValue(120.0);
    bpmSlider.setColour(juce::Slider::backgroundColourId, BlueprintColors::background());
    bpmSlider.setColour(juce::Slider::trackColourId, BlueprintColors::blueprintLines());
    bpmSlider.setColour(juce::Slider::thumbColourId, BlueprintColors::active());
    bpmSlider.onValueChange = [this]() {
        // Update text editor when slider changes
        bpmInput.setText(juce::String(static_cast<int>(bpmSlider.getValue())), false);
        if (onBPMChanged)
            onBPMChanged(bpmSlider.getValue());
        // Restore focus to parent after slider adjustment
        if (onRequestFocus) onRequestFocus();
    };
    
    // BPM input text editor
    addAndMakeVisible(bpmInput);
    bpmInput.setInputRestrictions(3, "0123456789");
    bpmInput.setText("120", false);
    bpmInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background());
    bpmInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary());
    bpmInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines());
    bpmInput.onReturnKey = [this]() { bpmInput.moveKeyboardFocusToSibling(true); };
    bpmInput.onFocusLost = [this]() {
        // Update slider when text editor changes
        double value = juce::jlimit(60.0, 200.0, bpmInput.getText().getDoubleValue());
        bpmSlider.setValue(value, juce::dontSendNotification);
        bpmInput.setText(juce::String(static_cast<int>(value)), false);  // Ensure valid display
        if (onBPMChanged)
            onBPMChanged(value);
    };
    // Set font after all other properties are configured
    bpmInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    
    addAndMakeVisible(syncStatusLabel);
    syncStatusLabel.setText("Internal Sync", juce::dontSendNotification);
    syncStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary());
    syncStatusLabel.setFont(GlobalUIScale::getInstance().getScaledFont(10.0f));
    syncStatusLabel.setJustificationType(juce::Justification::centredRight);
    
    // UI Scale controls
    addAndMakeVisible(uiScaleLabel);
    uiScaleLabel.setText("UI Scale:", juce::dontSendNotification);
    uiScaleLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    uiScaleLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());
    
    addAndMakeVisible(uiScaleCombo);
    
    // Initialize screen constraints and populate combo with valid options
    updateScaleComboOptions();
    
    // Load and set current scale factor
    float currentScale = GlobalUIScale::getInstance().getScaleFactor();
    setUIScale(currentScale);
    uiScaleCombo.setColour(juce::ComboBox::backgroundColourId, BlueprintColors::background());
    uiScaleCombo.setColour(juce::ComboBox::textColourId, BlueprintColors::textPrimary());
    uiScaleCombo.setColour(juce::ComboBox::outlineColourId, BlueprintColors::blueprintLines());
    uiScaleCombo.onChange = [this]() {
        if (validScaleOptions.empty()) return;
        
        int selectedIndex = uiScaleCombo.getSelectedItemIndex();
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(validScaleOptions.size()))
        {
            float newScale = validScaleOptions[selectedIndex];
            // Use constraint-aware scaling with user feedback
            GlobalUIScale::getInstance().setScaleFactorWithConstraints(newScale, this, true);
            
            // Save scale setting to PresetManager if available
            if (onSettingsChanged)
                onSettingsChanged();
                
            // Restore focus to parent after selection
            if (onRequestFocus) onRequestFocus();
        }
    };
    
    // Theme controls
    addAndMakeVisible(themeLabel);
    themeLabel.setText("Theme:", juce::dontSendNotification);
    themeLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    themeLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());

    addAndMakeVisible(themeCombo);
    themeCombo.addItem("Dark", 1);
    themeCombo.addItem("Light", 2);
    themeCombo.addItem("Auto", 3);
    themeCombo.setSelectedId(1, juce::dontSendNotification); // Default to Dark
    themeCombo.setColour(juce::ComboBox::backgroundColourId, BlueprintColors::background());
    themeCombo.setColour(juce::ComboBox::textColourId, BlueprintColors::textPrimary());
    themeCombo.setColour(juce::ComboBox::outlineColourId, BlueprintColors::blueprintLines());
    themeCombo.onChange = [this]() {
        int selectedId = themeCombo.getSelectedId();
        ThemeManager::ThemeType themeType;

        if (selectedId == 1)
            themeType = ThemeManager::ThemeType::Dark;
        else if (selectedId == 2)
            themeType = ThemeManager::ThemeType::Light;
        else
            themeType = ThemeManager::ThemeType::Auto;

        ThemeManager::getInstance().setTheme(themeType);

        // Start/stop system theme monitoring based on selection
        if (themeType == ThemeManager::ThemeType::Auto)
            ThemeManager::getInstance().startSystemThemeMonitoring();
        else
            ThemeManager::getInstance().stopSystemThemeMonitoring();

        // Save setting
        if (onSettingsChanged)
            onSettingsChanged();

        // Restore focus to parent after selection
        if (onRequestFocus) onRequestFocus();
    };

    // Always On Top controls
    addAndMakeVisible(alwaysOnTopLabel);
    alwaysOnTopLabel.setText("Always On Top:", juce::dontSendNotification);
    alwaysOnTopLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    alwaysOnTopLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary());

    addAndMakeVisible(alwaysOnTopToggle);
    alwaysOnTopToggle.setToggleState(false, juce::dontSendNotification);
    alwaysOnTopToggle.setColour(juce::ToggleButton::textColourId, BlueprintColors::textPrimary());
    alwaysOnTopToggle.setColour(juce::ToggleButton::tickColourId, BlueprintColors::active());
    alwaysOnTopToggle.setColour(juce::ToggleButton::tickDisabledColourId, BlueprintColors::textSecondary());
    alwaysOnTopToggle.onClick = [this]() {
        // Find the main window and set always on top property
        if (auto* topLevel = getTopLevelComponent())
        {
            topLevel->setAlwaysOnTop(alwaysOnTopToggle.getToggleState());
        }

        // Save setting
        if (onSettingsChanged)
            onSettingsChanged();

        // Restore focus to parent after toggle
        if (onRequestFocus) onRequestFocus();
    };

}

inline void GlobalSettingsTab::setSyncStatus(bool isExternal, double externalBPM)
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

inline void GlobalSettingsTab::applyPreset(const ControllerPreset& preset)
{
    // Apply MIDI channel
    midiChannelCombo.setSelectedId(preset.midiChannel, juce::dontSendNotification);

    // Apply theme (if available in preset)
    if (preset.themeName.isNotEmpty())
    {
        setTheme(preset.themeName);
    }

    // Apply UI scale factor
    setUIScale(preset.uiScale);

    // Apply Always On Top setting
    setAlwaysOnTop(preset.alwaysOnTop);
}

inline void GlobalSettingsTab::updateScaleComboOptions()
{
    // Update screen constraints for current component
    GlobalUIScale::getInstance().updateScreenConstraints(this);
    
    // Get valid scale options based on screen constraints
    validScaleOptions = GlobalUIScale::getInstance().getValidScaleOptions(this);
    
    // Clear existing combo box items
    uiScaleCombo.clear();
    
    // Populate combo box with valid scale options
    for (size_t i = 0; i < validScaleOptions.size(); ++i)
    {
        float scale = validScaleOptions[i];
        juce::String scaleText = juce::String(static_cast<int>(scale * 100)) + "%";
        
        // Add disabled indicator for scales that would be clamped
        auto constraints = GlobalUIScale::getInstance().getCurrentScreenConstraints();
        if (constraints.isValid)
        {
            if (scale < constraints.minScale + 0.01f)
                scaleText += " (min)";
            else if (scale > constraints.maxScale - 0.01f)
                scaleText += " (max)";
        }
        
        uiScaleCombo.addItem(scaleText, static_cast<int>(i + 1));
    }
}

inline float GlobalSettingsTab::getUIScale() const
{
    if (validScaleOptions.empty()) return 1.0f;
    
    int selectedIndex = uiScaleCombo.getSelectedItemIndex();
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(validScaleOptions.size()))
        return validScaleOptions[selectedIndex];
    
    return 1.0f;
}

inline void GlobalSettingsTab::setUIScale(float scale)
{
    // Ensure we have valid scale options
    if (validScaleOptions.empty())
        updateScaleComboOptions();
    
    // Find the closest matching scale factor in valid options
    int bestIndex = 0;
    float minDifference = std::abs(scale - validScaleOptions[0]);
    
    for (size_t i = 1; i < validScaleOptions.size(); ++i)
    {
        float difference = std::abs(scale - validScaleOptions[i]);
        if (difference < minDifference)
        {
            minDifference = difference;
            bestIndex = static_cast<int>(i);
        }
    }
    
    uiScaleCombo.setSelectedItemIndex(bestIndex, juce::dontSendNotification);
}

inline void GlobalSettingsTab::scaleFactorChanged(float newScale)
{
    // No need to update BPM slider text box since we use separate TextEditor

    // Update fonts for all labels
    globalHeader.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    midiChannelLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    bpmLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    syncStatusLabel.setFont(GlobalUIScale::getInstance().getScaledFont(10.0f));
    themeLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    uiScaleLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    alwaysOnTopLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));

    // Update BPM TextEditor font and force refresh
    bpmInput.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    auto bpmText = bpmInput.getText();
    bpmInput.clear();
    bpmInput.setText(bpmText, false);

    // Update scale combo options (constraints may have changed)
    updateScaleComboOptions();
    setUIScale(newScale);  // Update selection to match new scale

    // Trigger layout and repaint
    resized();
    repaint();
}

inline void GlobalSettingsTab::themeChanged(ThemeManager::ThemeType newTheme, const ThemeManager::ThemePalette& palette)
{
    // Repaint to apply new theme colors
    // The BlueprintColors namespace automatically reflects the new theme
    repaint();
}

inline juce::String GlobalSettingsTab::getThemeName() const
{
    return ThemeManager::getInstance().getThemeName(ThemeManager::getInstance().getThemeType());
}

inline void GlobalSettingsTab::setTheme(const juce::String& themeName)
{
    ThemeManager::getInstance().setThemeFromString(themeName);

    // Update combo box selection
    int selectedId = 1; // Default to Dark
    if (themeName.equalsIgnoreCase("Light"))
        selectedId = 2;
    else if (themeName.equalsIgnoreCase("Auto"))
        selectedId = 3;

    themeCombo.setSelectedId(selectedId, juce::dontSendNotification);

    // Start/stop system theme monitoring based on selection
    if (themeName.equalsIgnoreCase("Auto"))
        ThemeManager::getInstance().startSystemThemeMonitoring();
    else
        ThemeManager::getInstance().stopSystemThemeMonitoring();
}