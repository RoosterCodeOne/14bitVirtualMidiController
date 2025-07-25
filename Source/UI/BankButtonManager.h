// BankButtonManager.h - Bank button setup and state management
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"

// Forward declaration
class BankManager;

//==============================================================================
class BankButtonManager
{
public:
    BankButtonManager() = default;
    
    // Setup bank buttons with look and feel and callbacks
    void setupBankButtons(juce::ToggleButton& bankAButton,
                         juce::ToggleButton& bankBButton,
                         juce::ToggleButton& bankCButton,
                         juce::ToggleButton& bankDButton,
                         CustomButtonLookAndFeel& customButtonLookAndFeel,
                         std::function<void(int)> onBankSelected) const
    {
        // Bank A button
        bankAButton.setButtonText("A");
        bankAButton.setLookAndFeel(&customButtonLookAndFeel);
        bankAButton.setToggleState(true, juce::dontSendNotification); // Start with A selected
        bankAButton.onClick = [onBankSelected]() { onBankSelected(0); };
        
        // Bank B button
        bankBButton.setButtonText("B");
        bankBButton.setLookAndFeel(&customButtonLookAndFeel);
        bankBButton.onClick = [onBankSelected]() { onBankSelected(1); };
        
        // Bank C button
        bankCButton.setButtonText("C");
        bankCButton.setLookAndFeel(&customButtonLookAndFeel);
        bankCButton.onClick = [onBankSelected]() { onBankSelected(2); };
        
        // Bank D button
        bankDButton.setButtonText("D");
        bankDButton.setLookAndFeel(&customButtonLookAndFeel);
        bankDButton.onClick = [onBankSelected]() { onBankSelected(3); };
    }
    
    // Update bank button states based on active bank
    void updateBankButtonStates(juce::ToggleButton& bankAButton,
                               juce::ToggleButton& bankBButton,
                               juce::ToggleButton& bankCButton,
                               juce::ToggleButton& bankDButton,
                               CustomButtonLookAndFeel& customButtonLookAndFeel,
                               int activeBank,
                               const juce::Colour& bankAColor,
                               const juce::Colour& bankBColor,
                               const juce::Colour& bankCColor,
                               const juce::Colour& bankDColor) const
    {
        // Update toggle states based on whether bank color is active (not darkgrey)
        // This supports both 4-slider mode (single bank) and 8-slider mode (paired banks)
        bankAButton.setToggleState(bankAColor != juce::Colours::darkgrey, juce::dontSendNotification);
        bankBButton.setToggleState(bankBColor != juce::Colours::darkgrey, juce::dontSendNotification);
        bankCButton.setToggleState(bankCColor != juce::Colours::darkgrey, juce::dontSendNotification);
        bankDButton.setToggleState(bankDColor != juce::Colours::darkgrey, juce::dontSendNotification);
        
        // Apply bank-specific colors to buttons
        customButtonLookAndFeel.setButtonColor(&bankAButton, bankAColor);
        customButtonLookAndFeel.setButtonColor(&bankBButton, bankBColor);
        customButtonLookAndFeel.setButtonColor(&bankCButton, bankCColor);
        customButtonLookAndFeel.setButtonColor(&bankDButton, bankDColor);
        
        // Repaint buttons to reflect color changes
        bankAButton.repaint();
        bankBButton.repaint();
        bankCButton.repaint();
        bankDButton.repaint();
    }
    
    // Clean up bank button look and feel references
    void cleanupBankButtons(juce::ToggleButton& bankAButton,
                           juce::ToggleButton& bankBButton,
                           juce::ToggleButton& bankCButton,
                           juce::ToggleButton& bankDButton,
                           CustomButtonLookAndFeel& customButtonLookAndFeel) const
    {
        // Clean up custom look and feel and button color mappings
        customButtonLookAndFeel.removeButtonColor(&bankAButton);
        customButtonLookAndFeel.removeButtonColor(&bankBButton);
        customButtonLookAndFeel.removeButtonColor(&bankCButton);
        customButtonLookAndFeel.removeButtonColor(&bankDButton);
        
        bankAButton.setLookAndFeel(nullptr);
        bankBButton.setLookAndFeel(nullptr);
        bankCButton.setLookAndFeel(nullptr);
        bankDButton.setLookAndFeel(nullptr);
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BankButtonManager)
};