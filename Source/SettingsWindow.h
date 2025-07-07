// SettingsWindow.h - LAZY INITIALIZATION VERSION (FIXED) -------
#pragma once
#include <JuceHeader.h>

//==============================================================================
class SettingsWindow : public juce::Component
{
public:
    SettingsWindow() : controlsInitialized(false), closeButton("X")
    {
        DBG("SettingsWindow constructor START - LAZY INITIALIZATION");
        
        setSize(700, 600);
        
        // Only create the essential controls in constructor
        addAndMakeVisible(closeButton);
        closeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
        closeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        closeButton.onClick = [this]() { setVisible(false); };
        
        addAndMakeVisible(midiChannelLabel);
        midiChannelLabel.setText("MIDI Channel:", juce::dontSendNotification);
        addAndMakeVisible(midiChannelCombo);
        for (int i = 1; i <= 16; ++i)
            midiChannelCombo.addItem("Channel " + juce::String(i), i);
        midiChannelCombo.setSelectedId(1);
        
        // Bank labels
        addAndMakeVisible(bankALabel);
        bankALabel.setText("Bank A", juce::dontSendNotification);
        bankALabel.setColour(juce::Label::textColourId, juce::Colours::red);
        bankALabel.setFont(juce::FontOptions(16.0f));
        
        addAndMakeVisible(bankBLabel);
        bankBLabel.setText("Bank B", juce::dontSendNotification);
        bankBLabel.setColour(juce::Label::textColourId, juce::Colours::blue);
        bankBLabel.setFont(juce::FontOptions(16.0f));
        
        DBG("SettingsWindow constructor COMPLETE - LAZY INITIALIZATION");
    }
    
    void setVisible(bool shouldBeVisible) override
    {
        if (shouldBeVisible && !controlsInitialized)
        {
            DBG("First time showing - initializing slider controls");
            initializeSliderControls();
        }
        
        Component::setVisible(shouldBeVisible);
    }
    
    void paint(juce::Graphics& g) override
    {
        // Semi-transparent background
        g.fillAll(juce::Colours::black.withAlpha(0.8f));
        
        // Settings panel
        auto bounds = getLocalBounds().reduced(50);
        g.setColour(juce::Colours::darkgrey);
        g.fillRoundedRectangle(bounds.toFloat(), 10.0f);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(18.0f));
        g.drawText("Settings - Lazy Loaded", bounds.removeFromTop(40), juce::Justification::centred);
        
        if (!controlsInitialized)
        {
            g.setFont(juce::FontOptions(14.0f));
            g.drawText("Loading controls...", bounds, juce::Justification::centred);
            return;
        }
        
        // Draw dashes between min/max range inputs
        g.setFont(juce::FontOptions(14.0f));
        bounds.removeFromTop(10);
        bounds.removeFromTop(30); // MIDI channel area
        bounds.removeFromTop(15); // Spacing
        bounds.removeFromTop(30); // Bank A label
        
        // Draw dashes for each slider
        for (int i = 0; i < 8; ++i)
        {
            if (i == 4)
            {
                bounds.removeFromTop(10); // Bank spacing
                bounds.removeFromTop(30); // Bank B label
            }
            
            auto row = bounds.removeFromTop(30);
            
            // Calculate dash position
            int dashX = 50 + 65 + 80 + 10 + 45 + 60 + 5 + 5;
            g.drawText("-", dashX, row.getY() + 5, 10, 20, juce::Justification::centred);
            
            bounds.removeFromTop(5); // Small spacing
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(50);
        
        // Close button
        closeButton.setBounds(bounds.getRight() - 30, bounds.getY() + 5, 25, 25);
        
        bounds.removeFromTop(50); // Title space
        
        // MIDI Channel
        auto channelArea = bounds.removeFromTop(30);
        midiChannelLabel.setBounds(channelArea.removeFromLeft(100));
        midiChannelCombo.setBounds(channelArea.removeFromLeft(120));
        
        bounds.removeFromTop(15); // Spacing
        
        if (!controlsInitialized)
            return; // Don't layout controls that don't exist yet
        
        // Bank A label
        bankALabel.setBounds(bounds.removeFromTop(25));
        bounds.removeFromTop(5); // Small spacing
        
        // Controls for Bank A (sliders 0-3)
        for (int i = 0; i < 4; ++i)
        {
            auto row = bounds.removeFromTop(30);
            
            ccLabels[i]->setBounds(row.removeFromLeft(65));
            ccInputs[i]->setBounds(row.removeFromLeft(50)); // Smaller for text input
            row.removeFromLeft(10);
            rangeLabels[i]->setBounds(row.removeFromLeft(45));
            minRangeInputs[i]->setBounds(row.removeFromLeft(60));
            row.removeFromLeft(5);
            row.removeFromLeft(10); // space for dash
            maxRangeInputs[i]->setBounds(row.removeFromLeft(60));
            row.removeFromLeft(10);
            colorCombos[i]->setBounds(row.removeFromLeft(80));
            
            bounds.removeFromTop(5);
        }
        
        bounds.removeFromTop(10); // Spacing between banks
        
        // Bank B label
        bankBLabel.setBounds(bounds.removeFromTop(25));
        bounds.removeFromTop(5);
        
        // Controls for Bank B (sliders 4-7)
        for (int i = 4; i < 8; ++i)
        {
            auto row = bounds.removeFromTop(30);
            
            ccLabels[i]->setBounds(row.removeFromLeft(65));
            ccInputs[i]->setBounds(row.removeFromLeft(50)); // Smaller for text input
            row.removeFromLeft(10);
            rangeLabels[i]->setBounds(row.removeFromLeft(45));
            minRangeInputs[i]->setBounds(row.removeFromLeft(60));
            row.removeFromLeft(5);
            row.removeFromLeft(10); // space for dash
            maxRangeInputs[i]->setBounds(row.removeFromLeft(60));
            row.removeFromLeft(10);
            colorCombos[i]->setBounds(row.removeFromLeft(80));
            
            if (i < 7)
                bounds.removeFromTop(5);
        }
    }
    
    int getMidiChannel() const { return midiChannelCombo.getSelectedId(); }
    
    int getCCNumber(int sliderIndex) const
    {
        if (!controlsInitialized) return sliderIndex; // fallback before initialization
        
        if (sliderIndex < ccInputs.size())
        {
            auto text = ccInputs[sliderIndex]->getText();
            int ccNumber = text.getIntValue();
            return juce::jlimit(0, 127, ccNumber); // Clamp to valid MIDI CC range
        }
        return sliderIndex; // fallback
    }
    
    std::pair<double, double> getCustomRange(int sliderIndex) const
    {
        if (!controlsInitialized) return {0.0, 16383.0}; // fallback before initialization
        
        if (sliderIndex < minRangeInputs.size() && sliderIndex < maxRangeInputs.size())
        {
            double minVal = minRangeInputs[sliderIndex]->getText().getDoubleValue();
            double maxVal = maxRangeInputs[sliderIndex]->getText().getDoubleValue();
            return {minVal, maxVal};
        }
        return {0.0, 16383.0}; // Default range
    }
    
    juce::Colour getSliderColor(int sliderIndex) const
    {
        if (!controlsInitialized)
        {
            // Return default bank colors before initialization
            return sliderIndex < 4 ? juce::Colours::red : juce::Colours::blue;
        }
        
        if (sliderIndex < colorCombos.size())
        {
            switch (colorCombos[sliderIndex]->getSelectedId())
            {
                case 2: return juce::Colours::red;
                case 3: return juce::Colours::blue;
                case 4: return juce::Colours::green;
                case 5: return juce::Colours::yellow;
                case 6: return juce::Colours::purple;
                case 7: return juce::Colours::orange;
                case 8: return juce::Colours::cyan;
                case 9: return juce::Colours::white;
                default: // Default - use bank color
                    return sliderIndex < 4 ? juce::Colours::red : juce::Colours::blue;
            }
        }
        return juce::Colours::cyan; // Fallback
    }
    
    std::function<void()> onSettingsChanged;
    
private:
    // Member variables - declared once here
    bool controlsInitialized;
    juce::TextButton closeButton;
    juce::Label midiChannelLabel;
    juce::ComboBox midiChannelCombo;
    juce::Label bankALabel, bankBLabel;
    juce::OwnedArray<juce::Label> ccLabels;
    juce::OwnedArray<juce::TextEditor> ccInputs;
    juce::OwnedArray<juce::Label> rangeLabels;
    juce::OwnedArray<juce::TextEditor> minRangeInputs;
    juce::OwnedArray<juce::TextEditor> maxRangeInputs;
    juce::OwnedArray<juce::ComboBox> colorCombos;
    
    void initializeSliderControls()
    {
        DBG("initializeSliderControls START");
        
        // Create controls for all 8 sliders
        for (int i = 0; i < 8; ++i)
        {
            DBG("Creating controls for slider " + juce::String(i));
            
            // Slider label
            auto* label = new juce::Label();
            ccLabels.add(label);
            addAndMakeVisible(label);
            label->setText("Slider " + juce::String(i + 1) + ":", juce::dontSendNotification);
            
            // CC input (text input instead of combo)
            auto* ccInput = new juce::TextEditor();
            ccInputs.add(ccInput);
            addAndMakeVisible(ccInput);
            ccInput->setText(juce::String(i), juce::dontSendNotification);
            ccInput->setInputRestrictions(3, "0123456789"); // Max 3 digits, numbers only
            ccInput->setTooltip("MIDI CC number (0-127)");
            
            // Add validation callbacks for CC input
            ccInput->onReturnKey = [this, ccInput]() { validateCCInput(ccInput); };
            ccInput->onFocusLost = [this, ccInput]() { validateCCInput(ccInput); };
            
            // Min range input
            auto* minInput = new juce::TextEditor();
            minRangeInputs.add(minInput);
            addAndMakeVisible(minInput);
            minInput->setText("0");
            minInput->setInputRestrictions(0, "-0123456789");
            
            // Add validation callbacks for min input
            minInput->onReturnKey = [this, minInput]() { validateRangeInput(minInput); };
            minInput->onFocusLost = [this, minInput]() { validateRangeInput(minInput); };
            
            // Max range input
            auto* maxInput = new juce::TextEditor();
            maxRangeInputs.add(maxInput);
            addAndMakeVisible(maxInput);
            maxInput->setText("16383");
            maxInput->setInputRestrictions(0, "-0123456789");
            
            // Add validation callbacks for max input - FIXED: Added missing closing brace
            maxInput->onReturnKey = [this, maxInput]() { validateRangeInput(maxInput); };
            maxInput->onFocusLost = [this, maxInput]() { validateRangeInput(maxInput); };
            
            // Range label
            auto* rangeLabel = new juce::Label();
            rangeLabels.add(rangeLabel);
            addAndMakeVisible(rangeLabel);
            rangeLabel->setText("Range:", juce::dontSendNotification);
            
            // Color selector
            auto* colorCombo = new juce::ComboBox();
            colorCombos.add(colorCombo);
            addAndMakeVisible(colorCombo);
            
            // Add color options
            colorCombo->addItem("Default", 1);
            colorCombo->addItem("Red", 2);
            colorCombo->addItem("Blue", 3);
            colorCombo->addItem("Green", 4);
            colorCombo->addItem("Yellow", 5);
            colorCombo->addItem("Purple", 6);
            colorCombo->addItem("Orange", 7);
            colorCombo->addItem("Cyan", 8);
            colorCombo->addItem("White", 9);
            
            colorCombo->setSelectedId(1); // Default
            
            DBG("Controls for slider " + juce::String(i) + " created");
        }
        
        controlsInitialized = true;
        
        // Trigger a repaint and relayout
        resized();
        repaint();
        
        // Notify that settings might have changed (to update any dependent components)
        if (onSettingsChanged)
            onSettingsChanged();
        
        DBG("initializeSliderControls COMPLETE");
    }
    
    void validateCCInput(juce::TextEditor* input)
    {
        auto text = input->getText();
        if (text.isEmpty())
        {
            input->setText("0", juce::dontSendNotification);
            return;
        }
        
        int ccNumber = text.getIntValue();
        ccNumber = juce::jlimit(0, 127, ccNumber); // Clamp to valid MIDI CC range (0-127)
        input->setText(juce::String(ccNumber), juce::dontSendNotification);
        
        // Notify that settings changed
        if (onSettingsChanged)
            onSettingsChanged();
            
        DBG("CC input validated: " + juce::String(ccNumber));
    }
    
    void validateRangeInput(juce::TextEditor* input)
    {
        auto text = input->getText();
        if (text.isEmpty())
        {
            input->setText("0", juce::dontSendNotification);
            return;
        }
        
        double value = text.getDoubleValue();
        value = juce::jlimit(-999999.0, 999999.0, value); // Reasonable range limits
        input->setText(juce::String(value, 2), juce::dontSendNotification);
        
        // Notify that settings changed
        if (onSettingsChanged)
            onSettingsChanged();
            
        DBG("Range input validated: " + juce::String(value));
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
};

//End SettingsWindow.h
//=====================
