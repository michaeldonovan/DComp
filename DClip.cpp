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

  kGainX = 75,
  kGainHandlesX = 47,
  kCeilingX = 537,
  kMeterX = 564,
  kGainY = 85,
  
  kGainCaptionX = 65,
  kCeilingCaptionX = 554,

  kCaptionY = 344,
  
  kCaptionHeight = 50,
  kCaptionWidth = 62,
  
  kFaderLength = 280,
  
  kSliderFrames = 63
};




DClip::DClip(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mGain(0.), mCeiling(0.), env(1., 250., GetSampleRate())
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kGain)->InitDouble("Gain", 0., kGainMin, kGainMax, 0.01, "dB");
  GetParam(kCeiling)->InitDouble("Ceiling", 0., kCeilingMin, kCeilingMax, 0.01, "dB");

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  
  IBitmap slider = pGraphics->LoadIBitmap(SLIDER_ID, SLIDER_FN, kSliderFrames);
  IBitmap sliderHandles = pGraphics->LoadIBitmap(SLIDERHANDLES_ID, SLIDERHANDLES_FN);

  pGraphics->AttachBackground(BACKGROUND_ID, BACKGROUND_FN);
  
  IRECT plotRECT = IRECT(152, 85, 527, 337);
  
  plot = new ILevelPlotControl(this, plotRECT, kPlot, &plotPreFillColor, &plotLineColor, 5);
  plot->setLineWeight(3.);
  plot->setResolution(ILevelPlotControl::kHighRes);
  plot->setYRange(ILevelPlotControl::k32dB);
  pGraphics->AttachControl(plot);
  
  mGainSlider = new IBitmapControl(this, kGainX, kGainY, &slider);
  mGainSliderHandles = new IFaderControl(this, kGainHandlesX, kGainY - 10, kFaderLength, kGain, &sliderHandles);
  mCeilingSliderHandles = new IFaderControl(this, kCeilingX, kGainY - 10, kFaderLength, kCeiling, &sliderHandles);
  mOutputMeter = new IBitmapControl(this, kMeterX, kGainY, &slider);

  IText caption = IText(16, &COLOR_WHITE, "Futura", IText::kStyleNormal, IText::kAlignCenter);
  
  mGainCaption = new ICaptionControl(this, IRECT(kGainCaptionX, kCaptionY, kGainCaptionX+kCaptionWidth, kCaptionY+kCaptionHeight), kGain, &caption, true);
  mCeilingCaption = new ICaptionControl(this, IRECT(kCeilingCaptionX, kCaptionY, kCeilingCaptionX+kCaptionWidth, kCaptionY+kCaptionHeight), kCeiling, &caption, true);

  pGraphics->AttachControl(mGainSlider);
  pGraphics->AttachControl(mGainSliderHandles);
  pGraphics->AttachControl(mOutputMeter);
  pGraphics->AttachControl(mCeilingSliderHandles);
  pGraphics->AttachControl(mGainCaption);
  pGraphics->AttachControl(mCeilingCaption);

  
  
  //mKnob = new IKnobMultiControl(this, 50, 50, kKnob, &knob);
 // pGraphics->AttachControl(mKnob);
 // pGraphics->AttachControl(new IKnobMultiControl(this, 150,150, kGain, &knob));
  AttachGraphics(pGraphics);
    
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
    
    *out1 = *in1 * DBToAmp(mGain);
    *out2 = *in2 * DBToAmp(mGain);
    
    double sample = .6 * (*out1 + *out2);
    double val = env.process(sample);
    plot->process(AmpToDB(val));
    val = AmpToDB(val);
    val = scaleValue(val, kCeilingMin, 2, 0, 1);
    mOutputMeter->SetValueFromPlug( (val));
    mOutputMeter->SetDirty();
  }
  
  plot->SetDirty(true);
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
      mGainSlider->SetValueFromPlug(scaleValue(mGain, kGainMin, kGainMax, 0, 1));
      mGainSlider->SetDirty();
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