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
  kGainHandlesX = 65,
  kCeilingX = 554,
  kMeterX = 563,
  kGainY = 85,
  
  kGainCaptionX = 66,
  kCeilingCaptionX = 554,

  kCaptionY = 344,
  
  kCaptionHeight = 50,
  kCaptionWidth = 62,
  
  kQualityX = 368,
  kQualityY = 351,
  
  kFaderLength = 264,
  
  kSliderFrames = 100
};




DClip::DClip(IPlugInstanceInfo instanceInfo)
:	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mGain(0.), mCeiling(0.), envPlotIn(0, 75, 75, GetSampleRate()), envPlotOut(0, 75, 75, GetSampleRate()), envMeter(0, 400, 50, GetSampleRate()), envGR(0, 75, 85, GetSampleRate()), mGainSmoother(5., GetSampleRate()), mCeilingSmoother(5., GetSampleRate()), duration(frameTime)
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
  pGraphics->AttachControl(plot);
  
  plotOut = new ILevelPlotControl(this, plotRECT, kPlot, &plotPostFillColor, &plotPostLineColor, 5);
  plotOut->setLineWeight(2.);
  plotOut->setResolution(ILevelPlotControl::kHighRes);
  plotOut->setYRange(ILevelPlotControl::k32dB);
  pGraphics->AttachControl(plotOut);
  
  GRplot = new ILevelPlotControl(this, plotRECT, kPlot, &grFillColor, &grLineColor, 5);
  GRplot->setLineWeight(1.5);
  GRplot->setReverseFill(true);
  //GRplot->setStroke(false);
  GRplot->setResolution(ILevelPlotControl::kHighRes);
  GRplot->setYRange(ILevelPlotControl::k32dB);
  pGraphics->AttachControl(GRplot);
  
  mDBMeter = new IBitmapControl(this, kGainX, kGainY, &slider);
  mGainSliderHandles = new IFaderControl(this, kGainHandlesX, kGainY - 5, kFaderLength, kGain, &sliderHandles);
  mCeilingSliderHandles = new IFaderControl(this, kCeilingX, kGainY - 5, kFaderLength, kCeiling, &sliderHandles);
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
  
  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
}

DClip::~DClip() {}

void DClip::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

  double* in1 = inputs[0];
  double* in2 = inputs[1];
  double* out1 = outputs[0];
  double* out2 = outputs[1];

  for (int s = 0; s < nFrames; ++s, ++in1, ++in2, ++out1, ++out2)
  {
    double sample1, sample2, inMax, gainSmoothed, ceilingSmoothedAmp, GR1, GR2;
    GR1 = 0;
    GR2 = 0;
    gainSmoothed = mGainSmoother.process(mGain);
    ceilingSmoothedAmp = DBToAmp(mCeilingSmoother.process(mCeiling));
    
    
    sample1 = *in1 * DBToAmp(gainSmoothed);
    sample2 = *in2 * DBToAmp(gainSmoothed);
    
    
    inMax = std::max(sample1, sample2);
    plot->process(AmpToDB(envPlotIn.process(inMax)));
    mDBMeter->SetValueFromPlug(scaleValue(mGain, kGainMin, kGainMax, 0, 1));

    
    if (sample1 > ceilingSmoothedAmp) {
      GR1 = sample1 - ceilingSmoothedAmp;
      sample1 = ceilingSmoothedAmp;
    }
    else if(sample1 < -1 * ceilingSmoothedAmp){
      GR1 = -1 * sample1 - ceilingSmoothedAmp;
      sample1 = -1 * ceilingSmoothedAmp;
    }
    
    if (sample2 > ceilingSmoothedAmp) {
      GR2 = sample2 - ceilingSmoothedAmp;
      sample2 = ceilingSmoothedAmp;
    }
    else if(sample2 < -1 * ceilingSmoothedAmp){
      GR2 =  -1 * sample2 - ceilingSmoothedAmp;
      sample2 = -1 * ceilingSmoothedAmp;
    }
    
    *out1 = sample1;
    *out2 = sample2;

    
    plotOut->process(AmpToDB(envPlotOut.process(std::max(sample1, sample2))));
    double gr = envGR.process(std::max(GR1,GR2));
    GRplot->process(scaleValue(AmpToDB(gr), -32, 2, 2, -32));

    mGRMeter->SetValueFromPlug(scaleValue(mCeiling, kCeilingMin, kCeilingMax, 1, 0));
    
 //   duration = (clock() - start_time) / (double)CLOCKS_PER_SEC;
    //if ( duration >= frameTime) {
      mGRMeter->SetDirty();
      mDBMeter->SetDirty();
      plot->SetDirty();
      plotOut->SetDirty();
      mShadow->SetDirty();
      start_time = clock();
      
    //}

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
      
    default:
      break;
  }
}

double DClip::scaleValue(double inValue, double inMin, double inMax, double outMin, double outMax){
  return ((outMax - outMin) * (inValue - inMin)) / (inMax - inMin) + outMin;
}