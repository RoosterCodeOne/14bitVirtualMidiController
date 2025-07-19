// MidiLearnWindow.h - MIDI Learn Mappings Display Window
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

//==============================================================================
class MidiLearnWindow : public juce::Component
{
public:
    MidiLearnWindow()
    {
        // Title label
        addAndMakeVisible(titleLabel);
        titleLabel.setText("MIDI Learn Mappings", juce::dontSendNotification);
        titleLabel.setFont(juce::FontOptions(18.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        
        // MIDI Input Device Selection Section
        addAndMakeVisible(inputDeviceLabel);
        inputDeviceLabel.setText("MIDI Input Device:", juce::dontSendNotification);
        inputDeviceLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        inputDeviceLabel.setJustificationType(juce::Justification::centredLeft);
        inputDeviceLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        
        addAndMakeVisible(inputDeviceCombo);
        inputDeviceCombo.setTextWhenNothingSelected("Select MIDI Input Device...");
        inputDeviceCombo.setColour(juce::ComboBox::backgroundColourId, BlueprintColors::background);
        inputDeviceCombo.setColour(juce::ComboBox::textColourId, BlueprintColors::textPrimary);
        inputDeviceCombo.setColour(juce::ComboBox::outlineColourId, BlueprintColors::blueprintLines);
        inputDeviceCombo.onChange = [this]() {
            if (onMidiDeviceSelected)
                onMidiDeviceSelected(inputDeviceCombo.getText());
        };
        
        addAndMakeVisible(refreshDevicesButton);
        refreshDevicesButton.setButtonText("Refresh");
        refreshDevicesButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::panel);
        refreshDevicesButton.setColour(juce::TextButton::textColourOffId, BlueprintColors::textPrimary);
        refreshDevicesButton.onClick = [this]() {
            refreshMidiDevices();
        };
        
        addAndMakeVisible(connectionStatusLabel);
        connectionStatusLabel.setText("No device selected", juce::dontSendNotification);
        connectionStatusLabel.setFont(juce::FontOptions(11.0f));
        connectionStatusLabel.setJustificationType(juce::Justification::centredLeft);
        connectionStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        
        // Initialize device list
        refreshMidiDevices();
        
        // Table headers
        addAndMakeVisible(sliderHeaderLabel);
        sliderHeaderLabel.setText("Slider", juce::dontSendNotification);
        sliderHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        sliderHeaderLabel.setJustificationType(juce::Justification::centred);
        sliderHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        sliderHeaderLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        
        addAndMakeVisible(channelHeaderLabel);
        channelHeaderLabel.setText("Input Channel", juce::dontSendNotification);
        channelHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        channelHeaderLabel.setJustificationType(juce::Justification::centred);
        channelHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        channelHeaderLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        
        addAndMakeVisible(ccHeaderLabel);
        ccHeaderLabel.setText("Input CC", juce::dontSendNotification);
        ccHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        ccHeaderLabel.setJustificationType(juce::Justification::centred);
        ccHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        ccHeaderLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        
        addAndMakeVisible(actionHeaderLabel);
        actionHeaderLabel.setText("Action", juce::dontSendNotification);
        actionHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        actionHeaderLabel.setJustificationType(juce::Justification::centred);
        actionHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        actionHeaderLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        
        // Clear All button
        addAndMakeVisible(clearAllButton);
        clearAllButton.setButtonText("Clear All");
        clearAllButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::panel);
        clearAllButton.setColour(juce::TextButton::textColourOffId, BlueprintColors::textPrimary);
        clearAllButton.onClick = [this]() {
            clearAllMappings();
        };
        
        // Status label
        addAndMakeVisible(statusLabel);
        statusLabel.setFont(juce::FontOptions(11.0f));
        statusLabel.setJustificationType(juce::Justification::centred);
        statusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        updateStatusLabel();
    }
    
    ~MidiLearnWindow()
    {
        mappingRows.clear();
    }
    
    void paint(juce::Graphics& g) override
    {
        // Background
        g.fillAll(BlueprintColors::background);
        
        // Header background
        auto headerBounds = getHeaderBounds();
        g.setColour(BlueprintColors::blueprintLines);
        g.fillRect(headerBounds);
        
        // Table grid lines
        g.setColour(BlueprintColors::blueprintLines);
        
        // Draw horizontal lines between rows
        int rowHeight = 25;
        int startY = headerBounds.getBottom();
        for (int i = 0; i <= mappingRows.size(); ++i)
        {
            int y = startY + (i * rowHeight);
            g.drawHorizontalLine(y, 10, getWidth() - 10);
        }
        
        // Draw vertical column separators
        auto tableBounds = getTableBounds();
        int colWidth = tableBounds.getWidth() / 4;
        for (int i = 1; i < 4; ++i)
        {
            int x = tableBounds.getX() + (i * colWidth);
            g.drawVerticalLine(x, headerBounds.getY(), startY + (mappingRows.size() * rowHeight));
        }
        
        // Draw table border
        g.drawRect(tableBounds.expanded(0, headerBounds.getHeight()), 1);
    }
    
    void resized() override
    {
        auto area = getLocalBounds();
        area.reduce(10, 10);
        
        // Title
        titleLabel.setBounds(area.removeFromTop(30));
        area.removeFromTop(10);
        
        // MIDI Input Device Selection Section
        inputDeviceLabel.setBounds(area.removeFromTop(20));
        area.removeFromTop(5);
        
        auto deviceRow = area.removeFromTop(25);
        inputDeviceCombo.setBounds(deviceRow.removeFromLeft(200));
        deviceRow.removeFromLeft(10);
        refreshDevicesButton.setBounds(deviceRow.removeFromLeft(70));
        
        area.removeFromTop(5);
        connectionStatusLabel.setBounds(area.removeFromTop(20));
        area.removeFromTop(15);
        
        // Table headers
        auto headerBounds = getHeaderBounds();
        int colWidth = headerBounds.getWidth() / 4;
        
        sliderHeaderLabel.setBounds(headerBounds.removeFromLeft(colWidth));
        channelHeaderLabel.setBounds(headerBounds.removeFromLeft(colWidth));
        ccHeaderLabel.setBounds(headerBounds.removeFromLeft(colWidth));
        actionHeaderLabel.setBounds(headerBounds);
        
        // Table rows
        layoutTableRows();
        
        // Bottom area
        auto bottomArea = area.removeFromBottom(60);
        bottomArea.removeFromTop(10);
        
        clearAllButton.setBounds(bottomArea.removeFromTop(25).reduced(100, 0));
        statusLabel.setBounds(bottomArea);
    }
    
    // Public methods
    void addMapping(int sliderIndex, int midiChannel, int ccNumber)
    {
        // Remove existing mapping for this slider if it exists
        removeMappingForSlider(sliderIndex);
        
        // Create new mapping row
        auto* newRow = new MappingRow(sliderIndex, midiChannel, ccNumber);
        newRow->onRemoveClicked = [this, sliderIndex]() {
            removeMappingForSlider(sliderIndex);
            if (onMappingCleared)
                onMappingCleared(sliderIndex);
        };
        
        mappingRows.add(newRow);
        addAndMakeVisible(newRow);
        
        layoutTableRows();
        updateStatusLabel();
        repaint();
        
        if (onMappingAdded)
            onMappingAdded(sliderIndex, midiChannel, ccNumber);
    }
    
    void removeMappingForSlider(int sliderIndex)
    {
        for (int i = mappingRows.size() - 1; i >= 0; --i)
        {
            if (mappingRows[i]->getSliderIndex() == sliderIndex)
            {
                mappingRows.remove(i);
                break;
            }
        }
        
        layoutTableRows();
        updateStatusLabel();
        repaint();
    }
    
    void clearAllMappings()
    {
        mappingRows.clear();
        updateStatusLabel();
        repaint();
        
        if (onAllMappingsCleared)
            onAllMappingsCleared();
    }
    
    void setConnectionStatus(const juce::String& deviceName, bool isConnected)
    {
        if (deviceName == "None")
        {
            connectionStatusLabel.setText("MIDI input disabled", juce::dontSendNotification);
            connectionStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        }
        else if (isConnected)
        {
            connectionStatusLabel.setText(deviceName + " (Connected)", juce::dontSendNotification);
            connectionStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::active);
        }
        else
        {
            connectionStatusLabel.setText(deviceName + " (Disconnected)", juce::dontSendNotification);
            connectionStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::warning);
        }
    }
    
    void setSelectedDevice(const juce::String& deviceName)
    {
        for (int i = 0; i < inputDeviceCombo.getNumItems(); ++i)
        {
            if (inputDeviceCombo.getItemText(i) == deviceName)
            {
                inputDeviceCombo.setSelectedId(inputDeviceCombo.getItemId(i), juce::dontSendNotification);
                break;
            }
        }
    }
    
    // Callbacks
    std::function<void(int sliderIndex, int midiChannel, int ccNumber)> onMappingAdded;
    std::function<void(int sliderIndex)> onMappingCleared;
    std::function<void()> onAllMappingsCleared;
    
    // MIDI device callbacks
    std::function<void(const juce::String& deviceName)> onMidiDeviceSelected;
    std::function<void()> onMidiDevicesRefreshed;
    
private:
    // Mapping row component
    class MappingRow : public juce::Component
    {
    public:
        MappingRow(int sliderIndex, int midiChannel, int ccNumber)
            : sliderIndex(sliderIndex), midiChannel(midiChannel), ccNumber(ccNumber)
        {
            // Slider label
            addAndMakeVisible(sliderLabel);
            sliderLabel.setText(juce::String(sliderIndex + 1), juce::dontSendNotification);
            sliderLabel.setFont(juce::FontOptions(11.0f));
            sliderLabel.setJustificationType(juce::Justification::centred);
            sliderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
            
            // Channel label
            addAndMakeVisible(channelLabel);
            channelLabel.setText(juce::String(midiChannel), juce::dontSendNotification);
            channelLabel.setFont(juce::FontOptions(11.0f));
            channelLabel.setJustificationType(juce::Justification::centred);
            channelLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
            
            // CC label
            addAndMakeVisible(ccLabel);
            ccLabel.setText(juce::String(ccNumber), juce::dontSendNotification);
            ccLabel.setFont(juce::FontOptions(11.0f));
            ccLabel.setJustificationType(juce::Justification::centred);
            ccLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
            
            // Remove button
            addAndMakeVisible(removeButton);
            removeButton.setButtonText("Remove");
            removeButton.setColour(juce::TextButton::buttonColourId, BlueprintColors::panel);
            removeButton.setColour(juce::TextButton::textColourOffId, BlueprintColors::textPrimary);
            removeButton.onClick = [this]() {
                if (onRemoveClicked)
                    onRemoveClicked();
            };
        }
        
        void resized() override
        {
            auto area = getLocalBounds();
            int colWidth = area.getWidth() / 4;
            
            sliderLabel.setBounds(area.removeFromLeft(colWidth));
            channelLabel.setBounds(area.removeFromLeft(colWidth));
            ccLabel.setBounds(area.removeFromLeft(colWidth));
            removeButton.setBounds(area.reduced(5, 2));
        }
        
        int getSliderIndex() const { return sliderIndex; }
        
        std::function<void()> onRemoveClicked;
        
    private:
        int sliderIndex;
        int midiChannel;
        int ccNumber;
        
        juce::Label sliderLabel;
        juce::Label channelLabel;
        juce::Label ccLabel;
        juce::TextButton removeButton;
    };
    
    juce::Rectangle<int> getHeaderBounds() const
    {
        auto area = getLocalBounds();
        area.reduce(10, 10);
        area.removeFromTop(40); // Title + gap
        area.removeFromTop(85); // MIDI device selection area (20+5+25+5+20+10)
        return area.removeFromTop(25);
    }
    
    juce::Rectangle<int> getTableBounds() const
    {
        auto area = getLocalBounds();
        area.reduce(10, 10);
        area.removeFromTop(40); // Title + gap
        area.removeFromTop(85); // MIDI device selection area
        area.removeFromBottom(60); // Bottom area
        return area;
    }
    
    void layoutTableRows()
    {
        auto tableBounds = getTableBounds();
        int rowHeight = 25;
        int startY = tableBounds.getY() + 25; // Below headers
        
        for (int i = 0; i < mappingRows.size(); ++i)
        {
            auto rowBounds = juce::Rectangle<int>(tableBounds.getX(), startY + (i * rowHeight), 
                                                tableBounds.getWidth(), rowHeight);
            mappingRows[i]->setBounds(rowBounds);
        }
    }
    
    void updateStatusLabel()
    {
        int count = mappingRows.size();
        juce::String text = juce::String(count) + " mapping" + (count == 1 ? "" : "s");
        statusLabel.setText(text, juce::dontSendNotification);
    }
    
    void refreshMidiDevices()
    {
        inputDeviceCombo.clear();
        
        // Add "None" option
        inputDeviceCombo.addItem("None (Disable MIDI Input)", 1);
        inputDeviceCombo.addSeparator();
        
        // Get available MIDI input devices
        auto midiInputs = juce::MidiInput::getAvailableDevices();
        
        if (midiInputs.isEmpty())
        {
            inputDeviceCombo.addItem("No MIDI devices found", 2);
            connectionStatusLabel.setText("No MIDI devices available", juce::dontSendNotification);
            connectionStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::warning);
        }
        else
        {
            for (int i = 0; i < midiInputs.size(); ++i)
            {
                auto deviceInfo = midiInputs[i];
                inputDeviceCombo.addItem(deviceInfo.name, i + 10); // Start IDs from 10
            }
            
            // Update status
            connectionStatusLabel.setText(juce::String(midiInputs.size()) + " device(s) found", 
                                        juce::dontSendNotification);
            connectionStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        }
        
        // Trigger callback to notify parent
        if (onMidiDevicesRefreshed)
            onMidiDevicesRefreshed();
    }
    
    // MIDI device selection UI components
    juce::Label inputDeviceLabel;
    juce::ComboBox inputDeviceCombo;
    juce::TextButton refreshDevicesButton;
    juce::Label connectionStatusLabel;
    
    // UI components
    juce::Label titleLabel;
    juce::Label sliderHeaderLabel;
    juce::Label channelHeaderLabel;
    juce::Label ccHeaderLabel;
    juce::Label actionHeaderLabel;
    juce::TextButton clearAllButton;
    juce::Label statusLabel;
    
    // Dynamic mapping rows
    juce::OwnedArray<MappingRow> mappingRows;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLearnWindow)
};