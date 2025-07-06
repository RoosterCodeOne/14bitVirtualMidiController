#include <JuceHeader.h>

//==============================================================================
class MidiSlider : public juce::Component
{
public:
    MidiSlider() : slider(juce::Slider::LinearVertical, juce::Slider::NoTextBox)
    {
        addAndMakeVisible(slider);
        slider.setRange(0.0, 16383.0, 1.0); // 14-bit range (0-16383)
        slider.onValueChange = [this]() {
            if (onValueChanged)
                onValueChanged(static_cast<int>(slider.getValue()));
        };
        
        // Style the slider to look like a tall rounded rectangle
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        slider.setColour(juce::Slider::trackColourId, juce::Colours::darkgrey);
        slider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);
    }
    
    void resized() override
    {
        slider.setBounds(getLocalBounds());
    }
    
    void setValue(double value)
    {
        slider.setValue(value, juce::dontSendNotification);
    }
    
    double getValue() const
    {
        return slider.getValue();
    }
    
    std::function<void(int)> onValueChanged;
    
private:
    juce::Slider slider;
};

//==============================================================================
class MidiControllerComponent : public juce::Component,
                               public juce::ComboBox::Listener
{
public:
    MidiControllerComponent()
    {
        // Create 8 sliders
        for (int i = 0; i < 8; ++i)
        {
            auto* slider = new MidiSlider();
            sliders.add(slider);
            addAndMakeVisible(slider);
            
            // Set up callback for each slider
            slider->onValueChanged = [this, i](int value) {
                sendMidiCC(i, value);
            };
        }
        
        // MIDI Channel selector
        addAndMakeVisible(midiChannelCombo);
        midiChannelCombo.addListener(this);
        for (int i = 1; i <= 16; ++i)
            midiChannelCombo.addItem("Channel " + juce::String(i), i);
        midiChannelCombo.setSelectedId(1); // Default to channel 1
        
        addAndMakeVisible(midiChannelLabel);
        midiChannelLabel.setText("MIDI Channel:", juce::dontSendNotification);
        midiChannelLabel.attachToComponent(&midiChannelCombo, true);
        
        // CC Number selectors for each slider
        for (int i = 0; i < 8; ++i)
        {
            auto* combo = new juce::ComboBox();
            ccCombos.add(combo);
            addAndMakeVisible(combo);
            combo->addListener(this);
            
            // Populate with common CC numbers
            for (int cc = 0; cc < 128; ++cc)
                combo->addItem("CC " + juce::String(cc), cc + 1);
            
            combo->setSelectedId(i + 1); // Default to CC 0-7
            
            auto* label = new juce::Label();
            ccLabels.add(label);
            addAndMakeVisible(label);
            label->setText("CC" + juce::String(i + 1) + ":", juce::dontSendNotification);
            label->attachToComponent(combo, true);
        }
        
        // Initialize MIDI output
        initializeMidiOutput();
    }
    
    ~MidiControllerComponent()
    {
        if (midiOutput)
            midiOutput->stopBackgroundThread();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);
        
        // Draw title
        g.setColour(juce::Colours::white);
        g.setFont(20.0f);
        g.drawText("Virtual MIDI Controller", 10, 10, getWidth() - 20, 30, juce::Justification::centred);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromTop(50); // Space for title
        
        // MIDI Channel selector at top
        auto topArea = area.removeFromTop(40);
        midiChannelCombo.setBounds(topArea.removeFromRight(120));
        
        // CC selectors area
        auto ccArea = area.removeFromTop(40);
        int ccWidth = ccArea.getWidth() / 8;
        for (int i = 0; i < 8; ++i)
        {
            auto ccBounds = ccArea.removeFromLeft(ccWidth);
            ccCombos[i]->setBounds(ccBounds.removeFromRight(60));
        }
        
        // Sliders area
        area.removeFromTop(10); // Small gap
        int sliderWidth = area.getWidth() / 8;
        for (int i = 0; i < 8; ++i)
        {
            auto sliderBounds = area.removeFromLeft(sliderWidth);
            sliderBounds.reduce(5, 0); // Small gap between sliders
            sliders[i]->setBounds(sliderBounds);
        }
    }
    
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override
    {
        // Handle combo box changes if needed
        // The actual CC numbers are read when sending MIDI
    }
    
private:
    void initializeMidiOutput()
    {
        auto midiDevices = juce::MidiOutput::getAvailableDevices();
        
        if (!midiDevices.isEmpty())
        {
            midiOutput = juce::MidiOutput::openDevice(midiDevices[0].identifier);
            if (midiOutput)
                midiOutput->startBackgroundThread();
        }
        else
        {
            // Create virtual MIDI output
            midiOutput = juce::MidiOutput::createNewDevice("JUCE Virtual Controller");
            if (midiOutput)
                midiOutput->startBackgroundThread();
        }
    }
    
    void sendMidiCC(int sliderIndex, int value14bit)
    {
        if (!midiOutput)
            return;
            
        int midiChannel = midiChannelCombo.getSelectedId() - 1; // Convert to 0-based
        int ccNumber = ccCombos[sliderIndex]->getSelectedId() - 1; // Convert to 0-based
        
        // Convert 14-bit value to MSB and LSB
        int msb = (value14bit >> 7) & 0x7F;  // Upper 7 bits
        int lsb = value14bit & 0x7F;         // Lower 7 bits
        
        // Send MSB (CC number)
        juce::MidiMessage msbMessage = juce::MidiMessage::controllerEvent(midiChannel + 1, ccNumber, msb);
        midiOutput->sendMessageNow(msbMessage);
        
        // Send LSB (CC number + 32 for LSB)
        if (ccNumber < 96) // Only send LSB for CC 0-95 (96-127 are reserved)
        {
            juce::MidiMessage lsbMessage = juce::MidiMessage::controllerEvent(midiChannel + 1, ccNumber + 32, lsb);
            midiOutput->sendMessageNow(lsbMessage);
        }
    }
    
    juce::OwnedArray<MidiSlider> sliders;
    juce::OwnedArray<juce::ComboBox> ccCombos;
    juce::OwnedArray<juce::Label> ccLabels;
    
    juce::ComboBox midiChannelCombo;
    juce::Label midiChannelLabel;
    
    std::unique_ptr<juce::MidiOutput> midiOutput;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiControllerComponent)
};

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
        setContentOwned(new MidiControllerComponent(), true);
        setResizable(true, true);
        setResizeLimits(800, 600, 1200, 800);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
        
        // Note: MIDI permissions are handled automatically by JUCE on modern macOS
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
    MidiControllerApplication() {}
    
    const juce::String getApplicationName() override { return "JUCE MIDI Controller"; }
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

// Note: ProjectInfo is automatically generated by Projucer, so we don't need to define it manually
