#include "BankManager.h"

//==============================================================================
BankManager::BankManager()
{
    DBG("BankManager: Created");
}

//==============================================================================
void BankManager::setActiveBank(int bankIndex)
{
    if (eightSliderMode)
    {
        // In 8-slider mode, clicking a bank switches to its pair
        if (bankIndex <= 1)
            currentBank = 0; // Clicking A or B shows A+B pair
        else
            currentBank = 2; // Clicking C or D shows C+D pair
    }
    else
    {
        // In 4-slider mode, show the individual bank
        currentBank = bankIndex;
    }
    
    calculateVisibility();
    
    if (onBankChanged)
        onBankChanged();
        
    if (onBankColorsChanged)
        onBankColorsChanged(getCurrentBankColors());
        
    if (onBankSelectionChanged)
        onBankSelectionChanged(bankIndex);
}

void BankManager::setSliderMode(bool isEightSliderMode)
{
    eightSliderMode = isEightSliderMode;
    
    calculateVisibility();
    
    if (onModeChanged)
        onModeChanged();
        
    if (onBankColorsChanged)
        onBankColorsChanged(getCurrentBankColors());
}

int BankManager::getActiveBank() const
{
    return currentBank;
}

bool BankManager::isEightSliderMode() const
{
    return eightSliderMode;
}

//==============================================================================
int BankManager::getVisibleSliderIndex(int position) const
{
    if (eightSliderMode)
    {
        // In 8-slider mode, show bank pairs: A+B (0-7) or C+D (8-15)
        int bankPair = currentBank >= 2 ? 1 : 0; // 0 for A+B, 1 for C+D
        return (bankPair * 8) + position;
    }
    else
    {
        // In 4-slider mode, show single bank
        return (currentBank * 4) + position;
    }
}

int BankManager::getVisibleSliderCount() const
{
    return eightSliderMode ? 8 : 4;
}

bool BankManager::isSliderVisible(int sliderIndex) const
{
    int visibleCount = getVisibleSliderCount();
    
    for (int i = 0; i < visibleCount; ++i)
    {
        if (getVisibleSliderIndex(i) == sliderIndex)
            return true;
    }
    
    return false;
}

//==============================================================================
BankManager::BankColors BankManager::getCurrentBankColors() const
{
    return getBankColors();
}

//==============================================================================
void BankManager::calculateVisibility()
{
    // Visibility calculation is handled by getVisibleSliderIndex()
    // This method exists for future expansion if needed
}

BankManager::BankColors BankManager::getBankColors() const
{
    BankColors colors;
    
    if (eightSliderMode)
    {
        // In 8-slider mode, light up both banks in the pair
        if (currentBank <= 1) // A+B pair
        {
            colors.bankA = juce::Colours::red;
            colors.bankB = juce::Colours::blue;
            colors.bankC = juce::Colours::darkgrey;
            colors.bankD = juce::Colours::darkgrey;
        }
        else // C+D pair
        {
            colors.bankA = juce::Colours::darkgrey;
            colors.bankB = juce::Colours::darkgrey;
            colors.bankC = juce::Colours::green;
            colors.bankD = juce::Colours::yellow;
        }
    }
    else
    {
        // In 4-slider mode, light up only the active bank
        colors.bankA = juce::Colours::darkgrey;
        colors.bankB = juce::Colours::darkgrey;
        colors.bankC = juce::Colours::darkgrey;
        colors.bankD = juce::Colours::darkgrey;
        
        switch (currentBank)
        {
            case 0: colors.bankA = juce::Colours::red; break;
            case 1: colors.bankB = juce::Colours::blue; break;
            case 2: colors.bankC = juce::Colours::green; break;
            case 3: colors.bankD = juce::Colours::yellow; break;
        }
    }
    
    return colors;
}