// VisualizerRenderer.h - Drawing methods for automation visualizer (grid, curve, ball, breakpoints)
#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"
#include "CurveCalculator.h"
#include "../UI/GlobalUIScale.h"

//==============================================================================
class VisualizerRenderer
{
public:
    VisualizerRenderer() = default;
    
    // Draw the complete visualizer
    void drawVisualizer(juce::Graphics& g,
                       const juce::Rectangle<float>& bounds,
                       const CurveCalculator::CurvePoints& curvePoints,
                       double delayTime,
                       double attackTime,
                       double returnTime,
                       bool showBall,
                       const juce::Point<float>& ballPosition) const
    {
        // Blueprint background
        g.fillAll(BlueprintColors::background());
        
        // Draw technical grid
        drawBlueprintGrid(g, bounds);
        
        // Draw automation curve
        drawAutomationCurve(g, curvePoints);
        
        // Draw phase breakpoints
        drawPhaseBreakpoints(g, curvePoints, delayTime, attackTime, returnTime);
        
        // Draw moving ball if automation is active
        if (showBall)
        {
            drawMovingBall(g, ballPosition);
        }
        
        // Draw border outline with scaled line width
        auto& scale = GlobalUIScale::getInstance();
        g.setColour(BlueprintColors::blueprintLines());
        g.drawRect(bounds, scale.getScaled(1.0f));
    }
    
    // Draw blueprint-style grid
    void drawBlueprintGrid(juce::Graphics& g, const juce::Rectangle<float>& bounds) const
    {
        auto& scale = GlobalUIScale::getInstance();
        g.setColour(BlueprintColors::blueprintLines().withAlpha(0.3f));
        
        // Scaled grid spacing for consistent appearance at all scales
        int gridSpacing = scale.getScaled(15);
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
    
    // Draw the automation curve
    void drawAutomationCurve(juce::Graphics& g, const CurveCalculator::CurvePoints& curvePoints) const
    {
        if (curvePoints.points.size() < 2) return;
        
        // Draw smooth curve line
        juce::Path curvePath;
        curvePath.startNewSubPath(curvePoints.points[0]);
        
        for (size_t i = 1; i < curvePoints.points.size(); ++i)
        {
            curvePath.lineTo(curvePoints.points[i]);
        }
        
        auto& scale = GlobalUIScale::getInstance();
        g.setColour(BlueprintColors::active());
        g.strokePath(curvePath, juce::PathStrokeType(scale.getScaled(2.0f)));
    }
    
    // Draw phase breakpoints (dots at important curve points)
    void drawPhaseBreakpoints(juce::Graphics& g,
                             const CurveCalculator::CurvePoints& curvePoints,
                             double delayTime,
                             double attackTime,
                             double returnTime) const
    {
        auto& scale = GlobalUIScale::getInstance();
        g.setColour(BlueprintColors::blueprintLines());
        float dotSize = scale.getScaled(3.0f);
        
        // Origin point
        if (!curvePoints.points.empty())
        {
            auto origin = curvePoints.points[0];
            g.fillEllipse(origin.x - dotSize/2, origin.y - dotSize/2, dotSize, dotSize);
        }
        
        // Delay end point
        if (delayTime > 0.0)
        {
            g.fillEllipse(curvePoints.delayEndPoint.x - dotSize/2, 
                         curvePoints.delayEndPoint.y - dotSize/2, dotSize, dotSize);
        }
        
        // Attack end point
        if (attackTime > 0.0)
        {
            g.fillEllipse(curvePoints.attackEndPoint.x - dotSize/2, 
                         curvePoints.attackEndPoint.y - dotSize/2, dotSize, dotSize);
        }
        
        // Return end point
        if (returnTime > 0.0)
        {
            g.fillEllipse(curvePoints.returnEndPoint.x - dotSize/2, 
                         curvePoints.returnEndPoint.y - dotSize/2, dotSize, dotSize);
        }
    }
    
    // Draw the animated ball
    void drawMovingBall(juce::Graphics& g, const juce::Point<float>& ballPosition) const
    {
        auto& scale = GlobalUIScale::getInstance();
        float ballRadius = scale.getScaled(4.0f);
        
        // Bright cyan ball with glow effect
        g.setColour(BlueprintColors::active());
        g.fillEllipse(ballPosition.x - ballRadius, ballPosition.y - ballRadius, 
                     ballRadius * 2, ballRadius * 2);
        
        // Subtle glow
        g.setColour(BlueprintColors::active().withAlpha(0.3f));
        g.fillEllipse(ballPosition.x - ballRadius * 1.5f, ballPosition.y - ballRadius * 1.5f, 
                     ballRadius * 3, ballRadius * 3);
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizerRenderer)
};