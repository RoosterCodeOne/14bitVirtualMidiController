# 14-bit Virtual MIDI Controller

A professional desktop MIDI controller application providing hardware-realistic slider control with true 14-bit resolution, advanced automation, and blueprint-style visual design.

![Blueprint Style Interface](https://img.shields.io/badge/Design-Blueprint%20Technical-cyan) ![MIDI Resolution](https://img.shields.io/badge/MIDI-14--bit%20(0--16383)-blue) ![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Windows-lightgrey) ![Framework](https://img.shields.io/badge/Framework-JUCE%208.x-orange)

## 🎛️ Key Features

- **True 14-bit MIDI Resolution**: Full 0-16383 range with configurable 7-bit/14-bit output per slider
- **16 Professional Sliders**: Hardware-realistic interaction with custom ranges, orientations, and display units
- **Advanced Automation System**: 3-phase automation (Delay → Attack → Return) with curve shaping
- **MIDI Learn Integration**: Direct pairing of automation configs with external MIDI controllers
- **Blueprint Visual Design**: Technical drawing aesthetic with cyan/navy color palette
- **Cross-Platform Support**: Native macOS and Windows applications via JUCE framework

## 🚀 Quick Start

### Prerequisites

- **JUCE Framework 8.x** (see installation instructions below)
- **macOS**: Xcode 14+ with Command Line Tools
- **Windows**: Visual Studio 2019+ with C++ development tools
- **MIDI Device**: Physical MIDI controller or virtual MIDI buses for full functionality

### JUCE Installation

#### Option 1: Direct Download (Recommended)
1. Visit [JUCE.com](https://juce.com/get-juce/download)
2. Download JUCE 8.x (latest stable release)
3. Extract to a permanent location (e.g., `~/Development/JUCE` or `C:\JUCE`)
4. Note the installation path for project setup

#### Option 2: Git Clone
```bash
git clone https://github.com/juce-framework/JUCE.git
cd JUCE
git checkout 8.0.0  # or latest 8.x release
```

### Platform-Specific Setup

#### macOS Setup
1. **Install Xcode** from Mac App Store
2. **Install Command Line Tools**:
   ```bash
   xcode-select --install
   ```
3. **Open Project**:
   ```bash
   open "Builds/MacOSX/14bit Virtual Midi Controller.xcodeproj"
   ```
4. **Configure JUCE Path** (if needed):
   - In Xcode: Project Settings → Build Settings → Search Paths
   - Verify JUCE module paths point to your JUCE installation

#### Windows Setup
1. **Install Visual Studio 2019+** with C++ development workload
2. **Open Project**:
   ```
   Builds/VisualStudio2022/14bit Virtual Midi Controller.sln
   ```
3. **Configure JUCE Path** (if needed):
   - Right-click project → Properties → C/C++ → Additional Include Directories
   - Verify paths point to your JUCE installation

### Building the Application

#### macOS Build
```bash
# Command line build
cd "Builds/MacOSX"
xcodebuild -project "14bit Virtual Midi Controller.xcodeproj" -scheme "14bit Virtual Midi Controller - App" -configuration Release build

# Or use Xcode GUI
# Open project → Select scheme → Product → Build (⌘+B)
```

#### Windows Build
```cmd
# Command line build (from VS Developer Command Prompt)
cd "Builds\VisualStudio2022"
msbuild "14bit Virtual Midi Controller.sln" /p:Configuration=Release /p:Platform=x64

# Or use Visual Studio GUI
# Open solution → Build → Build Solution (Ctrl+Shift+B)
```

### Running the Application

#### macOS
```bash
# Run from Xcode or navigate to build output
open "Builds/MacOSX/build/Release/14bit Virtual Midi Controller.app"
```

#### Windows
```cmd
# Run from Visual Studio or navigate to build output
"Builds\VisualStudio2022\x64\Release\App\14bit Virtual Midi Controller.exe"
```

## 🎹 Usage Guide

### Basic Operation
1. **Launch Application**: Open the built executable
2. **Connect MIDI Device**: Use system MIDI settings or the app's device selection
3. **Configure Sliders**: Right-click any slider for settings and automation options
4. **MIDI Learn**: Click "Learn" button, then click any control to map to incoming MIDI

### Key Workflow Features
- **Settings Window**: Configure individual slider ranges, units, and behavior
- **Automation Config Management**: Save/load complex automation setups
- **Preset System**: Store complete controller configurations
- **Bank Switching**: Toggle between 4-slider and 8-slider modes
- **Real-time MIDI Monitor**: Debug MIDI communication

### MIDI Configuration
- **Output Channel**: Configurable MIDI output channel (1-16)
- **CC Assignments**: Individual CC numbers per slider (0-127)
- **Resolution Mode**: 7-bit or 14-bit output per slider
- **Feedback Prevention**: Automatic filtering of own MIDI output

## 🏗️ Architecture Overview

### Core Systems
- **MIDI Processing**: True 14-bit resolution with device management
- **Slider Interaction**: Hardware-realistic mouse and keyboard control
- **Display Management**: Custom range mapping with automatic precision
- **Automation Engine**: 16 concurrent automations with curve shaping
- **Settings Management**: Modular tabbed interface with real-time application
- **Visual Rendering**: Blueprint technical drawing style throughout

### Key Components
- **DebugMidiController**: Main application controller (1200+ lines)
- **SimpleSliderControl**: Individual slider component (800+ lines)
- **AutomationEngine**: Time-based automation system
- **MidiManager**: MIDI I/O and device management
- **CustomLookAndFeel**: Blueprint visual styling (400+ lines)

For detailed technical documentation, see `architecture.md`.

## 🔧 Development

### Project Structure
```
14bit Virtual Midi Controller/
├── Source/                          # All application source code
│   ├── Core/                        # Business logic and data management
│   ├── UI/                          # User interface components
│   ├── Components/                  # Specialized UI components
│   ├── Graphics/                    # Visual rendering utilities
│   ├── Main Components/             # Primary application components
│   ├── Visual Components/           # Custom UI elements
│   └── Main.cpp                     # Application entry point
├── Assets/                          # Application resources
├── Builds/                          # Platform-specific build files
│   ├── MacOSX/                      # Xcode project
│   └── VisualStudio2022/           # Visual Studio solution
└── JuceLibraryCode/                 # JUCE framework integration
```

### Building from Source
1. **Clone Repository**: `git clone [repository-url]`
2. **Install JUCE**: Follow installation instructions above
3. **Configure Project**: Update JUCE paths if necessary
4. **Build**: Use platform-specific build instructions
5. **Run**: Execute built application

### JUCE Project Configuration
- **Project Type**: Desktop Application
- **JUCE Modules**: Core, Audio Basics, Audio Devices, GUI Basics, GUI Extra
- **C++ Standard**: C++17
- **Target Platforms**: macOS 10.13+, Windows 10+

## 🐛 Troubleshooting

### Common Issues

#### JUCE Framework Not Found
- **Symptom**: Build errors about missing JUCE headers
- **Solution**: Verify JUCE installation path in project settings
- **Details**: Update include paths to point to your JUCE installation

#### MIDI Device Not Detected
- **Symptom**: No MIDI output or input detection
- **Solution**: Check system MIDI settings and device permissions
- **macOS**: System Preferences → Security & Privacy → Privacy → Automation
- **Windows**: Ensure MIDI device drivers are installed

#### Application Won't Launch
- **Symptom**: Crash on startup or won't open
- **Solution**: Check for missing system dependencies
- **macOS**: Verify code signing and security settings
- **Windows**: Install Visual C++ Redistributable if needed

#### Build Errors
- **Symptom**: Compilation failures
- **Solution**: Verify correct JUCE version (8.x) and C++ standard (C++17)
- **Details**: Check compiler toolchain compatibility

### Debug Mode
For development and troubleshooting:
1. Build in Debug configuration
2. Enable console output for detailed MIDI logging
3. Use MIDI Monitor feature for real-time debugging

## 📋 Requirements

### System Requirements
- **macOS**: 10.13 High Sierra or later
- **Windows**: Windows 10 version 1809 or later
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 50MB for application
- **MIDI**: USB MIDI device or virtual MIDI setup

### Development Requirements
- **JUCE Framework**: 8.x (latest stable)
- **macOS Development**: Xcode 14+, Command Line Tools
- **Windows Development**: Visual Studio 2019+, Windows SDK
- **C++ Standard**: C++17 or later

## 📝 License

This project uses the JUCE framework. Please review JUCE licensing terms for your use case:
- **JUCE Personal License**: Free for personal/educational use
- **JUCE Commercial License**: Required for commercial distribution

## 🤝 Contributing

This is primarily a personal project. For significant contributions:
1. Review `architecture.md` for technical guidelines
2. Follow established code patterns and naming conventions
3. Maintain modular architecture with clean component boundaries
4. Test thoroughly on both macOS and Windows platforms

## 📞 Support

For technical issues:
1. Check troubleshooting section above
2. Verify JUCE framework setup
3. Review console output for error details
4. Ensure MIDI device compatibility

---

**Built with JUCE Framework** | **Blueprint Visual Design** | **Professional MIDI Control**