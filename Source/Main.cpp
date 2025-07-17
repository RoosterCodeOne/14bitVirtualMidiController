// Main.cpp - Production Version
#include <JuceHeader.h>
#include "DebugMidiController.h"

//==============================================================================
class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name)
        : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel()
                                .findColour(juce::ResizableWindow::backgroundColourId),
                         DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        
        auto* controller = new DebugMidiController();
        setContentOwned(controller, true);
        
        // Calculate dimensions
        int optimalHeight = 660; // Further reduced for compact layout (685 - 25px from topAreaHeight reduction)
        int defaultWidth = 490; // 4-slider mode by default
        
        // Set resizable before setting any constraints
        setResizable(true, true);
        
        // Set up constrainer with initial fixed constraints
        // Note: DebugMidiController will update these constraints after initialization
        constrainer.setMinimumWidth(defaultWidth);
        constrainer.setMaximumWidth(defaultWidth);
        constrainer.setMinimumHeight(optimalHeight);
        constrainer.setMaximumHeight(optimalHeight);
        
        // Apply constrainer
        setConstrainer(&constrainer);
        
        // Start with default width to show 4-slider mode by default
        centreWithSize(defaultWidth, optimalHeight);
        setVisible(true);
    }
    
    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
    
private:
    juce::ComponentBoundsConstrainer constrainer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

//==============================================================================
class MidiControllerApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "14-Bit Virtual MIDI Controller"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }
    
    void initialise(const juce::String& commandLine) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }
    
    void shutdown() override
    {
        mainWindow = nullptr;
    }
    
    void systemRequestedQuit() override
    {
        quit();
    }
    
private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION(MidiControllerApplication)
