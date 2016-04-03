#include "DComp.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

using std::max;

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

  kSlidersY = 88,
  kThresholdX = 21,
  kGainX = 505,
  kMixX = 559,
  
  kBigKnobsY = 322,
  kAttackX = 22,
  kReleaseX = kAttackX + 104,
  
  kSmallKnobsY = 354,
  kHoldX = 238,
  kRatioX = kHoldX + 68,
  kKneeX = kRatioX + 68,
  kSCKnobsX = 547,
  kSCKnobY= 318,
  kSCKnob2Y = kSCKnobY + 57,
  kFaderLength = 264,
  
  kPlotTimeScale = 4,
  
  
  kKnobFrames = 63,
  kSliderFrames = 68
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
  mKneeSmoother.init(5., GetSampleRate());
  
  //Envelope Followers
  envPlotIn.init(15, 100, 15, GetSampleRate());
  envPlotOut.init(15, 100, 15, GetSampleRate());
  mComp.init(10., 100., 0., 4, .5, GetSampleRate());


  //Parameters
  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kGain)->InitDouble("Gain", 0., kGainMin, kGainMax, 0.01, "dB");
  GetParam(kThreshold)->InitDouble("Threshold", -4., kThresholdMin, kThresholdMax, 0.01, "dB");
  GetParam(kAttack)->InitDouble("Attack", 10., 0., 250., .1, "ms");
  GetParam(kAttack)->SetShape(2.);
  GetParam(kHold)->InitDouble("Hold", 0., 0., 300., 1., "ms");
  GetParam(kRelease)->InitDouble("Release", 250, 10., 1000., 1., "ms");
  GetParam(kRelease)->SetShape(2.);
  GetParam(kRatio)->InitDouble("Ratio", 4., 1., 100., 0.1, ": 1");
  GetParam(kRatio)->SetShape(4.);
  GetParam(kKnee)->InitDouble("Knee", 0.5, 0., 1., 0.1);
  GetParam(kMix)->InitDouble("Mix", 100., 0., 100., 1., "%");
  GetParam(kCutoffHP)->InitDouble("Highpass", 20., 20., 20000, 1, "Hz");
  GetParam(kCutoffHP)->SetShape(2.);
  GetParam(kCutoffLP)->InitDouble("Lowpass", 20000., 20., 20000, 1, "Hz");
  GetParam(kCutoffLP)->SetShape(2.);
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
  IBitmap sliderRev = pGraphics->LoadIBitmap(SLIDERREV_ID, SLIDERREV_FN, kSliderFrames);
  IBitmap shadow = pGraphics->LoadIBitmap(SHADOW_ID, SHADOW_FN);
  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
  IBitmap smallKnob = pGraphics->LoadIBitmap(SMALLKNOB_ID, SMALLKNOB_FN, kKnobFrames);
  pGraphics->AttachBackground(BACKGROUND_ID, BACKGROUND_FN);
  
  //374 x 183
  IRECT plotRECT = IRECT(101, 71, 475, 254);
  IText caption = IText(14, &COLOR_WHITE, "Futura", IText::kStyleNormal, IText::kAlignCenter);
  
  plot = new ILevelPlotControl(this, plotRECT, -1, &plotPreFillColor, &plotPreLineColor, kPlotTimeScale);
  plot->setResolution(ILevelPlotControl::kHighRes);
  plot->setYRange(ILevelPlotControl::k32dB);
  plot->setGradientFill(true);
  plot->setStroke(false);
  pGraphics->AttachControl(plot);
  
  plotOut = new ILevelPlotControl(this, plotRECT, -1, &plotPostFillColor, &plotPostLineColor, kPlotTimeScale);
  plotOut->setStroke(true);
  plotOut->setResolution(ILevelPlotControl::kMaxRes);
  plotOut->setLineWeight(2.);
  plotOut->setYRange(ILevelPlotControl::k32dB);
  plotOut->setGradientFill(true);
  pGraphics->AttachControl(plotOut);

  
  GRplot = new ILevelPlotControl(this, plotRECT, -1, &grFillColor, &grLineColor, kPlotTimeScale);
  GRplot->setLineWeight(3.);
  GRplot->setReverseFill(true);
  GRplot->setFillEnable(false);
  //GRplot->setStroke(false);
  GRplot->setResolution(ILevelPlotControl::kHighRes);
  GRplot->setAAquality(ILevelPlotControl::kGood);
  GRplot->setYRange(ILevelPlotControl::k32dB);
  pGraphics->AttachControl(GRplot);
  
  
  compPlot = new ICompressorPlotControl(this, IRECT(plotRECT.L, plotRECT.T, plotRECT.L + plotRECT.H(), plotRECT.T + plotRECT.H()), -1, &plotPreFillColor, &plotCompLineColor, &mComp);
  compPlot->calc();
  compPlot->setLineWeight(3.);
  pGraphics->AttachControl(compPlot);
  
  threshPlot= new IThresholdPlotControl(this, plotRECT, -1, &threshLineColor, &mComp);
  threshPlot->setLineWeight(2.);
  pGraphics->AttachControl(threshPlot);
  
  
  
  mShadow = new IBitmapControl(this, plotRECT.L , plotRECT.T, &shadow);
  pGraphics->AttachControl(new IKnobMultiControlText(this, kThresholdX, kSlidersY, kThreshold, &slider, &caption));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kGainX, kSlidersY, kGain, &slider, &caption));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kMixX, kSlidersY, kMix, &slider, &caption));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kAttackX, kBigKnobsY, kAttack, &knob, &caption));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kReleaseX, kBigKnobsY, kRelease, &knob, &caption));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kHoldX, kSmallKnobsY, kHold, &smallKnob, &caption));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kRatioX, kSmallKnobsY, kRatio, &smallKnob, &caption));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kKneeX, kSmallKnobsY, kKnee, &smallKnob, &caption));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kSCKnobsX, kSCKnobY, kCutoffLP, &smallKnob, &caption));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kSCKnobsX, kSCKnob2Y, kCutoffHP, &smallKnob, &caption));
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
    if(mComp.getRatio() != mRatio){
      mComp.setRatio(mRatioSmoother.process(mRatio));
      compPlot->calc();
    }
    if(mComp.getThreshold() != mThreshold){
      mComp.setThreshold(mThresholdSmoother.process(mThreshold));
      compPlot->calc();
    }
    if(mComp.getKnee() != mKnee) {
      mComp.setKnee(mKneeSmoother.process(mKnee));
      compPlot->calc();
    }
    if(mixSmoothed != mMix) mixSmoothed = mMixSmoother.process(mMix);
    if(mLowpass.getCutoff() != mCuttoffLP) mLowpass.setCutoffFreq(mLPSmoother.process(mCuttoffLP));
    if(mHighpass.getCutoff() != mCuttoffHP) mHighpass.setCutoffFreq(mHPSmoother.process(mCuttoffHP));
    
    
    sample1 = *in1;
    sample2 = *in2;
  
    sampleDry1 = sample1;
    sampleDry2 = sample2;
    
    
    if(!mSidechainEnable){
      sample1 = mLowpass.processAudioSample(sample1, 0);
      sample1 = mHighpass.processAudioSample(sample1, 0);
      sample2 = mLowpass.processAudioSample(sample2, 1);
      sample2 = mHighpass.processAudioSample(sample2, 1);

      gr = mComp.processStereo(sample1, sample2);
    }
    
    sample1 = sampleDry1 * DBToAmp(gr);
    sample2 = sampleDry2 * DBToAmp(gr);
    

    
    sample1 *= DBToAmp(gainSmoothed);
    sample2 *= DBToAmp(gainSmoothed);
    *out1 = sample1 * mixSmoothed + sampleDry1 * (1 - mixSmoothed);
    *out2 = sample2 * mixSmoothed + sampleDry2 * (1 - mixSmoothed);

    
    plot->process(AmpToDB(envPlotIn.process(max(sampleDry1, sampleDry2))));
    plotOut->process(AmpToDB(envPlotOut.process(max(sample1, sample2))));
    GRplot->process(scaleValue(gr, 2, -32, 2, -32));

  
    
    plot->SetDirty();
    plotOut->SetDirty();
    mShadow->SetDirty();
    compPlot->SetDirty();
    threshPlot->SetDirty();
    GRplot->SetDirty();
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
      break;
      
    case kThreshold:
      mThreshold = GetParam(kThreshold)->Value();
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
      mKnee = GetParam(kKnee)->Value() * 2.;
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