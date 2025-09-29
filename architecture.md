# 14-bit Virtual MIDI Controller - Architecture Overview

**Last Updated**: September 11, 2025 (Action Tooltip System Implementation Complete)  
**Status**: Production-ready with complete adaptive UI scaling system (75%-200%) and comprehensive action confirmation system

## Quick Project Summary

JUCE-based desktop application providing hardware-realistic 14-bit MIDI slider control with professional automation, blueprint-style UI, adaptive scaling system, and comprehensive action confirmation feedback.

**Key Features**: 16 sliders, 4/8 slider modes, 4 banks, MIDI learn, advanced automation, preset management, adaptive screen-aware UI scaling, action tooltip system

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
- **Action Feedback**: `actionTooltipLabel` - Comprehensive action confirmation system with thread-safe updates

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

### September 29, 2025: Automation Visualizer Curve Issues Investigation
**Issue**: Automation visualizer ball movement and return curve mirroring problems
**Investigation**:
- Fixed ball movement to follow curved attack path by applying curve transformation to Y position
- Fixed return curve segmentation by using consistent 20-step resolution (was variable steps)
- Updated inverse curve calculation to match AutomationEngine behavior exactly
- **Remaining Issue**: Return curves still don't visually mirror attack curves as expected for professional automation display

**Files Modified**:
- `Source/Graphics/CurveCalculator.h` - Fixed ball position calculation and curve generation consistency
- Attack phase ball movement: Now applies `applyCurve()` to Y position for proper curve following
- Return curve generation: Fixed step count from variable to consistent 20 steps
- Inverse curve calculation: Updated to match AutomationEngine formula exactly

**Future Considerations**:
- Consider alternative visualizer implementation for more robust curve mirroring
- Current mathematical approach may need geometric/visual approach instead of inverse curve formulas
- Professional automation tools typically show perfect visual mirrors regardless of mathematical inverse

### September 11, 2025: Action Tooltip System Implementation Complete
**Feature**: Comprehensive action confirmation system replacing keyboard speed tooltip
**Implementation**: Thread-safe action feedback with intelligent text truncation and comprehensive coverage
**Files Modified**: 
- `DebugMidiController.h` - Replaced `movementSpeedLabel` with `actionTooltipLabel`, added `updateActionTooltip()` method
- `MainControllerLayout.h` - Updated layout method parameter names for new tooltip system
- `SimpleSliderControl.h` - Added callbacks for automation, timing mode, and lock state changes
- `AutomationControlPanel.h` - Added time mode change callback system
- `ControllerSettingsTab.h` - Added slider reset action tracking
- `SettingsWindow.h` - Added slider reset callback forwarding

**Key Features**:
- **Action Coverage**: Keyboard speed, automation operations, slider resets, timing modes, MIDI learning, preset loading, lock/unlock
- **Smart Formatting**: Slider-specific actions use "(Slider Name) - Action" format with custom or fallback naming
- **Thread Safety**: All tooltip updates use `MessageManager::callAsync()` for safe cross-thread operation
- **Text Truncation**: Messages >40 characters automatically truncated with "..." for optimal display
- **Blueprint Integration**: Maintains original tooltip positioning and styling in bottom-left area

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
- Action tooltip not appearing → Check callback connections and `updateActionTooltip()` calls
- Tooltip text truncated → Expected for messages >40 chars; check original message content

**Key Methods**:
- `setScaleFactorWithConstraints()` - Constraint-aware scale setting with validation
- `updateScreenConstraints()` - Initialize/refresh screen dimension constraints
- `getValidScaleOptions()` - Get screen-appropriate scale options for UI
- `scaleFactorChanged()` - Reference implementation for scale-aware resizing
- `toggleSettingsWindow()`/`toggleLearnWindow()` - Window mode switching
- `updateWindowConstraints()` - Window constraint management
- `updateActionTooltip()` - Thread-safe action confirmation with text truncation

## Action Tooltip System

### Tracked Actions
- **Keyboard Speed**: "Keyboard Speed: [X]"
- **Automation**: "Automation Started/Stopped", "([SliderName]) - Automation Config Saved/Loaded/Copied/Pasted/Reset: [Name]"
- **Slider Operations**: "([SliderName]) - Reset/Locked/Unlocked"
- **MIDI**: "MIDI Mapping: Ch[X] CC[Y]", "MIDI Mapping Cleared"
- **System**: "Preset Loaded: [Name]", "Timing Mode: Seconds/Beats"

### Implementation Notes
- Thread-safe updates via `MessageManager::callAsync()`
- 40-character truncation with "..." for long messages
- Slider-specific format: "(Slider Name) - Action"
- Positioned in bottom-left area (former keyboard speed tooltip location)
- Blueprint styling with `textSecondary` color and 10pt scaled font