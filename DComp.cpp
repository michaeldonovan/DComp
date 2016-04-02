#include "DComp.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
  kGain = 0,
  kThreshold,
  kAttack,
  kRelease,
  kHold,
  kRatio,
  kKnee,
  kMode,
  kMix,
  kSidechain,
  kSCAudition,
  kCutoffHP,
  kCutoffLP,
  kHPEnable,
  kLPEnable,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kGainX = 74,
  kGainHandlesX = 65,
  kThresholdX = 554,
  kMeterX = 563,
  kGainY = 85,
  
  kGainCaptionX = 66,
  kThresholdCaptionX = 554,

  kCaptionY = 344,
  
  kCaptionHeight = 50,
  kCaptionWidth = 62,
  
  kQualityX = 368,
  kQualityY = 351,
  
  kFaderLength = 264,
  
  kSliderFrames = 100
};




DComp::DComp(IPlugInstanceInfo instanceInfo)
:	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mGain(0.), mThreshold(0.)
{
  TRACE;
  
  //Param Smoothers
  mGainSmoother.init(5., GetSampleRate());
  mThresholdSmoother.init(5., GetSampleRate());
  mAttackSmoother.init(5., GetSampleRate());
  mReleaseSmoother.init(5., GetSampleRate());
  mHoldSmoother.init(5., GetSampleRate());
  mRatioSmoother.init(5., GetSampleRate());
  mMixSmoother.init(5., GetSampleRate());
  mHPSmoother.init(5., GetSampleRate());
  mLPSmoother.init(5., GetSampleRate());

  
  //Envelope Followers
  envGR.init(0, 75, 85, GetSampleRate());
  envPlotIn.init(0, 75, 75, GetSampleRate());
  envPlotOut.init(0, 75, 75, GetSampleRate());
  mComp.init(10., 100., 0., 4, .5, GetSampleRate());


  //Parameters
  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kGain)->InitDouble("Gain", 0., kGainMin, kGainMax, 0.01, "dB");
  GetParam(kThreshold)->InitDouble("Threshold", 0., kThresholdMin, kThresholdMax, 0.01, "dB");
  GetParam(kAttack)->InitDouble("Attack", 10., 0., 250., 1., "ms");
  GetParam(kHold)->InitDouble("Hold", 0., 0., 250., 1., "ms");
  GetParam(kRelease)->InitDouble("Release", 500., 0., 750., 1., "ms");
  GetParam(kRatio)->InitDouble("Ratio", 2., 1., 10., 0.1, ": 1");
  GetParam(kKnee)->InitDouble("Knee", 0.5, 0., 1., 0.01, "%");
  GetParam(kMix)->InitDouble("Mix", 100., 0., 100., 1., "%");
  GetParam(kCutoffHP)->InitDouble("Highpass", 20., 20., 20000, 1, "Hz");
  GetParam(kCutoffLP)->InitDouble("Lowpass", 20000., 20., 20000, 1, "Hz");
  GetParam(kHPEnable)->InitBool("Highpass: Enabled", false);
  GetParam(kLPEnable)->InitBool("Lowpass: Enabled", false);

  GetParam(kSidechain)->InitBool("Sidechain", false);
  GetParam(kSCAudition)->InitBool("Audition Sidechain", false);
  
  GetParam(kMode)->InitEnum("Mode", 0, 1);
  GetParam(kMode)->SetDisplayText(0, "Clean");

  
  //Filters
  mHighpass.setSampleRate(GetSampleRate());
  mHighpass.setFilter(SVFHighpass, mCuttoffHP, 0.707, 0.);
  mLowpass.setSampleRate(GetSampleRate());
  mLowpass.setFilter(SVFLowpass, mCuttoffLP, 0.707, 0.);
  
  
  
  
  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight, 30);
  
  IBitmap slider = pGraphics->LoadIBitmap(SLIDER_ID, SLIDER_FN, kSliderFrames);
  IBitmap sliderHandles = pGraphics->LoadIBitmap(SLIDERHANDLES_ID, SLIDERHANDLES_FN);
  IBitmap sliderRev = pGraphics->LoadIBitmap(SLIDERREV_ID, SLIDERREV_FN, kSliderFrames);
  IBitmap shadow = pGraphics->LoadIBitmap(SHADOW_ID, SHADOW_FN);

  pGraphics->AttachBackground(BACKGROUND_ID, BACKGROUND_FN);
  
  IRECT plotRECT = IRECT(152, 85, 527, 337);
  IText caption = IText(16, &COLOR_WHITE, "Futura", IText::kStyleNormal, IText::kAlignCenter);
  
  plot = new ILevelPlotControl(this, plotRECT, -1, &plotPreFillColor, &plotPreLineColor, 5);
  plot->setResolution(ILevelPlotControl::kHighRes);
  plot->setYRange(ILevelPlotControl::k32dB);
  plot->setStroke(false);
  pGraphics->AttachControl(plot);
  
  plotOut = new ILevelPlotControl(this, plotRECT, -1, &plotPostFillColor, &plotPostLineColor, 5);
  plotOut->setLineWeight(2.);
  plotOut->setResolution(ILevelPlotControl::kHighRes);
  plotOut->setYRange(ILevelPlotControl::k32dB);
  pGraphics->AttachControl(plotOut);


  
  
  GRplot = new ILevelPlotControl(this, plotRECT, -1, &grFillColor, &grLineColor, 5);
  GRplot->setLineWeight(1.5);
  GRplot->setReverseFill(true);
  //GRplot->setStroke(false);
  GRplot->setResolution(ILevelPlotControl::kHighRes);
  GRplot->setYRange(ILevelPlotControl::k32dB);
  pGraphics->AttachControl(GRplot);
  
  mGainFader = new IBitmapControl(this, kGainX, kGainY, &slider);
  mGainFaderHandle = new IFaderControl(this, kGainHandlesX, kGainY - 5, kFaderLength, kGain, &sliderHandles);

  mGainCaption->SetValueFromPlug(mGain);
  mThresholdCaption->SetValueFromPlug(mThreshold);

  
  
  mGainCaption = new ICaptionControl(this, IRECT(kGainCaptionX, kCaptionY, kGainCaptionX+kCaptionWidth, kCaptionY+kCaptionHeight+50), kGain, &caption, true);
  mThresholdCaption = new ICaptionControl(this, IRECT(kThresholdCaptionX, kCaptionY, kThresholdCaptionX+kCaptionWidth, kCaptionY+kCaptionHeight+50), kThreshold, &caption, true);
  
  
  
  mShadow = new IBitmapControl(this, plotRECT.L , plotRECT.T, &shadow);
  

  pGraphics->AttachControl(mGainFader);
  pGraphics->AttachControl(mGainFaderHandle);
  pGraphics->AttachControl(mGainCaption);
  pGraphics->AttachControl(mThresholdCaption);
  pGraphics->AttachControl(mShadow);
  
 // mKnob = new IKnobMultiControl(this, 50, 50, kKnob, &knob);
 // pGraphics->AttachControl(mKnob);
 // pGraphics->AttachControl(new IKnobMultiControl(this, 150,150, kGain, &knob));
  AttachGraphics(pGraphics);
  
  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
}

DComp::~DComp() {}

void DComp::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

  double* in1 = inputs[0];
  double* in2 = inputs[1];
  double* out1 = outputs[0];
  double* out2 = outputs[1];

  for (int s = 0; s < nFrames; ++s, ++in1, ++in2, ++out1, ++out2)
  {

    gainSmoothed = mGainSmoother.process(mGain);
    mComp.setThreshold(mThresholdSmoother.process(mThreshold));
    mComp.setAttack(mAttackSmoother.process(mAttack));
    double sampleDry1, sampleDry2, sample1, sample2, inMax, gr, gainSmoothed, mixSmoothed;

    if(gainSmoothed != mGain) gainSmoothed = mGainSmoother.process(mGain);
    if(mComp.getAttack() != mAttack) mComp.setAttack(mAttackSmoother.process(mAttack));
    if(mComp.getRelease() != mRelease) mComp.setRelease(mReleaseSmoother.process(mRelease));
    if(mComp.getHold() != mHold) mComp.setHold(mHoldSmoother.process(mHold));
    if(mComp.getRatio() != mRatio) mComp.setRatio(mAttackSmoother.process(mRatio));
    if(mComp.getThreshold() != mThreshold) mComp.setThreshold(mThresholdSmoother.process(mThreshold));
    if(mixSmoothed != mMix) mixSmoothed = mMixSmoother.process(mMix);
    if(mLowpass.getCutoff() != mCuttoffLP) mLowpass.setCutoffFreq(mLPSmoother.process(mCuttoffLP));
    if(mHighpass.getCutoff() != mCuttoffHP) mHighpass.setCutoffFreq(mHPSmoother.process(mCuttoffHP));
    
    
    sample1 = *in1 * DBToAmp(gainSmoothed);
    sample2 = *in2 * DBToAmp(gainSmoothed);
    sampleDry1 = sample1;
    sampleDry2 = sample2;
    
    inMax = std::max(sample1, sample2);
    plot->process(AmpToDB(envPlotIn.process(inMax)));
    mGainFader->SetValueFromPlug(scaleValue(mGain, kGainMin, kGainMax, 0, 1));
    
    
    sample1 *= DBToAmp(gainSmoothed);
    sample2 *= DBToAmp(gainSmoothed);
    *out1 = sample1;
    *out2 = sample2;

    
    
    plotOut->process(AmpToDB(envPlotOut.process(std::max(sample1, sample2))));

    
    GRplot->process(scaleValue(envGR.process(gr)), -32, 2, 2, -32));

    

    
    plot->SetDirty();
    plotOut->SetDirty();
    mShadow->SetDirty();

  }
  
}

void DComp::Reset()
{
  TRACE;
  IMutexLock lock(this);
}

void DComp::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);

  switch (paramIdx)
  {
    case kGain:
      mGain = GetParam(kGain)->Value();
      mGainCaption->SetValueFromPlug(mGain);
      mGainFader->SetDirty();
      break;
      
    case kThreshold:
      mThreshold = GetParam(kThreshold)->Value();
      mThresholdCaption->SetValueFromPlug(mThreshold);

      break;
      
    case kAttack:
      mAttack = GetParam(kAttack)->Value();
      break;
      
    case kHold:
      mHold = GetParam(kHold)->Value();
      break;
      
    case kRelease:
      mRelease = GetParam(kRelease)->Value();
      break;
      
    case kRatio:
      mRatio = GetParam(kRatio)->Value();
      break;
      
    case kKnee:
      mKnee = GetParam(kKnee)->Value();
      break;
      
    case kMode:
      mMode = GetParam(kMode)->Value();
      break;
      
    case kMix:
      mMix = GetParam(kMix)->Value() / 100.;
      break;
      
    case kCutoffHP:
      mCuttoffHP = GetParam(kCutoffHP)->Value();
      break;
      
    case kCutoffLP:
      mCuttoffLP = GetParam(kCutoffLP)->Value();
      break;
      
    case kSidechain:
      mSidechainEnable = GetParam(kSidechain)->Value();
      break;
    
    case kSCAudition:
      mSCAudition = GetParam(kSCAudition)->Value();
      break;
      
    case kLPEnable:
      mLPEnable = GetParam(kLPEnable)->Value();
      break;
      
    case kHPEnable:
      mHPEnable = GetParam(kHPEnable)->Value();
      break;
      
    default:
      break;
  }
}

inline double DComp::scaleValue(double inValue, double inMin, double inMax, double outMin, double outMax){
  return ((outMax - outMin) * (inValue - inMin)) / (inMax - inMin) + outMin;
}