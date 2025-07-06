#include <JuceHeader.h>

// Forward declaration
class SettingsWindow;

//==============================================================================
class CustomSliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto value = slider.getValue();
        float norm = juce::jmap<float>(value, slider.getMinimum(), slider.getMaximum(), 0.0f, 1.0f);
        sliderPos = juce::jmap(norm, (float)y + height - 4.0f, (float)y + 4.0f);
        
        int sidePadding = 4;
        int topBottomPadding = 4;
        auto trackBounds = juce::Rectangle<float>((float)x + sidePadding, (float)y + topBottomPadding,
                                                 (float)width - 2 * sidePadding, (float)height - 2 * topBottomPadding);
        auto filled = trackBounds.withTop(sliderPos);

        // Background track
        g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
        g.fillRoundedRectangle(trackBounds, 8.0f);

        // Filled portion (value)
        g.setColour(juce::Colours::cyan);
        g.fillRoundedRectangle(filled, 8.0f);

        // Thumb
        auto thumbRadius = 8.0f;
        g.setColour(juce::Colours::white);
        g.fillEllipse(trackBounds.getCentreX() - thumbRadius, sliderPos - thumbRadius,
                     thumbRadius * 2.0f, thumbRadius * 2.0f);
        
        // Thumb border
        g.setColour(juce::Colours::darkgrey);
        g.drawEllipse(trackBounds.getCentreX() - thumbRadius, sliderPos - thumbRadius,
                     thumbRadius * 2.0f, thumbRadius * 2.0f, 1.5f);
    }

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;
        layout.sliderBounds = slider.getLocalBounds();
        return layout;
    }
};

//==============================================================================
class SettingsWindow : public juce::Component
{
public:
    SettingsWindow() : closeButton("X")
    {
        setSize(400, 500);  // Increased height for 8 sliders
        
        // Close button
        addAndMakeVisible(closeButton);
        closeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
        closeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        closeButton.onClick = [this]() { setVisible(false); };
        
        // MIDI Channel
        addAndMakeVisible(midiChannelLabel);
        midiChannelLabel.setText("MIDI Channel:", juce::dontSendNotification);
        addAndMakeVisible(midiChannelCombo);
        for (int i = 1; i <= 16; ++i)
            midiChannelCombo.addItem("Channel " + juce::String(i), i);
        midiChannelCombo.setSelectedId(1);
        
        // CC selectors for each slider
        for (int i = 0; i < 8; ++i)  // Now 8 sliders
        {
            auto* label = new juce::Label();
            ccLabels.add(label);
            addAndMakeVisible(label);
            label->setText("Slider " + juce::String(i + 1) + ":", juce::dontSendNotification);
            
            auto* combo = new juce::ComboBox();
            ccCombos.add(combo);
            addAndMakeVisible(combo);
            for (int cc = 0; cc < 128; ++cc)
                combo->addItem("CC " + juce::String(cc), cc + 1);
            combo->setSelectedId(i + 1); // Default to CC 0-7
        }
        
        // Bank labels
        addAndMakeVisible(bankALabel);
        bankALabel.setText("Bank A", juce::dontSendNotification);
        bankALabel.setColour(juce::Label::textColourId, juce::Colours::red);
        bankALabel.setFont(juce::Font(16.0f));
        
        addAndMakeVisible(bankBLabel);
        bankBLabel.setText("Bank B", juce::dontSendNotification);
        bankBLabel.setColour(juce::Label::textColourId, juce::Colours::blue);
        bankBLabel.setFont(juce::Font(16.0f));
    }
    
    void paint(juce::Graphics& g) override
    {
        // Semi-transparent background
        g.fillAll(juce::Colours::black.withAlpha(0.8f));
        
        // Settings panel
        auto bounds = getLocalBounds().reduced(50);
        g.setColour(juce::Colours::darkgrey);
        g.fillRoundedRectangle(bounds.toFloat(), 10.0f);
        
        g.setColour(juce::Colours::white);
        g.setFont(18.0f);
        g.drawText("Settings", bounds.removeFromTop(40), juce::Justification::centred);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(50);
        
        // Close button
        closeButton.setBounds(bounds.getRight() - 30, bounds.getY() + 5, 25, 25);
        
        bounds.removeFromTop(50); // Title space
        
        // MIDI Channel
        auto channelArea = bounds.removeFromTop(30);
        midiChannelLabel.setBounds(channelArea.removeFromLeft(100));
        midiChannelCombo.setBounds(channelArea.removeFromLeft(120));
        
        bounds.removeFromTop(15); // Spacing
        
        // Bank A label
        bankALabel.setBounds(bounds.removeFromTop(25));
        bounds.removeFromTop(5); // Small spacing
        
        // CC selectors for Bank A (sliders 0-3)
        for (int i = 0; i < 4; ++i)
        {
            auto ccArea = bounds.removeFromTop(25);
            if (ccLabels[i] != nullptr && ccCombos[i] != nullptr)
            {
                ccLabels[i]->setBounds(ccArea.removeFromLeft(100));
                ccCombos[i]->setBounds(ccArea.removeFromLeft(120));
            }
            bounds.removeFromTop(5); // Small spacing between items
        }
        
        bounds.removeFromTop(10); // Spacing between banks
        
        // Bank B label
        bankBLabel.setBounds(bounds.removeFromTop(25));
        bounds.removeFromTop(5); // Small spacing
        
        // CC selectors for Bank B (sliders 4-7)
        for (int i = 4; i < 8; ++i)
        {
            auto ccArea = bounds.removeFromTop(25);
            if (ccLabels[i] != nullptr && ccCombos[i] != nullptr)
            {
                ccLabels[i]->setBounds(ccArea.removeFromLeft(100));
                ccCombos[i]->setBounds(ccArea.removeFromLeft(120));
            }
            if (i < 7)
                bounds.removeFromTop(5); // Small spacing between items
        }
    }
    
    int getMidiChannel() const { return midiChannelCombo.getSelectedId(); }
    int getCCNumber(int sliderIndex) const
    {
        if (sliderIndex < ccCombos.size())
            return ccCombos[sliderIndex]->getSelectedId() - 1;
        return sliderIndex; // fallback
    }
    
    std::function<void()> onSettingsChanged;
    
private:
    juce::TextButton closeButton;
    juce::Label midiChannelLabel;
    juce::ComboBox midiChannelCombo;
    juce::Label bankALabel, bankBLabel;
    juce::OwnedArray<juce::Label> ccLabels;
    juce::OwnedArray<juce::ComboBox> ccCombos;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
};

//==============================================================================
class SimpleSliderControl : public juce::Component, public juce::Timer
{
public:
    SimpleSliderControl(int sliderIndex, std::function<void(int, int)> midiCallback)
        : index(sliderIndex), sendMidiCallback(midiCallback)
    {
        // Main slider with custom look
        addAndMakeVisible(mainSlider);
        mainSlider.setSliderStyle(juce::Slider::LinearVertical);
        mainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        mainSlider.setRange(0.0, 16383.0, 1.0);
        mainSlider.setLookAndFeel(&customLookAndFeel);
        
        // Safe callback - update label and send MIDI
        mainSlider.onValueChange = [this]() {
            if (!isAutomating) // Only update if not currently automating
            {
                int value = (int)mainSlider.getValue();
                currentValueLabel.setText(juce::String(value), juce::dontSendNotification);
                if (sendMidiCallback)
                    sendMidiCallback(index, value);
            }
        };
        
        // Manual override detection
        mainSlider.onDragStart = [this]() {
            if (isAutomating)
            {
                DBG("Manual override detected for slider " + juce::String(index));
                stopTimer();
                isAutomating = false;
                goButton.setButtonText("GO");
            }
        };
        
        // Current value label
        addAndMakeVisible(currentValueLabel);
        currentValueLabel.setText("0", juce::dontSendNotification);
        currentValueLabel.setJustificationType(juce::Justification::centred);
        currentValueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::black);
        currentValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        
        // Delay slider
        addAndMakeVisible(delaySlider);
        delaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
        delaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        delaySlider.setRange(0.0, 10.0, 0.1);
        delaySlider.setValue(0.0);
        delaySlider.setTextValueSuffix(" s");
        
        addAndMakeVisible(delayLabel);
        delayLabel.setText("Delay:", juce::dontSendNotification);
        delayLabel.attachToComponent(&delaySlider, true);
        
        // Attack slider
        addAndMakeVisible(attackSlider);
        attackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        attackSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
        attackSlider.setRange(0.0, 30.0, 0.1);
        attackSlider.setValue(1.0);
        attackSlider.setTextValueSuffix(" s");
        
        addAndMakeVisible(attackLabel);
        attackLabel.setText("Attack:", juce::dontSendNotification);
        attackLabel.attachToComponent(&attackSlider, true);
        
        // Target value input
        addAndMakeVisible(targetInput);
        targetInput.setInputRestrictions(5, "0123456789");
        targetInput.setText("8192", juce::dontSendNotification);
        targetInput.onReturnKey = [this]() { validateTargetValue(); };
        targetInput.onFocusLost = [this]() { validateTargetValue(); };
        
        addAndMakeVisible(targetLabel);
        targetLabel.setText("Target:", juce::dontSendNotification);
        targetLabel.attachToComponent(&targetInput, true);
        
        // GO button with automation functionality
        addAndMakeVisible(goButton);
        goButton.setButtonText("GO");
        goButton.onClick = [this]() { startAutomation(); };
    }
    
    ~SimpleSliderControl()
    {
        // CRITICAL: Stop timer before destruction
        stopTimer();
        // CRITICAL: Remove look and feel before destruction
        mainSlider.setLookAndFeel(nullptr);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        
        // Main slider takes most of the space
        mainSlider.setBounds(area.removeFromTop(area.getHeight() - 120));
        
        // Current value label
        currentValueLabel.setBounds(area.removeFromTop(25));
        
        // Delay slider
        auto delayArea = area.removeFromTop(25);
        delaySlider.setBounds(delayArea.removeFromLeft(delayArea.getWidth() - 50));
        
        // Attack slider
        auto attackArea = area.removeFromTop(25);
        attackSlider.setBounds(attackArea.removeFromLeft(attackArea.getWidth() - 50));
        
        // Target and GO button
        auto bottomArea = area.removeFromTop(25);
        goButton.setBounds(bottomArea.removeFromRight(40));
        bottomArea.removeFromRight(5); // spacing
        targetInput.setBounds(bottomArea.removeFromRight(60));
    }
    
    double getValue() const { return mainSlider.getValue(); }
    
    // Timer callback for automation
    void timerCallback() override
    {
        if (!isAutomating) return;
        
        double currentTime = juce::Time::getMillisecondCounterHiRes();
        double elapsed = (currentTime - automationStartTime) / 1000.0; // Convert to seconds
        
        if (elapsed < delayTime)
        {
            // Still in delay phase
            return;
        }
        
        double attackElapsed = elapsed - delayTime;
        
        if (attackElapsed >= attackTime)
        {
            // Animation complete
            mainSlider.setValue(targetValue, juce::dontSendNotification);
            currentValueLabel.setText(juce::String(targetValue), juce::dontSendNotification);
            if (sendMidiCallback)
                sendMidiCallback(index, targetValue);
            
            stopTimer();
            isAutomating = false;
            goButton.setButtonText("GO");
            
            DBG("Animation complete for slider " + juce::String(index) + " - reached target: " + juce::String(targetValue));
            return;
        }
        
        // Interpolate value
        double progress = attackElapsed / attackTime;
        double currentValue = startValue + (targetValue - startValue) * progress;
        
        mainSlider.setValue(currentValue, juce::dontSendNotification);
        currentValueLabel.setText(juce::String((int)currentValue), juce::dontSendNotification);
        if (sendMidiCallback)
            sendMidiCallback(index, (int)currentValue);
    }
    
private:
    void validateTargetValue()
    {
        auto text = targetInput.getText();
        if (text.isEmpty())
        {
            targetInput.setText("0", juce::dontSendNotification);
            return;
        }
        
        int value = text.getIntValue();
        value = juce::jlimit(0, 16383, value);
        targetInput.setText(juce::String(value), juce::dontSendNotification);
        
        DBG("Validated target value: " + juce::String(value) + " for slider " + juce::String(index));
    }
    
    void startAutomation()
    {
        if (isAutomating) return;
        
        validateTargetValue();
        auto targetText = targetInput.getText();
        if (targetText.isEmpty()) return;
        
        targetValue = targetText.getIntValue();
        startValue = mainSlider.getValue();
        
        if (std::abs(targetValue - startValue) < 1.0)
        {
            DBG("Already at target value for slider " + juce::String(index));
            return; // Already at target
        }
        
        delayTime = delaySlider.getValue();
        attackTime = attackSlider.getValue();
        
        DBG("Starting automation for slider " + juce::String(index) +
            " from " + juce::String(startValue) +
            " to " + juce::String(targetValue) +
            " (delay: " + juce::String(delayTime) + "s, attack: " + juce::String(attackTime) + "s)");
        
        if (attackTime <= 0.0)
        {
            // Instant change
            mainSlider.setValue(targetValue, juce::dontSendNotification);
            currentValueLabel.setText(juce::String(targetValue), juce::dontSendNotification);
            if (sendMidiCallback)
                sendMidiCallback(index, targetValue);
            DBG("Instant change for slider " + juce::String(index));
            return;
        }
        
        isAutomating = true;
        automationStartTime = juce::Time::getMillisecondCounterHiRes();
        goButton.setButtonText("...");
        
        startTimer(16); // ~60fps updates
    }
    
    int index;
    std::function<void(int, int)> sendMidiCallback;
    CustomSliderLookAndFeel customLookAndFeel;
    juce::Slider mainSlider;
    juce::Label currentValueLabel;
    juce::Slider delaySlider, attackSlider;
    juce::Label delayLabel, attackLabel, targetLabel;
    juce::TextEditor targetInput;
    juce::TextButton goButton;
    
    // Animation state
    bool isAutomating = false;
    double automationStartTime = 0.0;
    double startValue = 0.0;
    double targetValue = 0.0;
    double delayTime = 0.0;
    double attackTime = 0.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSliderControl)
};

//==============================================================================
class DebugMidiController : public juce::Component
{
public:
    DebugMidiController()
    {
        // Create slider controls with MIDI callback
        for (int i = 0; i < 8; ++i)  // Now 8 sliders total
        {
            auto* sliderControl = new SimpleSliderControl(i, [this](int sliderIndex, int value) {
                sendMidiCC(sliderIndex, value);
            });
            sliderControls.add(sliderControl);
            addAndMakeVisible(sliderControl);
        }
        
        // Bank buttons
        addAndMakeVisible(bankAButton);
        bankAButton.setButtonText("A");
        bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
        bankAButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        bankAButton.onClick = [this]() { setBank(0); };
        
        addAndMakeVisible(bankBButton);
        bankBButton.setButtonText("B");
        bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        bankBButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        bankBButton.onClick = [this]() { setBank(1); };
        
        // Settings button
        addAndMakeVisible(settingsButton);
        settingsButton.setButtonText("Settings");
        settingsButton.onClick = [this]() {
            addAndMakeVisible(settingsWindow);
            settingsWindow.setBounds(getLocalBounds());
            settingsWindow.toFront(true);
        };
        
        // Settings window
        addChildComponent(settingsWindow);
        
        // Initialize MIDI output
        initializeMidiOutput();
        
        // Set initial bank
        setBank(0);
    }
    
    ~DebugMidiController()
    {
        if (midiOutput)
            midiOutput->stopBackgroundThread();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        
        g.setColour(juce::Colours::white);
        g.setFont(24.0f);
        g.drawText("Debug MIDI Controller", 10, 10, getWidth() - 20, 40, juce::Justification::centred);
        
        // Show MIDI status - moved to top left with padding
        g.setFont(14.0f);
        juce::String status = midiOutput ? "MIDI: Connected" : "MIDI: Disconnected";
        g.drawText(status, 10, 10, 200, 20, juce::Justification::left);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        area.removeFromTop(80); // Title + status space
        
        // Settings button - positioned on left under MIDI status
        settingsButton.setBounds(10, 35, 100, 25);
        
        // Bank buttons - positioned on top right
        int buttonWidth = 40;
        int buttonHeight = 25;
        int rightMargin = 10;
        bankBButton.setBounds(getWidth() - rightMargin - buttonWidth, 10, buttonWidth, buttonHeight);
        bankAButton.setBounds(getWidth() - rightMargin - (buttonWidth * 2) - 5, 10, buttonWidth, buttonHeight);
        
        // Reserve space for button area
        area.removeFromTop(40);
        
        // Divide space between visible sliders (4 at a time)
        int sliderWidth = area.getWidth() / 4;
        for (int i = 0; i < 4; ++i)
        {
            int sliderIndex = currentBank * 4 + i;
            if (sliderIndex < sliderControls.size())
            {
                auto sliderBounds = area.removeFromLeft(sliderWidth);
                sliderBounds.reduce(10, 0); // Gap between sliders
                sliderControls[sliderIndex]->setBounds(sliderBounds);
            }
        }
        
        // Settings window
        if (settingsWindow.isVisible())
            settingsWindow.setBounds(getLocalBounds());
    }
    
private:
    void setBank(int bank)
    {
        currentBank = bank;
        
        // Update button colors
        if (bank == 0)
        {
            bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
        }
        else
        {
            bankAButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
            bankBButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
        }
        
        // Hide all sliders first
        for (auto* slider : sliderControls)
            slider->setVisible(false);
        
        // Show only the sliders for the current bank
        for (int i = 0; i < 4; ++i)
        {
            int sliderIndex = currentBank * 4 + i;
            if (sliderIndex < sliderControls.size())
                sliderControls[sliderIndex]->setVisible(true);
        }
        
        resized(); // Re-layout
    }
    
    void initializeMidiOutput()
    {
        auto midiDevices = juce::MidiOutput::getAvailableDevices();
        
        if (!midiDevices.isEmpty())
        {
            midiOutput = juce::MidiOutput::openDevice(midiDevices[0].identifier);
            DBG("Connected to MIDI device: " + midiDevices[0].name);
        }
        else
        {
            // Create virtual MIDI output
            midiOutput = juce::MidiOutput::createNewDevice("JUCE Virtual Controller");
            DBG("Created virtual MIDI device: JUCE Virtual Controller");
        }
        
        if (midiOutput)
            midiOutput->startBackgroundThread();
    }
    
    void sendMidiCC(int sliderIndex, int value14bit)
    {
        if (!midiOutput)
        {
            DBG("No MIDI output available");
            return;
        }
        
        // Use settings from settings window
        int midiChannel = settingsWindow.getMidiChannel();
        int ccNumber = settingsWindow.getCCNumber(sliderIndex);
        
        // Convert 14-bit value to MSB and LSB
        int msb = (value14bit >> 7) & 0x7F;  // Upper 7 bits
        int lsb = value14bit & 0x7F;         // Lower 7 bits
        
        // Send MSB (CC number)
        juce::MidiMessage msbMessage = juce::MidiMessage::controllerEvent(midiChannel, ccNumber, msb);
        midiOutput->sendMessageNow(msbMessage);
        
        // Send LSB (CC number + 32 for LSB)
        if (ccNumber < 96) // Only send LSB for CC 0-95 (96-127 are reserved)
        {
            juce::MidiMessage lsbMessage = juce::MidiMessage::controllerEvent(midiChannel, ccNumber + 32, lsb);
            midiOutput->sendMessageNow(lsbMessage);
        }
        
        DBG("MIDI CC sent - Slider: " + juce::String(sliderIndex) +
            ", Channel: " + juce::String(midiChannel) +
            ", CC: " + juce::String(ccNumber) +
            ", 14-bit value: " + juce::String(value14bit) +
            " (MSB: " + juce::String(msb) + ", LSB: " + juce::String(lsb) + ")");
    }
    
    juce::OwnedArray<SimpleSliderControl> sliderControls;
    juce::TextButton settingsButton;
    juce::TextButton bankAButton, bankBButton;
    SettingsWindow settingsWindow;
    std::unique_ptr<juce::MidiOutput> midiOutput;
    int currentBank = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugMidiController)
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
        setContentOwned(new DebugMidiController(), true);
        setResizable(true, true);
        setResizeLimits(800, 600, 1200, 800);
        centreWithSize(1000, 700);
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
    const juce::String getApplicationName() override { return "Debug MIDI Controller"; }
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
