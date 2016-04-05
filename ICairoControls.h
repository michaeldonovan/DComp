//
//  ICairoControls.h
//  DClip
//
//  Created by Michael Donovan on 3/28/16.
//  All rights reserved
//

#ifndef ICairoControls_h
#define ICairoControls_h

#include "IControl.h"
#include <cairo.h>
#include <valarray>
#include <ctime>
#include "EnvelopeFollower.h"

using std::valarray;


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
public:
    ICairoPlotControl(IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* fillColor, IColor* lineColor, bool fillEnable=true) : IControl(pPlug, pR), mColorFill(fillColor), mColorLine(lineColor), mFill(fillEnable), mRange(1), mLineWeight(2.)
    {
#ifndef IPLUG_RETINA_SUPPORT
        mWidth = mRECT.W();
        mHeight = mRECT.H();
#else
        mWidth = mRECT.W() * 2;
        mHeight = mRECT.H() * 2;
#endif
       
        
        mVals = new valarray<double>(0., mWidth);
        
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
        cr = cairo_create(surface);
    }
    
    ~ICairoPlotControl(){
        delete mVals;
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
    }
    
    void setFillEnable(bool b){
        mFill = b;
    }
    
    void setLineColor(IColor* color){
        mColorLine.setFromIColor(color);
    }
    
    void setFillColor(IColor* color){
        mColorFill.setFromIColor(color);
    }
    
    void setLineWeight(double w){
        mLineWeight = w;
    }
    
    void setRange(double range){
        mRange = range;
    }
    
    void setVals(valarray<double>* vals){
        delete mVals;
        
        mVals = vals;
        
        SetDirty(true);
    }
    
    /*
     * plotVals
     * arg: valarray<double>* vals - a pointer to a valarray of values to plot. Points will be evenly spaced along the x-axis, connected by lines.
     */
    void plotVals(valarray<double>* vals, bool normalize=false){
        double scalar;
        
        delete mVals;
        
        mVals = vals;
        
        if (normalize) {
            scalar = 1. / mVals->max();
        }
        else{
            scalar = 1. / mRange;
        }
        
        if(scalar != 1.) *mVals *= scalar;
        
        SetDirty(true);
    }
    
    bool Draw(IGraphics* pGraphics){
        double mSpacing = (double)mWidth / mVals->size()  ;
        
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        cairo_restore(cr);
        
        //surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
        //cr = cairo_create(surface);
        
        cairo_set_line_width(cr, mLineWeight);
        
        //Starting point in bottom left corner.
        cairo_move_to(cr, 0, mHeight);
        
        //Draw data points
        for (int i = 0, x = 0; x < mWidth && i < mVals->size(); i++, x += mSpacing) {
            cairo_line_to(cr, x, mVals->operator[](i));
        }
        
        //Endpoint in bottom right corner
        cairo_line_to(cr, mWidth, mHeight);
        
        cairo_close_path(cr);
        
        if(mFill){
            cairo_set_source_rgba(cr, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
            
            cairo_path_t* path = cairo_copy_path(cr);
            
            cairo_fill(cr);
            
            
            cairo_append_path(cr, path);
            
            cairo_set_source_rgba(cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);
            
            cairo_stroke(cr);
            
            cairo_path_destroy(path);
        }
        else{
            cairo_set_source_rgba(cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);
            cairo_stroke(cr);
        }
        
        cairo_surface_flush(surface);
        
        unsigned int *data = (unsigned int*)cairo_image_surface_get_data(surface);
        //Bind to LICE
        LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap(data, mWidth, mHeight, mWidth, false);
        
        //Render
#ifndef IPLUG_RETINA_SUPPORT
        IBitmap result(&WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
#else
        
        IBitmap result(&WrapperBitmap, &WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
#endif
        return pGraphics->DrawBitmap(&result, &this->mRECT);
    }
    
    
    //Accessors//
    CColor getColorFill(){ return mColorFill; }
    CColor getColorLine(){ return mColorLine; }
    bool getFill(){ return mFill; }
    int getWidth(){ return mWidth; }
    int getHeight(){ return mHeight; }
    double getRange() { return mRange; }
    
protected:
    CColor mColorFill;
    CColor mColorLine;
    bool mFill;
    int mWidth, mHeight;
    double mRange, mLineWeight;
    valarray<double>* mVals;
    cairo_surface_t *surface;
    cairo_t *cr;
    
    inline double scaleValue(double inValue, double inMin, double inMax, double outMin, double outMax){
        return ((outMax - outMin) * (inValue - inMin)) / (inMax - inMin) + outMin;
    }
};


 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class ILevelPlotControl : public ICairoPlotControl
{
public:
    enum kResolution{
        kLowRes,
        kMidRes,
        kHighRes,
        kMaxRes
    };
    
    enum kYRange{
        k16dB,
        k32dB,
        k48dB
    };
    
    enum AAQuality{
        kFast,
        kGood,
        kBest
    };
    
    ILevelPlotControl(IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* fillColor, IColor* lineColor, double timeScale=5., bool fillEnable=true) : ICairoPlotControl(pPlug, pR, paramIdx, fillColor, lineColor, fillEnable), mTimeScale(timeScale), mBufferLength(0.), mYRange(-32), mStroke(true), mHeadroom(2), mReverseFill(false), mGradientFill(false)
    {

        mXRes = mWidth/2.;
        mDrawVals = new valarray<double>(mHeight, mXRes);
        mBuffer = new valarray<double>(0., mTimeScale * mPlug->GetSampleRate() / (double)mXRes);
        setResolution(kHighRes);
        setLineWeight(2.);
    }
    
    ~ILevelPlotControl(){
        delete mDrawVals;
        delete mBuffer;
    }
    
    void setAAquality(int quality){
        switch(quality){
            case kFast:
                cairo_set_antialias(cr, CAIRO_ANTIALIAS_FAST);
                break;
            case kGood:
                cairo_set_antialias(cr, CAIRO_ANTIALIAS_GOOD);
                break;
            case kBest:
                cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
                break;
        }
    }
    
    void setReverseFill(bool rev){
        mReverseFill = rev;
    }
    
    void setResolution(int res){
        switch (res) {
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
        mBuffer->resize(mTimeScale * mPlug->GetSampleRate() / (double)mXRes, -48.);
        mBufferLength = 0;
        if(mReverseFill){
            mDrawVals->resize(mXRes, -2);
        }
        else{
            mDrawVals->resize(mXRes, mHeight);
        }
        mSpacing = mWidth / mXRes;
        
    }
    
    void setYRange(int yRangeDB){
        switch (yRangeDB) {
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
    
    void setStroke(bool stroke){
        mStroke = stroke;
    }
    
    void setGradientFill(bool grad){
        mGradientFill = grad;
    }
    void process(double sample){
        mBuffer->operator[](mBufferLength) = sample;
        mBufferLength++;
        
        if(mBufferLength >= mBuffer->size()){
            double average;
            
            *mDrawVals = mDrawVals->shift(1);
            
            average = mBuffer->sum() / (double)mBuffer->size();
            average = scaleValue(average, mYRange, 2, 0, 1);
            mDrawVals->operator[](mDrawVals->size() - 1) = percentToCoordinates(average);
            
            mBufferLength = 0;
        }
    }
    
    virtual bool IsDirty(){
        return false;
    }
    
    bool Draw(IGraphics* pGraphics){
        
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        cairo_restore(cr);
        
        //surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
        //cr = cairo_create(surface);
        
        cairo_set_line_width(cr, mLineWeight);
        
        
        //        if(mGridLines){
        //            drawDBLines(cr);
        //        }
        
        //Starting point in bottom left corner.
        if(mReverseFill){
            cairo_move_to(cr, -3, -2);
        }
        else{
            cairo_move_to(cr, -2, mHeight+2);
        }
        
        //Draw data points
        for (int i = 0, x = 0; x < mWidth && i < mDrawVals->size(); i++) {
            cairo_line_to(cr, x, mDrawVals->operator[](i));
            x += mSpacing;
        }
        
        cairo_line_to(cr, mWidth+1, mDrawVals->operator[](mDrawVals->size()-1));
        //Endpoint in bottom right corner
        if(mReverseFill){
            cairo_line_to(cr, mWidth+2, -2);
        }
        else{
            cairo_line_to(cr, mWidth+2, mHeight+2);
        }
        
        cairo_close_path(cr);
        
        if(mFill && mStroke){
            cairo_path_t* path = cairo_copy_path(cr);
            
            if(mGradientFill){
                cairo_pattern_t* grad = cairo_pattern_create_linear(0, 0, 0, mHeight);
                
                cairo_pattern_add_color_stop_rgba(grad, .75, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
                cairo_pattern_add_color_stop_rgba(grad, 1, mColorFill.R, mColorFill.G, mColorFill.B, .3);
                
                cairo_set_source(cr, grad);
                cairo_fill(cr);
                cairo_pattern_destroy(grad);
            }
            else{
                cairo_set_source_rgba(cr, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
                cairo_fill(cr);
            }
            
            cairo_append_path(cr, path);
            
            cairo_set_source_rgba(cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);
            
            
            cairo_stroke(cr);
            
            cairo_path_destroy(path);
        }
        else if(mStroke){
            cairo_set_source_rgba(cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);
            cairo_stroke(cr);
        }
        else if(mFill){
            if(mGradientFill){
                cairo_pattern_t* grad = cairo_pattern_create_linear(0, 0, 0, mHeight);
                
                cairo_pattern_add_color_stop_rgba(grad, .75, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
                cairo_pattern_add_color_stop_rgba(grad, 1, mColorFill.R, mColorFill.G, mColorFill.B, .3);
                
                cairo_set_source(cr, grad);
                cairo_fill(cr);
                
                cairo_pattern_destroy(grad);
            }
            else{
                cairo_set_source_rgba(cr, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
                cairo_fill(cr);
            }
        }
        cairo_surface_flush(surface);
        
        unsigned int *data = (unsigned int*)cairo_image_surface_get_data(surface);
        //Bind to LICE
        LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap(data, this->mRECT.W(), this->mRECT.H(), this->mRECT.W(), false);
        
        //Render
        //}
#ifndef IPLUG_RETINA_SUPPORT
        IBitmap result(&WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
#else
        IBitmap result(&WrapperBitmap, &WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
#endif
        return pGraphics->DrawBitmap(&result, &this->mRECT);
    }
    
protected:
    double mTimeScale;
    int mBufferLength, mXRes, mSpacing, mYRange, mHeadroom;
    valarray<double> *mBuffer, *mDrawVals;
    bool mStroke, mReverseFill, mGradientFill;
    
    
    inline double percentToCoordinates(double value) {
        return getHeight() - value * getHeight();
    }

};


class IGRPlotControl : public ICairoPlotControl{
public:
    enum kResolution{
        kLowRes,
        kMidRes,
        kHighRes,
        kMaxRes
    };
    
    enum kYRange{
        k16dB,
        k32dB,
        k48dB
    };
    
    IGRPlotControl(IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* preFillColor, IColor* postFillColor, IColor* postLineColor, IColor* GRFillColor, IColor* GRLineColor, double timeScale=5.) : ICairoPlotControl(pPlug, pR, paramIdx, postFillColor, postLineColor, true), mTimeScale(timeScale), mBufferLength(0.), mYRange(-32), mHeadroom(2), sr(mPlug->GetSampleRate()), mPreFillColor(preFillColor), mGRFillColor(GRFillColor), mGRLineColor(GRLineColor)
    {
        mXRes = mWidth/2.;
        mDrawValsPre = new valarray<double>(mHeight, mXRes);
        mDrawValsPost = new valarray<double>(mHeight, mXRes);
        mDrawValsGR = new valarray<double>(mHeight, mXRes);

        mBufferPre = new valarray<double>(0., mTimeScale * sr / (double)mXRes);
        mBufferPost = new valarray<double>(0., mTimeScale * sr / (double)mXRes);

        mEnvPre.init(compressor::kPeak, 0, 75, 75, sr);
        mEnvPost.init(compressor::kPeak, 0, 75, 75, sr);
        mEnvGR.init(compressor::kPeak, 0, 75, 85, sr);

        setResolution(kHighRes);
        setLineWeight(2.);
    }
    
    ~IGRPlotControl(){
        delete mDrawValsPre;
        delete mDrawValsPost;
        delete mDrawValsGR;
        delete mBufferPre;
        delete mBufferPost;
    }
    
    
    
    void setResolution(int res){
        switch (res) {
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
        mBufferPre->resize(mTimeScale * sr / (double)mXRes, -48.);
        mBufferPost->resize(mTimeScale * sr / (double)mXRes, -48.);
        mDrawValsPre->resize(mXRes, mHeight);
        mDrawValsPost->resize(mXRes, mHeight);
        mDrawValsGR->resize(mXRes, mHeight);

        mBufferLength = 0;

        mSpacing = mWidth / mXRes;
        
    }
    
    void setYRange(int yRangeDB){
        switch (yRangeDB) {
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
    
    void process(double sampleIn, double sampleOut){
        mBufferPre->operator[](mBufferLength) = mEnvPre.process(sampleIn);
        mBufferPost->operator[](mBufferLength) = mEnvPre.process(sampleOut);

        
        
        //plotOut->process(AmpToDB(envPlotOut.process(std::max(sample1, sample2))));
       // double gr = envGR.process(std::max(GR1,GR2));
       // GRplot->process(scaleValue(AmpToDB(gr), -32, 2, 2, -32));

        
        mBufferLength++;
        
        if(mBufferLength >= mBufferPre->size()){
            double averagePre, averagePost, GR;
            
            *mDrawValsPre = mDrawValsPre->shift(1);
            *mDrawValsPost = mDrawValsPost->shift(1);
            *mDrawValsGR = mDrawValsGR->shift(1);

            averagePre = mBufferPre->sum() / (double)mBufferPre->size();
            averagePost = mBufferPost->sum() / (double)mBufferPost->size();

            GR = averagePost - averagePre;
            
            averagePre = AmpToDB(scaleValue(averagePre, mYRange, mHeadroom, 0, 1));
            mDrawValsPre->operator[](mDrawValsPre->size() - 1) = percentToCoordinates(averagePre);
            
            averagePost = AmpToDB(scaleValue(averagePost, mYRange, mHeadroom, 0, 1));
            mDrawValsPost->operator[](mDrawValsPost->size() - 1) = percentToCoordinates(averagePost);

            GR = scaleValue(AmpToDB(GR), mYRange, mHeadroom, mHeadroom, mYRange);
            mDrawValsGR->operator[](mDrawValsGR->size() - 1) = percentToCoordinates(GR);

            
            mBufferLength = 0;
        }
    }
    
    bool Draw(IGraphics* pGraphics){
        
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        cairo_restore(cr);
        
        //surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
        //cr = cairo_create(surface);
        
        cairo_set_line_width(cr, mLineWeight);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_FAST);
        
        ////////////////////////////////////////////////////////////////////////////////PRE
        
        //Starting point in bottom left corner.
        cairo_move_to(cr, -1, mHeight+1);

        //Draw data points
        for (int i = 0, x = 0; x < mWidth && i < mDrawValsPre->size(); i++) {
            cairo_line_to(cr, x, mDrawValsPre->operator[](i));
            x += mSpacing;
        }
        
        cairo_line_to(cr, mWidth+1, mDrawValsPre->operator[](mDrawValsPre->size()-1));
        //Endpoint in bottom right corner
        cairo_line_to(cr, mWidth+1, mHeight+1);
        
        cairo_close_path(cr);
        
        cairo_path_t* pathPre = cairo_copy_path(cr);
        
        ////////////////////////////////////////////////////////////////////////////////PRE

        ////////////////////////////////////////////////////////////////////////////////POST

        //Starting point in bottom left corner.
        cairo_move_to(cr, -1, mHeight+1);
        
        //Draw data points
        for (int i = 0, x = 0; x < mWidth && i < mDrawValsPost->size(); i++) {
            cairo_line_to(cr, x, mDrawValsPost->operator[](i));
            x += mSpacing;
        }
        
        cairo_line_to(cr, mWidth+1, mDrawValsPost->operator[](mDrawValsPost->size()-1));
        //Endpoint in bottom right corner
        cairo_line_to(cr, mWidth+1, mHeight+1);
        
        cairo_close_path(cr);
        
        cairo_path_t* pathPost = cairo_copy_path(cr);
        
        //Starting point in bottom left corner.
        cairo_move_to(cr, -1, -1);
        
        //Draw data points
        for (int i = 0, x = 0; x < mWidth && i < mDrawValsPost->size(); i++) {
            cairo_line_to(cr, x, mDrawValsPost->operator[](i));
            x += mSpacing;
        }
        
        cairo_line_to(cr, mWidth+1, mDrawValsPost->operator[](mDrawValsPost->size()-1));
        //Endpoint in bottom right corner
        cairo_line_to(cr, mWidth+1, -1);
        
        cairo_close_path(cr);
        
        cairo_path_t* pathClip = cairo_copy_path(cr);

        ////////////////////////////////////////////////////////////////////////////////POST

        
        ////////////////////////////////////////////////////////////////////////////////POST
        
        //Starting point in top left corner.
        cairo_move_to(cr, -1, -1);
        
        //Draw data points
        for (int i = 0, x = 0; x < mWidth && i < mDrawValsGR->size(); i++) {
            cairo_line_to(cr, x, mDrawValsGR->operator[](i));
            x += mSpacing;
        }
        
        cairo_line_to(cr, mWidth+1, mDrawValsGR->operator[](mDrawValsGR->size()-1));
        
        //Endpoint in top right corner
        cairo_line_to(cr, mWidth+1, -1);
        
        cairo_close_path(cr);
        
        cairo_path_t* pathGR = cairo_copy_path(cr);
        
        ////////////////////////////////////////////////////////////////////////////////POST
        
        
        cairo_new_path(cr);
        
        cairo_append_path(cr, pathClip);
        cairo_clip(cr);
        cairo_new_path(cr);
        cairo_append_path(cr, pathPre);
        cairo_set_source_rgba(cr, mPreFillColor.R, mPreFillColor.G, mPreFillColor.B, mPreFillColor.A);
        cairo_fill(cr);
        
        cairo_reset_clip(cr);
        cairo_append_path(cr, pathPost);
        cairo_set_source_rgba(cr, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
        cairo_fill(cr);
        cairo_append_path(cr, pathPost);
        cairo_set_source_rgba(cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);
        cairo_set_line_width(cr, mLineWeight);
        cairo_stroke(cr);

        cairo_append_path(cr, pathGR);
        cairo_set_source_rgba(cr, mGRFillColor.R, mGRFillColor.G, mGRFillColor.B, mGRFillColor.A);
        cairo_fill(cr);
        cairo_append_path(cr, pathGR);
        cairo_set_source_rgba(cr, mGRLineColor.R, mGRLineColor.G, mGRLineColor.B, mGRLineColor.A);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);
        
        cairo_path_destroy(pathClip);
        cairo_path_destroy(pathPre);
        cairo_path_destroy(pathPost);
        cairo_path_destroy(pathGR);

        cairo_surface_flush(surface);
        
        unsigned int *data = (unsigned int*)cairo_image_surface_get_data(surface);
        //Bind to LICE
        LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap(data, this->mRECT.W(), this->mRECT.H(), this->mRECT.W(), false);
        
        //Render
        //}
#ifndef IPLUG_RETINA_SUPPORT
        IBitmap result(&WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
#else
        IBitmap result(&WrapperBitmap, &WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
#endif        
        return pGraphics->DrawBitmap(&result, &this->mRECT);
    }
    
protected:
    double mTimeScale, sr;
    int mBufferLength, mXRes, mSpacing, mYRange, mHeadroom;
    valarray<double> *mBufferPre, *mBufferPost, *mDrawValsPre, *mDrawValsPost, *mDrawValsGR;
    envFollower mEnvPre, mEnvPost, mEnvGR;
    
    CColor mPreFillColor, mGRLineColor, mGRFillColor;
    
    inline double percentToCoordinates(double value) {
        return mHeight - value * mHeight;
    }
    
    inline double scaleValue(double inValue, double inMin, double inMax, double outMin, double outMax){
        return ((outMax - outMin) * (inValue - inMin)) / (inMax - inMin) + outMin;
    }
};


class ICompressorPlotControl : public ICairoPlotControl
{
public:
    ICompressorPlotControl(IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* fillColor, IColor* lineColor, compressor* comp, bool fillEnable=false) : ICairoPlotControl(pPlug, pR, paramIdx, fillColor, lineColor, fillEnable), mYRange(-32), mHeadroom(2.){
        setLineWeight(2.);
        mComp = comp;
    }
    
    void calc(){
        double threshCoord = scaleValue(mComp->getThreshold(), mYRange, mHeadroom, 0, mWidth);
        
        x1  = scaleValue(mComp->getKneeBoundL(), mYRange, mHeadroom, 0, mWidth) ;
        y1 = mHeight - x1;
        
        xCP = threshCoord;
        yCP = mHeight - threshCoord;
        
        x2 = scaleValue(mComp->getKneeBoundU(), mYRange, mHeadroom, 0, mWidth);
        y2 = yCP - ((x2 - xCP) / mComp->getRatio());
        
        x3 = mWidth+2;
        
        y3 = yCP - ((mWidth + 2 - xCP) / mComp->getRatio());
        
        SetDirty();
    }
    
    bool Draw(IGraphics* pGraphics){
        
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        cairo_restore(cr);
        
        //surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
        //cr = cairo_create(surface);
        
        cairo_set_line_width(cr, mLineWeight);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_FAST);
        
        //fill background
        cairo_set_source_rgba(cr, mColorFill.R, mColorFill.G, mColorFill.B, mColorFill.A);
        cairo_rectangle(cr, 0, 0, mWidth, mHeight);
        cairo_fill(cr);
        
        cairo_set_source_rgba(cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);

        //Starting point in bottom left corner.
        cairo_move_to(cr, -1, mHeight + 1);
 
        if(mComp->getKnee() > 0.){
            cairo_line_to(cr, x1, y1);
            cairo_curve_to(cr, xCP, yCP, xCP, yCP, x2, y2);
            cairo_line_to(cr, x3, y3);
        }
        else{
            cairo_line_to(cr, xCP, yCP);
            cairo_line_to(cr, x3, y3);
        }
        
        cairo_stroke(cr);
        
        cairo_surface_flush(surface);
        
        unsigned int *data = (unsigned int*)cairo_image_surface_get_data(surface);
        //Bind to LICE
        LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap(data, this->mRECT.W(), this->mRECT.H(), this->mRECT.W(), false);
        
        //Render
        //}
#ifndef IPLUG_RETINA_SUPPORT
        IBitmap result(&WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
#else
        IBitmap result(&WrapperBitmap, &WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
#endif
        return pGraphics->DrawBitmap(&result, &this->mRECT);
    }
    
private:
    double mThreshold, mKnee, mRatio, mKneeBoundL, mKneeBoundU, mHeadroom;
    double x1, y1, xCP, yCP, x2, y2, x3, y3;
    int mYRange;
    compressor *mComp;
};


class IThresholdPlotControl : public ICairoPlotControl
{
public:
    IThresholdPlotControl(IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* lineColor, compressor* comp) : ICairoPlotControl(pPlug, pR, paramIdx, (IColor*)&COLOR_BLACK, lineColor, false), mYRange(-32), mHeadroom(2)
    {
        mComp = comp;
    }

    
    
    bool Draw(IGraphics* pGraphics){
        
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        cairo_restore(cr);
        
        //surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
        //cr = cairo_create(surface);
        
        cairo_set_line_width(cr, mLineWeight);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_FAST);
        
        cairo_set_source_rgba(cr, mColorLine.R, mColorLine.G, mColorLine.B, mColorLine.A);
        
        double dashes[] = {6.0,  /* ink */
            3.0,  /* skip */
            6.0,  /* ink */
            3.0   /* skip*/
        };
        int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
        double offset = -5.0;
        
        cairo_set_dash (cr, dashes, ndash, offset);
        
        double threshCoord = scaleValue(mComp->getThreshold(), mYRange, mHeadroom, 0, mHeight);
        
        cairo_move_to(cr, threshCoord, 0);
        
        cairo_line_to(cr, threshCoord, mHeight);
        
        cairo_stroke(cr);
        
        cairo_move_to(cr, 0, mHeight - threshCoord);
        cairo_line_to(cr, mWidth, mHeight-threshCoord);
        
        cairo_stroke(cr);
        
        cairo_pattern_t* grad = cairo_pattern_create_linear(mHeight, 0, mHeight+5, 0);
        
        cairo_pattern_add_color_stop_rgba(grad, 0, .1, .1, .1, .4);
        cairo_pattern_add_color_stop_rgba(grad, 1, .1, .1, .1, 0);
        
        cairo_set_source(cr, grad);
   
        cairo_rectangle(cr, mHeight, 0, mWidth, mHeight);

        cairo_fill(cr);
        cairo_pattern_destroy(grad);
        
        cairo_surface_flush(surface);
        
        unsigned int *data = (unsigned int*)cairo_image_surface_get_data(surface);
        //Bind to LICE
        LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap(data, this->mRECT.W(), this->mRECT.H(), this->mRECT.W(), false);
        
        //Render
        //}
#ifndef IPLUG_RETINA_SUPPORT
        IBitmap result(&WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
#else
        IBitmap result(&WrapperBitmap, &WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
#endif
        return pGraphics->DrawBitmap(&result, &this->mRECT);
    }
    
    
private:
    int mYRange;
    double mHeadroom;
    compressor* mComp;
};
#endif /* ICairoControls.h */
