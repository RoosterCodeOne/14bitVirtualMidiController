// AutomationVisualizer.h - Blueprint-style automation curve visualization
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "Graphics/CurveCalculator.h"
#include "Graphics/VisualizerRenderer.h"
#include "UI/GlobalUIScale.h"

//==============================================================================
class AutomationVisualizer : public juce::Component, public juce::Timer, public GlobalUIScale::ScaleChangeListener
{
public:
    enum class VisualizerState
    {
        Idle,           // Shows curve based on current knob settings
        Locked,         // Automation running - curve locked, ball moving
        Stopped         // Automation stopped - loads current knob settings
    };
    
    AutomationVisualizer()
        : currentState(VisualizerState::Idle),
          delayTime(0.0), attackTime(1.0), returnTime(0.0), curveValue(1.0),
          ballPosition(0.0f, 0.0f), showBall(false),
          animationStartTime(0.0), totalAnimationDuration(0.0),
          animationDelayTime(0.0), animationAttackTime(1.0), animationReturnTime(0.0)
    {
        // Note: Size will be set by parent component (AutomationControlPanel)
        // Register for scale change notifications for internal drawing scaling
        GlobalUIScale::getInstance().addScaleChangeListener(this);
        
        updateCurvePoints();
        // Force initial repaint to show default curve immediately on startup
        repaint();
    }
    
    ~AutomationVisualizer()
    {
        stopTimer();
        
        // Unregister from scale change notifications
        GlobalUIScale::getInstance().removeScaleChangeListener(this);
    }
    
    // Core interface methods
    void setParameters(double delay, double attack, double returnTime, double curve)
    {
        if (currentState != VisualizerState::Locked)
        {
            delayTime = delay;
            attackTime = attack;
            this->returnTime = returnTime;
            curveValue = curve;
            updateCurvePoints();
            repaint();
        }
    }
    
    // Start self-contained animation using provided knob values
    void startAnimation(double currentDelay, double currentAttack, double currentReturn)
    {
        if (currentState != VisualizerState::Locked) return;
        
        // Use the actual current knob values for animation timing
        animationDelayTime = currentDelay;
        animationAttackTime = currentAttack;
        animationReturnTime = currentReturn;
        
        // Record animation start time
        animationStartTime = juce::Time::getMillisecondCounterHiRes();
        
        // Calculate total animation duration from actual knob values
        totalAnimationDuration = animationDelayTime + animationAttackTime + animationReturnTime;
        
        // Debug logging
        DBG("AutomationVisualizer: Starting animation - Delay: " << animationDelayTime 
            << "s, Attack: " << animationAttackTime << "s, Return: " << animationReturnTime 
            << "s, Total: " << totalAnimationDuration << "s");
        
        // Start internal animation timer
        if (totalAnimationDuration > 0.0)
        {
            startTimer(16); // 60fps animation
        }
    }
    
    void stopAnimation()
    {
        stopTimer();
        showBall = false;
        repaint();
    }
    
    void setVisualizerState(VisualizerState state)
    {
        if (currentState != state)
        {
            currentState = state;
            
            switch (state)
            {
                case VisualizerState::Idle:
                    showBall = false;
                    stopTimer();
                    break;
                    
                case VisualizerState::Locked:
                    lockCurveForAutomation(delayTime, attackTime, returnTime);
                    break;
                    
                case VisualizerState::Stopped:
                    unlockCurve();
                    break;
            }
            repaint();
        }
    }
    
    void lockCurveForAutomation(double currentDelay, double currentAttack, double currentReturn)
    {
        currentState = VisualizerState::Locked;
        showBall = true;
        startAnimation(currentDelay, currentAttack, currentReturn); // Start self-contained animation
        repaint();
    }
    
    void unlockCurve()
    {
        currentState = VisualizerState::Idle;
        stopAnimation(); // Stop self-contained animation
        updateCurvePoints(); // Reload current knob settings
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        renderer.drawVisualizer(g, bounds, currentCurvePoints, delayTime, attackTime, returnTime,
                               showBall && currentState == VisualizerState::Locked, ballPosition);
    }
    
    void resized() override
    {
        // Recalculate curve when component bounds are set/changed
        updateCurvePoints();
        repaint();
    }
    
    void timerCallback() override
    {
        // Self-contained animation using knob values
        if (currentState == VisualizerState::Locked)
        {
            updateBallPositionFromTime();
            repaint();
        }
    }
    
    // Scale change notification implementation
    void scaleFactorChanged(float newScale) override
    {
        // Note: Size is managed by parent component
        // We only need to update internal drawing elements and recalculate curve points
        updateCurvePoints(); // Recalculate curve points for new size
        repaint(); // Redraw with new scaling
    }
    
private:
    // State variables
    VisualizerState currentState;
    double delayTime, attackTime, returnTime, curveValue;
    juce::Point<float> ballPosition;
    bool showBall;
    
    // Self-contained animation timing
    double animationStartTime;
    double totalAnimationDuration;
    double animationDelayTime, animationAttackTime, animationReturnTime;
    
    // Modular components
    CurveCalculator curveCalculator;
    VisualizerRenderer renderer;
    CurveCalculator::CurvePoints currentCurvePoints;
    
    // Ball positioning using curve calculator
    void updateBallPositionFromTime()
    {
        if (totalAnimationDuration <= 0.0) return;
        
        // Calculate elapsed animation time
        double currentTime = juce::Time::getMillisecondCounterHiRes();
        double elapsed = (currentTime - animationStartTime) / 1000.0; // Convert to seconds
        
        // Use curve calculator to determine ball position
        ballPosition = curveCalculator.calculateBallPosition(currentCurvePoints, elapsed,
                                                            animationDelayTime, animationAttackTime, animationReturnTime, curveValue);
        
        // Stop animation when complete
        if (elapsed >= totalAnimationDuration)
        {
            stopTimer();
        }
    }
    
    void updateCurvePoints()
    {
        auto bounds = getLocalBounds().toFloat();
        currentCurvePoints = curveCalculator.calculateCurvePoints(bounds, delayTime, attackTime, returnTime, curveValue);
    }
    
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationVisualizer)
};