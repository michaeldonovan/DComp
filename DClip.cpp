#include "DClip.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
  kGain = 0,
  kCeiling,
  kMode,
  kQuality,
  kPlotRes,
  kPlotRange,
  kPlot,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kGainX = 74,
  kGainHandlesX = 48,
  kCeilingX = 537,
  kMeterX = 563,
  kGainY = 85,
  
  kGainCaptionX = 66,
  kCeilingCaptionX = 554,

  kCaptionY = 344,
  
  kCaptionHeight = 50,
  kCaptionWidth = 62,
  
  kQualityX = 368,
  kQualityY = 351,
  
  kFaderLength = 280,
  
  kSliderFrames = 100
};




DClip::DClip(IPlugInstanceInfo instanceInfo)
:	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mGain(0.), mCeiling(0.), envPlotIn(0, 100, 75, GetSampleRate()), envPlotOut(0, 100, 75, GetSampleRate()), envMeter(0, 400, 50, GetSampleRate()), envGR(0, 500, 10, GetSampleRate()), mGainSmoother(5., GetSampleRate()), mCeilingSmoother(5., GetSampleRate()), duration(frameTime), mOversampling(2)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kGain)->InitDouble("Gain", 0., kGainMin, kGainMax, 0.01, "dB");
  GetParam(kCeiling)->InitDouble("Ceiling", 0., kCeilingMin, kCeilingMax, 0.01, "dB");
  GetParam(kQuality)->InitEnum("Quality", 0, 3);
  GetParam(kQuality)->SetDisplayText(0, "2x");
  GetParam(kQuality)->SetDisplayText(1, "4x");
  GetParam(kQuality)->SetDisplayText(2, "8x");

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight, 30);
  //pGraphics->SetStrictDrawing(false);
  
  
  IBitmap slider = pGraphics->LoadIBitmap(SLIDER_ID, SLIDER_FN, kSliderFrames);
  IBitmap sliderHandles = pGraphics->LoadIBitmap(SLIDERHANDLES_ID, SLIDERHANDLES_FN);
  IBitmap sliderRev = pGraphics->LoadIBitmap(SLIDERREV_ID, SLIDERREV_FN, kSliderFrames);
  IBitmap shadow = pGraphics->LoadIBitmap(SHADOW_ID, SHADOW_FN);

  pGraphics->AttachBackground(BACKGROUND_ID, BACKGROUND_FN);
  
  
  
  IRECT plotRECT = IRECT(152, 85, 527, 337);
  IText caption = IText(16, &COLOR_WHITE, "Futura", IText::kStyleNormal, IText::kAlignCenter);
  
  plot = new ILevelPlotControl(this, plotRECT, kPlot, &plotPreFillColor, &plotPreLineColor, 5);
  plot->setResolution(ILevelPlotControl::kHighRes);
  plot->setYRange(ILevelPlotControl::k32dB);
  plot->setStroke(false);
  plot->setSampleRate(GetSampleRate() * mOversampling);
  pGraphics->AttachControl(plot);
  
  plotOut = new ILevelPlotControl(this, plotRECT, kPlot, &plotPostFillColor, &plotPostLineColor, 5);
  plotOut->setLineWeight(2.);
  plotOut->setResolution(ILevelPlotControl::kHighRes);
  plotOut->setYRange(ILevelPlotControl::k32dB);
  plotOut->setSampleRate(GetSampleRate() * mOversampling);
  pGraphics->AttachControl(plotOut);
  
  
  mDBMeter = new IBitmapControl(this, kGainX, kGainY, &slider);
  mGainSliderHandles = new IFaderControl(this, kGainHandlesX, kGainY - 10, kFaderLength, kGain, &sliderHandles);
  mCeilingSliderHandles = new IFaderControl(this, kCeilingX, kGainY - 10, kFaderLength, kCeiling, &sliderHandles);
  mGRMeter = new IBitmapControl(this, kMeterX, kGainY, &sliderRev);

  
  
  mGainCaption = new ICaptionControl(this, IRECT(kGainCaptionX, kCaptionY, kGainCaptionX+kCaptionWidth, kCaptionY+kCaptionHeight+50), kGain, &caption, true);
  mCeilingCaption = new ICaptionControl(this, IRECT(kCeilingCaptionX, kCaptionY, kCeilingCaptionX+kCaptionWidth, kCaptionY+kCaptionHeight+50), kCeiling, &caption, true);
  
  mShadow = new IBitmapControl(this, plotRECT.L , plotRECT.T, &shadow);
  
  pGraphics->AttachControl(new ITextControl(this, IRECT(kQualityX, kQualityY, kQualityX+50, kQualityY+30), &caption, "Quality:"));
  pGraphics->AttachControl(new IPopUpMenuControl(this, IRECT(kQualityX + 55, kQualityY-1, kQualityX+100, kQualityY+20), yellow, yellow, yellow, kQuality));
  pGraphics->AttachControl(mDBMeter);
  pGraphics->AttachControl(mGainSliderHandles);
  pGraphics->AttachControl(mGRMeter);
  pGraphics->AttachControl(mCeilingSliderHandles);
  pGraphics->AttachControl(mGainCaption);
  pGraphics->AttachControl(mCeilingCaption);
  pGraphics->AttachControl(mShadow);
  
 // mKnob = new IKnobMultiControl(this, 50, 50, kKnob, &knob);
 // pGraphics->AttachControl(mKnob);
 // pGraphics->AttachControl(new IKnobMultiControl(this, 150,150, kGain, &knob));
  AttachGraphics(pGraphics);
  
  start_time = clock();
  stop_time = clock();
  
  mAntiAlias.Calc(0.5 / (double)mOversampling);
  mUpsample.Reset();
  mDownsample.Reset();

  
  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
}

DClip::~DClip() {}

void DClip::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.
  
  for (int i = 0; i < channelCount; i++) {
    double* input = inputs[i];
    double* output = outputs[i];
    
    for (int s = 0; s < nFrames; ++s, ++input, ++output) {
      {
        double sample, gainSmoothed, ceilingSmoothedAmp, GR1;
        GR1 = 0;
        gainSmoothed = mGainSmoother.process(mGain);
        ceilingSmoothedAmp = DBToAmp(mCeilingSmoother.process(mCeiling));
        
        
        sample = *input * DBToAmp(gainSmoothed);
        
        
        plot->process(AmpToDB(envPlotIn.process(sample)));
        mDBMeter->SetValueFromPlug(scaleValue(AmpToDB(envMeter.process(sample)), kCeilingMin, 2, 0, 1));
        
        
        for (int j = 0; j < mOversampling; ++j)
        {
          //UpSample
          if (j > 0) sample = 0.;
          mUpsample.Process(sample, mAntiAlias.Coeffs());
          sample = (double)mOversampling * mUpsample.Output();
          
          
          if (WDL_DENORMAL_OR_ZERO_DOUBLE_AGGRESSIVE(&sample))
            sample = 0.;
          else if (sample > ceilingSmoothedAmp) {
            GR1 = sample - ceilingSmoothedAmp;
            sample = ceilingSmoothedAmp;
          }
          else if(sample < -1 * ceilingSmoothedAmp){
            GR1 = -1 * sample - ceilingSmoothedAmp;
            sample = -1 * ceilingSmoothedAmp;
          }
          
          // Downsample
          mDownsample.Process(sample, mAntiAlias.Coeffs());
          if (j == 0) *output = mDownsample.Output();
        }
        
        
        
        plotOut->process(AmpToDB(envPlotOut.process(sample)));
        mGRMeter->SetValueFromPlug(scaleValue(AmpToDB(envGR.process(GR1)), -32, 2, 0, 1));
        
        mGRMeter->SetDirty();
        mDBMeter->SetDirty();
        plot->SetDirty();
        plotOut->SetDirty();
        mShadow->SetDirty();
      }
    }
  }
}

void DClip::Reset()
{
  TRACE;
  IMutexLock lock(this);
}

void DClip::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);

  switch (paramIdx)
  {
    case kGain:
      mGain = GetParam(kGain)->Value();
      //mGainCaption->SetDirty();
      break;
      
    case kCeiling:
     mCeiling = GetParam(kCeiling)->Value();
//      mCeilingCaption->SetDirty();
      break;
      
    case kQuality:
      mOversampling = pow(2, GetParam(kQuality)->Value() + 1);
      mAntiAlias.Calc(0.5 / (double)mOversampling);
      mUpsample.Reset();
      mDownsample.Reset();

      plot->setSampleRate(GetSampleRate() * mOversampling);
      plotOut->setSampleRate(GetSampleRate() * mOversampling);
      break;
      
    default:
      break;
  }
}

double DClip::scaleValue(double inValue, double inMin, double inMax, double outMin, double outMax){
  return ((outMax - outMin) * (inValue - inMin)) / (inMax - inMin) + outMin;
}