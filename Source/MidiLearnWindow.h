// MidiLearnWindow.h - MIDI Learn Mappings Display Window
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "Core/Midi7BitController.h" // For MidiTargetType
#include "UI/GlobalUIScale.h"

//==============================================================================
class MidiLearnWindow : public juce::Component
{
public:
    MidiLearnWindow()
    {
        // Title label
        addAndMakeVisible(titleLabel);
        titleLabel.setText("MIDI Learn Mappings", juce::dontSendNotification);
        titleLabel.setFont(GlobalUIScale::getInstance().getScaledFont(18.0f).boldened());
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        
        // MIDI Input Device Selection Section
        addAndMakeVisible(inputDeviceLabel);
        inputDeviceLabel.setText("MIDI Input Device:", juce::dontSendNotification);
        inputDeviceLabel.setFont(GlobalUIScale::getInstance().getScaledFont(14.0f).boldened());
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
        refreshDevicesButton.setLookAndFeel(&customButtonLookAndFeel);
        refreshDevicesButton.onClick = [this]() {
            refreshMidiDevices();
        };
        
        addAndMakeVisible(connectionStatusLabel);
        connectionStatusLabel.setText("No device selected", juce::dontSendNotification);
        connectionStatusLabel.setFont(GlobalUIScale::getInstance().getScaledFont(11.0f));
        connectionStatusLabel.setJustificationType(juce::Justification::centredLeft);
        connectionStatusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        connectionStatusLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        connectionStatusLabel.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
        
        // Initialize device list
        refreshMidiDevices();
        
        // Table headers
        addAndMakeVisible(sliderHeaderLabel);
        sliderHeaderLabel.setText("Target", juce::dontSendNotification);
        sliderHeaderLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f).boldened());
        sliderHeaderLabel.setJustificationType(juce::Justification::centred);
        sliderHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        sliderHeaderLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        
        addAndMakeVisible(channelHeaderLabel);
        channelHeaderLabel.setText("Input Channel", juce::dontSendNotification);
        channelHeaderLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f).boldened());
        channelHeaderLabel.setJustificationType(juce::Justification::centred);
        channelHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        channelHeaderLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        
        addAndMakeVisible(ccHeaderLabel);
        ccHeaderLabel.setText("Input CC", juce::dontSendNotification);
        ccHeaderLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f).boldened());
        ccHeaderLabel.setJustificationType(juce::Justification::centred);
        ccHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        ccHeaderLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        
        addAndMakeVisible(actionHeaderLabel);
        actionHeaderLabel.setText("Action", juce::dontSendNotification);
        actionHeaderLabel.setFont(GlobalUIScale::getInstance().getScaledFont(12.0f).boldened());
        actionHeaderLabel.setJustificationType(juce::Justification::centred);
        actionHeaderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
        actionHeaderLabel.setColour(juce::Label::backgroundColourId, BlueprintColors::background);
        
        // Clear All button
        addAndMakeVisible(clearAllButton);
        clearAllButton.setButtonText("Clear All");
        clearAllButton.setLookAndFeel(&customButtonLookAndFeel);
        clearAllButton.onClick = [this]() {
            clearAllMappings();
        };
        
        // Status label
        addAndMakeVisible(statusLabel);
        statusLabel.setFont(GlobalUIScale::getInstance().getScaledFont(11.0f));
        statusLabel.setJustificationType(juce::Justification::centred);
        statusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        updateStatusLabel();
    }
    
    ~MidiLearnWindow()
    {
        // Clean up custom look and feel
        refreshDevicesButton.setLookAndFeel(nullptr);
        clearAllButton.setLookAndFeel(nullptr);
        
        mappingRows.clear();
    }
    
    void paint(juce::Graphics& g) override
    {
        // Window background (slightly lighter than main background)
        g.fillAll(BlueprintColors::windowBackground);
        
        // Draw complete window outline - blueprint style
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        g.drawRect(getLocalBounds().toFloat(), 1.0f);
        
        // Header section background
        auto headerBounds = getHeaderBounds();
        g.setColour(BlueprintColors::sectionBackground);
        g.fillRect(headerBounds);
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        g.drawRect(headerBounds.toFloat(), 1.0f);
        
        // Table section background
        auto tableBounds = getTableBounds();
        int rowHeight = 25;
        int startY = headerBounds.getBottom();
        int tableHeight = mappingRows.size() * rowHeight;
        juce::Rectangle<int> tableAreaBounds(tableBounds.getX(), startY, tableBounds.getWidth(), tableHeight);
        
        g.setColour(BlueprintColors::sectionBackground);
        g.fillRect(tableAreaBounds);
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        g.drawRect(tableAreaBounds.toFloat(), 1.0f);
        
        // Table grid lines
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.6f));
        
        // Draw horizontal lines between rows
        for (int i = 0; i <= mappingRows.size(); ++i)
        {
            int y = startY + (i * rowHeight);
            g.drawHorizontalLine(y, 10, getWidth() - 10);
        }
        
        // Draw vertical column separators
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
        // Legacy method - delegates to new target-based method
        addMapping(MidiTargetType::SliderValue, sliderIndex, midiChannel, ccNumber);
    }
    
    void addMapping(MidiTargetType targetType, int sliderIndex, int midiChannel, int ccNumber)
    {
        // Remove existing mapping for this target/slider combination if it exists
        removeTargetMapping(targetType, sliderIndex);
        
        // Create new mapping row with target type
        auto* newRow = new MappingRow(targetType, sliderIndex, midiChannel, ccNumber);
        newRow->onRemoveClicked = [this, targetType, sliderIndex]() {
            removeTargetMapping(targetType, sliderIndex);
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
    
    void addConfigMapping(const juce::String& configId, const juce::String& configName, int midiChannel, int ccNumber)
    {
        // Remove existing mapping for this CC/Channel combination if it exists
        for (int i = mappingRows.size() - 1; i >= 0; --i)
        {
            if (mappingRows[i]->getCCNumber() == ccNumber && mappingRows[i]->getMidiChannel() == midiChannel)
            {
                mappingRows.remove(i);
                break;
            }
        }
        
        // Create new mapping row for automation config
        auto* newRow = new MappingRow(configId, configName, midiChannel, ccNumber);
        newRow->onRemoveClicked = [this, configId]() {
            removeConfigMapping(configId);
            if (onConfigMappingCleared)
                onConfigMappingCleared(configId);
        };
        
        mappingRows.add(newRow);
        addAndMakeVisible(newRow);
        
        layoutTableRows();
        updateStatusLabel();
        repaint();
        
        if (onConfigMappingAdded)
            onConfigMappingAdded(configId, midiChannel, ccNumber);
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
    
    void removeTargetMapping(MidiTargetType targetType, int sliderIndex)
    {
        for (int i = mappingRows.size() - 1; i >= 0; --i)
        {
            if (mappingRows[i]->getTargetType() == targetType && 
                mappingRows[i]->getSliderIndex() == sliderIndex)
            {
                mappingRows.remove(i);
                break;
            }
        }
        
        layoutTableRows();
        updateStatusLabel();
        repaint();
    }
    
    void removeConfigMapping(const juce::String& configId)
    {
        for (int i = mappingRows.size() - 1; i >= 0; --i)
        {
            if (mappingRows[i]->getConfigId() == configId)
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
    
    // Config mapping callbacks
    std::function<void(const juce::String& configId, int midiChannel, int ccNumber)> onConfigMappingAdded;
    std::function<void(const juce::String& configId)> onConfigMappingCleared;
    
    // MIDI device callbacks
    std::function<void(const juce::String& deviceName)> onMidiDeviceSelected;
    std::function<void()> onMidiDevicesRefreshed;
    
private:
    // Mapping row component
    class MappingRow : public juce::Component
    {
    public:
        // Legacy constructor for backward compatibility
        MappingRow(int sliderIndex, int midiChannel, int ccNumber)
            : MappingRow(MidiTargetType::SliderValue, sliderIndex, midiChannel, ccNumber)
        {
        }
        
        // New constructor with target type support
        MappingRow(MidiTargetType targetType, int sliderIndex, int midiChannel, int ccNumber)
            : targetType(targetType), sliderIndex(sliderIndex), midiChannel(midiChannel), ccNumber(ccNumber), isConfigMapping(false)
        {
            setupLabelsAndButton();
            
            // Target label (changed from simple slider number to target description)
            MidiTargetInfo targetInfo{targetType, sliderIndex, ccNumber, midiChannel};
            sliderLabel.setText(targetInfo.getDisplayName(), juce::dontSendNotification);
        }
        
        // Config mapping constructor
        MappingRow(const juce::String& configId, const juce::String& configName, int midiChannel, int ccNumber)
            : targetType(MidiTargetType::AutomationConfig), sliderIndex(-1), midiChannel(midiChannel), ccNumber(ccNumber), 
              isConfigMapping(true), configId(configId)
        {
            setupLabelsAndButton();
            
            // Config name as target
            sliderLabel.setText(configName, juce::dontSendNotification);
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
        MidiTargetType getTargetType() const { return targetType; }
        int getCCNumber() const { return ccNumber; }
        int getMidiChannel() const { return midiChannel; }
        juce::String getConfigId() const { return configId; }
        bool isConfig() const { return isConfigMapping; }
        
        std::function<void()> onRemoveClicked;
        
    private:
        MidiTargetType targetType;
        int sliderIndex;
        int midiChannel;
        int ccNumber;
        bool isConfigMapping;
        juce::String configId;
        
        juce::Label sliderLabel;
        juce::Label channelLabel;
        juce::Label ccLabel;
        juce::TextButton removeButton;
        
        void setupLabelsAndButton()
        {
            // Target/Config label
            addAndMakeVisible(sliderLabel);
            sliderLabel.setFont(GlobalUIScale::getInstance().getScaledFont(11.0f));
            sliderLabel.setJustificationType(juce::Justification::centred);
            sliderLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
            
            // Channel label
            addAndMakeVisible(channelLabel);
            channelLabel.setText(juce::String(midiChannel), juce::dontSendNotification);
            channelLabel.setFont(GlobalUIScale::getInstance().getScaledFont(11.0f));
            channelLabel.setJustificationType(juce::Justification::centred);
            channelLabel.setColour(juce::Label::textColourId, BlueprintColors::textPrimary);
            
            // CC label
            addAndMakeVisible(ccLabel);
            ccLabel.setText(juce::String(ccNumber), juce::dontSendNotification);
            ccLabel.setFont(GlobalUIScale::getInstance().getScaledFont(11.0f));
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
    CustomButtonLookAndFeel customButtonLookAndFeel;
    
    // Dynamic mapping rows
    juce::OwnedArray<MappingRow> mappingRows;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLearnWindow)
};