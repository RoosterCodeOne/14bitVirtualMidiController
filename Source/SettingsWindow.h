// SettingsWindow.h - Modular Tabbed Interface Version
#pragma once
#include <JuceHeader.h>
#include "PresetManager.h"
#include "CustomLookAndFeel.h"
#include "Core/SliderDisplayManager.h"
#include "UI/GlobalSettingsTab.h"
#include "UI/ControllerSettingsTab.h"
#include "UI/PresetManagementTab.h"
#include "UI/AboutTab.h"
#include "UI/GlobalUIScale.h"
#include "UI/ThemeManager.h"

//==============================================================================
class SettingsWindow : public juce::Component,
                       public GlobalUIScale::ScaleChangeListener,
                       public ThemeManager::ThemeChangeListener
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
    bool getUseDeadzone(int sliderIndex) const;
    
    // Preset system interface
    ControllerPreset getCurrentPreset() const;
    void applyPreset(const ControllerPreset& preset);
    PresetManager& getPresetManager();
    
    // Per-slider selection methods
    int getSelectedSlider() const { return selectedSlider; }
    int getSelectedBank() const { return selectedBank; }
    void selectSlider(int sliderIndex);
    void updateBankSelection(int bankIndex);
    void applyRangePreset(int sliderIndex, int rangeType);

    // Copy/Paste operations
    void copySlider(int sliderIndex);
    void pasteSlider(int sliderIndex);
    void resetSlider(int sliderIndex);
    bool hasClipboardData() const { return hasClipboard; }
    
    // New per-slider settings access methods
    double getIncrement(int sliderIndex) const;
    bool isStepCustom(int sliderIndex) const;
    bool useDeadzone(int sliderIndex) const;
    SliderOrientation getSliderOrientation(int sliderIndex) const;
    BipolarSettings getBipolarSettings(int sliderIndex) const;
    juce::String getSliderDisplayName(int sliderIndex) const;
    bool getShowAutomation(int sliderIndex) const;
    
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
    std::function<void(int)> onSliderReset;
    std::function<void(int, MidiInputMode)> onSliderMidiInputModeChanged;
    
    // Keyboard handling
    bool keyPressed(const juce::KeyPress& key) override;
    
    // Scale change notification implementation
    void scaleFactorChanged(float newScale) override;

    // Theme change notification implementation
    void themeChanged(ThemeManager::ThemeType newTheme, const ThemeManager::ThemePalette& palette) override;

private:
    // Shared state (must be declared before tabs that use them)
    PresetManager presetManager;
    int selectedBank = 0;
    int selectedSlider = 0;
    bool controlsInitialized = false;
    bool updatingFromMainWindow = false;

    // Tab management using raw pointers for JUCE compatibility
    juce::TabbedComponent* tabbedComponent;
    std::unique_ptr<GlobalSettingsTab> globalTab;
    std::unique_ptr<ControllerSettingsTab> controllerTab;
    std::unique_ptr<PresetManagementTab> presetTab;
    std::unique_ptr<AboutTab> aboutTab;

    // Data storage for all 16 sliders
    struct SliderSettings {
        int ccNumber = 0;
        double rangeMin = 0.0;
        double rangeMax = 16383.0;
        double increment = 1.0;
        bool isCustomStep = false; // true = user-set custom step, false = auto-calculated step
        bool useDeadzone = true;
        int colorId = 1;
        SliderOrientation orientation = SliderOrientation::Normal;
        BipolarSettings bipolarSettings;
        juce::String customName = "";
        bool showAutomation = true; // NEW: Default to automation shown
        
        SliderSettings()
        {
            ccNumber = 0;
            rangeMin = 0.0;
            rangeMax = 16383.0;
            increment = 1.0;
            isCustomStep = false;
            useDeadzone = true;
            colorId = 1;
            orientation = SliderOrientation::Normal;
            bipolarSettings = BipolarSettings(); // Center value now auto-calculated
            customName = "";
            showAutomation = true; // Default to automation shown
        }
        
        // Update bipolar center when range changes
        // updateBipolarCenter() method removed - center value is now automatically calculated
    };
    SliderSettings sliderSettingsData[16];

    // Clipboard for copy/paste slider settings
    bool hasClipboard = false;
    SliderSettings clipboardSettings;
    
    // Private methods
    void setupTabs();
    void setupCommunication();
    void initializeSliderData();
    void setSelectedSlider(int sliderIndex);
    void updateControlsForSelectedSlider();
    void saveCurrentSliderSettings();
    void validateAndApplyCCNumber(int value);
    // applyOutputMode method removed - system always uses 14-bit output
    void validateAndApplyRange(double minVal, double maxVal);
    void applyIncrements(double increment);
    void applyInputMode(bool useDeadzone);
    void selectColor(int colorId);
    void resetCurrentSlider();
    void cycleSliderInBank(int bankIndex);
    void applyOrientationToSlider(int sliderIndex); // Apply orientation to actual slider
    void applyMidiInputModeToSlider(int sliderIndex); // Apply MIDI input mode to actual slider
    
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

    // Register for scale change notifications
    GlobalUIScale::getInstance().addScaleChangeListener(this);

    // Register for theme change notifications
    ThemeManager::getInstance().addThemeChangeListener(this);
}

inline SettingsWindow::~SettingsWindow()
{
    // Remove scale change listener
    GlobalUIScale::getInstance().removeScaleChangeListener(this);

    // Remove theme change listener
    ThemeManager::getInstance().removeThemeChangeListener(this);

    // Clean up tab component manually since it's a raw pointer
    delete tabbedComponent;
    tabbedComponent = nullptr;
}

inline void SettingsWindow::setupTabs()
{
    // Create tabbed component using raw pointer approach
    tabbedComponent = new juce::TabbedComponent(juce::TabbedButtonBar::TabsAtTop);
    addAndMakeVisible(tabbedComponent);
    
    auto& scale = GlobalUIScale::getInstance();
    tabbedComponent->setTabBarDepth(scale.getScaled(30));
    tabbedComponent->setOutline(0);
    
    // Disable keyboard focus for tabbed component to prevent it from intercepting arrow keys
    tabbedComponent->setWantsKeyboardFocus(false);
    
    // Create tab instances
    globalTab = std::make_unique<GlobalSettingsTab>(this);
    controllerTab = std::make_unique<ControllerSettingsTab>(this);
    presetTab = std::make_unique<PresetManagementTab>(this, presetManager);
    aboutTab = std::make_unique<AboutTab>(this);
    
    // Add tabs to tabbed component
    tabbedComponent->addTab("Global", BlueprintColors::windowBackground(), globalTab.get(), false);
    tabbedComponent->addTab("Slider", BlueprintColors::windowBackground(), controllerTab.get(), false);
    tabbedComponent->addTab("Presets", BlueprintColors::windowBackground(), presetTab.get(), false);
    tabbedComponent->addTab("About", BlueprintColors::windowBackground(), aboutTab.get(), false);
    
    // Set tab colors
    tabbedComponent->setColour(juce::TabbedComponent::backgroundColourId, BlueprintColors::windowBackground());
    tabbedComponent->setColour(juce::TabbedComponent::outlineColourId, BlueprintColors::blueprintLines());
    tabbedComponent->setColour(juce::TabbedButtonBar::tabOutlineColourId, BlueprintColors::blueprintLines());
    tabbedComponent->setColour(juce::TabbedButtonBar::tabTextColourId, BlueprintColors::textSecondary());
    tabbedComponent->setColour(juce::TabbedButtonBar::frontTextColourId, BlueprintColors::active().withAlpha(0.3f));
}

inline void SettingsWindow::setupCommunication()
{
    // Global tab callbacks
    globalTab->onSettingsChanged = [this]() {
        if (onSettingsChanged)
            onSettingsChanged();
    };
    
    globalTab->onBPMChanged = [this](double bpm) {
        if (onBPMChanged)
            onBPMChanged(bpm);
    };


    globalTab->onRequestFocus = [this]() {
        if (isVisible() && isShowing() && !hasKeyboardFocus(true))
        {
            toFront(true);
        }
    };
    
    // Controller tab callbacks
    controllerTab->onSettingsChanged = [this]() {
        if (onSettingsChanged)
            onSettingsChanged();
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
    
    // Callback for slider reset action
    controllerTab->onSliderReset = [this](int sliderIndex) {
        if (onSliderReset)
            onSliderReset(sliderIndex);
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

        // Reset global settings (MIDI channel, BPM, UI scale, etc.)
        ControllerPreset defaultPreset; // Creates preset with default values
        globalTab->applyPreset(defaultPreset);

        // Force immediate refresh of settings controls for current slider
        updateControlsForSelectedSlider();
        controllerTab->updateBankSelectorAppearance(selectedBank);

        if (onSettingsChanged)
            onSettingsChanged();

        // Also notify parent to reset sliders to default values/states
        if (onPresetLoaded)
        {
            onPresetLoaded(defaultPreset);
        }
    };
    
    // About tab callbacks
    aboutTab->onRequestFocus = [this]() {
        if (isVisible() && isShowing() && !hasKeyboardFocus(true))
        {
            toFront(true);
        }
    };
}

inline void SettingsWindow::initializeSliderData()
{
    // Initialize slider settings data with defaults
    for (int i = 0; i < 16; ++i)
    {
        auto& settings = sliderSettingsData[i];
        settings.ccNumber = i + 10; // Start at CC 10 to avoid conflicts
        // Note: is14Bit field removed - system always uses 14-bit output
        settings.rangeMin = 0.0;
        settings.rangeMax = 16383.0;
        settings.increment = 1.0; // Will be auto-calculated
        settings.isCustomStep = false; // Start with auto-calculated step
        settings.useDeadzone = true;
        settings.orientation = SliderOrientation::Normal;
        settings.bipolarSettings = BipolarSettings(); // Center value now auto-calculated
        settings.customName = ""; // Clear custom names on reset
        settings.showAutomation = true; // Reset to default automation visibility
        
        // Set default colors based on bank (using direct mapping 0-7)
        int bankIndex = i / 4;
        switch (bankIndex)
        {
            case 0: settings.colorId = 0; break; // Red
            case 1: settings.colorId = 1; break; // Blue
            case 2: settings.colorId = 2; break; // Green
            case 3: settings.colorId = 3; break; // Yellow
            default: settings.colorId = 0; break; // Default to red
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
    auto& scale = GlobalUIScale::getInstance();
    
    // Blueprint aesthetic background
    auto bounds = getLocalBounds().toFloat();
    
    // Blueprint window background (slightly lighter than main background)
    g.setColour(BlueprintColors::windowBackground());
    g.fillAll();
    
    // Draw complete window outline - blueprint style
    g.setColour(BlueprintColors::blueprintLines().withAlpha(0.6f));
    g.drawRect(bounds, scale.getScaledLineThickness());
}

inline void SettingsWindow::resized()
{
    auto& scale = GlobalUIScale::getInstance();
    auto bounds = getLocalBounds().reduced(scale.getScaled(10));
    if (tabbedComponent)
        tabbedComponent->setBounds(bounds);
}

// Public API implementation - maintained for backward compatibility
inline int SettingsWindow::getMidiChannel() const
{
    return globalTab->getMidiChannel();
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
        // Use direct mapping array (same as ControllerSettingsTab)
        const juce::Colour colors[] = {
            juce::Colours::red, juce::Colours::blue, juce::Colours::green, juce::Colours::yellow,
            juce::Colours::purple, juce::Colours::orange, juce::Colours::cyan, juce::Colours::white
        };
        
        if (colorId >= 0 && colorId < 8)
            return colors[colorId];
        
        // Fallback to default bank colors
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
    return juce::Colours::cyan;
}

inline bool SettingsWindow::getUseDeadzone(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
    {
        return sliderSettingsData[sliderIndex].useDeadzone;
    }
    return false; // Default to Direct mode
}

inline ControllerPreset SettingsWindow::getCurrentPreset() const
{
    ControllerPreset preset;
    preset.name = "Current State";
    preset.midiChannel = getMidiChannel();
    preset.themeName = globalTab->getThemeName(); // Get current theme from GlobalSettingsTab
    preset.uiScale = globalTab->getUIScale(); // Get current UI scale from GlobalSettingsTab
    preset.alwaysOnTop = globalTab->getAlwaysOnTop(); // Get current Always On Top setting
    
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
            preset.sliders.getReference(i).orientation = static_cast<int>(settings.orientation);
            // bipolarCenter removed - now automatically calculated from range
            preset.sliders.getReference(i).customName = settings.customName;
            preset.sliders.getReference(i).showAutomation = settings.showAutomation;
        }
    }
    
    return preset;
}

inline void SettingsWindow::applyPreset(const ControllerPreset& preset)
{
    // Apply to global tab
    globalTab->applyPreset(preset);
    
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
        settings.orientation = static_cast<SliderOrientation>(sliderPreset.orientation);
        // bipolarCenter removed - now automatically calculated from range
        settings.customName = sliderPreset.customName;
        settings.showAutomation = sliderPreset.showAutomation;
        
        // Apply orientation to the actual slider
        applyOrientationToSlider(i);
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

inline void SettingsWindow::applyRangePreset(int sliderIndex, int rangeType)
{
    if (sliderIndex < 0 || sliderIndex >= 16)
        return;

    auto& settings = sliderSettingsData[sliderIndex];

    // Map range type to actual range values
    // Range type IDs from SliderContextMenu enum
    switch (rangeType)
    {
        case 1: // Range_0_127 (7-bit MIDI)
            settings.rangeMin = 0.0;
            settings.rangeMax = 127.0;
            break;
        case 2: // Range_Minus100_Plus100
            settings.rangeMin = -100.0;
            settings.rangeMax = 100.0;
            break;
        case 3: // Range_0_1
            settings.rangeMin = 0.0;
            settings.rangeMax = 1.0;
            break;
        case 4: // Range_0_16383 (14-bit MIDI)
            settings.rangeMin = 0.0;
            settings.rangeMax = 16383.0;
            break;
        default:
            return; // Unknown range type
    }

    // Update UI if this is the currently selected slider
    if (sliderIndex == selectedSlider && controllerTab && controlsInitialized)
    {
        updateControlsForSelectedSlider();
    }

    // Notify parent that settings have changed
    if (onSettingsChanged)
        onSettingsChanged();
}

inline void SettingsWindow::copySlider(int sliderIndex)
{
    if (sliderIndex < 0 || sliderIndex >= 16)
        return;

    // Copy all settings from the specified slider to clipboard
    clipboardSettings = sliderSettingsData[sliderIndex];
    hasClipboard = true;
}

inline void SettingsWindow::pasteSlider(int sliderIndex)
{
    if (sliderIndex < 0 || sliderIndex >= 16 || !hasClipboard)
        return;

    // Paste all settings from clipboard to the specified slider
    // Note: We keep the CC number as-is (don't paste it) to avoid conflicts
    auto& settings = sliderSettingsData[sliderIndex];
    int originalCCNumber = settings.ccNumber; // Preserve original CC number

    settings = clipboardSettings;
    settings.ccNumber = originalCCNumber; // Restore original CC number

    // Update UI if this is the currently selected slider
    if (sliderIndex == selectedSlider && controllerTab && controlsInitialized)
    {
        updateControlsForSelectedSlider();
    }

    // Apply orientation to the actual slider
    applyOrientationToSlider(sliderIndex);

    // Apply MIDI input mode to the actual slider
    applyMidiInputModeToSlider(sliderIndex);

    // Notify parent that settings have changed
    if (onSettingsChanged)
        onSettingsChanged();
}

inline void SettingsWindow::resetSlider(int sliderIndex)
{
    if (sliderIndex < 0 || sliderIndex >= 16)
        return;

    // Reset slider to default settings
    auto& settings = sliderSettingsData[sliderIndex];

    settings.ccNumber = sliderIndex + 10; // Start at CC 10
    settings.rangeMin = 0.0;
    settings.rangeMax = 16383.0;
    settings.increment = 1.0;
    settings.isCustomStep = false;
    settings.useDeadzone = true;
    settings.orientation = SliderOrientation::Normal;
    settings.bipolarSettings = BipolarSettings();
    settings.customName = "";
    settings.showAutomation = true;

    // Set default color based on bank
    int bankIndex = sliderIndex / 4;
    switch (bankIndex)
    {
        case 0: settings.colorId = 0; break; // Red
        case 1: settings.colorId = 1; break; // Blue
        case 2: settings.colorId = 2; break; // Green
        case 3: settings.colorId = 3; break; // Yellow
        default: settings.colorId = 0; break;
    }

    // Update UI if this is the currently selected slider
    if (sliderIndex == selectedSlider && controllerTab && controlsInitialized)
    {
        updateControlsForSelectedSlider();
    }

    // Apply orientation to the actual slider
    applyOrientationToSlider(sliderIndex);

    // Apply MIDI input mode to the actual slider
    applyMidiInputModeToSlider(sliderIndex);

    // Notify parent that settings have changed
    if (onSettingsChanged)
        onSettingsChanged();

    // Trigger the reset callback for any additional handling
    if (onSliderReset)
        onSliderReset(sliderIndex);
}


inline double SettingsWindow::getIncrement(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
        return sliderSettingsData[sliderIndex].increment;
    return 1.0;
}

inline bool SettingsWindow::isStepCustom(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
        return sliderSettingsData[sliderIndex].isCustomStep;
    return false;
}

inline bool SettingsWindow::useDeadzone(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
        return sliderSettingsData[sliderIndex].useDeadzone;
    return true;
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
    globalTab->setBPM(bpm);
}

inline double SettingsWindow::getBPM() const
{
    return globalTab->getBPM();
}

inline void SettingsWindow::setSyncStatus(bool isExternal, double externalBPM)
{
    globalTab->setSyncStatus(isExternal, externalBPM);
}

inline juce::String SettingsWindow::getSliderDisplayName(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
    {
        return sliderSettingsData[sliderIndex].customName; // Return custom name or empty string
    }
    return "";
}

inline bool SettingsWindow::getShowAutomation(int sliderIndex) const
{
    if (sliderIndex >= 0 && sliderIndex < 16)
        return sliderSettingsData[sliderIndex].showAutomation;
    return true; // Default to automation shown
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
        // Up: Switch between banks A->B->C->D->A, reset to first slider in bank
        int newBank = (selectedSlider / 4 + 3) % 4; // Previous bank
        setSelectedSlider(newBank * 4);
        return true;
    }
    else if (key == juce::KeyPress::downKey)
    {
        // Down: Switch between banks A->B->C->D->A, reset to first slider in bank
        int newBank = (selectedSlider / 4 + 1) % 4; // Next bank
        setSelectedSlider(newBank * 4);
        return true;
    }
    else if (key == juce::KeyPress::leftKey)
    {
        // Left: Navigate to previous slider globally (cross-bank navigation with wraparound)
        // Examples: 4->3, 4->3 (B1->A4), 0->15 (A1->D4)
        int newSlider = (selectedSlider + 15) % 16; // Previous slider globally (wraparound from 0 to 15)
        setSelectedSlider(newSlider); // Automatically switches banks and updates visual feedback
        return true;
    }
    else if (key == juce::KeyPress::rightKey)
    {
        // Right: Navigate to next slider globally (cross-bank navigation with wraparound)  
        // Examples: 3->4, 3->4 (A4->B1), 15->0 (D4->A1)
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
        // Always 14-bit mode
        settings.rangeMin = controllerTab->getCurrentRangeMin();
        settings.rangeMax = controllerTab->getCurrentRangeMax();
        settings.increment = controllerTab->getCurrentIncrement();
        settings.isCustomStep = controllerTab->getCurrentIsCustomStep();
        settings.useDeadzone = controllerTab->getCurrentUseDeadzone();
        settings.colorId = controllerTab->getCurrentColorId();
        settings.orientation = controllerTab->getCurrentOrientation();
        // bipolarSettings.centerValue removed - now automatically calculated
        settings.bipolarSettings.snapThreshold = controllerTab->getCurrentSnapThreshold();
        settings.customName = controllerTab->getCurrentCustomName();
        settings.showAutomation = controllerTab->getCurrentShowAutomation();
        
        // Bipolar center automatically calculated - no manual update needed
        
        // Apply orientation to the actual slider
        applyOrientationToSlider(selectedSlider);

        // Apply MIDI input mode to the actual slider
        applyMidiInputModeToSlider(selectedSlider);
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
            settings.rangeMin,
            settings.rangeMax,
            settings.increment,
            settings.isCustomStep,
            settings.useDeadzone,
            settings.colorId,
            settings.orientation,
            settings.customName,
            settings.bipolarSettings.snapThreshold,
            settings.showAutomation
        );
        
        controllerTab->updateControlsForSelectedSlider(selectedSlider);
        controllerTab->updateBankSelectorAppearance(selectedBank);
    }
}

inline void SettingsWindow::applyOrientationToSlider(int sliderIndex)
{
    // This method triggers the main controller to update slider orientations
    // The actual application happens in DebugMidiController::updateSliderSettings()
    if (onSettingsChanged)
        onSettingsChanged();
}

inline void SettingsWindow::applyMidiInputModeToSlider(int sliderIndex)
{
    // Apply the MIDI input mode from the settings data to the actual slider
    if (sliderIndex >= 0 && sliderIndex < 16 && onSliderMidiInputModeChanged)
    {
        const auto& settings = sliderSettingsData[sliderIndex];
        MidiInputMode mode = settings.useDeadzone ? MidiInputMode::Deadzone : MidiInputMode::Direct;
        onSliderMidiInputModeChanged(sliderIndex, mode);
    }
}

inline void SettingsWindow::scaleFactorChanged(float newScale)
{
    // Update tab bar depth for new scale
    if (tabbedComponent)
    {
        auto& scale = GlobalUIScale::getInstance();
        tabbedComponent->setTabBarDepth(scale.getScaled(30));
    }

    // Trigger layout updates when scale changes
    resized();
    repaint();

    // Notify all tabs of scale change
    if (globalTab)
        globalTab->repaint();

    if (controllerTab)
        controllerTab->repaint();

    if (presetTab)
        presetTab->repaint();

    if (aboutTab)
        aboutTab->repaint();

    // Update tabbed component layout
    if (tabbedComponent)
        tabbedComponent->resized();
}

inline void SettingsWindow::themeChanged(ThemeManager::ThemeType newTheme, const ThemeManager::ThemePalette& palette)
{
    // Repaint entire window with new theme
    repaint();

    // Notify all tabs of theme change
    if (globalTab)
        globalTab->repaint();

    if (controllerTab)
        controllerTab->repaint();

    if (presetTab)
        presetTab->repaint();

    if (aboutTab)
        aboutTab->repaint();

    // Repaint tabbed component
    if (tabbedComponent)
        tabbedComponent->repaint();
}
