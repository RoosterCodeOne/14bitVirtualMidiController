# Recent Changes - 14-bit Virtual MIDI Controller

## September 2025

### September 6, 2025: Settings Window Scale-Aware Resizing Fix
**Issue**: Settings window width calculation bug at non-100% UI scale factors (175%, 150%, etc.)
- Settings window toggle caused incorrect width, cutting off sliders
- Last visible slider pushed out of bounds, third slider cut off  
- Issue persisted even after closing/reopening settings window

**Root Cause**: `toggleSettingsWindow()` caller was passing hardcoded `350` instead of scaled width

**Files Fixed**:
- `DebugMidiController.h:1409`: Changed from hardcoded `350` to `MainControllerLayout::Constants::getSettingsPanelWidth()`
- `WindowManager.h`: Updated width calculation logic to match working `scaleFactorChanged()` pattern
  - Lines 79-81: Settings window width calculation
  - Lines 144-148: Learn window entry width calculation  
  - Lines 167-171: Learn window exit width calculation

**Technical Details**:
- Now uses consistent `contentWidth + settingsWidth` pattern across all window methods
- Properly uses scaled panel width from `MainControllerLayout::Constants::getSettingsPanelWidth()`
- Eliminates "in between" width values that caused layout issues

### September 5, 2025: Settings Window UI Scaling Completion
**Git Hash**: f3237f7, 89df207, b74be34
- Completed settings window fonts scaling implementation
- Fixed scaling logic in automation area
- Made target value font and tooltip font scale-aware
- Resolved slider area scaling inconsistencies

### Previous Scaling Infrastructure (August-September 2025)
**Git Hash**: 653f565, 67cf002, 14bb662
- Implemented comprehensive UI scaling system (75%-200%)
- Added `GlobalUIScale.h` singleton pattern
- Created `WindowManager.h` for scale-aware window constraints
- Fixed automatic window resize on scaling changes
- Integrated scaling with existing `PresetManager` for persistence

## Key Patterns Established

### Scale-Aware Development Standards
1. **Always use**: `GlobalUIScale::getInstance().getScaled(value)` for any UI dimensions
2. **Window Management**: Use `WindowManager.h` methods, never hardcode window widths
3. **Layout Constants**: Use `MainControllerLayout::Constants::` methods for scaled values
4. **Reference Implementation**: `scaleFactorChanged()` in `DebugMidiController.h:1135-1180` shows correct pattern

### Critical Implementation Pattern
```cpp
// CORRECT pattern (from scaleFactorChanged)
int contentWidth = bankManager.isEightSliderMode() ? scale.getScaled(970) : scale.getScaled(490);
int settingsWidth = (isInSettingsMode || isInLearnMode) ? MainControllerLayout::Constants::getSettingsPanelWidth() : 0;
int targetWidth = contentWidth + settingsWidth;

// INCORRECT pattern (old implementation)  
int targetWidth = isInSettingsMode ? (contentAreaWidth + 350) : contentAreaWidth; // Hardcoded 350!
```

## System Status
- **Production Ready**: All major scaling issues resolved
- **UI Scaling**: Fully functional 75%-200% with proper constraint management
- **Window Management**: Scale-aware resizing and constraint system working correctly
- **Settings/Learn Windows**: Both use consistent width calculation patterns