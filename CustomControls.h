#ifndef CUSTOM_CONTROLS_H
#define CUSTOM_CONTROLS_H

#include <valarray>
#include <cairo.h>
#include "IControl.h"
#include "DSP/DSP.h"
#include "DSP/EnvelopeFollower.h"

class IKnobMultiControlText : public IKnobMultiControl
{
private:
    IRECT mTextRECT, mImgRECT;
    IBitmap mBitmap;
    bool mShowParamLabel;

public:
    IKnobMultiControlText (IPlugBase* pPlug, int x, int y, int paramIdx, IBitmap* pBitmap, IText* pText,
                           bool showParamLabel = true, int offset = 0);

    ~IKnobMultiControlText ();

    bool Draw (IGraphics* pGraphics);

    void OnMouseDown (int x, int y, IMouseMod* pMod);

    void OnMouseDblClick (int x, int y, IMouseMod* pMod);

    void setCaptionOffset (int offset);
};

/**
 *  An IKnobMultiControl with mouse snapping functionality for use as a fader.
 *
 */
class IFaderControlText : public IKnobMultiControl
{
private:
    IRECT mTextRECT, mImgRECT;
    IBitmap mBitmap;
    bool mShowParamLabel;

public:
    IFaderControlText (IPlugBase* pPlug, int x, int y, int paramIdx, IBitmap* pBitmap, IText* pText,
                       bool showParamLabel = true, int offset = 0);

    ~IFaderControlText ();

    bool Draw (IGraphics* pGraphics);

    void OnMouseDown (int x, int y, IMouseMod* pMod);

    void OnMouseDblClick (int x, int y, IMouseMod* pMod);

    void OnMouseDrag (int x, int y, int dX, int dY, IMouseMod* pMod);

    void SnapToMouse (int x, int y);

    void setCaptionOffset (int offset);
};

using std::valarray;

/**
 *  A struct for storing color data for use with Cairo
 *  Can be initialized with a pointer to an IColor
 */
struct CColor
{
public:
    double A, R, G, B;

    /**
     *  Constructor
     *  @param ic A pointer to an IColor
     */
    CColor (IColor* ic)
    {
        A = ic->A / 255.;
        R = ic->R / 255.;
        G = ic->G / 255.;
        B = ic->B / 255.;
    }

    /**
     *  Constructor
     *  @param a Alpha value in range [0,1]
     *  @param r Red value in range [0,1]
     *  @param g Green value in range [0,1]
     *  @param b Blue value in range [0,1]
     */
    CColor (double a = 1., double r = 0., double g = 0., double b = 0.)
    {
        A = a;
        R = r;
        G = g;
        B = b;
        Clamp ();
    }

    bool operator== (const CColor& rhs)
    {
        return (rhs.A == A && rhs.R == R && rhs.G == G && rhs.B == B);
    }

    bool operator!= (const CColor& rhs)
    {
        return !operator== (rhs);
    }

    bool Empty () const
    {
        return A == 0 && R == 0 && G == 0 && B == 0;
    }

    void Clamp ()
    {
        A = IPMIN (A, 1.);
        R = IPMIN (R, 1.);
        G = IPMIN (G, 1.);
        B = IPMIN (B, 1.);
    }

    void setFromIColor (IColor* ic)
    {
        A = ic->A / 255.;
        R = ic->R / 255.;
        G = ic->G / 255.;
        B = ic->B / 255.;
    }
};

/**
 * An IControl that plots a set of data points using Cairo
 */
class ICairoPlotControl : public IControl
{
public:
    /**
     Antialiasing quality options
     */
    enum AAQuality
    {
        kNone,
        kFast,
        kGood,
        kBest
    };

    ICairoPlotControl (IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* fillColor, IColor* lineColor,
                       bool fillEnable = true);

    ~ICairoPlotControl ();

    /**
     *  Set whether or not a fill will be drawn under the plot points
     *
     *  @param b True=Enabled, False=Disabled
     */
    void setFillEnable (bool b);

    /**
     *  Set the color of the plot line
     *
     *  @param color A Pointer to an IColor
     */
    void setLineColor (IColor* color);

    /**
     *  Set the color of the fill
     *
     *  @param color A pointer to an IColor
     */
    void setFillColor (IColor* color);

    /**
     *  Set antialiasing quality
     *
     *  @see AAQualtiy
     *  @param quality An int in range [0,4]
     */
    void setAAquality (int quality);

    /**
     *  Set the line weight of the plot
     *
     *  @param w Line weight in pixels
     */
    void setLineWeight (double w);

    /**
     *  Set the Y-axis range
     *
     *  @param range The Y-axis range
     */
    void setRange (double range);

    /*
     *  Plot a set of values
     *  Points will be evenly spaced along the horizontal axis
     *
     *  @param vals Pointer to a valarray of doubles to be plotted
     *  @param normalize If true, values will be scaled so that the max value is at the top of the plot. Default = false
     */
    void plotVals (valarray<double>* vals, bool normalize = false);

    /**
     *  Draw the plot. To be called by IGraphics.
     *
     *  @param pGraphics Pointer to IGraphics
     *
     *  @return True if drawn
     */
    bool Draw (IGraphics* pGraphics);

    // Accessors//
    CColor getColorFill ();
    CColor getColorLine ();
    bool getFill ();
    int getWidth ();
    int getHeight ();
    double getRange ();

protected:
    CColor mColorFill;
    CColor mColorLine;
    bool mFill;
    int mWidth, mHeight;
    double mRange, mLineWeight;
    valarray<double>* mVals;
    cairo_surface_t* surface;
    cairo_t* cr;
    bool mRetina;

    /**
     *  Scale a value from range [inMin, inMax] to [outMin, outMax]
     *
     *  @param inValue Value to be scaled
     *  @param inMin   Min of input range
     *  @param inMax   Max of input range
     *  @param outMin  Min of output range
     *  @param outMax  Max of output range
     *
     *  @return Scaled value
     */
    inline double scaleValue (double inValue, double inMin, double inMax, double outMin, double outMax);

    inline double percentToCoordinates (double value);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ILevelPlotControl : public ICairoPlotControl
{
public:
    /**
     *  Resolution settings, determines number of data points to be plotted
     *  kLowRes -> mRECT.W() / 8.
     *  kMidRes -> mRECT.W() / 4.
     *  kHighRes -> mRECT.W() / 2.
     *  kMaxRes -> mRECT.W()
     */
    enum kResolution
    {
        kLowRes,
        kMidRes,
        kHighRes,
        kMaxRes
    };

    /**
     *  Y-axis range settings. Determines min value of Y-axis
     */
    enum kYRange
    {
        k16dB,
        k32dB,
        k48dB
    };

    /**
     *  Constructor
     *
     *  @param pPlug        Pointer to IPlugBase
     *  @param pR           IRECT for the plot
     *  @param fillColor    Pointer to an IColor for plot fill
     *  @param lineColor    Pointer to an IColor for plot line
     *  @param timeScale    X-Axis range in seconds (Default = 5)
     *  @param paramIdx     Parameter index for IControl (Default = -1)
     */
    ILevelPlotControl (IPlugBase* pPlug, IRECT pR, IColor* fillColor, IColor* lineColor, double timeScale = 5.,
                       bool fillEnable = true, int paramIdx = -1);

    ~ILevelPlotControl ();

    /**
     *  If enabled, fill will be drawn above line instead of below
     *
     *  @param rev True = reverse fill
     */
    void setReverseFill (bool rev);

    /**
     *  Set the horizontal resolution of the plot (number of points to be plotted)
     *
     *  @see kResolution
     *  @param res Resolution value in range [0,3]
     */
    void setResolution (int res);

    /**
     *  Set the minimum value of the Y-Axis
     *
     *  @see   kYRange
     *  @param yRangeDB Range value
     */
    void setYRange (int yRangeDB);

    /**
     *  Set whether or not the plot line will be drawn
     *
     *  @param stroke True = enabled
     */
    void setStroke (bool stroke);

    /**
     *  If true, plot fill will be a vertical, linear gradient from mColorFill to transparent
     *
     *  @param grad True = gradient fill enabled
     */
    void setGradientFill (bool grad);

    /**
     *  Takes a sample of audio to be added to the plot
     *  For a smooth plot, sample should be processed by an envelope follower
     *  @param sample A sample of audio
     */
    void process (double sample);

    /**
     *  Draws the plot
     *
     *  @param pGraphics Pointer to IGraphics
     *
     *  @return True if drawn
     */
    bool Draw (IGraphics* pGraphics);

protected:
    double mTimeScale;
    int mBufferLength, mXRes, mRes, mSpacing, mYRange, mHeadroom;
    valarray<double>*mBuffer, *mDrawVals;
    bool mStroke, mReverseFill, mGradientFill;
};

class IGRPlotControl : public ICairoPlotControl
{
public:
    enum kResolution
    {
        kLowRes,
        kMidRes,
        kHighRes,
        kMaxRes
    };

    enum kYRange
    {
        k16dB,
        k32dB,
        k48dB
    };

    IGRPlotControl (IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* preFillColor, IColor* postFillColor,
                    IColor* postLineColor, IColor* GRFillColor, IColor* GRLineColor, double timeScale = 5.);

    ~IGRPlotControl ();

    void setResolution (int res);

    void setYRange (int yRangeDB);

    void setGradientFill (bool enabled);

    void process (double sampleIn, double sampleOut, double sampleGR);

    bool Draw (IGraphics* pGraphics);

protected:
    double mTimeScale, sr;
    int mBufferLength, mXRes, mSpacing, mYRange, mHeadroom, mRes;
    valarray<double>*mBufferPre, *mBufferPost, *mBufferGR, *mDrawValsPre, *mDrawValsPost, *mDrawValsGR;

    bool mGradientFill;

    CColor mPreFillColor, mGRLineColor, mGRFillColor;
};

/**
 *  An ICairoPlotControl class for plotting response curve of a compressor
 *  Designed to be overlayed on an ILevelPlotControl
 *
 *  @see compressor
 *  @see ICairoPlotControl
 *  @see ILevelPlotControl
 */
class ICompressorPlotControl : public ICairoPlotControl
{
public:
    /**
     *  Constructor
     *
     *  @param pPlug        Pointer to IPlugBase
     *  @param pR           IRECT
     *  @param lineColor    Pointer to an IColor
     *  @param fillColor    Pointer to an IColor
     *  @param comp         Pointer to a compressor
     *  @param paramIdx     Parameter index (Default = -1)
     */
    ICompressorPlotControl (IPlugBase* pPlug, IRECT pR, IColor* lineColor, IColor* fillColor, compressor* comp,
                            int paramIdx = -1);

    /**
     *  Update the compressor response curve. Call when compressor settings are changed.
     */
    void calc ();

    /**
     *  Draw the plot
     *
     *  @param pGraphics Pointer to IGraphics
     *
     *  @return True if drawn
     */
    bool Draw (IGraphics* pGraphics);

private:
    double mHeadroom;

    /**
     *  Coordinates of lower bound of knee
     */
    double x1, y1;

    /**
     *  Coordinates of control point for knee bezier curve
     */
    double xCP, yCP;

    /**
     *  Coordinates of upper bound of knee
     */
    double x2, y2;

    /**
     *  Coordinates where response curve exits mRECT
     */
    double x3, y3;
    int mYRange;
    compressor* mComp;
};

/**
 *  An ICairoPlotControl class for plotting the threshold of a compressor
 *  Designed to be overlayed on an ILevelPlotControl and ICompressorControl
 *
 *  @see compressor
 *  @see ICairoPlotControl
 *  @see ILevelPlotControl
 *  @see ICompressorControl
 */
class IThresholdPlotControl : public ICairoPlotControl
{
public:
    IThresholdPlotControl (IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* lineColor, compressor* comp);

    bool Draw (IGraphics* pGraphics);

private:
    int mYRange;
    double mHeadroom;
    compressor* mComp;
};

#endif //CUSTOM_CONTROLS_H
