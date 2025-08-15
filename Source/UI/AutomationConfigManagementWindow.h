//==============================================================================
// AutomationConfigManagementWindow.h - Advanced config management with MIDI learn integration
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"
#include "../Core/AutomationConfigManager.h"
#include "../Core/AutomationConfig.h"

//==============================================================================
/**
 * Table model for automation configs with MIDI learn capability
 */
class AutomationConfigTableModel : public juce::TableListBoxModel
{
public:
    enum Columns
    {
        ConfigName = 1,
        SliderNumber = 2,
        MidiInput = 3
    };
    
    AutomationConfigTableModel(AutomationConfigManager& configManager)
        : configManager(configManager)
    {
        refreshData();
    }
    
    void refreshData()
    {
        configs = configManager.getAllConfigs();
        // Sort by name for consistent display
        std::sort(configs.begin(), configs.end(), 
            [](const AutomationConfig& a, const AutomationConfig& b) {
                return a.name < b.name;
            });
    }
    
    // TableListBoxModel overrides
    int getNumRows() override
    {
        return static_cast<int>(configs.size());
    }
    
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
        auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height));
        
        // Custom selection highlighting - MIDI learn ready takes priority over regular selection
        if (isRowReadyForMidiLearn(rowNumber))
        {
            DBG("Painting amber highlighting for MIDI learn ready row: " + juce::String(rowNumber));
            // Row ready for MIDI learn - use warning color (amber/orange)
            g.setColour(BlueprintColors::warning.withAlpha(0.4f));
            g.fillRoundedRectangle(bounds.reduced(1.0f), 2.0f);
            
            // Draw ready-for-learn border - thicker for emphasis
            g.setColour(BlueprintColors::warning);
            g.drawRoundedRectangle(bounds.reduced(1.0f), 2.0f, 2.0f);
        }
        else if (isRowSelected(rowNumber))
        {
            // Selected row - use blueprint active color with rounded corners
            g.setColour(BlueprintColors::active.withAlpha(0.4f));
            g.fillRoundedRectangle(bounds.reduced(1.0f), 2.0f);
            
            // Draw selection border 
            g.setColour(BlueprintColors::active);
            g.drawRoundedRectangle(bounds.reduced(1.0f), 2.0f, 1.5f);
        }
        else if (rowNumber % 2 == 0)
        {
            // Alternating row background
            g.setColour(BlueprintColors::panel.withAlpha(0.1f));
            g.fillRoundedRectangle(bounds.reduced(1.0f), 2.0f);
        }
    }
    
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override
    {
        if (rowNumber >= static_cast<int>(configs.size()))
            return;
            
        const auto& config = configs[rowNumber];
        
        // Enhanced text color based on selection and MIDI learn state
        juce::Colour textColor;
        if (isRowSelected(rowNumber))
        {
            textColor = BlueprintColors::textPrimary.brighter(0.2f);
        }
        else if (isRowReadyForMidiLearn(rowNumber))
        {
            textColor = BlueprintColors::warning.brighter(0.3f);
        }
        else
        {
            textColor = BlueprintColors::textSecondary;
        }
        
        g.setColour(textColor);
        g.setFont(juce::Font(12.0f, isRowSelected(rowNumber) ? juce::Font::bold : juce::Font::plain));
        
        juce::String text;
        switch (columnId)
        {
            case ConfigName:
                text = config.name;
                break;
                
            case SliderNumber:
                text = config.originalSliderIndex >= 0 ? 
                       juce::String(config.originalSliderIndex + 1) : "-";
                break;
                
            case MidiInput:
                if (isRowReadyForMidiLearn(rowNumber))
                    text = "Ready...";
                else
                    text = "Ch 1 CC 10"; // Placeholder - will be populated from MIDI learn data
                break;
        }
        
        g.drawText(text, 4, 0, width - 8, height, juce::Justification::centredLeft, true);
    }
    
    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent& event) override
    {
        if (rowNumber >= static_cast<int>(configs.size()))
            return;
            
        const auto& config = configs[rowNumber];
        
        // Handle row selection
        setSelectedRow(rowNumber);
        
        // Handle column-specific actions
        switch (columnId)
        {
            case MidiInput:
                DBG("MIDI Input column clicked - row: " + juce::String(rowNumber) + ", config: " + config.id);
                if (onMidiLearnClicked)
                    onMidiLearnClicked(config.id, rowNumber);
                // Don't call onConfigSelected for MIDI Input column clicks
                return;
                
            default:
                // General row selection - notify selection change
                if (onConfigSelected)
                    onConfigSelected(config.id, rowNumber);
                break;
        }
    }
    
    // Get config at row
    AutomationConfig getConfigAt(int rowNumber) const
    {
        if (rowNumber >= 0 && rowNumber < static_cast<int>(configs.size()))
            return configs[rowNumber];
        return AutomationConfig();
    }
    
    // Selection management
    void setSelectedRow(int rowNumber)
    {
        if (selectedRowIndex != rowNumber)
        {
            selectedRowIndex = rowNumber;
            // Trigger repaint to update visual highlighting
            if (auto* tableComponent = getTableListBox())
                tableComponent->repaint();
        }
    }
    
    void clearSelection()
    {
        setSelectedRow(-1);
    }
    
    int getSelectedRow() const
    {
        return selectedRowIndex;
    }
    
    bool isRowSelected(int rowNumber) const
    {
        return selectedRowIndex == rowNumber;
    }
    
    // MIDI learn state management
    void setRowReadyForMidiLearn(int rowNumber, bool isReady)
    {
        DBG("Table model: Setting row " + juce::String(rowNumber) + " ready for MIDI learn: " + (isReady ? "true" : "false"));
        if (isReady)
            midiLearnReadyRow = rowNumber;
        else
            midiLearnReadyRow = -1;
            
        DBG("Table model: MIDI learn ready row is now: " + juce::String(midiLearnReadyRow));
            
        // Trigger repaint to update visual highlighting
        if (auto* tableComponent = getTableListBox())
            tableComponent->repaint();
    }
    
    bool isRowReadyForMidiLearn(int rowNumber) const
    {
        return midiLearnReadyRow == rowNumber;
    }
    
    int getMidiLearnReadyRow() const
    {
        return midiLearnReadyRow;
    }
    
    // Get selected config
    AutomationConfig getSelectedConfig() const
    {
        return getConfigAt(selectedRowIndex);
    }
    
    // Helper to get parent table component for repainting
    juce::TableListBox* getTableListBox()
    {
        // This will be set by the AutomationConfigManagementWindow
        return parentTableComponent;
    }
    
    void setParentTableComponent(juce::TableListBox* table)
    {
        parentTableComponent = table;
    }
    
    // Callbacks
    std::function<void(const juce::String& configId, int rowNumber)> onMidiLearnClicked;
    std::function<void(const juce::String& configId, int rowNumber)> onConfigSelected;
    
private:
    AutomationConfigManager& configManager;
    std::vector<AutomationConfig> configs;
    
    // Selection state
    int selectedRowIndex = -1;        // Currently selected row (-1 = none)
    int midiLearnReadyRow = -1;       // Row ready for MIDI learn (-1 = none)
    juce::TableListBox* parentTableComponent = nullptr;  // Reference for repainting
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationConfigTableModel)
};

//==============================================================================
/**
 * Main automation config management window
 * Provides complete config management with MIDI learn integration
 */
class AutomationConfigManagementWindow : public juce::DocumentWindow
{
public:
    enum class Mode
    {
        Save,      // Save mode: show input box and save button
        Load,      // Load mode: show load and load & save buttons
        Manage     // Manage mode: full management interface
    };
    
    AutomationConfigManagementWindow(AutomationConfigManager& configManager, Mode mode = Mode::Manage)
        : juce::DocumentWindow("Automation Config Manager", 
                              BlueprintColors::background, 
                              juce::DocumentWindow::closeButton | juce::DocumentWindow::minimiseButton)
        , configManager(configManager)
        , currentMode(mode)
        , tableModel(configManager)
    {
        setupWindow();
        setupComponents();
        setupLayout();
        
        // Setup callbacks
        setupCallbacks();
        
        setResizable(false, false);
        setUsingNativeTitleBar(true);
        
        // Set window to always stay on top
        setAlwaysOnTop(true);
        
        centreWithSize(475, 200);
        
        // Initial data refresh
        refreshConfigList();
    }
    
    ~AutomationConfigManagementWindow() override = default;
    
    void setMode(Mode newMode, int targetSliderIndex = -1)
    {
        currentMode = newMode;
        currentTargetSlider = targetSliderIndex;
        updateModeSpecificUI();
    }
    
    void refreshConfigList()
    {
        // Preserve selection if possible
        auto selectedConfig = getSelectedConfig();
        juce::String selectedConfigId = selectedConfig.isValid() ? selectedConfig.id : juce::String();
        
        tableModel.refreshData();
        configTable.updateContent();
        
        // Try to restore selection if the config still exists
        if (selectedConfigId.isNotEmpty())
        {
            for (int i = 0; i < tableModel.getNumRows(); ++i)
            {
                auto config = tableModel.getConfigAt(i);
                if (config.id == selectedConfigId)
                {
                    selectConfig(i);
                    break;
                }
            }
        }
    }
    
    void highlightConfigCreationSource(bool shouldHighlight)
    {
        isHighlightingSource = shouldHighlight;
        if (onSourceHighlightChanged)
            onSourceHighlightChanged(shouldHighlight, currentTargetSlider);
    }
    
    // Selection management for external access
    void selectConfig(int rowNumber)
    {
        tableModel.setSelectedRow(rowNumber);
    }
    
    void clearConfigSelection()
    {
        tableModel.clearSelection();
    }
    
    AutomationConfig getSelectedConfig() const
    {
        return tableModel.getSelectedConfig();
    }
    
    int getSelectedConfigRow() const
    {
        return tableModel.getSelectedRow();
    }
    
    // MIDI learn state management
    void setConfigReadyForMidiLearn(int rowNumber, bool isReady = true)
    {
        DBG("Setting config ready for MIDI learn - row: " + juce::String(rowNumber) + ", ready: " + (isReady ? "true" : "false"));
        tableModel.setRowReadyForMidiLearn(rowNumber, isReady);
        
        if (isReady)
        {
            statusLabel.setText("Config ready for MIDI learn - send MIDI CC...", juce::dontSendNotification);
        }
        else
        {
            statusLabel.setText("", juce::dontSendNotification);
        }
    }
    
    int getMidiLearnReadyConfig() const
    {
        return tableModel.getMidiLearnReadyRow();
    }
    
    // Learn mode state management
    void setLearnModeActive(bool isActive)
    {
        isLearnModeActive = isActive;
        learnModeIndicator.setVisible(isActive);
        
        if (isActive)
        {
            DBG("Config Manager: Learn Mode activated");
        }
        else
        {
            DBG("Config Manager: Learn Mode deactivated");
            // Clear any MIDI learn ready state when learn mode ends
            setConfigReadyForMidiLearn(-1, false);
        }
    }
    
    bool getLearnModeActive() const
    {
        return isLearnModeActive;
    }
    
    // Callbacks for external integration
    std::function<void(const AutomationConfig& config, int targetSlider)> onLoadConfig;
    std::function<void(const AutomationConfig& config, int targetSlider, bool alsoSave)> onLoadAndSaveConfig;
    std::function<void(const juce::String& configName, int sourceSlider)> onSaveNewConfig;
    std::function<void(bool highlight, int sliderIndex)> onSourceHighlightChanged;
    std::function<void(const juce::String& configId)> onStartMidiLearn;
    std::function<juce::String(int sliderIndex)> onGetSliderCustomName;
    std::function<void(const juce::String& configId, int rowNumber)> onConfigSelectionChanged;
    
private:
    AutomationConfigManager& configManager;
    Mode currentMode;
    int currentTargetSlider = -1;
    bool isHighlightingSource = false;
    bool isLearnModeActive = false;
    
    // UI Components
    AutomationConfigTableModel tableModel;
    juce::TableListBox configTable;
    
    // Mode-specific components
    juce::Label inputLabel;
    juce::TextEditor configNameInput;
    juce::TextButton saveButton;
    juce::TextButton loadButton;
    juce::TextButton loadAndSaveButton;
    juce::TextButton deleteButton;
    
    // Status and info
    juce::Label statusLabel;
    juce::Label modeLabel;
    juce::Label learnModeIndicator;
    
    void setupWindow()
    {
        setContentOwned(new juce::Component(), true);
        auto* content = getContentComponent();
        
        // Setup table with custom mouse handling for empty area clicks
        configTable.getViewport()->setScrollBarsShown(true, false);
        
        // Override mouse handling for empty area selection clearing
        configTable.setMouseClickGrabsKeyboardFocus(false);
        
        // Add all components to content
        content->addAndMakeVisible(configTable);
        content->addAndMakeVisible(inputLabel);
        content->addAndMakeVisible(configNameInput);
        content->addAndMakeVisible(saveButton);
        content->addAndMakeVisible(loadButton);
        content->addAndMakeVisible(loadAndSaveButton);
        content->addAndMakeVisible(deleteButton);
        content->addAndMakeVisible(statusLabel);
        content->addAndMakeVisible(modeLabel);
        content->addAndMakeVisible(learnModeIndicator);
        
        // Add mouse listener to clear selection on empty clicks
        configTable.addMouseListener(this, true);
    }
    
    void setupComponents()
    {
        // Configure table
        configTable.setModel(&tableModel);
        configTable.getHeader().addColumn("Config Name", AutomationConfigTableModel::ConfigName, 250, 150, 300);
        configTable.getHeader().addColumn("Slider #", AutomationConfigTableModel::SliderNumber, 80, 60, 100);
        configTable.getHeader().addColumn("MIDI Input", AutomationConfigTableModel::MidiInput, 120, 100, 150);
        
        // Set up table component reference for repainting
        tableModel.setParentTableComponent(&configTable);
        
        // Disable default JUCE selection highlighting - we handle it ourselves
        configTable.setMultipleSelectionEnabled(false);
        configTable.setRowSelectedOnMouseDown(false);
        
        configTable.setColour(juce::ListBox::backgroundColourId, BlueprintColors::panel);
        configTable.setColour(juce::ListBox::outlineColourId, BlueprintColors::active);
        
        // Input components
        inputLabel.setText("Config Name:", juce::dontSendNotification);
        inputLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        inputLabel.setJustificationType(juce::Justification::centredRight);
        
        configNameInput.setColour(juce::TextEditor::backgroundColourId, BlueprintColors::panel);
        configNameInput.setColour(juce::TextEditor::textColourId, BlueprintColors::textPrimary);
        configNameInput.setColour(juce::TextEditor::outlineColourId, BlueprintColors::active);
        configNameInput.setFont(12.0f);
        
        // Buttons
        saveButton.setButtonText("Save Config");
        loadButton.setButtonText("Load");
        loadAndSaveButton.setButtonText("Load & Save");
        deleteButton.setButtonText("Delete");
        
        // Apply blueprint styling to buttons
        for (auto* button : { &saveButton, &loadButton, &loadAndSaveButton, &deleteButton })
        {
            button->setColour(juce::TextButton::buttonColourId, BlueprintColors::panel);
            button->setColour(juce::TextButton::textColourOffId, BlueprintColors::textSecondary);
            button->setColour(juce::TextButton::textColourOnId, BlueprintColors::textPrimary);
        }
        
        // Status labels
        statusLabel.setColour(juce::Label::textColourId, BlueprintColors::textSecondary);
        statusLabel.setFont(10.0f);
        
        modeLabel.setColour(juce::Label::textColourId, BlueprintColors::active);
        modeLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        
        // Learn mode indicator
        learnModeIndicator.setText("Learn Mode Active", juce::dontSendNotification);
        learnModeIndicator.setColour(juce::Label::textColourId, BlueprintColors::warning);
        learnModeIndicator.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        learnModeIndicator.setJustificationType(juce::Justification::centredRight);
        learnModeIndicator.setVisible(false); // Hidden by default
    }
    
    void setupCallbacks()
    {
        // Table callbacks
        tableModel.onMidiLearnClicked = [this](const juce::String& configId, int rowNumber) {
            // Set this config as ready for MIDI learn
            setConfigReadyForMidiLearn(rowNumber, true);
            
            if (onStartMidiLearn)
                onStartMidiLearn(configId);
        };
        
        tableModel.onConfigSelected = [this](const juce::String& configId, int rowNumber) {
            // Notify external components about selection change
            if (onConfigSelectionChanged)
                onConfigSelectionChanged(configId, rowNumber);
        };
        
        
        // Button callbacks
        saveButton.onClick = [this]() {
            handleSaveConfig();
        };
        
        loadButton.onClick = [this]() {
            handleLoadConfig(false);
        };
        
        loadAndSaveButton.onClick = [this]() {
            handleLoadConfig(true);
        };
        
        deleteButton.onClick = [this]() {
            handleDeleteConfig();
        };
        
        
        // Input validation
        configNameInput.onTextChange = [this]() {
            updateSaveButtonState();
        };
    }
    
    void setupLayout()
    {
        auto* content = getContentComponent();
        content->setSize(475, 200);
        
        content->setBounds(0, 0, 475, 200);
        
        // This will be called in resized()
        layoutComponents();
    }
    
    void layoutComponents()
    {
        auto* content = getContentComponent();
        if (!content) return;
        
        auto area = content->getLocalBounds().reduced(8);
        
        // Mode label at top left, learn mode indicator at top right
        auto topArea = area.removeFromTop(20);
        auto learnIndicatorArea = topArea.removeFromRight(120);
        modeLabel.setBounds(topArea);
        learnModeIndicator.setBounds(learnIndicatorArea);
        area.removeFromTop(3);
        
        // Table takes most of the space
        auto tableArea = area.removeFromTop(area.getHeight() - 40);
        configTable.setBounds(tableArea);
        
        area.removeFromTop(3);
        
        // Dynamic bottom panel based on mode (more compact)
        auto bottomPanel = area.removeFromTop(25);
        
        if (currentMode == Mode::Save)
        {
            // Save mode: input box + Save button (no label)
            auto buttonArea = bottomPanel.removeFromRight(60);
            saveButton.setBounds(buttonArea);
            bottomPanel.removeFromRight(3);
            
            configNameInput.setBounds(bottomPanel);
        }
        else if (currentMode == Mode::Load || currentMode == Mode::Manage)
        {
            // Selection mode: Load, Load & Save, Delete buttons (more compact)
            int buttonWidth = 55;
            int wideButtonWidth = 70;
            int buttonSpacing = 3;
            
            deleteButton.setBounds(bottomPanel.removeFromRight(buttonWidth));
            bottomPanel.removeFromRight(buttonSpacing);
            loadAndSaveButton.setBounds(bottomPanel.removeFromRight(wideButtonWidth));
            bottomPanel.removeFromRight(buttonSpacing);
            loadButton.setBounds(bottomPanel.removeFromRight(buttonWidth));
        }
        
        // Status at very bottom (more space for tooltip)
        area.removeFromTop(3);
        statusLabel.setBounds(area);
    }
    
    void updateModeSpecificUI()
    {
        juce::String modeText;
        switch (currentMode)
        {
            case Mode::Save:
                modeText = "Save Mode";
                inputLabel.setVisible(false);
                configNameInput.setVisible(true);
                saveButton.setVisible(true);
                loadButton.setVisible(false);
                loadAndSaveButton.setVisible(false);
                deleteButton.setVisible(false);
                highlightConfigCreationSource(true);
                break;
                
            case Mode::Load:
                modeText = "Load Mode";
                inputLabel.setVisible(false);
                configNameInput.setVisible(false);
                saveButton.setVisible(false);
                loadButton.setVisible(true);
                loadAndSaveButton.setVisible(true);
                deleteButton.setVisible(true);
                highlightConfigCreationSource(false);
                break;
                
            case Mode::Manage:
                modeText = "Management Mode";
                inputLabel.setVisible(false);
                configNameInput.setVisible(false);
                saveButton.setVisible(false);
                loadButton.setVisible(true);
                loadAndSaveButton.setVisible(true);
                deleteButton.setVisible(true);
                highlightConfigCreationSource(false);
                break;
        }
        
        if (currentTargetSlider >= 0)
            modeText += " (Slider " + juce::String(currentTargetSlider + 1) + ")";
            
        modeLabel.setText(modeText, juce::dontSendNotification);
        
        layoutComponents();
        updateSaveButtonState();
    }
    
    void updateSaveButtonState()
    {
        bool hasValidInput = !configNameInput.getText().trim().isEmpty();
        saveButton.setEnabled(hasValidInput && (currentMode == Mode::Save || currentMode == Mode::Manage));
    }
    
    
    void handleSaveConfig()
    {
        juce::String name = configNameInput.getText().trim();
        if (name.isEmpty()) return;
        
        if (onSaveNewConfig && currentTargetSlider >= 0)
        {
            onSaveNewConfig(name, currentTargetSlider);
            configNameInput.clear();
            refreshConfigList();
            
            // Update tooltip with most recent action
            juce::String sliderInfo = "Slider " + juce::String(currentTargetSlider + 1);
            if (onGetSliderCustomName)
            {
                auto customName = onGetSliderCustomName(currentTargetSlider);
                if (!customName.isEmpty())
                    sliderInfo += " (" + customName + ")";
            }
            
            setHelpText("Config saved: " + name);
            statusLabel.setText("Config saved: " + name, juce::dontSendNotification);
            
            // Switch back to selection mode after saving
            setMode(Mode::Load, currentTargetSlider);
        }
    }
    
    void handleLoadConfig(bool alsoSave)
    {
        int selectedRow = getSelectedConfigRow();
        if (selectedRow < 0) 
        {
            statusLabel.setText("Please select a config to load", juce::dontSendNotification);
            return;
        }
        
        auto config = getSelectedConfig();
        if (!config.isValid()) return;
        
        if (alsoSave && onLoadAndSaveConfig)
        {
            onLoadAndSaveConfig(config, currentTargetSlider, true);
            // Switch to save mode after loading with pre-populated name
            setMode(Mode::Save, currentTargetSlider);
            
            // Get slider custom name if available
            juce::String sliderName = "Slider " + juce::String(currentTargetSlider + 1);
            if (onGetSliderCustomName)
            {
                auto customName = onGetSliderCustomName(currentTargetSlider);
                if (!customName.isEmpty())
                    sliderName += " (" + customName + ")";
            }
            
            configNameInput.setText(config.name + " - " + sliderName);
            statusLabel.setText("Config loaded and ready to save", juce::dontSendNotification);
        }
        else if (onLoadConfig)
        {
            onLoadConfig(config, currentTargetSlider);
            
            // Update tooltip with most recent action
            juce::String sliderInfo = "Slider " + juce::String(currentTargetSlider + 1);
            if (onGetSliderCustomName)
            {
                auto customName = onGetSliderCustomName(currentTargetSlider);
                if (!customName.isEmpty())
                    sliderInfo += " (" + customName + ")";
            }
            
            setHelpText("Config " + config.name + " loaded on " + sliderInfo);
            statusLabel.setText("Config loaded: " + config.name, juce::dontSendNotification);
        }
    }
    
    void handleDeleteConfig()
    {
        int selectedRow = getSelectedConfigRow();
        if (selectedRow < 0) 
        {
            statusLabel.setText("Please select a config to delete", juce::dontSendNotification);
            return;
        }
        
        auto config = getSelectedConfig();
        if (!config.isValid()) return;
        
        deleteConfigById(config.id);
    }
    
    void loadConfigById(const juce::String& configId, bool alsoSave)
    {
        auto config = configManager.loadConfig(configId);
        if (!config.isValid()) return;
        
        if (alsoSave && onLoadAndSaveConfig)
            onLoadAndSaveConfig(config, currentTargetSlider, true);
        else if (onLoadConfig)
            onLoadConfig(config, currentTargetSlider);
    }
    
    void deleteConfigById(const juce::String& configId)
    {
        configManager.deleteConfig(configId);
        refreshConfigList();
        statusLabel.setText("Config deleted", juce::dontSendNotification);
    }
    
    void resized() override
    {
        juce::DocumentWindow::resized();
        layoutComponents();
    }
    
    void closeButtonPressed() override
    {
        highlightConfigCreationSource(false);
        setVisible(false);
    }
    
    void mouseDown(const juce::MouseEvent& event) override
    {
        // Check if click was in empty area of table (not on a row)
        if (event.eventComponent == &configTable)
        {
            auto clickPosition = event.getPosition();
            int rowAtPosition = configTable.getRowContainingPosition(clickPosition.x, clickPosition.y);
            
            if (rowAtPosition < 0 || rowAtPosition >= tableModel.getNumRows())
            {
                // Clicked in empty area - clear selection
                clearConfigSelection();
                setConfigReadyForMidiLearn(-1, false);
            }
        }
        
        // Call parent implementation
        juce::DocumentWindow::mouseDown(event);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationConfigManagementWindow)
};