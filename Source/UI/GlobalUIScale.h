#pragma once

#include <JuceHeader.h>
#include <functional>
#include <vector>

/**
 * GlobalUIScale - Singleton class providing application-wide UI scaling functionality
 * 
 * This header-only implementation provides:
 * - Scale factors: 75%, 100%, 125%, 150%, 175%, 200%
 * - Template helper functions for scaling numeric values
 * - Font scaling with proportional sizing
 * - Scale change notification system
 * - Integration with existing PresetManager for persistence
 */
class GlobalUIScale
{
public:
    // Scale factor options
    static constexpr float SCALE_75 = 0.75f;
    static constexpr float SCALE_100 = 1.0f;
    static constexpr float SCALE_125 = 1.25f;
    static constexpr float SCALE_150 = 1.5f;
    static constexpr float SCALE_175 = 1.75f;
    static constexpr float SCALE_200 = 2.0f;
    
    // Available scale factors array for UI dropdowns
    static constexpr float AVAILABLE_SCALES[] = {
        SCALE_75, SCALE_100, SCALE_125, SCALE_150, SCALE_175, SCALE_200
    };
    static constexpr int NUM_SCALE_OPTIONS = 6;
    
    // Scale change listener interface
    class ScaleChangeListener
    {
    public:
        virtual ~ScaleChangeListener() = default;
        virtual void scaleFactorChanged(float newScale) = 0;
    };
    
    // Singleton access
    static GlobalUIScale& getInstance()
    {
        static GlobalUIScale instance;
        return instance;
    }
    
    // Core scaling methods
    float getScaleFactor() const { return currentScale; }
    
    void setScaleFactor(float scale)
    {
        // Validate scale factor
        bool validScale = false;
        for (int i = 0; i < NUM_SCALE_OPTIONS; ++i)
        {
            if (std::abs(scale - AVAILABLE_SCALES[i]) < 0.01f)
            {
                validScale = true;
                break;
            }
        }
        
        if (!validScale)
        {
            DBG("Invalid scale factor: " + juce::String(scale) + ", defaulting to 100%");
            scale = SCALE_100;
        }
        
        if (std::abs(currentScale - scale) > 0.01f)
        {
            currentScale = scale;
            notifyScaleChangeListeners();
        }
    }
    
    // Template scaling functions for different numeric types
    template<typename T>
    T getScaled(T value) const
    {
        static_assert(std::is_arithmetic_v<T>, "T must be a numeric type");
        return static_cast<T>(value * currentScale);
    }
    
    // Specialized font scaling
    juce::Font getScaledFont(float baseFontSize) const
    {
        return juce::Font(baseFontSize * currentScale);
    }
    
    // Font scaling with specific font family
    juce::Font getScaledFont(const juce::String& fontName, float baseFontSize, int styleFlags = juce::Font::plain) const
    {
        return juce::Font(fontName, baseFontSize * currentScale, styleFlags);
    }
    
    // Listener management
    void addScaleChangeListener(ScaleChangeListener* listener)
    {
        if (listener != nullptr && std::find(listeners.begin(), listeners.end(), listener) == listeners.end())
        {
            listeners.push_back(listener);
        }
    }
    
    void removeScaleChangeListener(ScaleChangeListener* listener)
    {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }
    
    // Utility methods for UI components
    juce::String getScaleDisplayString() const
    {
        return juce::String(static_cast<int>(currentScale * 100)) + "%";
    }
    
    int getScaleIndex() const
    {
        for (int i = 0; i < NUM_SCALE_OPTIONS; ++i)
        {
            if (std::abs(currentScale - AVAILABLE_SCALES[i]) < 0.01f)
                return i;
        }
        return 1; // Default to 100%
    }
    
    void setScaleByIndex(int index)
    {
        if (index >= 0 && index < NUM_SCALE_OPTIONS)
        {
            setScaleFactor(AVAILABLE_SCALES[index]);
        }
    }
    
    // Persistence methods (to be integrated with PresetManager)
    juce::var getScaleAsVar() const
    {
        return juce::var(currentScale);
    }
    
    void setScaleFromVar(const juce::var& var)
    {
        if (var.isDouble() || var.isInt())
        {
            setScaleFactor(static_cast<float>(var));
        }
    }
    
    // Convenience methods for common UI calculations
    int getScaledCornerRadius(int baseRadius = 2) const
    {
        return getScaled(baseRadius);
    }
    
    float getScaledLineThickness(float baseThickness = 1.0f) const
    {
        return getScaled(baseThickness);
    }
    
    int getScaledSpacing(int baseSpacing = 5) const
    {
        return getScaled(baseSpacing);
    }
    
    // Reset to default scale
    void resetToDefault()
    {
        setScaleFactor(SCALE_100);
    }
    
private:
    GlobalUIScale() : currentScale(SCALE_100) {}
    ~GlobalUIScale() = default;
    
    // Prevent copying
    GlobalUIScale(const GlobalUIScale&) = delete;
    GlobalUIScale& operator=(const GlobalUIScale&) = delete;
    
    float currentScale;
    std::vector<ScaleChangeListener*> listeners;
    
    void notifyScaleChangeListeners()
    {
        // Create a copy of listeners to avoid issues if listeners are modified during notification
        auto listenersCopy = listeners;
        for (auto* listener : listenersCopy)
        {
            if (listener != nullptr)
            {
                listener->scaleFactorChanged(currentScale);
            }
        }
    }
};

// Global convenience functions for easy access
namespace UIScale
{
    inline GlobalUIScale& get() { return GlobalUIScale::getInstance(); }
    
    template<typename T>
    inline T scaled(T value) { return GlobalUIScale::getInstance().getScaled(value); }
    
    inline juce::Font scaledFont(float size) { return GlobalUIScale::getInstance().getScaledFont(size); }
    
    inline float factor() { return GlobalUIScale::getInstance().getScaleFactor(); }
}