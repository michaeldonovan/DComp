#include "CustomControls.h"
#include <string>

IKnobMultiControlText::IKnobMultiControlText (IPlugBase* pPlug, int x, int y, int paramIdx, IBitmap* pBitmap,
                                              IText* pText, bool showParamLabel, int offset)
    : IKnobMultiControl (pPlug, x, y, paramIdx, pBitmap), mBitmap (*pBitmap), mShowParamLabel (showParamLabel)
{
    mRECT = IRECT (mRECT.L, mRECT.T, mRECT.R, mRECT.B + 10 + offset);
    mText = *pText;
    mTextRECT = IRECT (mRECT.L, mRECT.B - 20, mRECT.R, mRECT.B);
    mImgRECT = IRECT (mRECT.L, mRECT.T, &mBitmap);
    mDisablePrompt = false;
}

IKnobMultiControlText::~IKnobMultiControlText () {}

bool IKnobMultiControlText::Draw (IGraphics* pGraphics)
{
    int i = 1 + int(0.5 + mValue * (double) (mBitmap.N - 1));
    i = BOUNDED (i, 1, mBitmap.N);
    pGraphics->DrawBitmap (&mBitmap, &mImgRECT, i, &mBlend);
    // pGraphics->FillIRect(&COLOR_WHITE, &mTextRECT);

    char disp[20];
    mPlug->GetParam (mParamIdx)->GetDisplayForHost (disp);

    std::string str (disp);

    if (CSTR_NOT_EMPTY (disp))
    {
        if (mShowParamLabel)
        {
            str += " ";
            str += mPlug->GetParam (mParamIdx)->GetLabelForHost ();
        }
        const char* cstr = str.c_str ();
        return pGraphics->DrawIText (&mText, (char*) cstr, &mTextRECT);
    }
    return true;
}

void IKnobMultiControlText::OnMouseDown (int x, int y, IMouseMod* pMod)
{
    if (mTextRECT.Contains (x, y)) PromptUserInput (&mTextRECT);
#ifdef RTAS_API
    else if (pMod->A)
    {
        if (mDefaultValue >= 0.0)
        {
            mValue = mDefaultValue;
            SetDirty ();
        }
    }
#endif
    else
    {
        OnMouseDrag (x, y, 0, 0, pMod);
    }
}

void IKnobMultiControlText::OnMouseDblClick (int x, int y, IMouseMod* pMod)
{
#ifdef RTAS_API
    PromptUserInput (&mTextRECT);
#else
    if (mDefaultValue >= 0.0)
    {
        mValue = mDefaultValue;
        SetDirty ();
    }
#endif
}

void IKnobMultiControlText::setCaptionOffset (int offset)
{
    mTextRECT = IRECT (mTextRECT.L, mTextRECT.B + offset, mTextRECT.R, mTextRECT.B + offset);
    SetDirty ();
}

IFaderControlText::IFaderControlText (IPlugBase* pPlug, int x, int y, int paramIdx, IBitmap* pBitmap, IText* pText,
                                      bool showParamLabel, int offset)
    : IKnobMultiControl (pPlug, x, y, paramIdx, pBitmap), mBitmap (*pBitmap), mShowParamLabel (showParamLabel)
{
    mRECT = IRECT (mRECT.L, mRECT.T, mRECT.R, mRECT.B + 10 + offset);
    mText = *pText;
    mTextRECT = IRECT (mRECT.L, mRECT.B - 20, mRECT.R, mRECT.B);
    mImgRECT = IRECT (mRECT.L, mRECT.T, &mBitmap);
    mDisablePrompt = false;
}

IFaderControlText::~IFaderControlText () {}

bool IFaderControlText::Draw (IGraphics* pGraphics)
{
    int i = 1 + int(0.5 + mValue * (double) (mBitmap.N - 1));
    i = BOUNDED (i, 1, mBitmap.N);
    pGraphics->DrawBitmap (&mBitmap, &mImgRECT, i, &mBlend);
    // pGraphics->FillIRect(&COLOR_WHITE, &mTextRECT);

    char disp[20];
    mPlug->GetParam (mParamIdx)->GetDisplayForHost (disp);

    std::string str (disp);

    if (CSTR_NOT_EMPTY (disp))
    {
        if (mShowParamLabel)
        {
            str += " ";
            str += mPlug->GetParam (mParamIdx)->GetLabelForHost ();
        }
        const char* cstr = str.c_str ();
        return pGraphics->DrawIText (&mText, (char*) cstr, &mTextRECT);
    }
    return true;
}

void IFaderControlText::OnMouseDown (int x, int y, IMouseMod* pMod)
{
    if (mTextRECT.Contains (x, y)) PromptUserInput (&mTextRECT);
#ifdef PROTOOLS
    if (pMod->A)
    {
        if (mDefaultValue >= 0.0)
        {
            mValue = mDefaultValue;
            SetDirty ();
            return;
        }
    }
    else
#endif
        if (pMod->R)
    {
        PromptUserInput ();
        return;
    }

    if (mImgRECT.Contains (x, y)) SnapToMouse (x, y);
}

void IFaderControlText::OnMouseDblClick (int x, int y, IMouseMod* pMod)
{
#ifdef RTAS_API
    PromptUserInput (&mTextRECT);
#else
    if (mDefaultValue >= 0.0)
    {
        mValue = mDefaultValue;
        SetDirty ();
    }
#endif
}

void IFaderControlText::OnMouseDrag (int x, int y, int dX, int dY, IMouseMod* pMod)
{
    return SnapToMouse (x, y);
}

void IFaderControlText::SnapToMouse (int x, int y)
{
    mValue = 1.0 - (double) (y - mImgRECT.T) / (double) mImgRECT.H ();

    SetDirty ();
}

void IFaderControlText::setCaptionOffset (int offset)
{
    mTextRECT = IRECT (mTextRECT.L, mTextRECT.B + offset, mTextRECT.R, mTextRECT.B + offset);
    SetDirty ();
}

ICairoPlotControl::ICairoPlotControl (IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* fillColor, IColor* lineColor,
                                      bool fillEnable)
    : IControl (pPlug, pR),
      mColorFill (fillColor),
      mColorLine (lineColor),
      mFill (fillEnable),
      mRange (1),
      mLineWeight (2.),
      mRetina (false)
{
    mWidth = mRECT.W ();
    mHeight = mRECT.H ();

    mVals = new valarray<double> (0., mWidth);

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, mWidth, mHeight);
    cr = cairo_create (surface);
}

ICairoPlotControl::~ICairoPlotControl ()
{
    delete mVals;
    cairo_destroy (cr);
    cairo_surface_destroy (surface);
}

void ICairoPlotControl::setFillEnable (bool b)
{
    mFill = b;
}

void ICairoPlotControl::setLineColor (IColor* color)
{
    mColorLine.setFromIColor (color);
}

void ICairoPlotControl::setFillColor (IColor* color)
{
    mColorFill.setFromIColor (color);
}

void ICairoPlotControl::setAAquality (int quality)
{
    switch (quality)
    {
        case kNone:
            cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
            break;
        case kFast:
            cairo_set_antialias (cr, CAIRO_ANTIALIAS_FAST);
            break;
        case kGood:
            cairo_set_antialias (cr, CAIRO_ANTIALIAS_GOOD);
            break;
        case kBest:
            cairo_set_antialias (cr, CAIRO_ANTIALIAS_BEST);
            break;
    }
}

void ICairoPlotControl::setLineWeight (double w)
{
    mLineWeight = w;
}

void ICairoPlotControl::setRange (double range)
{
    mRange = range;
}

void ICairoPlotControl::plotVals (valarray<double>* vals, bool normalize)
{
    double scalar;

    delete mVals;

    mVals = vals;

    if (normalize)
    {
        scalar = 1. / mVals->max ();
    }
    else
    {
        scalar = 1. / mRange;
    }

    if (scalar != 1.) *mVals *= scalar;

    SetDirty (true);
}

bool ICairoPlotControl::Draw (IGraphics* pGraphics)
{
    double mSpacing = (double) mWidth / mVals->size ();

    cairo_save (cr);
    cairo_set_source_rgba (cr, 0, 0, 0, 0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_restore (cr);

    // surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
    // cr = cairo_create(surface);
    if (mRetina)
    {
        cairo_set_line_width (cr, mLineWeight * 2);
    }
    else
    {
        cairo_set_line_width (cr, mLineWeight);
    }

    // Starting point in bottom left corner.
    cairo_move_to (cr, 0, mHeight);

    // Draw data points
    for (int i = 0, x = 0; x < mWidth && i < mVals->size (); i++, x += mSpacing)
    {
        cairo_line_to (cr, x, mVals->operator[] (i));
    }

    // Endpoint in bottom right corner
    cairo_line_to (cr, mWidth, mHeight);

    cairo_close_path (cr);

    if (mFill)
    {
        cairo_set_source_rgba (cr, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);

        cairo_path_t* path = cairo_copy_path (cr);

        cairo_fill (cr);

        cairo_append_path (cr, path);

        cairo_set_source_rgba (cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);

        cairo_stroke (cr);

        cairo_path_destroy (path);
    }
    else
    {
        cairo_set_source_rgba (cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);
        cairo_stroke (cr);
    }

    cairo_surface_flush (surface);

    unsigned int* data = (unsigned int*) cairo_image_surface_get_data (surface);
    // Bind to LICE
    LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap (data, mWidth, mHeight, mWidth, false);

    // Render
    //}
    IBitmap result;

    result = IBitmap (&WrapperBitmap, WrapperBitmap.getWidth (), WrapperBitmap.getHeight ());

    return pGraphics->DrawBitmap (&result, &this->mRECT);
}

// Accessors//
CColor ICairoPlotControl::getColorFill ()
{
    return mColorFill;
}
CColor ICairoPlotControl::getColorLine ()
{
    return mColorLine;
}
bool ICairoPlotControl::getFill ()
{
    return mFill;
}
int ICairoPlotControl::getWidth ()
{
    return mWidth;
}
int ICairoPlotControl::getHeight ()
{
    return mHeight;
}
double ICairoPlotControl::getRange ()
{
    return mRange;
}

inline double ICairoPlotControl::scaleValue (double inValue, double inMin, double inMax, double outMin, double outMax)
{
    return ((outMax - outMin) * (inValue - inMin)) / (inMax - inMin) + outMin;
}

inline double ICairoPlotControl::percentToCoordinates (double value)
{
    return mHeight - value * mHeight;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ILevelPlotControl::ILevelPlotControl (IPlugBase* pPlug, IRECT pR, IColor* fillColor, IColor* lineColor,
                                      double timeScale, bool fillEnable, int paramIdx)
    : ICairoPlotControl (pPlug, pR, paramIdx, fillColor, lineColor, fillEnable),
      mTimeScale (timeScale),
      mBufferLength (0.),
      mYRange (-32),
      mStroke (true),
      mHeadroom (2),
      mReverseFill (false),
      mGradientFill (false)
{
    mRes = kHighRes;
    mXRes = mWidth / 2.;
    mDrawVals = new valarray<double> (mHeight, mXRes);
    mBuffer = new valarray<double> (0., mTimeScale * mPlug->GetSampleRate () / (double) mXRes);
    setResolution (kHighRes);
    setLineWeight (2.);
}

ILevelPlotControl::~ILevelPlotControl ()
{
    delete mDrawVals;
    delete mBuffer;
}

void ILevelPlotControl::setReverseFill (bool rev)
{
    mReverseFill = rev;
}

void ILevelPlotControl::setResolution (int res)
{
    mRes = res;
    switch (mRes)
    {
        case kLowRes:
            mXRes = mWidth / 8.;
            break;

        case kMidRes:
            mXRes = mWidth / 4.;
            break;

        case kHighRes:
            mXRes = mWidth / 2.;
            break;

        case kMaxRes:

            mXRes = mWidth;
            break;

        default:
            mXRes = mWidth / 2.;
            break;
    }

    mBuffer->resize (mTimeScale * mPlug->GetSampleRate () / (double) mXRes, -48.);
    mBufferLength = 0;
    if (mReverseFill)
    {
        mDrawVals->resize (mXRes, -2);
    }
    else
    {
        mDrawVals->resize (mXRes, mHeight);
    }
    mSpacing = mWidth / mXRes;
}

void ILevelPlotControl::setYRange (int yRangeDB)
{
    switch (yRangeDB)
    {
        case k16dB:
            mYRange = -16;
            break;

        case k32dB:
            mYRange = -32;
            break;

        case k48dB:
            mYRange = -48;
            break;

        default:
            break;
    }
}

void ILevelPlotControl::setStroke (bool stroke)
{
    mStroke = stroke;
}

void ILevelPlotControl::setGradientFill (bool grad)
{
    mGradientFill = grad;
}
void ILevelPlotControl::process (double sample)
{
    mBuffer->operator[] (mBufferLength) = sample;
    mBufferLength++;

    if (mBufferLength >= mBuffer->size ())
    {
        double average;

        *mDrawVals = mDrawVals->shift (1);

        average = mBuffer->sum () / (double) mBuffer->size ();
        average = scaleValue (average, mYRange, 2, 0, 1);
        mDrawVals->operator[] (mDrawVals->size () - 1) = percentToCoordinates (average);

        mBufferLength = 0;
    }
}

bool ILevelPlotControl::Draw (IGraphics* pGraphics)
{
    cairo_save (cr);
    cairo_set_source_rgba (cr, 0, 0, 0, 0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_restore (cr);

    // surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
    // cr = cairo_create(surface);

    if (mRetina)
    {
        cairo_set_line_width (cr, mLineWeight * 2);
    }
    else
    {
        cairo_set_line_width (cr, mLineWeight);
    }

    //        if(mGridLines){
    //            drawDBLines(cr);
    //        }

    // Starting point in bottom left corner.
    if (mReverseFill)
    {
        cairo_move_to (cr, -8, -8);
    }
    else
    {
        cairo_move_to (cr, -4, mHeight + 4);
    }

    // Draw data points
    for (int i = 0, x = 0; x < mWidth && i < mDrawVals->size (); i++)
    {
        cairo_line_to (cr, x, mDrawVals->operator[] (i));
        x += mSpacing;
    }

    cairo_line_to (cr, mWidth + 8, mDrawVals->operator[] (mDrawVals->size () - 1));
    // Endpoint in bottom right corner
    if (mReverseFill)
    {
        cairo_line_to (cr, mWidth + 8, -8);
    }
    else
    {
        cairo_line_to (cr, mWidth + 8, mHeight + 8);
    }

    cairo_close_path (cr);

    if (mFill && mStroke)
    {
        cairo_path_t* path = cairo_copy_path (cr);

        if (mGradientFill)
        {
            cairo_pattern_t* grad = cairo_pattern_create_linear (0, 0, 0, mHeight);

            cairo_pattern_add_color_stop_rgba (grad, .5, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
            cairo_pattern_add_color_stop_rgba (grad, 1, mColorFill.R, mColorFill.G, mColorFill.B, .3);

            cairo_set_source (cr, grad);
            cairo_fill (cr);
            cairo_pattern_destroy (grad);
        }
        else
        {
            cairo_set_source_rgba (cr, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
            cairo_fill (cr);
        }

        cairo_append_path (cr, path);

        cairo_set_source_rgba (cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);

        cairo_stroke (cr);

        cairo_path_destroy (path);
    }
    else if (mStroke)
    {
        cairo_set_source_rgba (cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);
        cairo_stroke (cr);
    }
    else if (mFill)
    {
        if (mGradientFill)
        {
            cairo_pattern_t* grad = cairo_pattern_create_linear (0, 0, 0, mHeight);

            cairo_pattern_add_color_stop_rgba (grad, .75, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
            cairo_pattern_add_color_stop_rgba (grad, 1, mColorFill.R, mColorFill.G, mColorFill.B, .3);

            cairo_set_source (cr, grad);
            cairo_fill (cr);

            cairo_pattern_destroy (grad);
        }
        else
        {
            cairo_set_source_rgba (cr, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
            cairo_fill (cr);
        }
    }
    cairo_surface_flush (surface);

    unsigned int* data = (unsigned int*) cairo_image_surface_get_data (surface);
    // Bind to LICE
    LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap (data, mWidth, mHeight, mWidth, false);

    // Render
    //}
    IBitmap result;
    result = IBitmap (&WrapperBitmap, WrapperBitmap.getWidth (), WrapperBitmap.getHeight ());
    return pGraphics->DrawBitmap (&result, &this->mRECT);
}

IGRPlotControl::IGRPlotControl (IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* preFillColor, IColor* postFillColor,
                                IColor* postLineColor, IColor* GRFillColor, IColor* GRLineColor, double timeScale)
    : ICairoPlotControl (pPlug, pR, paramIdx, postFillColor, postLineColor, true),
      mTimeScale (timeScale),
      mBufferLength (0.),
      mYRange (-32),
      mHeadroom (2),
      sr (mPlug->GetSampleRate ()),
      mPreFillColor (preFillColor),
      mGRFillColor (GRFillColor),
      mGRLineColor (GRLineColor),
      mRes (2.),
      mGradientFill (true)
{
    mXRes = mWidth / 2.;
    mDrawValsPre = new valarray<double> (mHeight, mXRes);
    mDrawValsPost = new valarray<double> (mHeight, mXRes);
    mDrawValsGR = new valarray<double> (mHeight, mXRes);

    mBufferPre = new valarray<double> (0., mTimeScale * sr / (double) mXRes);
    mBufferPost = new valarray<double> (0., mTimeScale * sr / (double) mXRes);
    mBufferGR = new valarray<double> (-2, mTimeScale * sr / (double) mXRes);

    setResolution (kHighRes);
    setLineWeight (2.);
}

IGRPlotControl::~IGRPlotControl ()
{
    delete mDrawValsPre;
    delete mDrawValsPost;
    delete mDrawValsGR;
    delete mBufferPre;
    delete mBufferPost;
}

void IGRPlotControl::setResolution (int res)
{
    mRes = res;
    switch (mRes)
    {
        case kLowRes:
            mXRes = mWidth / 8.;
            break;
        case kMidRes:
            mXRes = mWidth / 4.;
            break;

        case kHighRes:
            mXRes = mWidth / 2.;
            break;

        case kMaxRes:
            mXRes = mWidth;
            break;

        default:
            mXRes = mWidth / 2.;
            break;
    }

    if (mRetina) mXRes /= 2;

    mBufferPre->resize (mTimeScale * sr / (double) mXRes, -48.);
    mBufferPost->resize (mTimeScale * sr / (double) mXRes, -48.);
    mBufferGR->resize (mTimeScale * sr / (double) mXRes, 0);

    mDrawValsPre->resize (mXRes, mHeight);
    mDrawValsPost->resize (mXRes, mHeight);
    mDrawValsGR->resize (mXRes, -2);

    mBufferLength = 0;

    mSpacing = mWidth / mXRes;
}

void IGRPlotControl::setYRange (int yRangeDB)
{
    switch (yRangeDB)
    {
        case k16dB:
            mYRange = -16;
            break;

        case k32dB:
            mYRange = -32;
            break;

        case k48dB:
            mYRange = -48;
            break;

        default:
            break;
    }
}

void IGRPlotControl::setGradientFill (bool enabled)
{
    mGradientFill = enabled;
}

void IGRPlotControl::process (double sampleIn, double sampleOut, double sampleGR)
{
    mBufferPre->operator[] (mBufferLength) = sampleIn;
    mBufferPost->operator[] (mBufferLength) = sampleOut;
    mBufferGR->operator[] (mBufferLength) = sampleGR;

    mBufferLength++;

    if (mBufferLength >= mBufferPre->size ())
    {
        double averagePre, averagePost, averageGR;

        *mDrawValsPre = mDrawValsPre->shift (1);
        *mDrawValsPost = mDrawValsPost->shift (1);
        *mDrawValsGR = mDrawValsGR->shift (1);

        averagePre = mBufferPre->sum () / (double) mBufferPre->size ();
        averagePost = mBufferPost->sum () / (double) mBufferPost->size ();
        averageGR = mBufferGR->sum () / (double) mBufferGR->size ();

        averagePre = scaleValue (averagePre, mYRange, mHeadroom, 0, 1);
        mDrawValsPre->operator[] (mDrawValsPre->size () - 1) = percentToCoordinates (averagePre);

        averagePost = scaleValue (averagePost, mYRange, mHeadroom, 0, 1);
        mDrawValsPost->operator[] (mDrawValsPost->size () - 1) = percentToCoordinates (averagePost);

        averageGR = scaleValue (averageGR, mYRange, mHeadroom, 0, 1);
        mDrawValsGR->operator[] (mDrawValsGR->size () - 1) = percentToCoordinates (averageGR);

        mBufferLength = 0;
    }
}

bool IGRPlotControl::Draw (IGraphics* pGraphics)
{
    cairo_save (cr);
    cairo_set_source_rgba (cr, 0, 0, 0, 0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_restore (cr);

    // surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
    // cr = cairo_create(surface);

    if (mRetina)
    {
        cairo_set_line_width (cr, mLineWeight * 2);
    }
    else
    {
        cairo_set_line_width (cr, mLineWeight);
    }
    ////////////////////////////////////////////////////////////////////////////////PRE

    // Starting point in bottom left corner.
    cairo_move_to (cr, -4, mHeight + 4);

    // Draw data points
    for (int i = 0, x = 0; x < mWidth && i < mDrawValsPre->size (); i++)
    {
        cairo_line_to (cr, x, mDrawValsPre->operator[] (i));
        x += mSpacing;
    }

    cairo_line_to (cr, mWidth + 4, mDrawValsPre->operator[] (mDrawValsPre->size () - 1));
    // Endpoint in bottom right corner
    cairo_line_to (cr, mWidth + 4, mHeight + 4);

    cairo_close_path (cr);

    cairo_path_t* pathPre = cairo_copy_path (cr);

    ////////////////////////////////////////////////////////////////////////////////PRE

    ////////////////////////////////////////////////////////////////////////////////POST
    cairo_new_path (cr);

    // Starting point in bottom left corner.
    cairo_move_to (cr, -4, mHeight + 4);

    // Draw data points
    for (int i = 0, x = 0; x < mWidth && i < mDrawValsPost->size (); i++)
    {
        cairo_line_to (cr, x, mDrawValsPost->operator[] (i));
        x += mSpacing;
    }

    cairo_line_to (cr, mWidth + 4, mDrawValsPre->operator[] (mDrawValsPre->size () - 1));
    // Endpoint in bottom right corner
    cairo_line_to (cr, mWidth + 4, mHeight + 4);

    cairo_close_path (cr);

    cairo_path_t* pathPost = cairo_copy_path (cr);

    ////////////////////////////////////////////////////////////////////////////////POST

    cairo_new_path (cr);

    // Starting point in top left corner.
    cairo_move_to (cr, -8, -8);

    // Draw data points
    for (int i = 0, x = 0; x < mWidth && i < mDrawValsGR->size (); i++)
    {
        cairo_line_to (cr, x, mDrawValsGR->operator[] (i));
        x += mSpacing;
    }

    cairo_line_to (cr, mWidth + 8, mDrawValsGR->operator[] (mDrawValsGR->size () - 1));

    // Endpoint in top right corner
    cairo_line_to (cr, mWidth + 8, -8);

    cairo_close_path (cr);

    cairo_path_t* pathGR = cairo_copy_path (cr);

    ////////////////////////////////////////////////////////////////////////////////GR

    cairo_new_path (cr);
    cairo_append_path (cr, pathPre);
    cairo_set_source_rgba (cr, mPreFillColor.R, mPreFillColor.G, mPreFillColor.B, mPreFillColor.A);
    cairo_fill (cr);

    cairo_new_path (cr);
    cairo_append_path (cr, pathPost);

    if (mGradientFill)
    {
        cairo_pattern_t* grad = cairo_pattern_create_linear (0, 0, 0, mHeight);

        cairo_pattern_add_color_stop_rgba (grad, .5, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
        cairo_pattern_add_color_stop_rgba (grad, 1, mColorFill.R, mColorFill.G, mColorFill.B, 0.3);

        cairo_set_source (cr, grad);
        cairo_fill (cr);
        cairo_pattern_destroy (grad);
    }
    else
    {
        cairo_set_source_rgba (cr, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
        cairo_fill (cr);
    }

    cairo_new_path (cr);

    cairo_append_path (cr, pathPost);
    cairo_set_source_rgba (cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);

    cairo_stroke (cr);

    if (mRetina)
    {
        cairo_set_line_width (cr, mLineWeight * 2 + 2);
    }
    else
    {
        cairo_set_line_width (cr, mLineWeight + 1);
    }

    cairo_new_path (cr);

    cairo_append_path (cr, pathGR);
    cairo_set_source_rgba (cr, mGRLineColor.R, mGRLineColor.G, mGRLineColor.B, mGRLineColor.A);

    cairo_stroke (cr);

    cairo_path_destroy (pathPre);
    cairo_path_destroy (pathPost);
    cairo_path_destroy (pathGR);

    cairo_surface_flush (surface);

    unsigned int* data = (unsigned int*) cairo_image_surface_get_data (surface);
    // Bind to LICE
    LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap (data, mWidth, mHeight, mWidth, false);

    // Render
    //}
    IBitmap result;
    result = IBitmap (&WrapperBitmap, WrapperBitmap.getWidth (), WrapperBitmap.getHeight ());
    return pGraphics->DrawBitmap (&result, &this->mRECT);
}

ICompressorPlotControl::ICompressorPlotControl (IPlugBase* pPlug, IRECT pR, IColor* lineColor, IColor* fillColor,
                                                compressor* comp, int paramIdx)
    : ICairoPlotControl (pPlug, pR, paramIdx, fillColor, lineColor, false), mYRange (-32), mHeadroom (2.)
{
    setLineWeight (2.);
    mComp = comp;
}

void ICompressorPlotControl::calc ()
{
    double threshCoord = scaleValue (mComp->getThreshold (), mYRange, mHeadroom, 0, mWidth);

    x1 = scaleValue (mComp->getKneeBoundL (), mYRange, mHeadroom, 0, mWidth);
    y1 = mHeight - x1;

    xCP = threshCoord;
    yCP = mHeight - threshCoord;

    x2 = scaleValue (mComp->getKneeBoundU (), mYRange, mHeadroom, 0, mWidth);
    y2 = yCP - ((x2 - xCP) / mComp->getRatio ());

    x3 = mWidth + 2;

    y3 = yCP - ((mWidth + 2 - xCP) / mComp->getRatio ());

    SetDirty ();
}

bool ICompressorPlotControl::Draw (IGraphics* pGraphics)
{
    cairo_save (cr);
    cairo_set_source_rgba (cr, 0, 0, 0, 0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_restore (cr);

    // surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
    // cr = cairo_create(surface);

    if (mRetina)
    {
        cairo_set_line_width (cr, mLineWeight * 2);
    }
    else
    {
        cairo_set_line_width (cr, mLineWeight);
    }

    // fill background
    cairo_set_source_rgba (cr, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
    cairo_rectangle (cr, 0, 0, mWidth, mHeight);
    cairo_fill (cr);

    cairo_set_source_rgba (cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);

    // Starting point in bottom left corner.
    cairo_move_to (cr, -1, mHeight + 1);

    if (mComp->getKnee () > 0.)
    {
        cairo_line_to (cr, x1, y1);
        cairo_curve_to (cr, xCP, yCP, xCP, yCP, x2, y2);
        cairo_line_to (cr, x3, y3);
    }
    else
    {
        cairo_line_to (cr, xCP, yCP);
        cairo_line_to (cr, x3, y3);
    }

    cairo_stroke (cr);

    cairo_surface_flush (surface);

    unsigned int* data = (unsigned int*) cairo_image_surface_get_data (surface);
    // Bind to LICE
    LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap (data, mWidth, mHeight, mWidth, false);

    // Render
    //
    IBitmap result;
    result = IBitmap (&WrapperBitmap, WrapperBitmap.getWidth (), WrapperBitmap.getHeight ());

    return pGraphics->DrawBitmap (&result, &this->mRECT);
}

IThresholdPlotControl::IThresholdPlotControl (IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* lineColor,
                                              compressor* comp)
    : ICairoPlotControl (pPlug, pR, paramIdx, (IColor*) &COLOR_BLACK, lineColor, false), mYRange (-32), mHeadroom (2)
{
    mComp = comp;
}

bool IThresholdPlotControl::Draw (IGraphics* pGraphics)
{
    cairo_save (cr);
    cairo_set_source_rgba (cr, 0, 0, 0, 0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_restore (cr);

    // surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
    // cr = cairo_create(surface);
    double dashes[] = {
        6.0, /* ink */
        3.0, /* skip */
        6.0, /* ink */
        3.0  /* skip*/
    };

    if (mRetina)
    {
        cairo_set_line_width (cr, mLineWeight * 2);
        for (int i = 0; i < 4; i++) dashes[i] *= 2;
    }
    else
    {
        cairo_set_line_width (cr, mLineWeight);
    }

    cairo_set_source_rgba (cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);

    int ndash = sizeof (dashes) / sizeof (dashes[0]);
    double offset = -5.0;

    cairo_set_dash (cr, dashes, ndash, offset);

    double threshCoord = scaleValue (mComp->getThreshold (), mYRange, mHeadroom, 0, mHeight);

    cairo_move_to (cr, threshCoord, 0);

    cairo_line_to (cr, threshCoord, mHeight);

    cairo_stroke (cr);

    cairo_move_to (cr, 0, mHeight - threshCoord);
    cairo_line_to (cr, mWidth, mHeight - threshCoord);

    cairo_stroke (cr);

    cairo_pattern_t* grad = cairo_pattern_create_linear (mHeight, 0, mHeight + 5, 0);

    cairo_pattern_add_color_stop_rgba (grad, 0, .1, .1, .1, .4);
    cairo_pattern_add_color_stop_rgba (grad, 1, .1, .1, .1, 0);

    cairo_set_source (cr, grad);

    cairo_rectangle (cr, mHeight, 0, mWidth, mHeight);

    cairo_fill (cr);
    cairo_pattern_destroy (grad);

    cairo_surface_flush (surface);

    unsigned int* data = (unsigned int*) cairo_image_surface_get_data (surface);
    // Bind to LICE
    LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap (data, mWidth, mHeight, mWidth, false);

    // Render
    //}
    IBitmap result (&WrapperBitmap, WrapperBitmap.getWidth (), WrapperBitmap.getHeight ());
    return pGraphics->DrawBitmap (&result, &this->mRECT);
}
