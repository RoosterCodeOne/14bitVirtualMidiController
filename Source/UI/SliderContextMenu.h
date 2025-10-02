#pragma once
#include <JuceHeader.h>

//==============================================================================
/**
 * SliderContextMenu - Right-click context menu for individual sliders
 * Provides range presets, copy/paste, reset, and bulk operations
 */
class SliderContextMenu : public juce::PopupMenu
{
public:
    // Menu item IDs
    enum MenuItems
    {
        // Range Presets
        RangePresetsStart = 1,
        Range_0_127 = 1,
        Range_Minus100_Plus100 = 2,
        Range_0_1 = 3,
        Range_0_16383 = 4,
        RangePresetsEnd = 10,

        // Separator IDs
        Separator1 = 11,

        // Copy/Paste/Reset
        CopySlider = 20,
        PasteSlider = 21,
        ResetSlider = 22,

        Separator2 = 23,

        // Bulk Operations
        BulkOpsStart = 30,
        SetAllInBank = 30,
        SetAllSliders = 31,
        CopyToBank = 32,
        CopyToAll = 33,
        BulkOpsEnd = 39,

        Separator3 = 40
    };

    SliderContextMenu()
        : currentSliderIndex(-1), hasClipboard(false)
    {
        DBG("SliderContextMenu created");
    }

    ~SliderContextMenu()
    {
        DBG("SliderContextMenu destroyed, last slider index was: " + juce::String(currentSliderIndex));
    }

    void showForSlider(int sliderIndex, juce::Point<int> position, juce::Component* parentComponent,
                       bool clipboardAvailable = false, std::shared_ptr<SliderContextMenu> self = nullptr)
    {
        currentSliderIndex = sliderIndex;
        hasClipboard = clipboardAvailable;

        // Clear previous menu items
        clear();

        // Range Presets submenu
        juce::PopupMenu rangePresetsMenu;
        rangePresetsMenu.addItem(Range_0_127, "0 - 127 (7-bit MIDI)");
        rangePresetsMenu.addItem(Range_Minus100_Plus100, "-100 to +100");
        rangePresetsMenu.addItem(Range_0_1, "0.0 - 1.0");
        rangePresetsMenu.addItem(Range_0_16383, "0 - 16383 (14-bit MIDI)");

        addSubMenu("Range Presets", rangePresetsMenu);

        addSeparator();

        // Copy/Paste/Reset
        addItem(CopySlider, "Copy Slider");
        addItem(PasteSlider, "Paste Slider", hasClipboard);
        addItem(ResetSlider, "Reset Slider");

        addSeparator();

        // Bulk Operations submenu
        juce::PopupMenu bulkOpsMenu;
        bulkOpsMenu.addItem(SetAllInBank, "Set All in Bank to This Value");
        bulkOpsMenu.addItem(SetAllSliders, "Set All Sliders to This Value");
        bulkOpsMenu.addSeparator();
        bulkOpsMenu.addItem(CopyToBank, "Copy Settings to All in Bank");
        bulkOpsMenu.addItem(CopyToAll, "Copy Settings to All Sliders");

        addSubMenu("Bulk Operations", bulkOpsMenu);

        // Show menu at the clicked position (convert to global coordinates)
        auto globalPos = parentComponent->localPointToGlobal(position);

        #if JUCE_MODAL_LOOPS_PERMITTED
        int result = show(0, globalPos.x, globalPos.y);
        handleMenuResult(result);
        // In synchronous mode, shared_ptr will handle cleanup automatically
        #else
        showMenuAsync(juce::PopupMenu::Options()
                         .withTargetScreenArea(juce::Rectangle<int>(globalPos.x, globalPos.y, 1, 1)),
                     [this, self](int result) {
                         handleMenuResult(result);
                         // self shared_ptr keeps this object alive until callback completes
                     });
        #endif
    }

    // Callbacks for menu actions

    // Range preset callbacks
    std::function<void(int sliderIndex, int rangeType)> onRangePresetSelected;

    // Copy/Paste/Reset callbacks
    std::function<void(int sliderIndex)> onCopySlider;
    std::function<void(int sliderIndex)> onPasteSlider;
    std::function<void(int sliderIndex)> onResetSlider;

    // Bulk operation callbacks
    std::function<void(int sliderIndex)> onSetAllInBank;
    std::function<void(int sliderIndex)> onSetAllSliders;
    std::function<void(int sliderIndex)> onCopyToBank;
    std::function<void(int sliderIndex)> onCopyToAll;

private:
    int currentSliderIndex = -1;
    bool hasClipboard = false;

    void handleMenuResult(int result)
    {
        try {
            if (result == 0) return; // User cancelled

            // Validate slider index
            if (currentSliderIndex < 0 || currentSliderIndex >= 16) {
                DBG("ERROR: Invalid slider index in handleMenuResult: " + juce::String(currentSliderIndex));
                return;
            }

            // Handle range presets
            if (result >= RangePresetsStart && result <= RangePresetsEnd)
            {
                if (onRangePresetSelected) {
                    DBG("Range preset selected: " + juce::String(result) + " for slider " + juce::String(currentSliderIndex));
                    onRangePresetSelected(currentSliderIndex, result);
                } else {
                    DBG("ERROR: onRangePresetSelected callback is null");
                }
                return;
            }

            // Handle other menu items
            switch (result)
            {
                case CopySlider:
                    if (onCopySlider) {
                        DBG("Calling onCopySlider for slider " + juce::String(currentSliderIndex));
                        onCopySlider(currentSliderIndex);
                    } else {
                        DBG("ERROR: onCopySlider callback is null");
                    }
                    break;

                case PasteSlider:
                    if (onPasteSlider) {
                        DBG("Calling onPasteSlider for slider " + juce::String(currentSliderIndex));
                        onPasteSlider(currentSliderIndex);
                    } else {
                        DBG("ERROR: onPasteSlider callback is null");
                    }
                    break;

                case ResetSlider:
                    if (onResetSlider) {
                        DBG("Calling onResetSlider for slider " + juce::String(currentSliderIndex));
                        onResetSlider(currentSliderIndex);
                    } else {
                        DBG("ERROR: onResetSlider callback is null");
                    }
                    break;

                case SetAllInBank:
                    if (onSetAllInBank) {
                        DBG("Calling onSetAllInBank for slider " + juce::String(currentSliderIndex));
                        onSetAllInBank(currentSliderIndex);
                    } else {
                        DBG("ERROR: onSetAllInBank callback is null");
                    }
                    break;

                case SetAllSliders:
                    if (onSetAllSliders) {
                        DBG("Calling onSetAllSliders for slider " + juce::String(currentSliderIndex));
                        onSetAllSliders(currentSliderIndex);
                    } else {
                        DBG("ERROR: onSetAllSliders callback is null");
                    }
                    break;

                case CopyToBank:
                    if (onCopyToBank) {
                        DBG("Calling onCopyToBank for slider " + juce::String(currentSliderIndex));
                        onCopyToBank(currentSliderIndex);
                    } else {
                        DBG("ERROR: onCopyToBank callback is null");
                    }
                    break;

                case CopyToAll:
                    if (onCopyToAll) {
                        DBG("Calling onCopyToAll for slider " + juce::String(currentSliderIndex));
                        onCopyToAll(currentSliderIndex);
                    } else {
                        DBG("ERROR: onCopyToAll callback is null");
                    }
                    break;

                default:
                    DBG("ERROR: Unknown menu result: " + juce::String(result));
                    break;
            }
        }
        catch (const std::exception& e) {
            DBG("EXCEPTION in handleMenuResult: " + juce::String(e.what()));
        }
        catch (...) {
            DBG("UNKNOWN EXCEPTION in handleMenuResult");
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderContextMenu)
};
