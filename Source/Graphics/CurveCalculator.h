// CurveCalculator.h - Curve mathematics and point generation for automation visualization
#pragma once
#include <JuceHeader.h>

//==============================================================================
class CurveCalculator
{
public:
    struct CurvePoints
    {
        std::vector<juce::Point<float>> points;
        juce::Point<float> delayEndPoint;
        juce::Point<float> attackEndPoint;
        juce::Point<float> returnEndPoint;
    };
    
    CurveCalculator() = default;
    
    // Calculate curve points based on automation parameters
    CurvePoints calculateCurvePoints(const juce::Rectangle<float>& bounds,
                                   double delayTime,
                                   double attackTime,
                                   double returnTime,
                                   double curveValue) const
    {
        CurvePoints result;
        result.points.clear();
        
        auto workingBounds = bounds.reduced(10.0f); // Margin for grid
        
        // If bounds are invalid, use default bounds
        if (workingBounds.getWidth() <= 0.0f || workingBounds.getHeight() <= 0.0f)
        {
            workingBounds = juce::Rectangle<float>(10.0f, 10.0f, 120.0f, 60.0f);
        }
        
        // Origin point (bottom-left)
        float originX = workingBounds.getX();
        float originY = workingBounds.getBottom();
        result.points.push_back({originX, originY});
        
        // Special case: If attack is 0, draw flat horizontal line
        if (attackTime <= 0.0)
        {
            calculateFlatLine(result, workingBounds, originX, originY);
            return result;
        }
        
        // Normal curve calculation
        calculateNormalCurve(result, workingBounds, delayTime, attackTime, returnTime, curveValue);
        
        return result;
    }
    
    // Apply curve shape transformation
    float applyCurve(float t, double curve) const
    {
        // Must match AutomationEngine::applyCurve() behavior exactly
        if (curve < 1.0)
        {
            // Exponential (0.0 = full exponential, slow start/fast finish)
            double exponent = 1.0 + (1.0 - curve) * 3.0; // Range: 1.0 to 4.0
            return std::pow(t, exponent);
        }
        else if (curve > 1.0)
        {
            // Logarithmic (2.0 = full logarithmic, fast start/slow finish)
            double exponent = 1.0 / (1.0 + (curve - 1.0) * 3.0); // Range: 1.0 to 0.25
            return std::pow(t, exponent);
        }
        else
        {
            // Linear curve (curve == 1.0)
            return t;
        }
    }

    // Calculate inverse curve for return phase (matches AutomationEngine behavior)
    double calculateInverseCurve(double curveValue) const
    {
        if (curveValue < 1.0)
        {
            // Attack was exponential, return should be logarithmic
            return 2.0 - curveValue; // Converts 0.0-1.0 to 2.0-1.0
        }
        else if (curveValue > 1.0)
        {
            // Attack was logarithmic, return should be exponential
            return 2.0 - curveValue; // Converts 1.0-2.0 to 1.0-0.0
        }
        else
        {
            // Attack was linear, return stays linear
            return 1.0;
        }
    }
    
    // Calculate ball position for animation
    juce::Point<float> calculateBallPosition(const CurvePoints& curvePoints,
                                           double elapsed,
                                           double animationDelayTime,
                                           double animationAttackTime,
                                           double animationReturnTime,
                                           double curveValue) const
    {
        if (curvePoints.points.empty()) return {0.0f, 0.0f};
        
        double totalAnimationDuration = animationDelayTime + animationAttackTime + animationReturnTime;
        if (totalAnimationDuration <= 0.0) return curvePoints.points[0];
        
        if (elapsed < animationDelayTime)
        {
            // DELAY PHASE: Move horizontally along delay line
            double delayProgress = animationDelayTime > 0.0 ? elapsed / animationDelayTime : 1.0;
            delayProgress = juce::jlimit(0.0, 1.0, delayProgress);
            
            if (!curvePoints.points.empty() && curvePoints.delayEndPoint.x > curvePoints.points[0].x)
            {
                float startX = curvePoints.points[0].x;
                float endX = curvePoints.delayEndPoint.x;
                float y = curvePoints.points[0].y;
                return {startX + (endX - startX) * (float)delayProgress, y};
            }
            else
            {
                return curvePoints.points[0];
            }
        }
        else if (elapsed < animationDelayTime + animationAttackTime)
        {
            // ATTACK PHASE: Move along attack slope to peak
            double attackElapsed = elapsed - animationDelayTime;
            double attackProgress = animationAttackTime > 0.0 ? attackElapsed / animationAttackTime : 1.0;
            attackProgress = juce::jlimit(0.0, 1.0, attackProgress);
            
            float startX = curvePoints.delayEndPoint.x;
            float startY = curvePoints.delayEndPoint.y;
            float endX = curvePoints.attackEndPoint.x;
            float endY = curvePoints.attackEndPoint.y;
            
            return {
                startX + (endX - startX) * (float)attackProgress,
                startY + (endY - startY) * (float)attackProgress
            };
        }
        else if (animationReturnTime > 0.0 && elapsed < totalAnimationDuration)
        {
            // RETURN PHASE: Move along return slope back to start level with inverse curve
            double returnElapsed = elapsed - animationDelayTime - animationAttackTime;
            double returnProgress = animationReturnTime > 0.0 ? returnElapsed / animationReturnTime : 1.0;
            returnProgress = juce::jlimit(0.0, 1.0, returnProgress);

            // Apply inverse curve to match AutomationEngine behavior
            double inverseCurve = calculateInverseCurve(curveValue);
            double curvedProgress = applyCurve(returnProgress, inverseCurve);

            float startX = curvePoints.attackEndPoint.x;
            float startY = curvePoints.attackEndPoint.y;
            float endX = curvePoints.returnEndPoint.x;
            float endY = curvePoints.returnEndPoint.y;

            return {
                startX + (endX - startX) * (float)returnProgress,
                startY + (endY - startY) * (float)curvedProgress
            };
        }
        else
        {
            // ANIMATION COMPLETE: Ball at final position
            return curvePoints.returnEndPoint.x > 0 ? curvePoints.returnEndPoint : curvePoints.attackEndPoint;
        }
    }
    
private:
    void calculateFlatLine(CurvePoints& result, const juce::Rectangle<float>& bounds,
                          float originX, float originY) const
    {
        float endX = bounds.getRight();
        for (int i = 1; i <= 20; ++i)
        {
            float t = i / 20.0f;
            float x = originX + (endX - originX) * t;
            result.points.push_back({x, originY});
        }
        
        // Set all endpoints to the same Y level (flat line)
        result.delayEndPoint = {originX + (endX - originX) * 0.33f, originY};
        result.attackEndPoint = {originX + (endX - originX) * 0.67f, originY};
        result.returnEndPoint = {endX, originY};
    }
    
    void calculateNormalCurve(CurvePoints& result, const juce::Rectangle<float>& bounds,
                             double delayTime, double attackTime, double returnTime, double curveValue) const
    {
        float originX = bounds.getX();
        float originY = bounds.getBottom();
        float totalWidth = bounds.getWidth();
        float maxHeight = bounds.getHeight();
        
        // Calculate phase proportions
        double totalTime = delayTime + attackTime + returnTime;
        if (totalTime <= 0.0) totalTime = attackTime;
        
        // Calculate phase widths
        float delayWidth = totalTime > 0 ? (delayTime / totalTime) * totalWidth : 0.0f;
        float attackWidth = totalTime > 0 ? (attackTime / totalTime) * totalWidth : totalWidth;
        float returnWidth = totalTime > 0 ? (returnTime / totalTime) * totalWidth : 0.0f;
        
        // Scale attack height: attack parameter directly maps to Y position
        float attackHeight = (attackTime / 30.0f) * maxHeight;
        attackHeight = juce::jlimit(0.0f, maxHeight, attackHeight);
        
        // Phase 1: Delay (horizontal line)
        result.delayEndPoint = {originX + delayWidth, originY};
        
        if (delayWidth > 0.0f)
        {
            int delaySteps = juce::jmax(1, (int)(delayWidth / 5.0f));
            for (int i = 1; i <= delaySteps; ++i)
            {
                float t = i / (float)delaySteps;
                result.points.push_back({originX + (delayWidth * t), originY});
            }
        }
        
        // Phase 2: Attack (curved upward)
        result.attackEndPoint = {result.delayEndPoint.x + attackWidth, originY - attackHeight};
        
        if (attackWidth > 0.0f)
        {
            int attackSteps = 20;
            for (int i = 1; i <= attackSteps; ++i)
            {
                float t = i / (float)attackSteps;
                float curvedT = applyCurve(t, curveValue);
                
                float x = result.delayEndPoint.x + (attackWidth * t);
                float y = originY - (attackHeight * curvedT);
                result.points.push_back({x, y});
            }
        }
        
        // Phase 3: Return (if > 0, slope downward with inverse curve)
        if (returnTime > 0.0 && returnWidth > 0.0f)
        {
            result.returnEndPoint = {result.attackEndPoint.x + returnWidth, originY};

            // Calculate inverse curve for return phase
            double inverseCurve = calculateInverseCurve(curveValue);

            int returnSteps = juce::jmax(1, (int)(returnWidth / 5.0f));
            for (int i = 1; i <= returnSteps; ++i)
            {
                float t = i / (float)returnSteps;
                float curvedT = applyCurve(t, inverseCurve); // Apply inverse curve
                float x = result.attackEndPoint.x + (returnWidth * t);
                float y = result.attackEndPoint.y + ((originY - result.attackEndPoint.y) * curvedT);
                result.points.push_back({x, y});
            }
        }
        else
        {
            result.returnEndPoint = result.attackEndPoint;
        }
        
        // Extend to fill remaining width if needed
        float currentEndX = result.returnEndPoint.x;
        float targetEndX = bounds.getRight();
        
        if (currentEndX < targetEndX && returnTime <= 0.0)
        {
            float remainingWidth = targetEndX - currentEndX;
            if (remainingWidth > 1.0f)
            {
                int extensionSteps = juce::jmax(1, (int)(remainingWidth / 5.0f));
                for (int i = 1; i <= extensionSteps; ++i)
                {
                    float t = i / (float)extensionSteps;
                    float x = currentEndX + (remainingWidth * t);
                    result.points.push_back({x, result.attackEndPoint.y});
                }
                result.returnEndPoint = {targetEndX, result.attackEndPoint.y};
            }
        }
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CurveCalculator)
};