// AutomationVisualizer.h - Blueprint-style automation curve visualization
#pragma once
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

//==============================================================================
class AutomationVisualizer : public juce::Component, public juce::Timer
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
          automationProgress(0.0), ballPosition(0.0f, 0.0f), showBall(false)
    {
        setSize(140, 80); // Default size for SimpleSliderControl integration
        calculateCurvePoints();
        // Force initial repaint to show default curve immediately on startup
        repaint();
    }
    
    ~AutomationVisualizer()
    {
        stopTimer();
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
            calculateCurvePoints();
            repaint();
        }
    }
    
    void setAutomationProgress(double progress)
    {
        automationProgress = juce::jlimit(0.0, 1.0, progress);
        if (currentState == VisualizerState::Locked)
        {
            updateBallPosition();
            repaint();
        }
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
                    lockCurveForAutomation();
                    break;
                    
                case VisualizerState::Stopped:
                    unlockCurve();
                    break;
            }
            repaint();
        }
    }
    
    void lockCurveForAutomation()
    {
        currentState = VisualizerState::Locked;
        showBall = true;
        startTimer(16); // 60fps animation
        repaint();
    }
    
    void unlockCurve()
    {
        currentState = VisualizerState::Idle;
        showBall = false;
        stopTimer();
        calculateCurvePoints(); // Reload current knob settings
        repaint();
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Blueprint background
        g.fillAll(BlueprintColors::background);
        
        // Draw technical grid
        drawBlueprintGrid(g, bounds);
        
        // Draw automation curve
        drawAutomationCurve(g, bounds);
        
        // Draw phase breakpoints
        drawPhaseBreakpoints(g, bounds);
        
        // Draw moving ball if automation is active
        if (showBall && currentState == VisualizerState::Locked)
        {
            drawMovingBall(g);
        }
        
        // Draw border outline
        g.setColour(BlueprintColors::blueprintLines);
        g.drawRect(bounds, 1.0f);
    }
    
    void timerCallback() override
    {
        // Smooth ball animation updates
        if (currentState == VisualizerState::Locked)
        {
            updateBallPosition();
            repaint();
        }
    }
    
private:
    // State variables
    VisualizerState currentState;
    double delayTime, attackTime, returnTime, curveValue;
    double automationProgress;
    juce::Point<float> ballPosition;
    bool showBall;
    
    // Curve calculation
    std::vector<juce::Point<float>> curvePoints;
    juce::Point<float> delayEndPoint, attackEndPoint, returnEndPoint;
    
    void calculateCurvePoints()
    {
        curvePoints.clear();
        auto bounds = getLocalBounds().toFloat().reduced(10.0f); // Margin for grid
            
        // If bounds are invalid (common during construction), use default bounds
        if (bounds.getWidth() <= 0.0f || bounds.getHeight() <= 0.0f)
        {
            bounds = juce::Rectangle<float>(10.0f, 10.0f, 120.0f, 60.0f); // Default working area
        }
        
        // Origin point (bottom-left)
        float originX = bounds.getX();
        float originY = bounds.getBottom();
        curvePoints.push_back({originX, originY});
        
        // Special case: If attack is 0, draw flat horizontal line across entire width
        if (attackTime <= 0.0)
        {
            // Draw horizontal line from left to right edge
            float endX = bounds.getRight();
            for (int i = 1; i <= 20; ++i)
            {
                float t = i / 20.0f;
                float x = originX + (endX - originX) * t;
                curvePoints.push_back({x, originY});
            }
            
            // Set all endpoints to the same Y level (flat line)
            delayEndPoint = {originX + (endX - originX) * 0.33f, originY};
            attackEndPoint = {originX + (endX - originX) * 0.67f, originY};
            returnEndPoint = {endX, originY};
            return;
        }
        
        // Normal curve calculation: Use full grid space efficiently
        float totalWidth = bounds.getWidth();
        float maxHeight = bounds.getHeight();
        
        // Calculate phase proportions (normalize to use full width)
        double totalTime = delayTime + attackTime + returnTime;
        if (totalTime <= 0.0) totalTime = attackTime; // Use attack as minimum
        
        // Calculate phase widths (use full grid width)
        float delayWidth = totalTime > 0 ? (delayTime / totalTime) * totalWidth : 0.0f;
        float attackWidth = totalTime > 0 ? (attackTime / totalTime) * totalWidth : totalWidth;
        float returnWidth = totalTime > 0 ? (returnTime / totalTime) * totalWidth : 0.0f;
        
        // Scale attack height: attack parameter directly maps to Y position
        // Max attack value (30.0) should reach top of grid
        float attackHeight = (attackTime / 30.0f) * maxHeight;
        attackHeight = juce::jlimit(0.0f, maxHeight, attackHeight);
        
        // Phase 1: Delay (horizontal line)
        delayEndPoint = {originX + delayWidth, originY};
        
        if (delayWidth > 0.0f)
        {
            // Add points along delay line
            int delaySteps = juce::jmax(1, (int)(delayWidth / 5.0f)); // More points for longer delays
            for (int i = 1; i <= delaySteps; ++i)
            {
                float t = i / (float)delaySteps;
                curvePoints.push_back({originX + (delayWidth * t), originY});
            }
        }
        
        // Phase 2: Attack (curved upward)
        attackEndPoint = {delayEndPoint.x + attackWidth, originY - attackHeight};
        
        if (attackWidth > 0.0f)
        {
            // Generate curved attack phase using curve parameter
            int attackSteps = 20;
            for (int i = 1; i <= attackSteps; ++i)
            {
                float t = i / (float)attackSteps;
                float curvedT = applyCurve(t, curveValue);
                
                float x = delayEndPoint.x + (attackWidth * t);
                float y = originY - (attackHeight * curvedT);
                curvePoints.push_back({x, y});
            }
        }
        
        // Phase 3: Return (if > 0, slope downward to complete the curve)
        if (returnTime > 0.0 && returnWidth > 0.0f)
        {
            returnEndPoint = {attackEndPoint.x + returnWidth, originY};
            
            // Generate return curve
            int returnSteps = juce::jmax(1, (int)(returnWidth / 5.0f));
            for (int i = 1; i <= returnSteps; ++i)
            {
                float t = i / (float)returnSteps;
                float x = attackEndPoint.x + (returnWidth * t);
                float y = attackEndPoint.y + ((originY - attackEndPoint.y) * t);
                curvePoints.push_back({x, y});
            }
        }
        else
        {
            returnEndPoint = attackEndPoint;
        }
        
        // Ensure curve uses full width by extending to right edge if needed
        float currentEndX = returnEndPoint.x;
        float targetEndX = bounds.getRight();
        
        if (currentEndX < targetEndX && returnTime <= 0.0)
        {
            // Extend horizontally to fill remaining width
            float remainingWidth = targetEndX - currentEndX;
            if (remainingWidth > 1.0f)
            {
                int extensionSteps = juce::jmax(1, (int)(remainingWidth / 5.0f));
                for (int i = 1; i <= extensionSteps; ++i)
                {
                    float t = i / (float)extensionSteps;
                    float x = currentEndX + (remainingWidth * t);
                    curvePoints.push_back({x, attackEndPoint.y});
                }
                returnEndPoint = {targetEndX, attackEndPoint.y};
            }
        }
    }
    
    float applyCurve(float t, double curve)
    {
        // Apply exponential/logarithmic curve based on curve parameter
        if (curve < 1.0)
        {
            // Logarithmic curve (fast start, slow end)
            return 1.0f - std::pow(1.0f - t, 1.0f / curve);
        }
        else if (curve > 1.0)
        {
            // Exponential curve (slow start, fast end)
            return std::pow(t, curve);
        }
        else
        {
            // Linear curve
            return t;
        }
    }
    
    void drawBlueprintGrid(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour(BlueprintColors::blueprintLines.withAlpha(0.3f));
        
        // Vertical grid lines
        int gridSpacing = 15;
        for (int x = gridSpacing; x < bounds.getWidth(); x += gridSpacing)
        {
            g.drawVerticalLine(bounds.getX() + x, bounds.getY(), bounds.getBottom());
        }
        
        // Horizontal grid lines
        for (int y = gridSpacing; y < bounds.getHeight(); y += gridSpacing)
        {
            g.drawHorizontalLine(bounds.getY() + y, bounds.getX(), bounds.getRight());
        }
    }
    
    void drawAutomationCurve(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        if (curvePoints.size() < 2) return;
        
        // Draw smooth curve line
        juce::Path curvePath;
        curvePath.startNewSubPath(curvePoints[0]);
        
        for (size_t i = 1; i < curvePoints.size(); ++i)
        {
            curvePath.lineTo(curvePoints[i]);
        }
        
        g.setColour(BlueprintColors::active);
        g.strokePath(curvePath, juce::PathStrokeType(2.0f));
    }
    
    void drawPhaseBreakpoints(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour(BlueprintColors::blueprintLines);
        float dotSize = 3.0f;
        
        // Origin point
        if (!curvePoints.empty())
        {
            auto origin = curvePoints[0];
            g.fillEllipse(origin.x - dotSize/2, origin.y - dotSize/2, dotSize, dotSize);
        }
        
        // Delay end point
        if (delayTime > 0.0)
        {
            g.fillEllipse(delayEndPoint.x - dotSize/2, delayEndPoint.y - dotSize/2, dotSize, dotSize);
        }
        
        // Attack end point
        if (attackTime > 0.0)
        {
            g.fillEllipse(attackEndPoint.x - dotSize/2, attackEndPoint.y - dotSize/2, dotSize, dotSize);
        }
        
        // Return end point
        if (returnTime > 0.0)
        {
            g.fillEllipse(returnEndPoint.x - dotSize/2, returnEndPoint.y - dotSize/2, dotSize, dotSize);
        }
    }
    
    void updateBallPosition()
    {
        if (curvePoints.empty()) return;
        
        // Calculate position along curve based on automation progress
        float totalPoints = static_cast<float>(curvePoints.size() - 1);
        float pointIndex = automationProgress * totalPoints;
        
        int baseIndex = static_cast<int>(pointIndex);
        float fraction = pointIndex - baseIndex;
        
        if (baseIndex >= static_cast<int>(curvePoints.size()) - 1)
        {
            ballPosition = curvePoints.back();
        }
        else
        {
            // Interpolate between two points
            auto p1 = curvePoints[baseIndex];
            auto p2 = curvePoints[baseIndex + 1];
            ballPosition = {
                p1.x + (p2.x - p1.x) * fraction,
                p1.y + (p2.y - p1.y) * fraction
            };
        }
    }
    
    void drawMovingBall(juce::Graphics& g)
    {
        float ballRadius = 4.0f;
        
        // Bright cyan ball with glow effect
        g.setColour(BlueprintColors::active);
        g.fillEllipse(ballPosition.x - ballRadius, ballPosition.y - ballRadius, 
                     ballRadius * 2, ballRadius * 2);
        
        // Subtle glow
        g.setColour(BlueprintColors::active.withAlpha(0.3f));
        g.fillEllipse(ballPosition.x - ballRadius * 1.5f, ballPosition.y - ballRadius * 1.5f, 
                     ballRadius * 3, ballRadius * 3);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutomationVisualizer)
};