# 14-bit Virtual MIDI Controller - Architecture Overview

**Last Updated**: September 8, 2025 (Adaptive Scaling Implementation)  
**Status**: Production-ready with intelligent screen-aware UI scaling system (75%-200%)

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

### September 8, 2025: Adaptive Scaling Implementation
**Feature**: Intelligent screen-aware UI scaling with automatic constraint validation
**Implementation**: Complete adaptive scaling system that prevents UI overflow on smaller screens
**Files Modified**: 
- `GlobalUIScale.h` - Added screen detection using JUCE's Desktop::getDisplays(), constraint calculation
- `GlobalSettingsTab.h` - Updated UI controls to show only valid scale options with visual indicators  
- `DebugMidiController.h` - Added constraint-aware initialization and preset loading

**Key Features**:
- Screen dimension detection with multi-monitor support
- Dynamic scale limit calculation (prevents 8-slider mode overflow on small screens)
- Smart UI dropdown showing only applicable scale options
- User feedback system with detailed constraint reasoning
- Maintains 75% minimum scale for readability, respects 200% maximum

### September 6, 2025: Settings Window Scale-Aware Resizing
**Issue**: Settings window width incorrect at non-100% scales, sliders cut off
**Root Cause**: Hardcoded `350` width instead of scaled `MainControllerLayout::Constants::getSettingsPanelWidth()`
**Files Fixed**: 
- `DebugMidiController.h:1409` - Changed from `350` to `MainControllerLayout::Constants::getSettingsPanelWidth()`
- `WindowManager.h` - Updated width calculation logic to match working `scaleFactorChanged()` pattern

## Implementation Guidelines

### Adaptive Scale-Aware Development
- **Always use**: `GlobalUIScale::getInstance().getScaled(value)` for dimensions
- **Constraint-aware scaling**: Use `setScaleFactorWithConstraints()` instead of `setScaleFactor()` 
- **Screen-aware setup**: Call `updateScreenConstraints()` during component initialization
- **Window constraints**: Use `WindowManager.h` methods, never hardcode widths
- **Layout**: Use `MainControllerLayout::Constants::` methods for scaled values
- **Valid options**: Use `getValidScaleOptions()` for UI dropdowns and validation
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
- Scale problems → Check `GlobalUIScale` usage, screen constraints, and `WindowManager` 
- Scale overflow → Verify `setScaleFactorWithConstraints()` usage instead of direct `setScaleFactor()`
- Invalid scale options → Check `getValidScaleOptions()` and screen constraint calculation
- MIDI learn issues → Check `Midi7BitController` and learn zone activation
- Window sizing → Verify `MainControllerLayout::Constants` usage, not hardcoded values
- Automation problems → Check `AutomationEngine` state and curve calculations

**Key Methods**:
- `setScaleFactorWithConstraints()` - Constraint-aware scale setting with validation
- `updateScreenConstraints()` - Initialize/refresh screen dimension constraints
- `getValidScaleOptions()` - Get screen-appropriate scale options for UI
- `scaleFactorChanged()` - Reference implementation for scale-aware resizing
- `toggleSettingsWindow()`/`toggleLearnWindow()` - Window mode switching
- `updateWindowConstraints()` - Window constraint management