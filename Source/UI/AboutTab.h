// AboutTab.h - About Information Tab
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"
#include "GlobalUIScale.h"

// Forward declaration
class SettingsWindow;

//==============================================================================
class AboutTab : public juce::Component, 
                  public GlobalUIScale::ScaleChangeListener
{
public:
    AboutTab(SettingsWindow* parentWindow);
    ~AboutTab();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;
    void mouseDown(const juce::MouseEvent& event) override;
    
    // Scale change notification implementation
    void scaleFactorChanged(float newScale) override;
    
    // Callback functions for communication with parent
    std::function<void()> onRequestFocus; // Callback to request focus restoration
    
private:
    SettingsWindow* parentWindow;
    
    // Section header
    juce::Label aboutHeader;
    
    // Placeholder content
    juce::Label placeholderLabel;
    
    // Private methods
    void setupAboutControls();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutTab)
};

//==============================================================================
// Implementation
inline AboutTab::AboutTab(SettingsWindow* parent)
    : parentWindow(parent)
{
    setupAboutControls();
    
    // Enable keyboard focus for tab
    setWantsKeyboardFocus(true);
    
    // Register for scale change notifications
    GlobalUIScale::getInstance().addScaleChangeListener(this);
}

inline AboutTab::~AboutTab()
{
    // Remove scale change listener
    GlobalUIScale::getInstance().removeScaleChangeListener(this);
}

inline void AboutTab::paint(juce::Graphics& g)
{
    auto& scale = GlobalUIScale::getInstance();
    
    // Blueprint aesthetic background
    g.setColour(BlueprintColors::windowBackground);
    g.fillAll();
    
    // Draw section background
    auto bounds = getLocalBounds().reduced(scale.getScaled(15));
    
    const int sectionSpacing = scale.getScaled(8);
    const int controlSpacing = scale.getScaled(4);
    const int labelHeight = scale.getScaled(18);
    const int headerHeight = scale.getScaled(22);
    
    // About section box
    auto section1Height = headerHeight + labelHeight + controlSpacing * 2;
    auto section1Bounds = bounds.removeFromTop(section1Height);
    section1Bounds = section1Bounds.expanded(scale.getScaled(8), scale.getScaled(4));
    
    g.setColour(BlueprintColors::sectionBackground);
    g.fillRoundedRectangle(section1Bounds.toFloat(), scale.getScaled(4.0f));
    g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
    g.drawRoundedRectangle(section1Bounds.toFloat(), scale.getScaled(4.0f), scale.getScaledLineThickness());
}

inline void AboutTab::resized()
{
    auto& scale = GlobalUIScale::getInstance();
    auto bounds = getLocalBounds().reduced(scale.getScaled(15));
    
    const int sectionSpacing = scale.getScaled(8);
    const int controlSpacing = scale.getScaled(4);
    const int labelHeight = scale.getScaled(18);
    const int headerHeight = scale.getScaled(22);
    
    // About section
    auto aboutBounds = bounds.removeFromTop(headerHeight + labelHeight + controlSpacing * 2);
    
    aboutHeader.setBounds(aboutBounds.removeFromTop(headerHeight));
    aboutBounds.removeFromTop(controlSpacing);
    
    // Placeholder content
    auto placeholderRow = aboutBounds.removeFromTop(labelHeight);
    placeholderLabel.setBounds(placeholderRow);
}

inline bool AboutTab::keyPressed(const juce::KeyPress& key)
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

inline void AboutTab::mouseDown(const juce::MouseEvent& event)
{
    // Handle mouse event normally
    Component::mouseDown(event);
    
    // Restore focus to parent SettingsWindow after mouse click
    if (onRequestFocus)
        onRequestFocus();
}

inline void AboutTab::setupAboutControls()
{
    // Section header
    addAndMakeVisible(aboutHeader);
    aboutHeader.setText("About", juce::dontSendNotification);
    aboutHeader.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    aboutHeader.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
    
    // Placeholder content
    addAndMakeVisible(placeholderLabel);
    placeholderLabel.setText("About content will be added here", juce::dontSendNotification);
    placeholderLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
    placeholderLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
}

inline void AboutTab::scaleFactorChanged(float newScale)
{
    // Update fonts for all labels
    aboutHeader.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
    placeholderLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f));
    
    // Trigger layout and repaint
    resized();
    repaint();
}