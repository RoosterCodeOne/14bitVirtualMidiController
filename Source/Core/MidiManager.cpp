#include "MidiManager.h"

//==============================================================================
MidiManager::MidiManager()
{
    DBG("MidiManager: Created");
}

MidiManager::~MidiManager()
{
    // Stop MIDI devices
    if (midiOutput)
        midiOutput->stopBackgroundThread();
        
    if (midiInput)
        midiInput->stop();
        
    DBG("MidiManager: Destroyed");
}

//==============================================================================
void MidiManager::initializeDevices()
{
    initializeOutput();
    initializeInput();
}

void MidiManager::initializeOutput()
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
        
    if (onDeviceConnectionChanged)
        onDeviceConnectionChanged();
}

void MidiManager::initializeInput()
{
    // Initialize MIDI input system
    // Actual device connection is handled by selectInputDevice()
    DBG("MIDI Input system initialized. Use device selection to connect.");
}

//==============================================================================
void MidiManager::selectInputDevice(const juce::String& deviceName)
{
    DBG("Selecting MIDI input device: " << deviceName);
    
    // Disconnect current device
    if (midiInput)
    {
        midiInput->stop();
        midiInput.reset();
        DBG("Disconnected previous MIDI input device");
    }
    
    // Handle "None" selection
    if (deviceName == "None (Disable MIDI Input)" || deviceName == "None")
    {
        selectedMidiDeviceName = "None";
        if (onConnectionStatusChanged)
            onConnectionStatusChanged("None", true);
        saveDevicePreference();
        DBG("MIDI input disabled");
        return;
    }
    
    // Find and connect to selected device
    auto midiDevices = juce::MidiInput::getAvailableDevices();
    bool deviceFound = false;
    
    for (const auto& device : midiDevices)
    {
        if (device.name == deviceName)
        {
            try
            {
                midiInput = juce::MidiInput::openDevice(device.identifier, this);
                
                if (midiInput)
                {
                    midiInput->start();
                    selectedMidiDeviceName = deviceName;
                    if (onConnectionStatusChanged)
                        onConnectionStatusChanged(deviceName, true);
                    saveDevicePreference();
                    deviceFound = true;
                    DBG("Successfully connected to MIDI device: " << deviceName);
                }
                else
                {
                    if (onConnectionStatusChanged)
                        onConnectionStatusChanged(deviceName, false);
                    DBG("Failed to open MIDI device: " << deviceName);
                }
            }
            catch (const std::exception& e)
            {
                DBG("Exception opening MIDI device " << deviceName << ": " << e.what());
                if (onConnectionStatusChanged)
                    onConnectionStatusChanged(deviceName, false);
            }
            break;
        }
    }
    
    if (!deviceFound)
    {
        DBG("MIDI device not found: " << deviceName);
        if (onConnectionStatusChanged)
            onConnectionStatusChanged(deviceName + " (Not Found)", false);
    }
    
    if (onDeviceConnectionChanged)
        onDeviceConnectionChanged();
}

//==============================================================================
bool MidiManager::isOutputConnected() const
{
    return midiOutput != nullptr;
}

bool MidiManager::isInputConnected() const
{
    return midiInput != nullptr;
}

juce::String MidiManager::getSelectedDeviceName() const
{
    return selectedMidiDeviceName;
}

//==============================================================================
void MidiManager::sendCC14Bit(int channel, int ccNumber, int value14bit)
{
    if (!midiOutput) return;
    
    // Convert 14-bit value to MSB and LSB
    int msb = (value14bit >> 7) & 0x7F;
    int lsb = value14bit & 0x7F;
    
    // Send MSB
    juce::MidiMessage msbMessage = juce::MidiMessage::controllerEvent(channel, ccNumber, msb);
    midiOutput->sendMessageNow(msbMessage);
    
    // Send LSB
    if (ccNumber < 96)
    {
        juce::MidiMessage lsbMessage = juce::MidiMessage::controllerEvent(channel, ccNumber + 32, lsb);
        midiOutput->sendMessageNow(lsbMessage);
    }
}

void MidiManager::sendCC14BitWithSlider(int sliderNumber, int channel, int ccNumber, int value14bit)
{
    if (!midiOutput) return;
    
    // Convert 14-bit value to MSB and LSB
    int msb = (value14bit >> 7) & 0x7F;
    int lsb = value14bit & 0x7F;
    
    // Send MSB
    juce::MidiMessage msbMessage = juce::MidiMessage::controllerEvent(channel, ccNumber, msb);
    midiOutput->sendMessageNow(msbMessage);
    
    // Send LSB
    if (ccNumber < 96)
    {
        juce::MidiMessage lsbMessage = juce::MidiMessage::controllerEvent(channel, ccNumber + 32, lsb);
        midiOutput->sendMessageNow(lsbMessage);
    }
    
    // Notify MIDI monitor of outgoing message
    if (onMidiSent)
    {
        juce::MessageManager::callAsync([this, sliderNumber, channel, ccNumber, msb, lsb, value14bit]() {
            onMidiSent(sliderNumber, channel, ccNumber, msb, lsb, value14bit);
        });
    }
}

// sendCC7BitWithSlider method removed as part of 7-bit mode cleanup
// System now always uses sendCC14BitWithSlider for universal compatibility

//==============================================================================
void MidiManager::saveDevicePreference()
{
    juce::File presetDir;
    if (getPresetDirectory)
        presetDir = getPresetDirectory();
    else
    {
        // Fallback to app data directory
        auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
        presetDir = appDataDir.getChildFile("VMC14_Presets");
        if (!presetDir.exists())
            presetDir.createDirectory();
    }
    
    auto prefFile = presetDir.getChildFile("midi_device_preference.txt");
    
    try
    {
        prefFile.replaceWithText(selectedMidiDeviceName);
        DBG("Saved MIDI device preference: " << selectedMidiDeviceName);
    }
    catch (const std::exception& e)
    {
        DBG("Failed to save MIDI device preference: " << e.what());
    }
}

void MidiManager::loadDevicePreference()
{
    juce::File presetDir;
    if (getPresetDirectory)
        presetDir = getPresetDirectory();
    else
    {
        // Fallback to app data directory
        auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
        presetDir = appDataDir.getChildFile("VMC14_Presets");
    }
    
    auto prefFile = presetDir.getChildFile("midi_device_preference.txt");
    
    if (prefFile.exists())
    {
        auto savedDevice = prefFile.loadFileAsString().trim();
        if (savedDevice.isNotEmpty())
        {
            selectedMidiDeviceName = savedDevice;
            DBG("Loaded MIDI device preference: " << savedDevice);
            
            // The parent controller will need to handle UI updates
            selectInputDevice(savedDevice);
        }
    }
    else
    {
        DBG("No saved MIDI device preference found");
    }
}

//==============================================================================
void MidiManager::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    if (message.isController())
    {
        int channel = message.getChannel();
        int ccNumber = message.getControllerNumber();
        int ccValue = message.getControllerValue();
        
        // Update activity indicator
        midiInputActivity = true;
        lastMidiInputTime = juce::Time::getMillisecondCounterHiRes();
        
        // Notify parent component about received MIDI
        if (onMidiReceived)
        {
            juce::MessageManager::callAsync([this, channel, ccNumber, ccValue]() {
                onMidiReceived(channel, ccNumber, ccValue);
            });
        }
        
        // Notify MIDI monitor about received MIDI
        if (onMidiReceiveForMonitor)
        {
            juce::MessageManager::callAsync([this, channel, ccNumber, ccValue]() {
                onMidiReceiveForMonitor(channel, ccNumber, ccValue, "External", -1);
            });
        }
    }
}