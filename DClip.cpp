#include "DClip.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
  kGain = 0,
  kCeiling,
  kAttack,
  kRelease,
  kKnee,
  kMode,
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
:	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mGain(0.), mCeiling(0.), mGainSmoother(5., GetSampleRate()), mCeilingSmoother(5., GetSampleRate())
{
  TRACE;
  
  //Envelope Followers
  envGR.init(0, 75, 85, GetSampleRate());
  envPlotIn.init(0, 75, 75, GetSampleRate());
  envPlotOut.init(0, 75, 75, GetSampleRate());
  compR.init(10., 100., 0., 4, .5, GetSampleRate());
  compL.init(10., 100., 0., 4, .5, GetSampleRate());
  compR.setMode(compressor::kLimiter);
  compL.setMode(compressor::kLimiter);

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kGain)->InitDouble("Gain", 0., kGainMin, kGainMax, 0.01, "dB");
  GetParam(kCeiling)->InitDouble("Ceiling", 0., kCeilingMin, kCeilingMax, 0.01, "dB");
  GetParam(kAttack)->InitDouble("Attack", 10., 0., 250., 0.01, "ms");
  GetParam(kRelease)->InitDouble("Release", 500., 0., 750., 0.01, "ms");
  GetParam(kKnee)->InitDouble("Knee", 0.5, 0., 1., 0.01, "%");
  GetParam(kMode)->InitEnum("Mode", 0, 1);
  GetParam(kMode)->SetDisplayText(0, "Limit");
  GetParam(kMode)->SetDisplayText(1, "Clip");

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
    double sampleDry1, sampleDry2, sample1, sample2, inMax, gainSmoothed, ceilingSmoothedAmp, GRL, GRR;
    GRL = 0;
    GRR = 0;
    gainSmoothed = mGainSmoother.process(mGain);
    ceilingSmoothedAmp = DBToAmp(mCeilingSmoother.process(mCeiling));
    
    
    sample1 = *in1 * DBToAmp(gainSmoothed);
    sample2 = *in2 * DBToAmp(gainSmoothed);
    sampleDry1 = sample1;
    sampleDry2 = sample2;
    
    inMax = std::max(sample1, sample2);
    plot->process(AmpToDB(envPlotIn.process(inMax)));
    mDBMeter->SetValueFromPlug(scaleValue(mGain, kGainMin, kGainMax, 0, 1));
    
    if(mMode==0){
      sample1 = compL.process(sample1);
      sample2 = compL.process(sample2);
      GRL = compL.getGainReductionDB();
      GRR = compR.getGainReductionDB();
    }
    else if(mMode==1){
      if (sample1 > ceilingSmoothedAmp) {
        GRL = sample1 - ceilingSmoothedAmp;
        sample1 = ceilingSmoothedAmp;
      }
      else if(sample1 < -1 * ceilingSmoothedAmp){
        GRL = -1 * sample1 - ceilingSmoothedAmp;
        sample1 = -1 * ceilingSmoothedAmp;
      }
      
      if (sample2 > ceilingSmoothedAmp) {
        GRR = sample2 - ceilingSmoothedAmp;
        sample2 = ceilingSmoothedAmp;
      }
      else if(sample2 < -1 * ceilingSmoothedAmp){
        GRR =  -1 * sample2 - ceilingSmoothedAmp;
        sample2 = -1 * ceilingSmoothedAmp;
      }
    }
  
    *out1 = sample1;
    *out2 = sample2;

    
    
    plotOut->process(AmpToDB(envPlotOut.process(std::max(sample1, sample2))));
    if(mMode == 0){
      GRplot->process(scaleValue(std::max(GRL, GRR), -32, 2, 2, -32));
    }
    else{
      GRplot->process(scaleValue(envGR.process(AmpToDB(std::max(GRL,GRR))), -32, 2, 2, -32));

    }

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
      
    case kAttack:
      mAttack = GetParam(kAttack)->Value();
      break;
      
    case kRelease:
      mRelease = GetParam(kRelease)->Value();
      break;
      
    case kKnee:
      mKnee = GetParam(kKnee)->Value();
      break;
      
    case kMode:
      mMode = GetParam(kMode)->Value();
   
    default:
      break;
  }
}

double DClip::scaleValue(double inValue, double inMin, double inMax, double outMin, double outMax){
  return ((outMax - outMin) * (inValue - inMin)) / (inMax - inMin) + outMin;
}