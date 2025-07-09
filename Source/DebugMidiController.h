// DebugMidiController.h - Production Version with Preset System
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "SimpleSliderControl.h"
#include "SettingsWindow.h"

//==============================================================================
class DebugMidiController : public juce::Component
{
public:
    DebugMidiController()
    {
        // Create 8 slider controls with MIDI callback
        for (int i = 0; i < 8; ++i)
        {
            auto* sliderControl = new SimpleSliderControl(i, [this](int sliderIndex, int value) {
                sendMidiCC(sliderIndex, value);
            });
            sliderControls.add(sliderControl);
            addAndMakeVisible(sliderControl);
        }
        
        // Bank buttons
        addAndMakeVisible(bankAButton);
        bankAButton.setButtonText("A");
        bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
        bankAButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        bankAButton.onClick = [this]() { setBank(0); };
        
        addAndMakeVisible(bankBButton);
        bankBButton.setButtonText("B");
        bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        bankBButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        bankBButton.onClick = [this]() { setBank(1); };
        
        // Settings button
        addAndMakeVisible(settingsButton);
        settingsButton.setButtonText("Settings");
        settingsButton.onClick = [this]() {
            addAndMakeVisible(settingsWindow);
            settingsWindow.setBounds(getLocalBounds());
            settingsWindow.toFront(true);
        };
        
        // Settings window
        addChildComponent(settingsWindow);
        settingsWindow.onSettingsChanged = [this]() { updateSliderSettings(); };
        settingsWindow.onPresetLoaded = [this](const ControllerPreset& preset) {
            applyPresetToSliders(preset);
        };
        
        // Initialize MIDI output
        initializeMidiOutput();
        
        // Set initial bank
        setBank(0);
        
        // Load auto-saved state
        loadAutoSavedState();
        
        // Apply initial settings
        updateSliderSettings();
    }
    
    ~DebugMidiController()
    {
        // Auto-save current state before destruction
        saveCurrentState();
        
        if (midiOutput)
            midiOutput->stopBackgroundThread();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(24.0f));
        g.drawText("14-Bit Virtual MIDI Controller", 10, 10, getWidth() - 20, 40, juce::Justification::centred);
        
        // Show MIDI status
        g.setFont(juce::FontOptions(14.0f));
        juce::String status = midiOutput ? "MIDI: Connected" : "MIDI: Disconnected";
        g.drawText(status, 10, 10, 200, 20, juce::Justification::left);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromTop(80); // Title + status space
        
        // Settings button - positioned on left under MIDI status
        settingsButton.setBounds(10, 35, 100, 25);
        
        // Bank buttons - positioned on top right
        int buttonWidth = 40;
        int buttonHeight = 25;
        int rightMargin = 10;
        bankBButton.setBounds(getWidth() - rightMargin - buttonWidth, 10, buttonWidth, buttonHeight);
        bankAButton.setBounds(getWidth() - rightMargin - (buttonWidth * 2) - 5, 10, buttonWidth, buttonHeight);
        
        // Reserve space for button area
        area.removeFromTop(40);
        
        // Divide space between visible sliders (4 at a time)
        int sliderWidth = area.getWidth() / 4;
        for (int i = 0; i < 4; ++i)
        {
            int sliderIndex = currentBank * 4 + i;
            if (sliderIndex < sliderControls.size())
            {
                auto sliderBounds = area.removeFromLeft(sliderWidth);
                sliderBounds.reduce(10, 0); // Gap between sliders
                sliderControls[sliderIndex]->setBounds(sliderBounds);
            }
        }
        
        // Settings window
        if (settingsWindow.isVisible())
            settingsWindow.setBounds(getLocalBounds());
    }
    
private:
    void updateSliderSettings()
    {
        for (int i = 0; i < sliderControls.size(); ++i)
        {
            // Update display range
            auto range = settingsWindow.getCustomRange(i);
            sliderControls[i]->setDisplayRange(range.first, range.second);
            
            // Update color
            auto color = settingsWindow.getSliderColor(i);
            sliderControls[i]->setSliderColor(color);
        }
        
        // Auto-save whenever settings change
        saveCurrentState();
    }
    
    void applyPresetToSliders(const ControllerPreset& preset)
    {
        // Apply slider values and lock states from preset
        for (int i = 0; i < juce::jmin(sliderControls.size(), preset.sliders.size()); ++i)
        {
            const auto& sliderPreset = preset.sliders[i];
            
            // Apply slider value (convert to internal MIDI range if needed)
            if (sliderControls[i])
            {
                sliderControls[i]->setValue(sliderPreset.currentValue);
                
                // Apply lock state
                if (sliderPreset.isLocked)
                    sliderControls[i]->setLocked(true);
                else
                    sliderControls[i]->setLocked(false);
                    
                // NEW: Apply delay and attack times
                sliderControls[i]->setDelayTime(sliderPreset.delayTime);
                sliderControls[i]->setAttackTime(sliderPreset.attackTime);
            }
        }
        
        // Update settings to reflect the new configuration
        updateSliderSettings();
    }
    
    void saveCurrentState()
    {
        auto preset = getCurrentControllerState();
        settingsWindow.getPresetManager().autoSaveCurrentState(preset);
    }
    
    void loadAutoSavedState()
    {
        auto preset = settingsWindow.getPresetManager().loadAutoSavedState();
        settingsWindow.applyPreset(preset);
        applyPresetToSliders(preset);
    }
    
    ControllerPreset getCurrentControllerState()
    {
        auto preset = settingsWindow.getCurrentPreset();
        
        // Add current slider values, lock states, and delay/attack times
        for (int i = 0; i < juce::jmin(sliderControls.size(), preset.sliders.size()); ++i)
        {
            if (sliderControls[i])
            {
                preset.sliders.getReference(i).currentValue = sliderControls[i]->getValue();
                preset.sliders.getReference(i).isLocked = sliderControls[i]->isLocked();
                preset.sliders.getReference(i).delayTime = sliderControls[i]->getDelayTime();    // NEW
                preset.sliders.getReference(i).attackTime = sliderControls[i]->getAttackTime();  // NEW
            }
        }
        
        return preset;
    }
    
    void setBank(int bank)
    {
        currentBank = bank;
        
        // Update button colors
        if (bank == 0)
        {
            bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        }
        else
        {
            bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
            bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
        }
        
        // Hide all sliders first
        for (auto* slider : sliderControls)
            slider->setVisible(false);
        
        // Show only the sliders for the current bank
        for (int i = 0; i < 4; ++i)
        {
            int sliderIndex = currentBank * 4 + i;
            if (sliderIndex < sliderControls.size())
                sliderControls[sliderIndex]->setVisible(true);
        }
        
        resized(); // Re-layout
    }
    
    void initializeMidiOutput()
    {
        auto midiDevices = juce::MidiOutput::getAvailableDevices();
        
        if (!midiDevices.isEmpty())
        {
            midiOutput = juce::MidiOutput::openDevice(midiDevices[0].identifier);
        }
        else
        {
            // Create virtual MIDI output
            midiOutput = juce::MidiOutput::createNewDevice("JUCE Virtual Controller");
        }
        
        if (midiOutput)
            midiOutput->startBackgroundThread();
    }
    
    void sendMidiCC(int sliderIndex, int value14bit)
    {
        if (!midiOutput) return;
        
        // Use settings from settings window
        int midiChannel = settingsWindow.getMidiChannel();
        int ccNumber = settingsWindow.getCCNumber(sliderIndex);
        
        // Convert 14-bit value to MSB and LSB
        int msb = (value14bit >> 7) & 0x7F;
        int lsb = value14bit & 0x7F;
        
        // Send MSB
        juce::MidiMessage msbMessage = juce::MidiMessage::controllerEvent(midiChannel, ccNumber, msb);
        midiOutput->sendMessageNow(msbMessage);
        
        // Send LSB
        if (ccNumber < 96)
        {
            juce::MidiMessage lsbMessage = juce::MidiMessage::controllerEvent(midiChannel, ccNumber + 32, lsb);
            midiOutput->sendMessageNow(lsbMessage);
        }
        
        // Trigger MIDI activity indicator AFTER successful MIDI send
        if (sliderIndex < sliderControls.size())
            sliderControls[sliderIndex]->triggerMidiActivity();
    }
    
    juce::OwnedArray<SimpleSliderControl> sliderControls;
    juce::TextButton settingsButton;
    juce::TextButton bankAButton, bankBButton;
    SettingsWindow settingsWindow;
    std::unique_ptr<juce::MidiOutput> midiOutput;
    int currentBank = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugMidiController)
};
