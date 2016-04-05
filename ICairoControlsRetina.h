//
//  ICairoControlsRetina.hpp
//  DComp
//
//  Created by Michael Donovan on 4/5/16.
//
//

#ifndef ICairoControlsRetina_h
#define ICairoControlsRetina_h

#include "IControl.h"
#include <cairo.h>
#include <valarray>
#include "EnvelopeFollower.h"

using std::valarray;


/**
 *  Struct to store color data for use with CairoGraphics
 *  Can be initialized with a pointer to an IColor
 */

struct CColor{
public:
    double A, R, G, B;
    
    CColor(IColor* ic){
        A = ic->A / 255.;
        R = ic->R / 255.;
        G = ic->G / 255.;
        B = ic->B / 255.;
    }
    
    CColor(double a = 1., double r = 0., double g = 0., double b = 0.);
    
    
    bool operator==(const CColor& rhs) { return (rhs.A == A && rhs.R == R && rhs.G == G && rhs.B == B); }
    
    bool operator!=(const CColor& rhs) { return !operator==(rhs); }
    
    bool Empty() const { return A == 0 && R == 0 && G == 0 && B == 0; }
    
    void Clamp() { A = IPMIN(A, 1.); R = IPMIN(R, 1.); G = IPMIN(G, 1.); B = IPMIN(B, 1.); }
    
    void setFromIColor(IColor* ic){
        A = ic->A / 255;
        R = ic->R / 255;
        G = ic->G / 255;
        B = ic->B / 255;
    }
};

class ICairoPlotControl : public IControl
{
protected:
    CColor mColorFill, mColorLine;
    double mRange, mLineWeight;
    int mWidth, mHeight;
    bool mFill;
    valarray<double>* mVals;
    cairo_surface_t *surface;
    cairo_t *cr;
    
    inline double scaleValue(double inValue, double inMin, double inMax, double outMin, double outMax){
        return ((outMax - outMin) * (inValue - inMin)) / (inMax - inMin) + outMin;
    }

public:
 
    /**
     *  Constructor
     *
     * @param pPlug         Instance of IPlugBase
     * @param pR            IRECT in which plot will be drawn
     * @param fillColor     Pointer to IColor for plot fill
     * @param lineColor     Pointer to IColor for plot line
     * @param fillEnable    Sets whether or not plot will be filled
     * @param paramIdx      Parameter index (-1 by default)
     */
    ICairoPlotControl(IPlugBase* pPlug, IRECT pR, IColor* fillColor, IColor* lineColor, bool fillEnable = true, int paramIdx = -1);
    
    /**
     *  Destructor
     */
    ~ICairoPlotControl();
    
    /**
     *  Plots a valarray of doubles
     *
     *  @param vals      Pointer to a valarray of doubles to be plotted
     *  @param normalize Normalizes based on max value in vals
     */
    void plotVals(valarray<double>* vals, bool normalize=false);

    /**
     *  Draws the plot
     *
     *  @param pGraphics IGraphics context to which the plot will be drawn
     *
     *  @return bool
     */
    bool Draw(IGraphics* pGraphics);
    
    //////////////////////////////////////////////////////////
    //Mutators
    
    /**
     *  Set whether or not the plot will be filled
     *
     *  @param enable
     */
    void setFillEnable(bool enable);
    
    /**
     *  Set line color
     *
     *  @param color
     */
    void setLineColor(IColor* color);
    
    /**
     *  Set fill color
     *
     *  @param color
     */
    void setFillColor(IColor* color);
    
    /**
     *  Set line weight
     *
     *  @param weightinpx Line weight in pixels
     */
    void setLineWeight(double weightinpx);
    
    /**
     *  Set range of Y axis
     *
     *  @param range
     */
    void setRange(double range);
    
    //end mutators
    //////////////////////////////////////////////////////////
    
    
    //////////////////////////////////////////////////////////
    //Accessors
    
    CColor getColorFill(){ return mColorFill; }
    CColor getColorLine(){ return mColorLine; }
    bool getFill(){ return mFill; }
    int getWidth(){ return mWidth; }
    int getHeight(){ return mHeight; }
    double getRange() { return mRange; }
    
    //end accessors
    //////////////////////////////////////////////////////////
    
    
};





#endif /* ICairoControlsRetina_h */
