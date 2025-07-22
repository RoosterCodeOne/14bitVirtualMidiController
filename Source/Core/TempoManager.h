// TempoManager.h - BPM Management and Tempo Conversion Infrastructure
#pragma once
#include <JuceHeader.h>
#include <functional>

//==============================================================================
class TempoManager
{
public:
    TempoManager(double defaultBPM = 120.0)
        : internalBPM(defaultBPM), externalBPM(0.0), useExternalBPM(false)
    {
        DBG("TempoManager: Created with default BPM " << defaultBPM);
    }
    
    ~TempoManager()
    {
        DBG("TempoManager: Destroyed");
    }
    
    //==============================================================================
    // Core BPM Management
    
    // Get the currently active BPM (external overrides internal if available)
    double getCurrentBPM() const
    {
        return useExternalBPM ? externalBPM : internalBPM;
    }
    
    // Set internal BPM manually (user setting)
    void setInternalBPM(double bpm)
    {
        bpm = juce::jlimit(MIN_BPM, MAX_BPM, bpm);
        if (internalBPM != bpm)
        {
            internalBPM = bpm;
            DBG("TempoManager: Internal BPM set to " << bpm);
            
            // Notify listeners of BPM change
            if (onBPMChanged)
                onBPMChanged(getCurrentBPM());
        }
    }
    
    // Set external BPM from DAW sync (future use)
    void setExternalBPM(double bpm)
    {
        bool wasUsingExternal = useExternalBPM;
        double previousBPM = getCurrentBPM();
        
        if (bpm > 0.0)
        {
            externalBPM = juce::jlimit(MIN_BPM, MAX_BPM, bpm);
            useExternalBPM = true;
            DBG("TempoManager: External BPM set to " << externalBPM << " (DAW sync active)");
        }
        else
        {
            useExternalBPM = false;
            DBG("TempoManager: External BPM disabled, using internal BPM " << internalBPM);
        }
        
        // Notify if the active BPM changed
        if (getCurrentBPM() != previousBPM || wasUsingExternal != useExternalBPM)
        {
            if (onBPMChanged)
                onBPMChanged(getCurrentBPM());
            if (onSyncModeChanged)
                onSyncModeChanged(useExternalBPM);
        }
    }
    
    // Check if using external (DAW) sync
    bool isUsingExternalSync() const { return useExternalBPM; }
    
    // Get internal BPM value (regardless of sync mode)
    double getInternalBPM() const { return internalBPM; }
    
    //==============================================================================
    // Time Conversion Utilities
    
    // Convert beats to seconds using current BPM
    double beatsToSeconds(double beats) const
    {
        double bpm = getCurrentBPM();
        return beats * (60.0 / bpm); // 60 seconds per minute / BPM = seconds per beat
    }
    
    // Convert seconds to beats using current BPM
    double secondsToBeats(double seconds) const
    {
        double bpm = getCurrentBPM();
        return seconds / (60.0 / bpm); // seconds / seconds per beat = beats
    }
    
    // Convert musical notation to seconds (1/4 note, 1/2 note, etc.)
    double musicalNotationToSeconds(const juce::String& notation) const
    {
        // Parse musical notation like "1/4", "1/8", "2", etc.
        if (notation.contains("/"))
        {
            // Fractional notation (1/4, 1/8, etc.)
            auto parts = juce::StringArray::fromTokens(notation, "/", "");
            if (parts.size() == 2)
            {
                double numerator = parts[0].getDoubleValue();
                double denominator = parts[1].getDoubleValue();
                if (denominator > 0.0)
                {
                    double beatValue = numerator / denominator;
                    return beatsToSeconds(beatValue);
                }
            }
        }
        else
        {
            // Whole number notation (1, 2, 4, etc. = bars)
            double bars = notation.getDoubleValue();
            double beatsPerBar = 4.0; // Assuming 4/4 time signature
            return beatsToSeconds(bars * beatsPerBar);
        }
        
        return 0.0; // Invalid notation
    }
    
    // Convert seconds to closest musical notation
    juce::String secondsToMusicalNotation(double seconds) const
    {
        double beats = secondsToBeats(seconds);
        
        // Define musical values in ascending order
        static const struct {
            double beatValue;
            juce::String notation;
        } musicalValues[] = {
            {0.0625, "1/16"},  // 1/16 note
            {0.125, "1/8"},    // 1/8 note
            {0.25, "1/4"},     // 1/4 note
            {0.5, "1/2"},      // 1/2 note
            {1.0, "1"},        // 1 beat (whole note in some contexts)
            {2.0, "2"},        // 2 beats
            {4.0, "1 bar"},    // 1 bar (4 beats)
            {8.0, "2 bars"},   // 2 bars
            {16.0, "4 bars"},  // 4 bars
            {32.0, "8 bars"},  // 8 bars
            {64.0, "16 bars"}  // 16 bars
        };
        
        // Find the closest musical notation
        juce::String closestNotation = "1/16";
        double minDistance = std::abs(beats - 0.0625);
        
        for (const auto& value : musicalValues)
        {
            double distance = std::abs(beats - value.beatValue);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestNotation = value.notation;
            }
        }
        
        return closestNotation;
    }
    
    //==============================================================================
    // MIDI Clock Infrastructure (for future DAW sync)
    
    // Process incoming MIDI clock messages
    void processMIDIClock(const juce::MidiMessage& message)
    {
        if (message.isMidiClock())
        {
            // MIDI clock: 24 clocks per quarter note
            clockCount++;
            
            if (clockCount >= 24)
            {
                // One quarter note has passed
                clockCount = 0;
                auto now = juce::Time::getMillisecondCounterHiRes();
                
                if (lastClockTime > 0.0)
                {
                    double timeSinceLastBeat = (now - lastClockTime) / 1000.0; // Convert to seconds
                    if (timeSinceLastBeat > 0.0)
                    {
                        double calculatedBPM = 60.0 / timeSinceLastBeat; // 60 seconds per minute
                        
                        // Smooth BPM calculation to avoid jitter
                        if (externalBPMHistory.size() >= BPM_HISTORY_SIZE)
                            externalBPMHistory.removeFirst();
                        
                        externalBPMHistory.add(calculatedBPM);
                        
                        // Calculate average BPM from recent history
                        double averageBPM = 0.0;
                        for (auto bpm : externalBPMHistory)
                            averageBPM += bpm;
                        averageBPM /= externalBPMHistory.size();
                        
                        setExternalBPM(averageBPM);
                    }
                }
                
                lastClockTime = now;
            }
        }
        else if (message.isMidiStart() || message.isMidiContinue())
        {
            clockCount = 0;
            lastClockTime = 0.0;
            DBG("TempoManager: MIDI clock start/continue received");
        }
        else if (message.isMidiStop())
        {
            clockCount = 0;
            lastClockTime = 0.0;
            DBG("TempoManager: MIDI clock stop received");
            // Could choose to disable external sync here if desired
        }
    }
    
    //==============================================================================
    // Callbacks for BPM changes
    std::function<void(double)> onBPMChanged;           // Called when active BPM changes
    std::function<void(bool)> onSyncModeChanged;       // Called when sync mode changes
    
    //==============================================================================
    // Constants
    static constexpr double MIN_BPM = 60.0;
    static constexpr double MAX_BPM = 200.0;
    static constexpr double DEFAULT_BPM = 120.0;
    
private:
    double internalBPM;                 // User-set BPM
    double externalBPM;                 // DAW-derived BPM
    bool useExternalBPM;                // Whether to use external BPM
    
    // MIDI Clock tracking
    int clockCount = 0;
    double lastClockTime = 0.0;
    static constexpr int BPM_HISTORY_SIZE = 8;
    juce::Array<double> externalBPMHistory;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TempoManager)
};