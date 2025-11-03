# 14-bit Virtual MIDI Controller - Architecture Overview

**Last Updated**: November 3, 2025 (Light Mode Theme Implementation)
**Status**: Production-ready with complete adaptive UI scaling system (75%-200%), comprehensive action confirmation system, full slider bulk operations, reorganized settings interface, and Dark/Light/Auto theme support

## Quick Project Summary

JUCE-based desktop application providing hardware-realistic 14-bit MIDI slider control with professional automation, blueprint-style UI, adaptive scaling system, comprehensive action confirmation feedback, powerful bulk slider operations, and dynamic theming.

**Key Features**: 16 sliders (4 banks × 4 sliders), 4/8 slider modes, MIDI learn, advanced automation, preset management, adaptive screen-aware UI scaling, action tooltip system, slider bulk operations (value & settings copy), Dark/Light/Auto themes

## Essential Architecture

### Core Components
```
DebugMidiController.h (1200+ lines) - Main application controller
SimpleSliderControl.h (800+ lines)  - Individual slider component  
SettingsWindow.h                    - Tabbed settings interface
MidiLearnWindow.h                   - MIDI mapping interface
```

### Key Systems
- **MIDI System**: `MidiManager.h`, `Midi7BitController.h` - True 14-bit MIDI with learn mode
- **Adaptive UI Scaling**: `GlobalUIScale.h`, `WindowManager.h` - Screen-aware scaling with intelligent constraints
- **Theme System**: `ThemeManager.h`, `CustomLookAndFeel.h` - Dynamic Dark/Light/Auto theming with system detection
- **Automation**: `AutomationEngine.h`, `AutomationConfigManager.h` - 3-phase curves with BPM sync
- **Layout**: `MainControllerLayout.h` - Scale-aware positioning and bounds
- **Display**: `SliderDisplayManager.h` - MIDI-to-display value mapping
- **Action Feedback**: `actionTooltipLabel` - Comprehensive action confirmation system with thread-safe updates
- **Context Menus**: `SliderContextMenu.h`, `AutomationContextMenu.h` - Right-click operations for sliders & automation
- **Bulk Operations**: Slider value & settings propagation across banks (A/B/C/D) or all 16 sliders

### Critical File Locations
```
Source/
├── DebugMidiController.h           # Main controller with bulk operation callbacks (lines 236-320)
├── SimpleSliderControl.h           # Individual slider logic with context menu (lines 1315-1407, 1443-1447)
├── SettingsWindow.h                # Settings interface with copy/paste/reset methods
├── MidiLearnWindow.h               # MIDI Learn interface with theme support
├── CustomLookAndFeel.h             # Blueprint styling with BlueprintColors namespace
├── UI/
│   ├── ThemeManager.h              # Theme system with Dark/Light/Auto modes and system detection
│   ├── SliderContextMenu.h         # Slider right-click menu (range presets, copy/paste, bulk ops)
│   ├── AutomationContextMenu.h     # Automation right-click menu (save/load/copy/paste configs)
│   ├── GlobalUIScale.h             # Adaptive scaling with screen detection
│   ├── WindowManager.h             # Window constraints & resizing
│   ├── MainControllerLayout.h      # Layout calculations
│   ├── GlobalSettingsTab.h         # Global settings with theme selector
│   └── ControllerSettingsTab.h     # Per-slider settings with theme support
└── Core/
    ├── MidiManager.h               # MIDI I/O
    └── AutomationEngine.h          # Automation system
```

## Recent Critical Updates

### November 3, 2025: Light Mode Theme Implementation (In Progress)
**Feature**: Dynamic theming system with Dark/Light/Auto modes and Ableton-inspired light theme
**Implementation**: Complete theme infrastructure with automatic color updates and system theme detection
**Status**: Core implementation complete, additional refinements pending

**Files Modified**:
- `ThemeManager.h:30-241` - Added `inputBackground` to ThemePalette, updated light theme colors
- `CustomLookAndFeel.h:19` - Added `inputBackground()` accessor to BlueprintColors namespace
- `MidiLearnWindow.h:10-12,107,114,391-421,483-491` - Added ThemeChangeListener support and `themeChanged()` implementation
- `GlobalSettingsTab.h:564-604` - Enhanced `themeChanged()` to update all input components
- `ControllerSettingsTab.h:8,16,31,212,219,1373-1425` - Added theme support with comprehensive color updates
- All TextEditor and ComboBox components updated to use `BlueprintColors::inputBackground()`

**Theme Color Palettes**:
- **Dark Theme** (unchanged): Blueprint-style dark blue/cyan theme (0x1A-0x24 range)
  - `inputBackground`: 0x1A1A2E (same as background)
- **Light Theme** (Ableton-inspired): Muted gray theme with proper contrast
  - `background`: 0xB0B0B0 (darker muted gray base)
  - `panel`: 0xB8B8B8 (darker gray for panels)
  - `sectionBackground`: 0xBCBCBC (section containers)
  - `inputBackground`: 0xDCDCDC (light gray for input fields - high contrast)
  - `textPrimary`: 0x1C1C1C (very dark text)
  - `textSecondary`: 0x4A5568 (medium-dark secondary text)

**Theme System Architecture**:
- `ThemeManager`: Singleton providing theme management with listener pattern
- `ThemeChangeListener`: Interface for components to receive theme updates
- `BlueprintColors`: Dynamic color namespace that auto-reflects current theme
- System theme monitoring for Auto mode (polls every 1 second on macOS/Windows)

**Component Theme Support**:
- All settings tabs properly update colors on theme change
- MIDI Learn window supports dynamic theming
- Input components use dedicated `inputBackground` color for proper contrast
- Labels, ComboBoxes, TextEditors, and Sliders all theme-aware

**Known Pending Work**:
- Additional light mode refinements and visual polish
- Potential toggle button styling adjustments
- Further color palette fine-tuning based on user feedback

### October 25, 2025: Settings UI Reorganization & Default MIDI Configuration
**Feature**: Reorganized Controller Settings Tab for improved usability and updated default MIDI values
**Implementation**: Two-section layout with proper grouping and conflict-free defaults

**Files Modified**:
- `ControllerSettingsTab.h:88` - Renamed section headers from `section1Header, section2Header, section3Header` to `displayHeader, utilitiesHeader`
- `ControllerSettingsTab.h:242-263` - Updated `paint()` method for two-section layout with proper spacing
- `ControllerSettingsTab.h:459-511` - Updated `setupPerSliderControls()` with new section headers
- `ControllerSettingsTab.h:683-794` - Reorganized `layoutPerSliderSections()` for Display/Utilities sections
- `ControllerSettingsTab.h:1324-1325` - Updated `scaleFactorChanged()` with new header names
- `PresetManager.h:71,83,122,145` - Changed default MIDI channel to 11 and CC numbers to start at 10
- `GlobalSettingsTab.h:247` - Set default MIDI channel combo to 11
- `SettingsWindow.h:314-331,351,656` - Updated reset logic and default CC numbers
- `SettingsWindow.h:314-331` - Fixed reset button to properly reset global settings

**UI Layout Changes**:
1. **Bank Selector**: [A][B][C][D] buttons at top
2. **Breadcrumb**: "Bank A > Slider 1" below bank selector (updates immediately with custom names)
3. **Display Section** (rounded box):
   - Name input
   - Range settings (Min - Max)
   - Custom Steps (with Auto button)
   - Orientation (Normal/Inverted/Bipolar)
   - Snap controls (visible only in Bipolar mode)
   - Color picker (clickable color box)
4. **Utilities Section** (rounded box, 8px gap above):
   - MIDI CC Number
   - Input Behavior (Deadzone/Direct)
   - Show Automation toggle
5. **Reset Slider Button**: At bottom with spacing above

**Default MIDI Configuration**:
- **Default Channel**: Channel 11 (was 1) - avoids conflicts with common controllers
- **Default CC Numbers**: Start at CC 10 (10-25 for sliders 0-15, was 0-15) - avoids low-number CC conflicts
- All reset operations now use these non-conflicting defaults

**Visual Improvements**:
- Proper section spacing with `.reduced(0, scale.getScaled(2))` for box insets
- Consistent `sectionSpacing` (8px scaled) between Display and Utilities sections
- Content properly fits within rounded rectangle bounds
- Scale-aware layout throughout

### October 7, 2025: Slider Context Menu Bulk Operations Complete
**Feature**: Full implementation of bulk operations for slider context menu
**Implementation**: Four bulk operations for efficient slider management across banks and all sliders
**Files Modified**:
- `SimpleSliderControl.h:1367-1396` - Removed TODOs, added callback invocations for bulk operations
- `SimpleSliderControl.h:1443-1447` - Added bulk operation callback declarations
- `DebugMidiController.h:236-320` - Implemented bulk operation logic with bank calculation and action tooltips

**Bulk Operations**:
1. **Set All in Bank to This Value** - Copies slider value to all sliders in same bank (A/B/C/D)
2. **Set All Sliders to This Value** - Copies slider value to all 16 sliders
3. **Copy Settings to All in Bank** - Copies all settings (range, orientation, bipolar, color, deadzone, etc.) to bank, preserves CC numbers
4. **Copy Settings to All Sliders** - Copies all settings to all 16 sliders, preserves CC numbers

**Implementation Details**:
- Banks organized as: A (0-3), B (4-7), C (8-11), D (12-15), calculated via `sliderIndex / 4`
- Settings copy uses existing clipboard mechanism from `SettingsWindow.copySlider()` / `pasteSlider()`
- Action tooltips provide user feedback: "([SliderName]) - Set Bank [X] to value" format
- Thread-safe updates via existing action tooltip system

### September 2025: Core Systems Stabilization
**Action Tooltip System**: Thread-safe action confirmation with "(Slider Name) - Action" formatting, 40-char truncation
**Adaptive Scaling**: Screen-aware UI scaling (75%-200%) with constraint validation and dynamic scale limits
**Font Scaling**: TextEditor clear/setText refresh pattern for immediate font updates across all input components

## Implementation Guidelines

### Theme-Aware Development
- **Colors**: Always use `BlueprintColors::` namespace functions, never hardcode colors
- **Input backgrounds**: Use `BlueprintColors::inputBackground()` for TextEditor and ComboBox backgrounds
- **Theme listener**: Implement `ThemeManager::ThemeChangeListener` interface for dynamic theme updates
- **Registration**: Register/unregister in constructor/destructor: `ThemeManager::getInstance().addThemeChangeListener(this)`
- **Update method**: Implement `themeChanged()` to update all color properties when theme changes
- **Testing**: Test components in both Dark and Light modes to ensure proper contrast and readability
- **Reference implementation**: Check `themeChanged()` methods in GlobalSettingsTab, ControllerSettingsTab, MidiLearnWindow

### Adaptive Scale-Aware Development
- **Dimensions**: `GlobalUIScale::getInstance().getScaled(value)` for all UI measurements
- **Fonts**: `GlobalUIScale::getInstance().getScaledFont(baseSize)` for all text components
- **TextEditor fonts**: Set in setup AND in `scaleFactorChanged()` with refresh pattern
- **Constraint-aware scaling**: Use `setScaleFactorWithConstraints()` instead of `setScaleFactor()`
- **Screen-aware setup**: Call `updateScreenConstraints()` during component initialization
- **Window constraints**: Use `WindowManager.h` methods, never hardcode widths
- **Layout**: Use `MainControllerLayout::Constants::` methods for scaled values
- **Reference implementation**: Check `scaleFactorChanged()` methods in settings tabs

### Font Scaling Patterns
- **TextEditor components**: `.setFont()` after setup, then clear/setText refresh in scaleFactorChanged
- **ComboBox components**: Font scaling requires custom LookAndFeel (not implemented - acceptable)
- **Slider text boxes**: Separate into standalone TextEditor for proper font scaling
- **Labels**: Standard `.setFont()` calls work immediately

### Common Patterns
- **MIDI Values**: Internal 0-16383, display mapping via `SliderDisplayManager`
- **Bank Organization**: 16 sliders = 4 banks × 4 sliders, bank index = `sliderIndex / 4`, range = `bank * 4` to `bank * 4 + 3`
- **Callbacks**: Components communicate via function callbacks, not direct references
- **Visual-Functional Split**: Visual elements in parent, JUCE components handle interaction
- **State Management**: Boolean flags + callback pattern for mode switching
- **Context Menus**: Shared pointer pattern for async menu safety, callbacks capture slider index by value
- **Theme Updates**: Listener pattern with `ThemeChangeListener` interface, auto-updates via `BlueprintColors` namespace

### Build System
- **Location**: `Builds/MacOSX/14bit Virtual Midi Controller.xcodeproj`
- **Platform**: macOS with JUCE framework
- **Dependencies**: JUCE library integrated via `JuceLibraryCode/`

## Documentation Structure

For detailed information, see `docs/` folder:
- `docs/detailed-architecture.md` - Complete system documentation
- `docs/implementation-examples.md` - Code patterns and examples  
- `docs/recent-changes.md` - Detailed change history

## Quick Debugging Reference

**Common Issues**:
- Bulk operations not working → Check callback wiring in `DebugMidiController.h:236-320` and declarations in `SimpleSliderControl.h:1443-1447`
- Context menu callbacks null → Verify lambda captures use `[this, sliderIdx]` pattern to capture by value
- Bank calculation wrong → Use `sliderIndex / 4` for bank index (0-3), `bank * 4` to `bank * 4 + 3` for range
- Settings not copying → Check `SettingsWindow.copySlider()` / `pasteSlider()` preserve CC numbers
- Scale problems → Check `GlobalUIScale` usage, screen constraints, and `WindowManager`
- Action tooltip not appearing → Check callback connections and `updateActionTooltip()` calls
- Settings UI content overflow → Verify `paint()` and `layoutPerSliderSections()` use matching spacing values
- Breadcrumb not updating → Check `updateBreadcrumbLabel()` is called in `applyCustomName()`
- Reset not working → Verify global settings reset in `onResetToDefaults` callback (SettingsWindow.h:314-331)
- Theme not updating → Check component implements `ThemeChangeListener` and registers/unregisters properly
- Wrong colors in light mode → Verify using `BlueprintColors::inputBackground()` for inputs, not `background()`
- Text invisible in theme → Check `themeChanged()` updates all label/input `textColourId` properties

**Key Methods**:
- `onSetAllInBank` / `onSetAllSliders` - Bulk value operations across bank or all sliders
- `onCopyToBank` / `onCopyToAll` - Bulk settings copy using clipboard mechanism
- `updateActionTooltip()` - Thread-safe action confirmation with text truncation
- `scaleFactorChanged()` - Reference implementation for scale-aware resizing
- `setScaleFactorWithConstraints()` - Constraint-aware scale setting with validation
- `themeChanged()` - Theme update callback, updates all color properties for current theme
- `BlueprintColors::*()` - Dynamic color accessors that auto-reflect current theme