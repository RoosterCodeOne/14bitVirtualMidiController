// GlobalSettingsTab.h - Global MIDI Channel and BPM Settings Tab
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"

// Forward declaration
class SettingsWindow;

//==============================================================================
class GlobalSettingsTab : public juce::Component
{
public:
    GlobalSettingsTab(SettingsWindow* parentWindow);
    ~GlobalSettingsTab();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;
    void mouseDown(const juce::MouseEvent& event) override;
    
    // Public interface for main window coordination
    void applyPreset(const ControllerPreset& preset);
    
    // Access methods for main window
    int getMidiChannel() const { return midiChannelCombo.getSelectedId(); }
    double getBPM() const { return bpmSlider.getValue(); }
    void setBPM(double bpm) { bpmSlider.setValue(bpm, juce::dontSendNotification); }
    void setSyncStatus(bool isExternal, double externalBPM = 0.0);
    
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
    juce::Label syncStatusLabel;
    
    // Placeholder for future scaling controls
    juce::Label scalingLabel;
    juce::Label scalingPlaceholder;
    
    // Private methods
    void setupGlobalControls();
    
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
}

inline GlobalSettingsTab::~GlobalSettingsTab()
{
    // Clean up if needed
}

inline void GlobalSettingsTab::paint(juce::Graphics& g)
{
    // Blueprint aesthetic background
    g.setColour(BlueprintColors::windowBackground);
    g.fillAll();
    
    // Draw section backgrounds
    auto bounds = getLocalBounds().reduced(15);
    
    const int sectionSpacing = 8;
    const int controlSpacing = 4;
    const int labelHeight = 18;
    const int headerHeight = 22;
    
    // Global Settings section box
    auto section1Height = headerHeight + (labelHeight + controlSpacing) * 2 + controlSpacing;
    auto section1Bounds = bounds.removeFromTop(section1Height);
    section1Bounds = section1Bounds.expanded(8, 4);
    
    g.setColour(BlueprintColors::sectionBackground);
    g.fillRoundedRectangle(section1Bounds.toFloat(), 4.0f);
    g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
    g.drawRoundedRectangle(section1Bounds.toFloat(), 4.0f, 1.0f);
    
    bounds.removeFromTop(sectionSpacing);
    
    // Window Scaling section box (placeholder for future implementation)
    auto section2Height = headerHeight + labelHeight + controlSpacing * 2;
    auto section2Bounds = bounds.removeFromTop(section2Height);
    section2Bounds = section2Bounds.expanded(8, 4);
    
    g.setColour(BlueprintColors::sectionBackground);
    g.fillRoundedRectangle(section2Bounds.toFloat(), 4.0f);
    g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
    g.drawRoundedRectangle(section2Bounds.toFloat(), 4.0f, 1.0f);
}

inline void GlobalSettingsTab::resized()
{
    auto bounds = getLocalBounds().reduced(15);
    
    const int sectionSpacing = 8;
    const int controlSpacing = 4;
    const int labelHeight = 18;
    const int headerHeight = 22;
    
    // Global Settings section
    auto globalBounds = bounds.removeFromTop(headerHeight + (labelHeight + controlSpacing) * 2 + controlSpacing);
    
    globalHeader.setBounds(globalBounds.removeFromTop(headerHeight));
    globalBounds.removeFromTop(controlSpacing);
    
    // MIDI Channel row
    auto channelRow = globalBounds.removeFromTop(labelHeight);
    midiChannelLabel.setBounds(channelRow.removeFromLeft(100));
    channelRow.removeFromLeft(8);
    midiChannelCombo.setBounds(channelRow.removeFromLeft(120));
    
    globalBounds.removeFromTop(controlSpacing);
    
    // BPM row  
    auto bpmRow = globalBounds.removeFromTop(labelHeight);
    bpmLabel.setBounds(bpmRow.removeFromLeft(40));
    bpmRow.removeFromLeft(8);
    bpmSlider.setBounds(bpmRow.removeFromLeft(120));
    bpmRow.removeFromLeft(8);
    syncStatusLabel.setBounds(bpmRow);
    
    bounds.removeFromTop(sectionSpacing);
    
    // Window Scaling section (placeholder)
    auto scalingBounds = bounds.removeFromTop(headerHeight + labelHeight + controlSpacing * 2);
    
    scalingLabel.setBounds(scalingBounds.removeFromTop(headerHeight));
    scalingBounds.removeFromTop(controlSpacing);
    
    auto placeholderRow = scalingBounds.removeFromTop(labelHeight);
    scalingPlaceholder.setBounds(placeholderRow);
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
    globalHeader.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    globalHeader.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
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
        // Restore focus to parent after combo selection
        if (onRequestFocus) onRequestFocus();
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
        // Restore focus to parent after slider adjustment
        if (onRequestFocus) onRequestFocus();
    };
    
    addAndMakeVisible(syncStatusLabel);
    syncStatusLabel.setText("Internal Sync", juce::dontSendNotification);
    syncStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
    syncStatusLabel.setFont(juce::FontOptions(10.0f));
    syncStatusLabel.setJustificationType(juce::Justification::centredRight);
    
    // Window Scaling section (placeholder)
    addAndMakeVisible(scalingLabel);
    scalingLabel.setText("Window Scaling", juce::dontSendNotification);
    scalingLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    scalingLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    addAndMakeVisible(scalingPlaceholder);
    scalingPlaceholder.setText("Scaling controls will be implemented here", juce::dontSendNotification);
    scalingPlaceholder.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
    scalingPlaceholder.setFont(juce::FontOptions(12.0f));
}

inline void GlobalSettingsTab::setSyncStatus(bool isExternal, double externalBPM)
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

inline void GlobalSettingsTab::applyPreset(const ControllerPreset& preset)
{
    // Apply MIDI channel
    midiChannelCombo.setSelectedId(preset.midiChannel, juce::dontSendNotification);
}