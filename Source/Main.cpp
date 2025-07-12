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
        
        setResizable(true, true);
        setResizeLimits(800, 650, 1200, 850);
        
        centreWithSize(1000, 750);
        setVisible(true);
    }
    
    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
    
private:
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
