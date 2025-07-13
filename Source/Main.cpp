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
        int optimalHeight = 650;
        int minWidth = 450; // 4×100 + 3×10 + 20 = 450px
        int maxWidth = 990; // 8×100 + 7×10 + 20 + 100 = 990px
        
        // Set resizable before setting any constraints
        setResizable(true, true);
        
        // Set up constrainer with proper limits
        constrainer.setMinimumWidth(minWidth);
        constrainer.setMaximumWidth(maxWidth);
        constrainer.setMinimumHeight(optimalHeight);
        constrainer.setMaximumHeight(optimalHeight);
        
        // Apply constrainer
        setConstrainer(&constrainer);
        
        // Start with minimum width to show 4-slider mode by default
        centreWithSize(minWidth, optimalHeight);
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
