// MidiMonitorWindow.h - MIDI Monitor Debug Window
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include <deque>
#include <map>
#include <vector>
#include <algorithm>
#include <mutex>

// Forward declarations
class MidiMonitorContent;

//==============================================================================
struct MidiMessage
{
    juce::String key;           // Unique identifier for deduplication
    juce::String displayText;   // Formatted message text
    double timestamp;           // High-resolution timestamp for cleanup
    bool isOutgoing;           // True for outgoing, false for incoming
    
    // Raw MIDI data for reference
    int sliderNumber = -1;     // 1-16 for outgoing, -1 for external
    int midiChannel = 1;
    int ccNumber = 0;
    int msbValue = 0;
    int lsbValue = 0;
    int combinedValue = 0;
    juce::String source;
    
    MidiMessage() = default;
    
    // Constructor for outgoing 14-bit messages
    MidiMessage(int slider, int channel, int cc, int msb, int lsb, int combined)
        : timestamp(juce::Time::getMillisecondCounterHiRes()), isOutgoing(true),
          sliderNumber(slider), midiChannel(channel), ccNumber(cc), 
          msbValue(msb), lsbValue(lsb), combinedValue(combined),
          source("Slider " + juce::String(slider))
    {
        // Generate unique key for outgoing messages
        key = "OUT_S" + juce::String(slider) + "_Ch" + juce::String(channel) + "_CC" + juce::String(cc);
        
        // Generate display text
        juce::String timeStr = formatTimestamp(timestamp);
        displayText = timeStr + " " + source + " → Ch:" + juce::String(channel) + 
                     " CC:" + juce::String(cc) + " MSB:" + juce::String(msb) + 
                     " LSB:" + juce::String(lsb) + " (Value:" + juce::String(combined) + ")";
    }
    
    // Constructor for incoming 7-bit messages  
    MidiMessage(int channel, int cc, int value, const juce::String& sourceText, int targetSlider = -1)
        : timestamp(juce::Time::getMillisecondCounterHiRes()), isOutgoing(false),
          sliderNumber(targetSlider), midiChannel(channel), ccNumber(cc), 
          msbValue(0), lsbValue(0), combinedValue(value), source(sourceText)
    {
        // Generate unique key for incoming messages
        if (targetSlider >= 1)
            key = "IN_Ch" + juce::String(channel) + "_CC" + juce::String(cc) + "_S" + juce::String(targetSlider);
        else
            key = "IN_Ch" + juce::String(channel) + "_CC" + juce::String(cc) + "_EXT";
        
        // Generate display text
        juce::String timeStr = formatTimestamp(timestamp);
        displayText = timeStr + " " + sourceText + " → Ch:" + juce::String(channel) + 
                     " CC:" + juce::String(cc) + " Val:" + juce::String(value);
        
        if (targetSlider >= 1)
            displayText += " (Slider " + juce::String(targetSlider) + ")";
    }
    
    // Update message with new values (maintains key, updates timestamp and display)
    void updateOutgoing(int msb, int lsb, int combined)
    {
        timestamp = juce::Time::getMillisecondCounterHiRes();
        msbValue = msb;
        lsbValue = lsb;
        combinedValue = combined;
        
        juce::String timeStr = formatTimestamp(timestamp);
        displayText = timeStr + " " + source + " → Ch:" + juce::String(midiChannel) + 
                     " CC:" + juce::String(ccNumber) + " MSB:" + juce::String(msb) + 
                     " LSB:" + juce::String(lsb) + " (Value:" + juce::String(combined) + ")";
    }
    
    void updateIncoming(int value)
    {
        timestamp = juce::Time::getMillisecondCounterHiRes();
        combinedValue = value;
        
        juce::String timeStr = formatTimestamp(timestamp);
        displayText = timeStr + " " + source + " → Ch:" + juce::String(midiChannel) + 
                     " CC:" + juce::String(ccNumber) + " Val:" + juce::String(value);
        
        if (sliderNumber >= 1)
            displayText += " (Slider " + juce::String(sliderNumber) + ")";
    }
    
private:
    static juce::String formatTimestamp(double timestampMs)
    {
        juce::Time time(static_cast<juce::int64>(timestampMs));
        return "[" + time.formatted("%H:%M:%S.") + 
               juce::String(static_cast<int>(timestampMs) % 1000).paddedLeft('0', 3) + "]";
    }
};

//==============================================================================
class MidiMonitorWindow : public juce::DocumentWindow
{
public:
    MidiMonitorWindow();
    ~MidiMonitorWindow();
    
    void closeButtonPressed() override;
    
    // Message logging methods (thread-safe)
    void logOutgoingMessage(int sliderNumber, int midiChannel, int ccNumber, int msbValue, int lsbValue, int combinedValue);
    void logIncomingMessage(int midiChannel, int ccNumber, int value, const juce::String& source, int targetSlider = -1);
    
    // Control methods
    void clearMessages();
    void setPaused(bool shouldPause);
    bool isPaused() const { return paused; }
    
    // Public method for content component access
    void setupTextEditor(juce::TextEditor& editor);

private:
    // Content component
    std::unique_ptr<MidiMonitorContent> content;
    
    // Message storage (thread-safe) - using map for deduplication
    std::map<juce::String, MidiMessage> messageMap;
    std::mutex messagesMutex;
    
    // Display state tracking
    juce::String lastOutgoingDisplay;
    juce::String lastIncomingDisplay;
    
    // Configuration
    static constexpr int MAX_MESSAGES = 50;  // Reduced from 1000 - unique message types only
    static constexpr double MESSAGE_LIFETIME_MS = 5000.0;  // 5 seconds
    static constexpr int UPDATE_INTERVAL_MS = 50;   // UI updates every 50ms
    static constexpr int CLEANUP_INTERVAL_MS = 1000; // Cleanup every 1 second
    bool paused = false;
    
    // Timers
    std::unique_ptr<juce::Timer> updateTimer;
    std::unique_ptr<juce::Timer> cleanupTimer;
    
    // Helper methods
    void updateTextAreas();
    void cleanupOldMessages();
    void generateDisplayStrings(juce::String& outgoingText, juce::String& incomingText);
    
    // Timer classes
    class UpdateTimer : public juce::Timer
    {
    public:
        UpdateTimer(MidiMonitorWindow& owner) : owner(owner) {}
        void timerCallback() override { owner.updateTextAreas(); }
    private:
        MidiMonitorWindow& owner;
    };
    
    class CleanupTimer : public juce::Timer
    {
    public:
        CleanupTimer(MidiMonitorWindow& owner) : owner(owner) {}
        void timerCallback() override { owner.cleanupOldMessages(); }
    private:
        MidiMonitorWindow& owner;
    };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMonitorWindow)
};
