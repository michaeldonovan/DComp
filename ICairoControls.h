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
    ICairoPlotControl(IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* fillColor, IColor* lineColor, bool fillEnable=true) : IControl(pPlug, pR), mColorFill(fillColor), mColorLine(lineColor), mFill(fillEnable), mRange(1), mLineWeight(4.)
    {
        mWidth = mRECT.W();
        mHeight = mRECT.H();
        
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
        double mSpacing = mVals->size() / (double)mWidth;
        
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        cairo_restore(cr);
        
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
        cr = cairo_create(surface);
        
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
        LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap(data, this->mRECT.W(), this->mRECT.H(), this->mRECT.W(), false);
        
        //Render
        IBitmap result(&WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
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

};





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
    
    ILevelPlotControl(IPlugBase* pPlug, IRECT pR, int paramIdx, IColor* fillColor, IColor* lineColor, double timeScale=5., bool fillEnable=true) : ICairoPlotControl(pPlug, pR, paramIdx, fillColor, lineColor, fillEnable), mTimeScale(timeScale), mBufferLength(0.), mYRange(-32)
    {
        mXRes = mWidth/2.;
        mDrawVals = new valarray<double>(mHeight, mXRes);
        mBuffer = new valarray<double>(0., mTimeScale * mPlug->GetSampleRate() / (double)mXRes);
        setResolution(kHighRes);
        setLineWeight(2.);
    }
    
    ~ILevelPlotControl(){ }
    
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
        mDrawVals->resize(mXRes, mHeight);
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
            
            //setVals(mDrawVals);
        }
    }
    
    
    bool Draw(IGraphics* pGraphics){
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        cairo_restore(cr);
        
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, mWidth, mHeight);
        cr = cairo_create(surface);
        
        cairo_set_line_width(cr, mLineWeight);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_GOOD);
        
        //Starting point in bottom left corner.
        cairo_move_to(cr, 0, mHeight);
        
        //Draw data points
        for (int i = 0, x = 0; x < mWidth && i < mDrawVals->size(); i++) {
            cairo_line_to(cr, x, mDrawVals->operator[](i));
            x += mSpacing;
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
        LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap(data, this->mRECT.W(), this->mRECT.H(), this->mRECT.W(), false);
        
        //Render
        IBitmap result(&WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
        return pGraphics->DrawBitmap(&result, &this->mRECT);
    }
    
private:
    double mTimeScale;
    int mBufferLength, mXRes, mSpacing, mYRange;
    valarray<double> *mBuffer, *mDrawVals;
    
    double percentToCoordinates(double value) {
        return getHeight() - value * getHeight();
    }
    
    double scaleValue(double inValue, double inMin, double inMax, double outMin, double outMax){
        return ((outMax - outMin) * (inValue - inMin)) / (inMax - inMin) + outMin;
    }

    
};


#endif /* ICairoControls.h */
