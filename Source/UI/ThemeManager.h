#pragma once

#include <JuceHeader.h>
#include <functional>
#include <vector>
#include <algorithm>

/**
 * ThemeManager - Singleton class providing application-wide theme management
 *
 * This header-only implementation provides:
 * - Multiple theme support: Dark (Blueprint), Light, Auto (system-based)
 * - Complete color palette definitions for each theme
 * - System theme detection (macOS/Windows)
 * - Theme change notification system
 * - Integration with PresetManager for persistence
 */
class ThemeManager
{
public:
    // Theme types
    enum class ThemeType
    {
        Dark = 0,
        Light = 1,
        Auto = 2
    };

    // Complete theme color palette
    struct ThemePalette
    {
        // Base backgrounds
        juce::Colour background;
        juce::Colour panel;
        juce::Colour windowBackground;
        juce::Colour sectionBackground;

        // Accents and lines
        juce::Colour blueprintLines;
        juce::Colour active;

        // Text colors
        juce::Colour textPrimary;
        juce::Colour textSecondary;

        // Status colors
        juce::Colour warning;
        juce::Colour success;
        juce::Colour inactive;

        // Additional UI elements
        juce::Colour sliderTrack;
        juce::Colour sliderThumb;
        juce::Colour border;

        ThemePalette() = default;
    };

    // Theme change listener interface
    class ThemeChangeListener
    {
    public:
        virtual ~ThemeChangeListener() = default;
        virtual void themeChanged(ThemeType newTheme, const ThemePalette& palette) = 0;
    };

    // Singleton access
    static ThemeManager& getInstance()
    {
        static ThemeManager instance;
        return instance;
    }

    // Core theme methods
    ThemeType getThemeType() const { return currentThemeType; }
    ThemeType getResolvedThemeType() const { return resolvedThemeType; }
    const ThemePalette& getCurrentPalette() const { return currentPalette; }

    void setTheme(ThemeType type)
    {
        if (currentThemeType != type)
        {
            currentThemeType = type;
            updateResolvedTheme();
        }
    }

    // Get theme name for UI display
    juce::String getThemeName(ThemeType type) const
    {
        switch (type)
        {
            case ThemeType::Dark: return "Dark";
            case ThemeType::Light: return "Light";
            case ThemeType::Auto: return "Auto";
            default: return "Unknown";
        }
    }

    // Get current theme name
    juce::String getCurrentThemeName() const
    {
        return getThemeName(currentThemeType);
    }

    // System theme detection
    bool isDarkModeEnabled() const
    {
        #if JUCE_MAC
            return isSystemDarkMode_macOS();
        #elif JUCE_WINDOWS
            return isSystemDarkMode_Windows();
        #else
            return true; // Default to dark mode on other platforms
        #endif
    }

    // Listener management
    void addThemeChangeListener(ThemeChangeListener* listener)
    {
        if (listener != nullptr && std::find(listeners.begin(), listeners.end(), listener) == listeners.end())
        {
            listeners.push_back(listener);
        }
    }

    void removeThemeChangeListener(ThemeChangeListener* listener)
    {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }

    // Persistence methods
    juce::String getThemeAsString() const
    {
        return getThemeName(currentThemeType);
    }

    void setThemeFromString(const juce::String& themeName)
    {
        if (themeName.equalsIgnoreCase("Dark"))
            setTheme(ThemeType::Dark);
        else if (themeName.equalsIgnoreCase("Light"))
            setTheme(ThemeType::Light);
        else if (themeName.equalsIgnoreCase("Auto"))
            setTheme(ThemeType::Auto);
    }

    int getThemeIndex() const
    {
        return static_cast<int>(currentThemeType);
    }

    void setThemeByIndex(int index)
    {
        if (index >= 0 && index <= 2)
        {
            setTheme(static_cast<ThemeType>(index));
        }
    }

    // Force update (useful for system theme changes in Auto mode)
    void refreshTheme()
    {
        updateResolvedTheme();
    }

    // Start/stop monitoring system theme changes (for Auto mode)
    void startSystemThemeMonitoring()
    {
        if (systemThemeTimer == nullptr)
        {
            systemThemeTimer = std::make_unique<SystemThemeTimer>(*this);
            systemThemeTimer->startTimer(1000); // Check every second
        }
    }

    void stopSystemThemeMonitoring()
    {
        if (systemThemeTimer != nullptr)
        {
            systemThemeTimer->stopTimer();
            systemThemeTimer.reset();
        }
    }

private:
    ThemeManager()
        : currentThemeType(ThemeType::Dark)
        , resolvedThemeType(ThemeType::Dark)
    {
        initializeThemePalettes();
        // Initialize currentPalette to dark theme by default
        currentPalette = darkPalette;
        updateResolvedTheme();
    }

    ~ThemeManager()
    {
        stopSystemThemeMonitoring();
    }

    // Prevent copying
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void initializeThemePalettes()
    {
        // Dark Theme (Blueprint style - existing theme)
        darkPalette.background = juce::Colour(0xFF1a1a2e);
        darkPalette.panel = juce::Colour(0xFF16213e);
        darkPalette.windowBackground = juce::Colour(0xFF1e2344);
        darkPalette.sectionBackground = juce::Colour(0xFF242951);
        darkPalette.blueprintLines = juce::Colour(0xFF00d4ff);
        darkPalette.textPrimary = juce::Colour(0xFFe8e8e8);
        darkPalette.textSecondary = juce::Colour(0xFFa0b4cc);
        darkPalette.active = juce::Colour(0xFF00d4ff);
        darkPalette.warning = juce::Colour(0xFFff8c42);
        darkPalette.success = juce::Colour(0xFF4ade80);
        darkPalette.inactive = juce::Colour(0xFF4a5568);
        darkPalette.sliderTrack = juce::Colour(0xFF2d3748);
        darkPalette.sliderThumb = juce::Colour(0xFFe2e8f0);
        darkPalette.border = juce::Colour(0xFF4a5568);

        // Light Theme (Professional clean style with darker backgrounds)
        lightPalette.background = juce::Colour(0xFFD8D8D8);      // Darker gray base
        lightPalette.panel = juce::Colour(0xFFE8E8E8);           // Light gray for panels
        lightPalette.windowBackground = juce::Colour(0xFFDDDDDD); // Medium-light gray
        lightPalette.sectionBackground = juce::Colour(0xFFE5E5E5); // Slightly lighter gray
        lightPalette.blueprintLines = juce::Colour(0xFF0891B2);  // Darker cyan for visibility
        lightPalette.textPrimary = juce::Colour(0xFF2C2C2C);
        lightPalette.textSecondary = juce::Colour(0xFF64748B);
        lightPalette.active = juce::Colour(0xFF0891B2);  // Darker cyan
        lightPalette.warning = juce::Colour(0xFFD97706);  // Darker amber
        lightPalette.success = juce::Colour(0xFF059669);  // Darker green
        lightPalette.inactive = juce::Colour(0xFFCBD5E1);
        lightPalette.sliderTrack = juce::Colour(0xFFCCCCCC);     // Darker track
        lightPalette.sliderThumb = juce::Colour(0xFFF0F0F0);     // Off-white thumb
        lightPalette.border = juce::Colour(0xFFB0B0B0);          // Darker border
    }

    void updateResolvedTheme()
    {
        ThemeType newResolvedType;

        if (currentThemeType == ThemeType::Auto)
        {
            newResolvedType = isDarkModeEnabled() ? ThemeType::Dark : ThemeType::Light;
        }
        else
        {
            newResolvedType = currentThemeType;
        }

        if (resolvedThemeType != newResolvedType)
        {
            resolvedThemeType = newResolvedType;
            currentPalette = (resolvedThemeType == ThemeType::Dark) ? darkPalette : lightPalette;
            notifyThemeChangeListeners();
        }
    }

    void notifyThemeChangeListeners()
    {
        // Create a copy of listeners to avoid issues if listeners are modified during notification
        auto listenersCopy = listeners;
        for (auto* listener : listenersCopy)
        {
            if (listener != nullptr)
            {
                listener->themeChanged(resolvedThemeType, currentPalette);
            }
        }
    }

    // Platform-specific system theme detection
    #if JUCE_MAC
    bool isSystemDarkMode_macOS() const
    {
        // Use JUCE's cross-platform dark mode detection
        return juce::Desktop::getInstance().isDarkModeActive();
    }
    #endif

    #if JUCE_WINDOWS
    bool isSystemDarkMode_Windows() const
    {
        // Use JUCE's cross-platform dark mode detection
        return juce::Desktop::getInstance().isDarkModeActive();
    }
    #endif

    // Timer class for monitoring system theme changes
    class SystemThemeTimer : public juce::Timer
    {
    public:
        SystemThemeTimer(ThemeManager& tm) : themeManager(tm), lastDarkModeState(tm.isDarkModeEnabled()) {}

        void timerCallback() override
        {
            if (themeManager.getThemeType() == ThemeType::Auto)
            {
                bool currentDarkMode = themeManager.isDarkModeEnabled();
                if (currentDarkMode != lastDarkModeState)
                {
                    lastDarkModeState = currentDarkMode;
                    themeManager.refreshTheme();
                }
            }
        }

    private:
        ThemeManager& themeManager;
        bool lastDarkModeState;
    };

    ThemeType currentThemeType;
    ThemeType resolvedThemeType;
    ThemePalette currentPalette;
    ThemePalette darkPalette;
    ThemePalette lightPalette;

    std::vector<ThemeChangeListener*> listeners;
    std::unique_ptr<SystemThemeTimer> systemThemeTimer;
};

// Global convenience functions for easy access
namespace Theme
{
    inline ThemeManager& get() { return ThemeManager::getInstance(); }
    inline const ThemeManager::ThemePalette& palette() { return ThemeManager::getInstance().getCurrentPalette(); }
    inline bool isDark() { return ThemeManager::getInstance().getResolvedThemeType() == ThemeManager::ThemeType::Dark; }
    inline bool isLight() { return ThemeManager::getInstance().getResolvedThemeType() == ThemeManager::ThemeType::Light; }
}
