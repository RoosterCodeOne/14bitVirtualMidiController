// DebugMidiController.h - TEST FULL SCALE (8 SLIDERS) ---------------
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
        DBG("DebugMidiController constructor START - FULL SCALE (8 SLIDERS)");
        
        // Create slider controls with MIDI callback - EXACTLY like original
        DBG("Creating 8 sliders");
        for (int i = 0; i < 8; ++i)  // 8 sliders total - same as original
        {
            DBG("Creating slider " + juce::String(i));
            auto* sliderControl = new SimpleSliderControl(i, [this](int sliderIndex, int value) {
                sendMidiCC(sliderIndex, value);
            });
            sliderControls.add(sliderControl);
            addAndMakeVisible(sliderControl);
            DBG("Slider " + juce::String(i) + " created successfully");
        }
        DBG("All 8 sliders created");
        
        // Bank buttons - EXACTLY like original
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
        DBG("Bank buttons created");
        
        // Settings button - EXACTLY like original
        addAndMakeVisible(settingsButton);
        settingsButton.setButtonText("Settings");
        settingsButton.onClick = [this]() {
            addAndMakeVisible(settingsWindow);
            settingsWindow.setBounds(getLocalBounds());
            settingsWindow.toFront(true);
        };
        DBG("Settings button created");
        
        // Settings window - EXACTLY like original
        addChildComponent(settingsWindow);
        settingsWindow.onSettingsChanged = [this]() { updateSliderSettings(); };
        DBG("Settings window created");
        
        // Initialize MIDI output - EXACTLY like original
        initializeMidiOutput();
        DBG("MIDI output initialized");
        
        // Set initial bank - EXACTLY like original
        DBG("Setting initial bank");
        setBank(0);
        DBG("Initial bank set");
        
        // Apply initial settings - EXACTLY like original
        DBG("Applying initial settings");
        updateSliderSettings();
        DBG("Initial settings applied");
        
        DBG("DebugMidiController constructor COMPLETE - FULL SCALE");
    }
    
    ~DebugMidiController()
    {
        DBG("DebugMidiController destructor START");
        
        if (midiOutput)
            midiOutput->stopBackgroundThread();
            
        DBG("DebugMidiController destructor COMPLETE");
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(24.0f));
        g.drawText("Debug MIDI Controller - FULL SCALE", 10, 10, getWidth() - 20, 40, juce::Justification::centred);
        
        // Show MIDI status
        g.setFont(juce::FontOptions(14.0f));
        juce::String status = midiOutput ? "MIDI: Connected" : "MIDI: Disconnected";
        g.drawText(status, 10, 10, 200, 20, juce::Justification::left);
    }
    
    void resized() override
    {
        DBG("DebugMidiController resized START - currentBank: " + juce::String(currentBank));
        
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
        
        // Divide space between visible sliders (4 at a time) - EXACTLY like original
        int sliderWidth = area.getWidth() / 4;
        for (int i = 0; i < 4; ++i)
        {
            int sliderIndex = currentBank * 4 + i;
            DBG("Positioning slider " + juce::String(sliderIndex));
            
            if (sliderIndex < sliderControls.size())
            {
                auto sliderBounds = area.removeFromLeft(sliderWidth);
                sliderBounds.reduce(10, 0); // Gap between sliders
                sliderControls[sliderIndex]->setBounds(sliderBounds);
                DBG("Slider " + juce::String(sliderIndex) + " positioned");
            }
        }
        
        // Settings window
        if (settingsWindow.isVisible())
            settingsWindow.setBounds(getLocalBounds());
            
        DBG("DebugMidiController resized COMPLETE");
    }
    
private:
    void updateSliderSettings()
    {
        DBG("updateSliderSettings START - " + juce::String(sliderControls.size()) + " sliders");
        
        for (int i = 0; i < sliderControls.size(); ++i)
        {
            DBG("Updating slider " + juce::String(i));
            
            // Update display range
            auto range = settingsWindow.getCustomRange(i);
            sliderControls[i]->setDisplayRange(range.first, range.second);
            
            // Update color
            auto color = settingsWindow.getSliderColor(i);
            sliderControls[i]->setSliderColor(color);
            
            DBG("Slider " + juce::String(i) + " updated");
        }
        
        DBG("updateSliderSettings COMPLETE");
    }
    
    void setBank(int bank)
    {
        DBG("setBank START - bank: " + juce::String(bank));
        
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
        
        DBG("setBank COMPLETE");
    }
    
    void initializeMidiOutput()
    {
        DBG("initializeMidiOutput START");
        
        auto midiDevices = juce::MidiOutput::getAvailableDevices();
        
        if (!midiDevices.isEmpty())
        {
            midiOutput = juce::MidiOutput::openDevice(midiDevices[0].identifier);
            DBG("Connected to MIDI device: " + midiDevices[0].name);
        }
        else
        {
            // Create virtual MIDI output
            midiOutput = juce::MidiOutput::createNewDevice("JUCE Virtual Controller");
            DBG("Created virtual MIDI device");
        }
        
        if (midiOutput)
            midiOutput->startBackgroundThread();
            
        DBG("initializeMidiOutput COMPLETE");
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
    }
    
    juce::OwnedArray<SimpleSliderControl> sliderControls;
    juce::TextButton settingsButton;
    juce::TextButton bankAButton, bankBButton;
    SettingsWindow settingsWindow;
    std::unique_ptr<juce::MidiOutput> midiOutput;
    int currentBank = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugMidiController)
};

//End DebugMidiController.h
//=====================
