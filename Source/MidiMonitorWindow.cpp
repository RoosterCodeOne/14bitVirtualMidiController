#include "MidiMonitorWindow.h"

//==============================================================================
class MidiMonitorContent : public juce::Component
{
public:
    MidiMonitorContent(MidiMonitorWindow& owner) : owner(owner)
    {
        setupComponents();
    }
    
    void paint(juce::Graphics& g) override
    {
        // Window background
        g.fillAll(BlueprintColors::windowBackground);
        
        // Draw separator line between columns
        auto bounds = getLocalBounds();
        int columnWidth = (bounds.getWidth() - 20) / 2;
        int separatorX = 10 + columnWidth;
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        g.drawVerticalLine(separatorX, 60, bounds.getHeight() - 50);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        area.reduce(10, 10);
        
        // Title
        titleLabel.setBounds(area.removeFromTop(25));
        area.removeFromTop(10);
        
        // Headers
        auto headerArea = area.removeFromTop(25);
        int columnWidth = headerArea.getWidth() / 2;
        outgoingHeaderLabel.setBounds(headerArea.removeFromLeft(columnWidth));
        incomingHeaderLabel.setBounds(headerArea);
        
        area.removeFromTop(5);
        
        // Bottom controls
        auto bottomArea = area.removeFromBottom(30);
        bottomArea.removeFromTop(5);
        
        auto buttonArea = bottomArea.removeFromBottom(25);
        clearButton.setBounds(buttonArea.removeFromLeft(80));
        buttonArea.removeFromLeft(10);
        pauseButton.setBounds(buttonArea.removeFromLeft(80));
        
        // Text areas (remaining space)
        columnWidth = area.getWidth() / 2;
        outgoingTextArea.setBounds(area.removeFromLeft(columnWidth - 2));
        area.removeFromLeft(4); // Gap for separator
        incomingTextArea.setBounds(area);
    }
    
private:
    MidiMonitorWindow& owner;
    
    // UI Components
    juce::Label titleLabel;
    juce::Label outgoingHeaderLabel;
    juce::Label incomingHeaderLabel;
    juce::TextEditor outgoingTextArea;
    juce::TextEditor incomingTextArea;
    juce::TextButton clearButton;
    juce::ToggleButton pauseButton;
    
    // Custom look and feel
    CustomButtonLookAndFeel customButtonLookAndFeel;
    
    void setupComponents()
    {
        // Title label
        addAndMakeVisible(titleLabel);
        titleLabel.setText("MIDI Monitor - Debug Information", juce::dontSendNotification);
        titleLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        
        // Column headers
        addAndMakeVisible(outgoingHeaderLabel);
        outgoingHeaderLabel.setText("OUTGOING", juce::dontSendNotification);
        outgoingHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        outgoingHeaderLabel.setJustificationType(juce::Justification::centred);
        outgoingHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::active);
        outgoingHeaderLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::panel);
        
        addAndMakeVisible(incomingHeaderLabel);
        incomingHeaderLabel.setText("INCOMING", juce::dontSendNotification);
        incomingHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        incomingHeaderLabel.setJustificationType(juce::Justification::centred);
        incomingHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::success);
        incomingHeaderLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::panel);
        
        // Text areas for message display
        addAndMakeVisible(outgoingTextArea);
        owner.setupTextEditor(outgoingTextArea);
        outgoingTextArea.setColour(juce::TextEditor::textColourId, BlueprintColors::active);
        
        addAndMakeVisible(incomingTextArea);
        owner.setupTextEditor(incomingTextArea);
        incomingTextArea.setColour(juce::TextEditor::textColourId, BlueprintColors::success);
        
        // Clear button
        addAndMakeVisible(clearButton);
        clearButton.setButtonText("Clear");
        clearButton.setLookAndFeel(&customButtonLookAndFeel);
        clearButton.onClick = [this]() { owner.clearMessages(); };
        
        // Pause/Resume toggle
        addAndMakeVisible(pauseButton);
        pauseButton.setButtonText("Pause");
        pauseButton.setLookAndFeel(&customButtonLookAndFeel);
        pauseButton.onClick = [this]() { 
            owner.setPaused(pauseButton.getToggleState());
            pauseButton.setButtonText(owner.isPaused() ? "Resume" : "Pause");
        };
    }
    
public:
    juce::TextEditor& getOutgoingTextArea() { return outgoingTextArea; }
    juce::TextEditor& getIncomingTextArea() { return incomingTextArea; }
    
    ~MidiMonitorContent()
    {
        clearButton.setLookAndFeel(nullptr);
        pauseButton.setLookAndFeel(nullptr);
    }
};

//==============================================================================
MidiMonitorWindow::MidiMonitorWindow()
    : DocumentWindow("MIDI Monitor - Debug Information", BlueprintColors::windowBackground, DocumentWindow::allButtons)
{
    // Create and set content component
    content = std::make_unique<MidiMonitorContent>(*this);
    setContentNonOwned(content.get(), true);
    
    // Set window properties
    setSize(600, 400);
    setResizable(true, true);
    setResizeLimits(400, 300, 1200, 800);
    
    // Start timers
    updateTimer = std::make_unique<UpdateTimer>(*this);
    updateTimer->startTimer(UPDATE_INTERVAL_MS);
    
    cleanupTimer = std::make_unique<CleanupTimer>(*this);
    cleanupTimer->startTimer(CLEANUP_INTERVAL_MS);
}

MidiMonitorWindow::~MidiMonitorWindow()
{
    // Stop timers
    if (updateTimer)
        updateTimer->stopTimer();
    if (cleanupTimer)
        cleanupTimer->stopTimer();
}

//==============================================================================
void MidiMonitorWindow::closeButtonPressed()
{
    setVisible(false);
}

//==============================================================================
void MidiMonitorWindow::logOutgoingMessage(int sliderNumber, int midiChannel, int ccNumber, 
                                          int msbValue, int lsbValue, int combinedValue)
{
    if (paused) return;
    
    std::lock_guard<std::mutex> lock(messagesMutex);
    
    // Create new message to get the key
    MidiMessage newMessage(sliderNumber, midiChannel, ccNumber, msbValue, lsbValue, combinedValue);
    
    // Check if message with this key already exists
    auto it = messageMap.find(newMessage.key);
    if (it != messageMap.end())
    {
        // Update existing message
        it->second.updateOutgoing(msbValue, lsbValue, combinedValue);
    }
    else
    {
        // Add new message, but check size limit first
        if (messageMap.size() >= MAX_MESSAGES)
        {
            // Remove oldest message (by timestamp)
            auto oldestIt = messageMap.begin();
            double oldestTime = oldestIt->second.timestamp;
            
            for (auto it = messageMap.begin(); it != messageMap.end(); ++it)
            {
                if (it->second.timestamp < oldestTime)
                {
                    oldestTime = it->second.timestamp;
                    oldestIt = it;
                }
            }
            messageMap.erase(oldestIt);
        }
        
        // Add new message
        messageMap[newMessage.key] = std::move(newMessage);
    }
}

void MidiMonitorWindow::logIncomingMessage(int midiChannel, int ccNumber, int value, 
                                          const juce::String& source, int targetSlider)
{
    if (paused) return;
    
    std::lock_guard<std::mutex> lock(messagesMutex);
    
    // Create new message to get the key
    MidiMessage newMessage(midiChannel, ccNumber, value, source, targetSlider);
    
    // Check if message with this key already exists
    auto it = messageMap.find(newMessage.key);
    if (it != messageMap.end())
    {
        // Update existing message
        it->second.updateIncoming(value);
    }
    else
    {
        // Add new message, but check size limit first
        if (messageMap.size() >= MAX_MESSAGES)
        {
            // Remove oldest message (by timestamp)
            auto oldestIt = messageMap.begin();
            double oldestTime = oldestIt->second.timestamp;
            
            for (auto it = messageMap.begin(); it != messageMap.end(); ++it)
            {
                if (it->second.timestamp < oldestTime)
                {
                    oldestTime = it->second.timestamp;
                    oldestIt = it;
                }
            }
            messageMap.erase(oldestIt);
        }
        
        // Add new message
        messageMap[newMessage.key] = std::move(newMessage);
    }
}

//==============================================================================
void MidiMonitorWindow::clearMessages()
{
    std::lock_guard<std::mutex> lock(messagesMutex);
    messageMap.clear();
    lastOutgoingDisplay.clear();
    lastIncomingDisplay.clear();
    
    // Clear text areas immediately
    if (content)
    {
        content->getOutgoingTextArea().clear();
        content->getIncomingTextArea().clear();
    }
}

void MidiMonitorWindow::setPaused(bool shouldPause)
{
    paused = shouldPause;
}

//==============================================================================
void MidiMonitorWindow::updateTextAreas()
{
    if (!content) return;
    
    std::lock_guard<std::mutex> lock(messagesMutex);
    
    // Generate current display strings
    juce::String outgoingText, incomingText;
    generateDisplayStrings(outgoingText, incomingText);
    
    // Only update if content has changed (optimization)
    auto& outgoingTextArea = content->getOutgoingTextArea();
    if (outgoingText != lastOutgoingDisplay)
    {
        outgoingTextArea.setText(outgoingText, false);
        outgoingTextArea.moveCaretToEnd();
        lastOutgoingDisplay = outgoingText;
    }
    
    auto& incomingTextArea = content->getIncomingTextArea();
    if (incomingText != lastIncomingDisplay)
    {
        incomingTextArea.setText(incomingText, false);
        incomingTextArea.moveCaretToEnd();
        lastIncomingDisplay = incomingText;
    }
}

void MidiMonitorWindow::cleanupOldMessages()
{
    if (paused) return;
    
    std::lock_guard<std::mutex> lock(messagesMutex);
    
    double currentTime = juce::Time::getMillisecondCounterHiRes();
    
    // Remove messages older than MESSAGE_LIFETIME_MS
    for (auto it = messageMap.begin(); it != messageMap.end();)
    {
        if (currentTime - it->second.timestamp > MESSAGE_LIFETIME_MS)
        {
            it = messageMap.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void MidiMonitorWindow::generateDisplayStrings(juce::String& outgoingText, juce::String& incomingText)
{
    // Separate messages by type and sort by timestamp (newest first)
    std::vector<const MidiMessage*> outgoingMessages;
    std::vector<const MidiMessage*> incomingMessages;
    
    for (const auto& pair : messageMap)
    {
        const auto& message = pair.second;
        if (message.isOutgoing)
            outgoingMessages.push_back(&message);
        else
            incomingMessages.push_back(&message);
    }
    
    // Sort by timestamp (newest first for better visibility)
    auto sortByTimestamp = [](const MidiMessage* a, const MidiMessage* b) {
        return a->timestamp > b->timestamp;
    };
    
    std::sort(outgoingMessages.begin(), outgoingMessages.end(), sortByTimestamp);
    std::sort(incomingMessages.begin(), incomingMessages.end(), sortByTimestamp);
    
    // Build display strings
    outgoingText.clear();
    for (const auto* message : outgoingMessages)
    {
        outgoingText += message->displayText + "\n";
    }
    
    incomingText.clear();
    for (const auto* message : incomingMessages)
    {
        incomingText += message->displayText + "\n";
    }
}

void MidiMonitorWindow::setupTextEditor(juce::TextEditor& editor)
{
    editor.setMultiLine(true);
    editor.setReturnKeyStartsNewLine(false);
    editor.setReadOnly(true);
    editor.setScrollbarsShown(true);
    editor.setCaretVisible(false);
    editor.setPopupMenuEnabled(true);
    
    // Terminal-style monospace font
    editor.setFont(juce::FontOptions("Courier New", 11.0f, juce::Font::plain));
    
    // Dark terminal colors
    editor.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::background);
    editor.setColour(juce::TextEditor::outlineColourId, BlueprintColors::blueprintLines.withAlpha(0.6f));
    editor.setColour(juce::TextEditor::focusedOutlineColourId, BlueprintColors::active);
    editor.setColour(juce::TextEditor::highlightColourId, BlueprintColors::active.withAlpha(0.3f));
    editor.setColour(juce::TextEditor::highlightedTextColourId, BlueprintColors::textPrimary);
    editor.setColour(juce::CaretComponent::caretColourId, BlueprintColors::active);
}