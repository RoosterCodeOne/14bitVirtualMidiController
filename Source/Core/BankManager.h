#pragma once
#include <JuceHeader.h>
#include <functional>

//==============================================================================
/**
 * BankManager handles bank switching logic, slider visibility calculations, and mode transitions
 * Extracted from DebugMidiController to provide clean separation of concerns
 */
class BankManager
{
public:
    BankManager();
    ~BankManager() = default;
    
    // Bank and mode management
    void setActiveBank(int bankIndex);
    void setSliderMode(bool isEightSliderMode);
    int getActiveBank() const;
    bool isEightSliderMode() const;
    
    // Slider visibility calculations
    int getVisibleSliderIndex(int position) const;
    int getVisibleSliderCount() const;
    bool isSliderVisible(int sliderIndex) const;
    
    // Bank color management
    struct BankColors {
        juce::Colour bankA, bankB, bankC, bankD;
    };
    BankColors getCurrentBankColors() const;
    
    // Callbacks for parent components
    std::function<void()> onBankChanged;
    std::function<void()> onModeChanged;
    std::function<void(const BankColors&)> onBankColorsChanged;
    std::function<void(int bankIndex)> onBankSelectionChanged;
    
private:
    // Internal state
    int currentBank = 0;
    bool eightSliderMode = false;
    
    // Internal helper methods
    void calculateVisibility();
    BankColors getBankColors() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BankManager)
};