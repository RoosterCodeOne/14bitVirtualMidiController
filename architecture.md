# 14-bit Virtual MIDI Controller - Architecture Overview

**Last Updated**: September 6, 2025 (Post Settings Window Scaling Fix)  
**Status**: Production-ready with comprehensive UI scaling system (75%-200%)

## Quick Project Summary

JUCE-based desktop application providing hardware-realistic 14-bit MIDI slider control with professional automation, blueprint-style UI, and comprehensive scaling system.

**Key Features**: 16 sliders, 4/8 slider modes, 4 banks, MIDI learn, advanced automation, preset management, scale-aware UI (75%-200%)

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
- **UI Scaling**: `GlobalUIScale.h`, `WindowManager.h` - 75%-200% scaling with constraint management
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
│   ├── GlobalUIScale.h             # Scaling system
│   ├── WindowManager.h             # Window constraints & resizing
│   └── MainControllerLayout.h      # Layout calculations
└── Core/
    ├── MidiManager.h               # MIDI I/O
    └── AutomationEngine.h          # Automation system
```

## Recent Critical Fixes

### September 6, 2025: Settings Window Scale-Aware Resizing
**Issue**: Settings window width incorrect at non-100% scales, sliders cut off
**Root Cause**: Hardcoded `350` width instead of scaled `MainControllerLayout::Constants::getSettingsPanelWidth()`
**Files Fixed**: 
- `DebugMidiController.h:1409` - Changed from `350` to `MainControllerLayout::Constants::getSettingsPanelWidth()`
- `WindowManager.h` - Updated width calculation logic to match working `scaleFactorChanged()` pattern

## Implementation Guidelines

### Scale-Aware Development
- **Always use**: `GlobalUIScale::getInstance().getScaled(value)` for dimensions
- **Window constraints**: Use `WindowManager.h` methods, never hardcode widths
- **Layout**: Use `MainControllerLayout::Constants::` methods for scaled values
- **Reference implementation**: Check `scaleFactorChanged()` in `DebugMidiController.h:1135-1180`

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
- Scale problems → Check `GlobalUIScale` usage and `WindowManager` constraints
- MIDI learn issues → Check `Midi7BitController` and learn zone activation
- Window sizing → Verify `MainControllerLayout::Constants` usage, not hardcoded values
- Automation problems → Check `AutomationEngine` state and curve calculations

**Key Methods**:
- `scaleFactorChanged()` - Reference implementation for scale-aware resizing
- `toggleSettingsWindow()`/`toggleLearnWindow()` - Window mode switching
- `updateWindowConstraints()` - Window constraint management