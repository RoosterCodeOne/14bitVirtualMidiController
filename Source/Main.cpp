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
        
        setResizable(true, false); // Allow width resize only, lock height
        
        // Calculate optimal height for threshold width (1280px)
        int optimalHeight = static_cast<int>(1280 / 1.6); // 800px height
        setResizeLimits(800, optimalHeight, 1800, optimalHeight);
        
        // Set up constrainer to lock height
        setConstrainer(&constrainer);
        constrainer.setMinimumHeight(optimalHeight);
        constrainer.setMaximumHeight(optimalHeight);
        
        centreWithSize(1000, optimalHeight);
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
