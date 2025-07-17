#pragma once
#include <JuceHeader.h>
#include <functional>

//==============================================================================
/**
 * MidiManager handles all MIDI input/output operations for the Virtual MIDI Controller
 * Extracted from DebugMidiController to provide clean separation of concerns
 */
class MidiManager : public juce::MidiInputCallback
{
public:
    MidiManager();
    ~MidiManager();
    
    // Initialization
    void initializeDevices();
    
    // Device management
    void selectInputDevice(const juce::String& deviceName);
    bool isOutputConnected() const;
    bool isInputConnected() const;
    juce::String getSelectedDeviceName() const;
    
    // MIDI communication
    void sendCC14Bit(int channel, int ccNumber, int value14bit);
    
    // Device preferences
    void saveDevicePreference();
    void loadDevicePreference();
    
    // Callbacks for parent components
    std::function<void(int channel, int cc, int value)> onMidiReceived;
    std::function<void()> onDeviceConnectionChanged;
    std::function<void(const juce::String& deviceName, bool connected)> onConnectionStatusChanged;
    std::function<juce::File()> getPresetDirectory;
    
    // Activity indicator support
    bool getMidiInputActivity() const { return midiInputActivity; }
    double getLastMidiInputTime() const { return lastMidiInputTime; }
    void resetMidiInputActivity() { midiInputActivity = false; }
    
private:
    // MidiInputCallback implementation
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    
    // Internal device management
    void initializeOutput();
    void initializeInput();
    
    // Member variables
    std::unique_ptr<juce::MidiOutput> midiOutput;
    std::unique_ptr<juce::MidiInput> midiInput;
    juce::String selectedMidiDeviceName;
    
    // Activity tracking
    bool midiInputActivity = false;
    double lastMidiInputTime = 0.0;
    static constexpr double MIDI_INPUT_ACTIVITY_DURATION = 150.0; // milliseconds
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiManager)
};