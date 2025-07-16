// MidiLearnWindow.h - MIDI Learn Mappings Display Window
#pragma once
#include <JuceHeader.h>

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
        titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
        
        // Table headers
        addAndMakeVisible(sliderHeaderLabel);
        sliderHeaderLabel.setText("Slider", juce::dontSendNotification);
        sliderHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        sliderHeaderLabel.setJustificationType(juce::Justification::centred);
        sliderHeaderLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
        sliderHeaderLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF33484A));
        
        addAndMakeVisible(channelHeaderLabel);
        channelHeaderLabel.setText("Input Channel", juce::dontSendNotification);
        channelHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        channelHeaderLabel.setJustificationType(juce::Justification::centred);
        channelHeaderLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
        channelHeaderLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF33484A));
        
        addAndMakeVisible(ccHeaderLabel);
        ccHeaderLabel.setText("Input CC", juce::dontSendNotification);
        ccHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        ccHeaderLabel.setJustificationType(juce::Justification::centred);
        ccHeaderLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
        ccHeaderLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF33484A));
        
        addAndMakeVisible(actionHeaderLabel);
        actionHeaderLabel.setText("Action", juce::dontSendNotification);
        actionHeaderLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        actionHeaderLabel.setJustificationType(juce::Justification::centred);
        actionHeaderLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
        actionHeaderLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF33484A));
        
        // Clear All button
        addAndMakeVisible(clearAllButton);
        clearAllButton.setButtonText("Clear All");
        clearAllButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404040));
        clearAllButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFEEEEEE));
        clearAllButton.onClick = [this]() {
            clearAllMappings();
        };
        
        // Status label
        addAndMakeVisible(statusLabel);
        statusLabel.setFont(juce::FontOptions(11.0f));
        statusLabel.setJustificationType(juce::Justification::centred);
        statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFCCCCCC));
        updateStatusLabel();
    }
    
    ~MidiLearnWindow()
    {
        mappingRows.clear();
    }
    
    void paint(juce::Graphics& g) override
    {
        // Background
        g.fillAll(juce::Colour(0xFF2D2D2D));
        
        // Header background
        auto headerBounds = getHeaderBounds();
        g.setColour(juce::Colour(0xFF33484A));
        g.fillRect(headerBounds);
        
        // Table grid lines
        g.setColour(juce::Colour(0xFF404040));
        
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
    
    // Callbacks
    std::function<void(int sliderIndex, int midiChannel, int ccNumber)> onMappingAdded;
    std::function<void(int sliderIndex)> onMappingCleared;
    std::function<void()> onAllMappingsCleared;
    
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
            sliderLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
            
            // Channel label
            addAndMakeVisible(channelLabel);
            channelLabel.setText(juce::String(midiChannel), juce::dontSendNotification);
            channelLabel.setFont(juce::FontOptions(11.0f));
            channelLabel.setJustificationType(juce::Justification::centred);
            channelLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
            
            // CC label
            addAndMakeVisible(ccLabel);
            ccLabel.setText(juce::String(ccNumber), juce::dontSendNotification);
            ccLabel.setFont(juce::FontOptions(11.0f));
            ccLabel.setJustificationType(juce::Justification::centred);
            ccLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFEEEEEE));
            
            // Remove button
            addAndMakeVisible(removeButton);
            removeButton.setButtonText("Remove");
            removeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF404040));
            removeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFEEEEEE));
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
        return area.removeFromTop(25);
    }
    
    juce::Rectangle<int> getTableBounds() const
    {
        auto area = getLocalBounds();
        area.reduce(10, 10);
        area.removeFromTop(40); // Title + gap
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