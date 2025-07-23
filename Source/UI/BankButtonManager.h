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
        // Update toggle states for bank buttons
        bankAButton.setToggleState(activeBank == 0, juce::dontSendNotification);
        bankBButton.setToggleState(activeBank == 1, juce::dontSendNotification);
        bankCButton.setToggleState(activeBank == 2, juce::dontSendNotification);
        bankDButton.setToggleState(activeBank == 3, juce::dontSendNotification);
        
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