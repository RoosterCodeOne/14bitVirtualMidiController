#pragma once

#include <JuceHeader.h>
#include <functional>
#include <vector>
#include <algorithm>

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
    
    // Screen dimension and scaling constraints
    struct ScreenConstraints
    {
        float minScale;
        float maxScale;
        juce::Rectangle<int> availableArea;
        bool isValid;
        
        ScreenConstraints() : minScale(SCALE_75), maxScale(SCALE_200), isValid(false) {}
    };
    
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
    
    // Screen-aware scaling methods
    ScreenConstraints calculateScreenConstraints(juce::Component* referenceComponent = nullptr) const
    {
        ScreenConstraints constraints;
        
        try 
        {
            // Get display information
            auto displays = juce::Desktop::getInstance().getDisplays();
            if (displays.displays.isEmpty())
            {
                DBG("No displays found, using default constraints");
                return constraints;
            }
            
            // Use primary display or the display containing the reference component
            auto primaryDisplay = displays.getPrimaryDisplay();
            if (referenceComponent)
            {
                auto componentCentre = referenceComponent->getBounds().getCentre();
                for (auto& display : displays.displays)
                {
                    if (display.totalArea.contains(componentCentre))
                    {
                        primaryDisplay = &display;
                        break;
                    }
                }
            }
            
            if (!primaryDisplay)
            {
                DBG("Primary display not found, using default constraints");
                return constraints;
            }
            
            // Calculate available screen area (subtract dock/taskbar areas)
            constraints.availableArea = primaryDisplay->userArea;
            
            // Calculate minimum scale based on minimum readable UI size
            // Minimum window dimensions at 100% scale: 4-slider mode = 490x660, 8-slider = 970x660
            const int minWindowWidth4 = 490;
            const int minWindowHeight = 660;
            const int minWindowWidth8 = 970;
            
            // Account for window decorations and OS elements (estimated)
            const int decorationPadding = 100;
            int usableWidth = constraints.availableArea.getWidth() - decorationPadding;
            int usableHeight = constraints.availableArea.getHeight() - decorationPadding;
            
            // Calculate maximum scale that allows both 4-slider and 8-slider modes
            float maxScaleFor4Slider = static_cast<float>(usableWidth) / minWindowWidth4;
            float maxScaleFor8Slider = static_cast<float>(usableWidth) / minWindowWidth8;
            float maxScaleForHeight = static_cast<float>(usableHeight) / minWindowHeight;
            
            // Use the most restrictive constraint (8-slider mode requires more width)
            constraints.maxScale = std::min({maxScaleFor8Slider, maxScaleForHeight, SCALE_200});
            
            // Ensure minimum scale for readability
            constraints.minScale = std::max(SCALE_75, 0.5f);
            
            // Round to nearest available scale factor
            constraints.maxScale = findNearestValidScale(constraints.maxScale, false);
            constraints.minScale = findNearestValidScale(constraints.minScale, true);
            
            constraints.isValid = true;
            
            DBG("Screen constraints calculated - Available area: " + 
                juce::String(constraints.availableArea.getWidth()) + "x" + 
                juce::String(constraints.availableArea.getHeight()) + 
                ", Scale range: " + juce::String(constraints.minScale, 2) + 
                " to " + juce::String(constraints.maxScale, 2));
        }
        catch (const std::exception& e)
        {
            DBG("Exception calculating screen constraints: " + juce::String(e.what()));
        }
        
        return constraints;
    }
    
    // Get current screen constraints
    ScreenConstraints getCurrentScreenConstraints() const { return cachedConstraints; }
    
    // Update cached screen constraints
    void updateScreenConstraints(juce::Component* referenceComponent = nullptr)
    {
        cachedConstraints = calculateScreenConstraints(referenceComponent);
    }
    
    void setScaleFactor(float scale)
    {
        setScaleFactorWithConstraints(scale);
    }
    
    // Screen-aware scale setting with automatic constraint validation
    float setScaleFactorWithConstraints(float scale, juce::Component* referenceComponent = nullptr, bool showUserFeedback = false)
    {
        // Update constraints if needed
        if (!cachedConstraints.isValid)
        {
            updateScreenConstraints(referenceComponent);
        }
        
        float originalScale = scale;
        float clampedScale = scale;
        
        // Apply screen-based constraints if available
        if (cachedConstraints.isValid)
        {
            clampedScale = juce::jlimit(cachedConstraints.minScale, cachedConstraints.maxScale, scale);
        }
        
        // Validate against available scale factors
        bool validScale = false;
        for (int i = 0; i < NUM_SCALE_OPTIONS; ++i)
        {
            if (std::abs(clampedScale - AVAILABLE_SCALES[i]) < 0.01f)
            {
                validScale = true;
                break;
            }
        }
        
        if (!validScale)
        {
            // Find nearest valid scale
            clampedScale = findNearestValidScale(clampedScale, false);
        }
        
        // Check if scale was clamped
        bool wasClamped = std::abs(originalScale - clampedScale) > 0.01f;
        
        if (wasClamped && showUserFeedback)
        {
            showScalingLimitFeedback(originalScale, clampedScale);
        }
        
        if (std::abs(currentScale - clampedScale) > 0.01f)
        {
            currentScale = clampedScale;
            notifyScaleChangeListeners();
        }
        
        return clampedScale;
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
            setScaleFactorWithConstraints(AVAILABLE_SCALES[index]);
        }
    }
    
    // Get available scale options respecting current screen constraints
    std::vector<float> getValidScaleOptions(juce::Component* referenceComponent = nullptr) const
    {
        std::vector<float> validScales;
        
        // Update constraints if needed
        auto constraints = cachedConstraints.isValid ? cachedConstraints : calculateScreenConstraints(referenceComponent);
        
        for (int i = 0; i < NUM_SCALE_OPTIONS; ++i)
        {
            float scale = AVAILABLE_SCALES[i];
            if (!constraints.isValid || 
                (scale >= constraints.minScale - 0.01f && scale <= constraints.maxScale + 0.01f))
            {
                validScales.push_back(scale);
            }
        }
        
        // Ensure at least one valid scale exists
        if (validScales.empty())
        {
            validScales.push_back(SCALE_100);
        }
        
        return validScales;
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
    mutable ScreenConstraints cachedConstraints;
    
    // Helper function to find nearest valid scale
    float findNearestValidScale(float targetScale, bool preferLower) const
    {
        float nearestScale = AVAILABLE_SCALES[0];
        float minDistance = std::abs(targetScale - nearestScale);
        
        for (int i = 1; i < NUM_SCALE_OPTIONS; ++i)
        {
            float distance = std::abs(targetScale - AVAILABLE_SCALES[i]);
            if (distance < minDistance || 
                (distance == minDistance && 
                 ((preferLower && AVAILABLE_SCALES[i] < nearestScale) ||
                  (!preferLower && AVAILABLE_SCALES[i] > nearestScale))))
            {
                nearestScale = AVAILABLE_SCALES[i];
                minDistance = distance;
            }
        }
        
        return nearestScale;
    }
    
    // Show user feedback when scaling is limited
    void showScalingLimitFeedback(float requestedScale, float actualScale) const
    {
        juce::String message;
        auto constraints = getCurrentScreenConstraints();
        
        if (actualScale < requestedScale)
        {
            message = "UI Scale limited to " + juce::String(static_cast<int>(actualScale * 100)) + 
                     "% (requested " + juce::String(static_cast<int>(requestedScale * 100)) + 
                     "%) due to screen size constraints";
        }
        else
        {
            message = "UI Scale increased to " + juce::String(static_cast<int>(actualScale * 100)) + 
                     "% (requested " + juce::String(static_cast<int>(requestedScale * 100)) + 
                     "%) to maintain readability";
        }
        
        if (constraints.isValid)
        {
            message += " [Screen: " + juce::String(constraints.availableArea.getWidth()) + "x" + 
                      juce::String(constraints.availableArea.getHeight()) + 
                      ", Range: " + juce::String(static_cast<int>(constraints.minScale * 100)) + 
                      "%-" + juce::String(static_cast<int>(constraints.maxScale * 100)) + "%]";
        }
        
        DBG("Adaptive Scaling: " + message);
        
        // Future: Could implement a subtle tooltip or status bar notification here
        // For now, debug output provides sufficient feedback for development and troubleshooting
    }
    
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