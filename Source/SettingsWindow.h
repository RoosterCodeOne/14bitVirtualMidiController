// SettingsWindow.h - Modular Tabbed Interface Version
#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"
#include "CustomLookAndFeel.h"
#include "Core/SliderDisplayManager.h"
#include "UI/ControllerSettingsTab.h"
#include "UI/PresetManagementTab.h"

//==============================================================================
class SettingsWindow : public juce::Component
{
public:
    SettingsWindow();
    ~SettingsWindow();
    
    void setVisible(bool shouldBeVisible) override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Public API - maintained for backward compatibility
    int getMidiChannel() const;
    int getCCNumber(int sliderIndex) const;
    std::pair<double, double> getCustomRange(int sliderIndex) const;
    juce::Colour getSliderColor(int sliderIndex) const;
    
    // Preset system interface
    ControllerPreset getCurrentPreset() const;
    void applyPreset(const ControllerPreset& preset);
    PresetManager& getPresetManager();
    
    // Per-slider selection methods
    int getSelectedSlider() const { return selectedSlider; }
    int getSelectedBank() const { return selectedBank; }
    void selectSlider(int sliderIndex);
    void updateBankSelection(int bankIndex);
    
    // New per-slider settings access methods
    bool is14BitOutput(int sliderIndex) const;
    double getIncrement(int sliderIndex) const;
    bool useDeadzone(int sliderIndex) const;
    juce::String getDisplayUnit(int sliderIndex) const;
    SliderOrientation getSliderOrientation(int sliderIndex) const;
    BipolarSettings getBipolarSettings(int sliderIndex) const;
    
    // BPM management methods
    void setBPM(double bpm);
    double getBPM() const;
    void setSyncStatus(bool isExternal, double externalBPM = 0.0);
    
    // Callback functions - maintained for backward compatibility
    std::function<void()> onSettingsChanged;
    std::function<void(const ControllerPreset&)> onPresetLoaded;
    std::function<void(double)> onBPMChanged;
    std::function<void(int)> onSelectedSliderChanged;
    std::function<void(int)> onBankSelectionChanged;
    
    // Keyboard handling
    bool keyPressed(const juce::KeyPress& key) override;
    
private:
    // Tab management using raw pointers for JUCE compatibility
    juce::TabbedComponent* tabbedComponent;
    std::unique_ptr<ControllerSettingsTab> controllerTab;
    std::unique_ptr<PresetManagementTab> presetTab;
    
    // Shared state
    PresetManager presetManager;
    int selectedBank = 0;
    int selectedSlider = 0;
    bool controlsInitialized = false;
    bool updatingFromMainWindow = false;
    
    // Data storage for all 16 sliders
    struct SliderSettings {
        int ccNumber = 0;
        bool is14Bit = true;
        double rangeMin = 0.0;
        double rangeMax = 16383.0;
        juce::String displayUnit;
        double increment = 1.0;
        bool useDeadzone = true;
        int colorId = 1;
        SliderOrientation orientation = SliderOrientation::Normal;
        BipolarSettings bipolarSettings;
        
        SliderSettings()
        {
            ccNumber = 0;
            is14Bit = true;
            rangeMin = 0.0;
            rangeMax = 16383.0;
            displayUnit = juce::String();
            increment = 1.0;
            useDeadzone = true;
            colorId = 1;
            orientation = SliderOrientation::Normal;
            bipolarSettings = BipolarSettings((rangeMin + rangeMax) / 2.0);
        }
    };
    SliderSettings sliderSettingsData[16];
    
    // Private methods
    void setupTabs();
    void setupCommunication();
    void initializeSliderData();
    void setSelectedSlider(int sliderIndex);
    void updateControlsForSelectedSlider();
    void saveCurrentSliderSettings();
    void validateAndApplyCCNumber(int value);
    void applyOutputMode(bool is14Bit);
    void validateAndApplyRange(double minVal, double maxVal);
    void applyDisplayUnit(const juce::String& unit);
    void applyIncrements(double increment);
    void applyInputMode(bool useDeadzone);
    void selectColor(int colorId);
    void resetCurrentSlider();
    void cycleSliderInBank(int bankIndex);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
};

//==============================================================================
// Implementation
inline SettingsWindow::SettingsWindow()
    : tabbedComponent(nullptr)
{
    setupTabs();
    setupCommunication();
    initializeSliderData();
    
    // Enable keyboard focus for arrow key handling
    setWantsKeyboardFocus(true);
}

inline SettingsWindow::~SettingsWindow()
{
    // Clean up tab component manually since it's a raw pointer
    delete tabbedComponent;
    tabbedComponent = nullptr;
}

inline void SettingsWindow::setupTabs()
{
    // Create tabbed component using raw pointer approach
    tabbedComponent = new juce::TabbedComponent(juce::TabbedButtonBar::TabsAtTop);
    addAndMakeVisible(tabbedComponent);
    tabbedComponent->setTabBarDepth(30);
    tabbedComponent->setOutline(0);
    
    // Disable keyboard focus for tabbed component to prevent it from intercepting arrow keys
    tabbedComponent->setWantsKeyboardFocus(false);
    
    // Create tab instances
    controllerTab = std::make_unique<ControllerSettingsTab>(this);
    presetTab = std::make_unique<PresetManagementTab>(this, presetManager);
    
    // Add tabs to tabbed component
    tabbedComponent->addTab("Controller", BlueprintColors::windowBackground, controllerTab.get(), false);
    tabbedComponent->addTab("Presets", BlueprintColors::windowBackground, presetTab.get(), false);
    
    // Set tab colors
    tabbedComponent->setColour(juce::TabbedComponent::backgroundColourId, BlueprintColors::windowBackground);
    tabbedComponent->setColour(juce::TabbedComponent::outlineColourId, BlueprintColors::blueprintLines);
    tabbedComponent->setColour(juce::TabbedButtonBar::tabOutlineColourId, BlueprintColors::blueprintLines);
    tabbedComponent->setColour(juce::TabbedButtonBar::tabTextColourId, BlueprintColors::textSecondary);
    tabbedComponent->setColour(juce::TabbedButtonBar::frontTextColourId, BlueprintColors::active.withAlpha(0.3f));
}

inline void SettingsWindow::setupCommunication()
{
    // Controller tab callbacks
    controllerTab->onSettingsChanged = [this]() {
        if (onSettingsChanged)
            onSettingsChanged();
    };
    
    controllerTab->onBPMChanged = [this](double bpm) {
        if (onBPMChanged)
            onBPMChanged(bpm);
    };
    
    controllerTab->onBankSelected = [this](int bankIndex) {
        selectedBank = bankIndex;
        // Don't call setSelectedSlider here - the selectedSlider is already set
        // by the cycling logic in cycleSliderInBank() and onSliderSettingChanged callback
        // Just notify parent about bank change for any external coordination
        if (onSelectedSliderChanged)
            onSelectedSliderChanged(selectedSlider);
            
        // Notify main window about bank selection change for bidirectional sync
        // But only if this change is not coming from the main window
        if (onBankSelectionChanged && !updatingFromMainWindow)
            onBankSelectionChanged(bankIndex);
    };
    
    controllerTab->onRequestFocus = [this]() {
        // Only request focus if this window is visible and doesn't already have focus
        if (isVisible() && isShowing() && !hasKeyboardFocus(true))
        {
            // Use a simple approach: request focus to be transferred to this component
            // This is safer than grabKeyboardFocus as it works with JUCE's focus system
            toFront(true); // Bring to front and give focus
        }
    };
    
    // Callback for when individual settings are changed (save current settings)
    controllerTab->onSliderSettingChanged = [this](int sliderIndex) {
        // Save current slider's settings when individual controls are modified
        // Note: sliderIndex should be the currently selected slider, not a new slider
        saveCurrentSliderSettings();
        if (onSettingsChanged)
            onSettingsChanged();
    };
    
    // Callback for when slider selection changes (bank cycling, etc.)
    controllerTab->onSliderSelectionChanged = [this](int sliderIndex) {
        // Save current slider settings before switching
        saveCurrentSliderSettings();
        
        // Update selected slider
        selectedSlider = sliderIndex;
        selectedBank = selectedSlider / 4;
        
        // Load new slider's settings
        updateControlsForSelectedSlider();
        
        if (onSelectedSliderChanged)
            onSelectedSliderChanged(selectedSlider);
    };
    
    // Preset tab callbacks
    presetTab->onPresetLoaded = [this](const ControllerPreset& preset) {
        applyPreset(preset);
        if (onPresetLoaded)
            onPresetLoaded(preset);
    };
    
    presetTab->onPresetSaved = [this]() {
        // Handle preset save - get current state and save it
        auto currentPreset = getCurrentPreset();
        // The preset name handling is done in the tab's dialog
        if (onSettingsChanged)
            onSettingsChanged();
    };
    
    presetTab->onPresetDeleted = [this]() {
        if (onSettingsChanged)
            onSettingsChanged();
    };
    
    presetTab->onResetToDefaults = [this]() {
        // Reset all slider settings to defaults
        initializeSliderData();
        controllerTab->updateControlsForSelectedSlider(selectedSlider);
        controllerTab->updateBankSelectorAppearance(selectedBank);
        
        if (onSettingsChanged)
            onSettingsChanged();
        
        // Also notify parent to reset sliders to default values/states
        if (onPresetLoaded)
        {
            ControllerPreset defaultPreset; // Creates preset with default values
            onPresetLoaded(defaultPreset);
        }
    };
}

inline void SettingsWindow::initializeSliderData()
{
    // Initialize slider settings data with defaults
    for (int i = 0; i < 16; ++i)
    {
        auto& settings = sliderSettingsData[i];
        settings.ccNumber = i;
        settings.is14Bit = true;
        settings.rangeMin = 0.0;
        settings.rangeMax = 16383.0;
        settings.displayUnit = juce::String();
        settings.increment = 1.0;
        settings.useDeadzone = true;
        
        // Set default colors based on bank
        int bankIndex = i / 4;
        switch (bankIndex)
        {
            case 0: settings.colorId = 2; break; // Red
            case 1: settings.colorId = 3; break; // Blue
            case 2: settings.colorId = 4; break; // Green
            case 3: settings.colorId = 5; break; // Yellow
            default: settings.colorId = 1; break; // Default
        }
    }
}

inline void SettingsWindow::setVisible(bool shouldBeVisible)
{
    if (shouldBeVisible && !controlsInitialized)
    {
        controlsInitialized = true;
        presetTab->refreshPresetList();
    }
    
    if (shouldBeVisible)
    {
        presetTab->refreshPresetList();
        // Ensure keyboard focus is properly set when window becomes visible
        // Use toFront instead of grabKeyboardFocus for safer focus management
        toFront(true);
    }
    
    Component::setVisible(shouldBeVisible);
}

inline void SettingsWindow::paint(juce::Graphics& g)
{
    // Blueprint aesthetic background
    auto bounds = getLocalBounds().toFloat();
    
    // Blueprint window background (slightly lighter than main background)
    g.setColour(BlueprintColors::windowBackground);
    g.fillAll();
    
    // Draw complete window outline - blueprint style
    g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
    g.drawRect(bounds, 1.0f);
}

inline void SettingsWindow::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    if (tabbedComponent)
        tabbedComponent->setBounds(bounds);
}

// Public API implementation - maintained for backward compatibility
inline int SettingsWindow::getMidiChannel() const
{
    return controllerTab->getMidiChannel();
}

inline int SettingsWindow::getCCNumber(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
        return sliderSettingsData[sliderIndex].ccNumber;
    return sliderIndex;
}

inline std::pair<double, double> SettingsWindow::getCustomRange(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
    {
        const auto& settings = sliderSettingsData[sliderIndex];
        return {settings.rangeMin, settings.rangeMax};
    }
    return {0.0, 16383.0};
}

inline juce::Colour SettingsWindow::getSliderColor(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
    {
        int colorId = sliderSettingsData[sliderIndex].colorId;
        switch (colorId)
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
            {
                // Return default bank colors
                int bankIndex = sliderIndex / 4;
                switch (bankIndex)
                {
                    case 0: return juce::Colours::red;
                    case 1: return juce::Colours::blue;
                    case 2: return juce::Colours::green;
                    case 3: return juce::Colours::yellow;
                    default: return juce::Colours::cyan;
                }
            }
        }
    }
    return juce::Colours::cyan;
}

inline ControllerPreset SettingsWindow::getCurrentPreset() const
{
    ControllerPreset preset;
    preset.name = "Current State";
    preset.midiChannel = getMidiChannel();
    
    // Read from internal slider settings data
    for (int i = 0; i < 16; ++i)
    {
        if (i < preset.sliders.size())
        {
            const auto& settings = sliderSettingsData[i];
            preset.sliders.getReference(i).ccNumber = settings.ccNumber;
            preset.sliders.getReference(i).minRange = settings.rangeMin;
            preset.sliders.getReference(i).maxRange = settings.rangeMax;
            preset.sliders.getReference(i).colorId = settings.colorId;
        }
    }
    
    return preset;
}

inline void SettingsWindow::applyPreset(const ControllerPreset& preset)
{
    // Apply to controller tab
    controllerTab->applyPreset(preset);
    
    // Apply slider settings to internal data
    for (int i = 0; i < juce::jmin(16, preset.sliders.size()); ++i)
    {
        const auto& sliderPreset = preset.sliders[i];
        auto& settings = sliderSettingsData[i];
        
        settings.ccNumber = sliderPreset.ccNumber;
        settings.rangeMin = sliderPreset.minRange;
        settings.rangeMax = sliderPreset.maxRange;
        settings.colorId = sliderPreset.colorId;
    }
    
    // Update controls for currently selected slider
    if (controlsInitialized)
    {
        updateControlsForSelectedSlider();
    }
}

inline PresetManager& SettingsWindow::getPresetManager()
{
    return presetManager;
}

inline void SettingsWindow::selectSlider(int sliderIndex)
{
    setSelectedSlider(sliderIndex);
}

inline void SettingsWindow::updateBankSelection(int bankIndex)
{
    // Update the controller tab's bank selection to match main window
    // Set flag to prevent circular callback
    updatingFromMainWindow = true;
    selectedBank = bankIndex;
    
    if (controllerTab)
    {
        controllerTab->updateBankSelectorAppearance(bankIndex);
    }
    
    updatingFromMainWindow = false;
}

inline bool SettingsWindow::is14BitOutput(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
        return sliderSettingsData[sliderIndex].is14Bit;
    return true;
}

inline double SettingsWindow::getIncrement(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
        return sliderSettingsData[sliderIndex].increment;
    return 1.0;
}

inline bool SettingsWindow::useDeadzone(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
        return sliderSettingsData[sliderIndex].useDeadzone;
    return true;
}

inline juce::String SettingsWindow::getDisplayUnit(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
        return sliderSettingsData[sliderIndex].displayUnit;
    return juce::String();
}

inline SliderOrientation SettingsWindow::getSliderOrientation(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
        return sliderSettingsData[sliderIndex].orientation;
    return SliderOrientation::Normal;
}

inline BipolarSettings SettingsWindow::getBipolarSettings(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
        return sliderSettingsData[sliderIndex].bipolarSettings;
    return BipolarSettings();
}

inline void SettingsWindow::setBPM(double bpm)
{
    controllerTab->setBPM(bpm);
}

inline double SettingsWindow::getBPM() const
{
    return controllerTab->getBPM();
}

inline void SettingsWindow::setSyncStatus(bool isExternal, double externalBPM)
{
    controllerTab->setSyncStatus(isExternal, externalBPM);
}

inline bool SettingsWindow::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        setVisible(false);
        return true;
    }
    else if (key == juce::KeyPress::upKey)
    {
        // Up: Switch between banks A→B→C→D→A, reset to first slider in bank
        int newBank = (selectedSlider / 4 + 3) % 4; // Previous bank
        setSelectedSlider(newBank * 4);
        return true;
    }
    else if (key == juce::KeyPress::downKey)
    {
        // Down: Switch between banks A→B→C→D→A, reset to first slider in bank
        int newBank = (selectedSlider / 4 + 1) % 4; // Next bank
        setSelectedSlider(newBank * 4);
        return true;
    }
    else if (key == juce::KeyPress::leftKey)
    {
        // Left: Navigate to previous slider globally (cross-bank navigation with wraparound)
        // Examples: 4→3, 4→3 (B1→A4), 0→15 (A1→D4)
        int newSlider = (selectedSlider + 15) % 16; // Previous slider globally (wraparound from 0 to 15)
        setSelectedSlider(newSlider); // Automatically switches banks and updates visual feedback
        return true;
    }
    else if (key == juce::KeyPress::rightKey)
    {
        // Right: Navigate to next slider globally (cross-bank navigation with wraparound)  
        // Examples: 3→4, 3→4 (A4→B1), 15→0 (D4→A1)
        int newSlider = (selectedSlider + 1) % 16; // Next slider globally (wraparound from 15 to 0)
        setSelectedSlider(newSlider); // Automatically switches banks and updates visual feedback
        return true;
    }
    
    return Component::keyPressed(key);
}

// Private methods
inline void SettingsWindow::setSelectedSlider(int sliderIndex)
{
    if (sliderIndex < 0 || sliderIndex >= 16)
        return;
        
    selectedSlider = sliderIndex;
    selectedBank = selectedSlider / 4;
    
    updateControlsForSelectedSlider();
    
    if (onSelectedSliderChanged)
        onSelectedSliderChanged(selectedSlider);
}

inline void SettingsWindow::saveCurrentSliderSettings()
{
    if (controlsInitialized && selectedSlider >= 0 && selectedSlider < 16)
    {
        // Save current UI values to the current slider's data
        auto& settings = sliderSettingsData[selectedSlider];
        settings.ccNumber = controllerTab->getCurrentCCNumber();
        settings.is14Bit = controllerTab->getCurrentIs14Bit();
        settings.rangeMin = controllerTab->getCurrentRangeMin();
        settings.rangeMax = controllerTab->getCurrentRangeMax();
        settings.displayUnit = controllerTab->getCurrentDisplayUnit();
        settings.increment = controllerTab->getCurrentIncrement();
        settings.useDeadzone = controllerTab->getCurrentUseDeadzone();
        settings.colorId = controllerTab->getCurrentColorId();
    }
}

inline void SettingsWindow::updateControlsForSelectedSlider()
{
    if (controlsInitialized && selectedSlider >= 0 && selectedSlider < 16)
    {
        const auto& settings = sliderSettingsData[selectedSlider];
        
        // Update the controller tab with the current slider's settings
        controllerTab->setSliderSettings(
            settings.ccNumber,
            settings.is14Bit,
            settings.rangeMin,
            settings.rangeMax,
            settings.displayUnit,
            settings.increment,
            settings.useDeadzone,
            settings.colorId
        );
        
        controllerTab->updateControlsForSelectedSlider(selectedSlider);
        controllerTab->updateBankSelectorAppearance(selectedBank);
    }
}
