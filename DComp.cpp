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
  
  kBigKnobsY = 318,
  kAttackX = 22,
  kReleaseX = kAttackX + 104,
  
  kSliderCaptionOffset = 8,
  kSmallKnobCaptionOffset = 5,
  kSmallKnobsY = 350,
  kHoldX = 238,
  kRatioX = kHoldX + 68,
  kKneeX = kRatioX + 68,
  kSCKnobsX = 547,
  kSCKnobY= 318,
  kSCKnob2Y = kSCKnobY + 57,
  kSCBypassX = 465,
  kSCBypassY = 282,
  kSCAudX = 581,
  kSCAudY = kSCBypassY + 1,
  kFaderLength = 264,
  
  kLPCaptionX = 477,
  kLPCaptionY = 347,
  kHPCaptionY = 403,
  
  kPlotTimeScale = 4,
  
  kHPx = 474,
  kLPy = 318,
  kHPy = kLPy + 56,
  
  kKnobFrames = 63,
  kSliderFrames = 68
};




DComp::DComp(IPlugInstanceInfo instanceInfo)
:	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mGain(0.), mThreshold(0.), mAttack(10.), mHold(0.), mRelease(250.), mRatio(4), mKnee(.5),mMix(1.), mCuttoffLP(20000.), mCuttoffHP(20.), mHPEnable(false), mLPEnable(false), mSCAudition(false), mSidechainEnable(false), mMode(0)
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
  GetParam(kGain)->InitDouble("Gain", 0., kGainMin, kGainMax, 0.1, "dB");
  GetParam(kThreshold)->InitDouble("Threshold", -4., kThresholdMin, kThresholdMax, 0.1, "dB");
  GetParam(kAttack)->InitDouble("Attack", 10., 0., 250., .1, "ms");
  GetParam(kAttack)->SetShape(2.);
  GetParam(kHold)->InitDouble("Hold", 0., 0., 300., 1., "ms");
  GetParam(kHold)->SetShape(2.);
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
  GetParam(kMode)->SetDisplayText(1, "Opto");

  
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
  IBitmap HPButton = pGraphics->LoadIBitmap(HPBUTTON_ID, HPBUTTON_FN, 2);
  IBitmap LPButton = pGraphics->LoadIBitmap(LPBUTTON_ID, LPBUTTON_FN, 2);
  IBitmap Audition = pGraphics->LoadIBitmap(AUDITION_ID, AUDITION_FN, 2);
  IBitmap Bypass = pGraphics->LoadIBitmap(BYPASS_ID, BYPASS_FN, 2);
  
  pGraphics->AttachBackground(BACKGROUND_ID, BACKGROUND_FN);
  
  //374 x 183
  IRECT plotRECT = IRECT(101, 71, 475, 254);
  
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
  pGraphics->AttachControl(new IFaderControlText(this, kThresholdX, kSlidersY, kThreshold, &slider, &sliderCaption, true, kSliderCaptionOffset));
  pGraphics->AttachControl(new IFaderControlText(this, kGainX, kSlidersY, kGain, &slider, &sliderCaption, true, kSliderCaptionOffset));
  pGraphics->AttachControl(new IFaderControlText(this, kMixX, kSlidersY, kMix, &slider, &sliderCaption, true, kSliderCaptionOffset));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kAttackX, kBigKnobsY, kAttack, &knob, &caption));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kReleaseX, kBigKnobsY, kRelease, &knob, &caption));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kHoldX, kSmallKnobsY, kHold, &smallKnob, &caption, true, kSmallKnobCaptionOffset));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kRatioX, kSmallKnobsY, kRatio, &smallKnob, &caption, true, kSmallKnobCaptionOffset));
  pGraphics->AttachControl(new IKnobMultiControlText(this, kKneeX, kSmallKnobsY, kKnee, &smallKnob, &caption, true, kSmallKnobCaptionOffset));
  pGraphics->AttachControl(new IKnobMultiControl(this, kSCKnobsX, kSCKnobY, kCutoffLP, &smallKnob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kSCKnobsX, kSCKnob2Y, kCutoffHP, &smallKnob));
  pGraphics->AttachControl(new ICaptionControl(this, IRECT(kLPCaptionX, kLPCaptionY, kLPCaptionX + 63, kLPCaptionY + 21), kCutoffLP, &cutoffCaption));
    pGraphics->AttachControl(new ICaptionControl(this, IRECT(kLPCaptionX, kHPCaptionY, kLPCaptionX + 63, kHPCaptionY + 21), kCutoffHP, &cutoffCaption));
  pGraphics->AttachControl(new ISwitchControl(this, kHPx, kHPy, kHPEnable, &HPButton));
  pGraphics->AttachControl(new ISwitchControl(this, kHPx, kLPy, kLPEnable, &LPButton));
  pGraphics->AttachControl(new ISwitchControl(this, kSCBypassX, kSCBypassY, kSidechain, &Bypass));
  pGraphics->AttachControl(new ISwitchControl(this, kSCAudX, kSCAudY, kSCAudition, &Audition));
  pGraphics->AttachControl(mShadow);
  
 // mKnob = new IKnobMultiControl(this, 50, 50, kKnob, &knob);
 // pGraphics->AttachControl(mKnob);
 // pGraphics->AttachControl(new IKnobMultiControl(this, 150,150, kGain, &knob));
  
  
  if (GetAPI() == kAPIVST2) // for VST2 we name individual outputs
  {
    SetInputLabel(0, "main input L");
    SetInputLabel(1, "main input R");
    SetInputLabel(2, "sc input L");
    SetInputLabel(3, "sc input R");
    SetOutputLabel(0, "output L");
    SetOutputLabel(1, "output R");
  }
  else // for AU and VST3 we name buses
  {
    SetInputBusLabel(0, "main input");
    SetInputBusLabel(1, "sc input");
    SetOutputBusLabel(0, "output");
  }
  
  
  
  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
}

DComp::~DComp() {

}

void DComp::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

  bool in1ic = IsInChannelConnected(0);
  bool in2ic = IsInChannelConnected(1);
  bool in3ic = IsInChannelConnected(2);
  bool in4ic = IsInChannelConnected(3);
  
  printf("%i %i %i %i, ------------------------- \n", in1ic, in2ic, in3ic, in4ic);
  
#ifdef RTAS_API
  double* in1 = inputs[0];
  double* in2 = inputs[1];
  double* scin1 = inputs[2];
  
  double* out1 = outputs[0];
  double* out2 = outputs[1];
#else
  double* in1 = inputs[0];
  double* in2 = inputs[1];
  double* scin1 = inputs[2];
  double* scin2 = inputs[3];
  
  double* out1 = outputs[0];
  double* out2 = outputs[1];



  //Stupid hack because logic connects the sidechain bus to the main bus when no sidechain is connected
  //see coreaudio mailing list
#ifdef AU_API
  if (GetHost() == kHostLogic)
  {
    if(!memcmp(in1, scin1, nFrames * sizeof(double)))
    {
      memset(scin1, 0, nFrames * sizeof(double));
      memset(scin2, 0, nFrames * sizeof(double));
    }
  }
#endif


  for (int s = 0; s < nFrames; ++s, ++scin1, ++scin2, ++in1, ++in2, ++out1, ++out2)
  {
    double sampleFiltered1, sampleFiltered2, sampleDry1, sampleDry2, gr, gainSmoothed, mixSmoothed;

    gainSmoothed = mGainSmoother.process(mGain);
    

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
    
    sampleDry1 = *in1;
    sampleDry2 = *in2;
    sampleFiltered1 = *in1;
    sampleFiltered2 = *in2;
    
    if(!mSidechainEnable){
      if(mLPEnable) {
        sampleFiltered1 = mLowpass.processAudioSample(sampleFiltered1, 0);
        sampleFiltered2 = mLowpass.processAudioSample(sampleFiltered2, 1);
      }
      if(mHPEnable){
        sampleFiltered1 = mHighpass.processAudioSample(sampleFiltered1, 0);
        sampleFiltered2 = mHighpass.processAudioSample(sampleFiltered2, 1);
      }
    
      gr = mComp.processStereo(sampleFiltered1, sampleFiltered2);
    }
    else{
      if(mLPEnable){
        *scin1 = mLowpass.processAudioSample(*scin1, 0);
        *scin2 = mLowpass.processAudioSample(*scin2, 1);
      }
      if(mHPEnable){
        *scin1 = mHighpass.processAudioSample(*scin1, 0);
        *scin2 = mHighpass.processAudioSample(*scin2, 1);
      }

      gr = mComp.processStereo(*scin1, *scin2);
    }
    
    *in1 *= DBToAmp(gr);
    *in2 *= DBToAmp(gr);
    
    *in1 *= DBToAmp(gainSmoothed);
    *in2 *= DBToAmp(gainSmoothed);
    
    if(!mSCAudition){
      *out1 = *in1 * mixSmoothed + sampleDry1 * (1 - mixSmoothed);
      *out2 = *in2 * mixSmoothed + sampleDry2 * (1 - mixSmoothed);
    }
    else if(mSidechainEnable){
      *out1 = *scin1;
      *out2 = *scin2;
    }
    else{
      *out1 = sampleFiltered1;
      *out2 = sampleFiltered2;
    }

    
    plot->process(AmpToDB(envPlotIn.process(max(sampleDry1, sampleDry2))));
    plotOut->process(AmpToDB(envPlotOut.process(max(*in1, *in2))));
    GRplot->process(scaleValue(gr, 2, -32, 2, -32));

  
    if(GetGUI()) {
      plot->SetDirty();
      plotOut->SetDirty();
      compPlot->SetDirty();
      threshPlot->SetDirty();
      GRplot->SetDirty();
      mShadow->SetDirty();
    }
  }
#endif
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
      mComp.setThreshold(mThresholdSmoother.process(mThreshold));
      compPlot->calc();
      threshPlot->SetDirty();
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
      mComp.setRatio(mRatioSmoother.process(mRatio));
      compPlot->calc();
      break;
      
    case kKnee:
      mKnee = GetParam(kKnee)->Value() * 2.;
      mComp.setKnee(mKneeSmoother.process(mKnee));
      compPlot->calc();
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