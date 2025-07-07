// Main.cpp - DEBUG VERSION ---------------
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
        DBG("MainWindow constructor START");
        
        setUsingNativeTitleBar(true);
        DBG("Set native title bar");
        
        // Try creating the controller first
        DBG("About to create DebugMidiController");
        auto* controller = new DebugMidiController();
        DBG("DebugMidiController created successfully");
        
        setContentOwned(controller, true);
        DBG("Content set");
        
        setResizable(true, true);
        setResizeLimits(800, 600, 1200, 800);
        DBG("Resize limits set");
        
        centreWithSize(1000, 700);
        DBG("Window centered");
        
        setVisible(true);
        DBG("Window made visible");
        
        DBG("MainWindow constructor COMPLETE");
    }
    
    void closeButtonPressed() override
    {
        DBG("Close button pressed");
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

//==============================================================================
class MidiControllerApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Debug MIDI Controller"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }
    
    void initialise(const juce::String& commandLine) override
    {
        DBG("Application initialise START");
        
        try
        {
            DBG("About to create MainWindow");
            mainWindow.reset(new MainWindow(getApplicationName()));
            DBG("MainWindow created successfully");
        }
        catch (const std::exception& e)
        {
            DBG("Exception caught: " + juce::String(e.what()));
        }
        catch (...)
        {
            DBG("Unknown exception caught");
        }
        
        DBG("Application initialise COMPLETE");
    }
    
    void shutdown() override
    {
        DBG("Application shutdown START");
        mainWindow = nullptr;
        DBG("Application shutdown COMPLETE");
    }
    
    void systemRequestedQuit() override
    {
        DBG("System requested quit");
        quit();
    }
    
private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION(MidiControllerApplication)

//End Main.cpp
//=====================
