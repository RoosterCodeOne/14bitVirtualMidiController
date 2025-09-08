# 14-bit Virtual MIDI Controller - Architecture Overview

**Last Updated**: September 8, 2025 (Scale-Aware Font Implementation Complete)  
**Status**: Production-ready with complete adaptive UI scaling system (75%-200%)

## Quick Project Summary

JUCE-based desktop application providing hardware-realistic 14-bit MIDI slider control with professional automation, blueprint-style UI, and adaptive scaling system that intelligently adjusts to screen constraints.

**Key Features**: 16 sliders, 4/8 slider modes, 4 banks, MIDI learn, advanced automation, preset management, adaptive screen-aware UI scaling

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
- **Automation**: `AutomationEngine.h`, `AutomationConfigManager.h` - 3-phase curves with BPM sync
- **Layout**: `MainControllerLayout.h` - Scale-aware positioning and bounds
- **Display**: `SliderDisplayManager.h` - MIDI-to-display value mapping

### Critical File Locations
```
Source/
├── DebugMidiController.h           # Main controller (primary file)
├── SimpleSliderControl.h           # Individual slider logic
├── SettingsWindow.h                # Settings interface
├── UI/
│   ├── GlobalUIScale.h             # Adaptive scaling with screen detection
│   ├── WindowManager.h             # Window constraints & resizing
│   ├── MainControllerLayout.h      # Layout calculations
│   └── GlobalSettingsTab.h         # Settings UI with constraint-aware controls
└── Core/
    ├── MidiManager.h               # MIDI I/O
    └── AutomationEngine.h          # Automation system
```

## Recent Critical Updates

### September 8, 2025: Scale-Aware Font Implementation Complete
**Feature**: Complete font scaling system for all UI input components
**Implementation**: TextEditor and input box fonts now scale immediately with UI scale changes
**Files Modified**: 
- `ControllerSettingsTab.h` - Added `.setFont()` calls to all TextEditor components with refresh pattern
- `GlobalSettingsTab.h` - Separated BPM slider text box into standalone TextEditor with bidirectional sync
- All settings input boxes now use clear/setText pattern to force immediate font refresh

**Key Features**:
- All TextEditor input boxes scale fonts immediately (no editing required to see changes)
- BPM input separated from slider for consistent scaling behavior  
- Font refresh pattern: `clear()` → `setText()` forces JUCE to show new fonts immediately
- Maintains 12pt base font size across all input components

### September 8, 2025: Adaptive Scaling Implementation  
**Feature**: Intelligent screen-aware UI scaling with automatic constraint validation
**Implementation**: Complete adaptive scaling system that prevents UI overflow on smaller screens
**Key Features**: Screen detection, dynamic scale limits, smart UI dropdowns, constraint validation

### September 6, 2025: Settings Window Scale-Aware Resizing
**Issue**: Settings window width incorrect at non-100% scales, sliders cut off
**Root Cause**: Hardcoded `350` width instead of scaled `MainControllerLayout::Constants::getSettingsPanelWidth()`
**Files Fixed**: 
- `DebugMidiController.h:1409` - Changed from `350` to `MainControllerLayout::Constants::getSettingsPanelWidth()`
- `WindowManager.h` - Updated width calculation logic to match working `scaleFactorChanged()` pattern

## Implementation Guidelines

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
- **Callbacks**: Components communicate via function callbacks, not direct references
- **Visual-Functional Split**: Visual elements in parent, JUCE components handle interaction
- **State Management**: Boolean flags + callback pattern for mode switching

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
- Input box fonts not scaling → Check TextEditor `.setFont()` calls and clear/setText refresh pattern
- Scale problems → Check `GlobalUIScale` usage, screen constraints, and `WindowManager` 
- Scale overflow → Verify `setScaleFactorWithConstraints()` usage instead of direct `setScaleFactor()`
- ComboBox fonts not scaling → Expected behavior (requires custom LookAndFeel)
- MIDI learn issues → Check `Midi7BitController` and learn zone activation
- Window sizing → Verify `MainControllerLayout::Constants` usage, not hardcoded values

**Key Methods**:
- `setScaleFactorWithConstraints()` - Constraint-aware scale setting with validation
- `updateScreenConstraints()` - Initialize/refresh screen dimension constraints
- `getValidScaleOptions()` - Get screen-appropriate scale options for UI
- `scaleFactorChanged()` - Reference implementation for scale-aware resizing
- `toggleSettingsWindow()`/`toggleLearnWindow()` - Window mode switching
- `updateWindowConstraints()` - Window constraint management