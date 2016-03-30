#include "DClip.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
  kGain = 0,
  kKnob,
  kMeter,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kGainX = 100,
  kGainY = 100,
  kKnobFrames = 60
};

DClip::DClip(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mGain(1.), env(1., 250., GetSampleRate())
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kGain)->InitDouble("Gain", 50., 0., 100.0, 0.01, "%");
  GetParam(kGain)->SetShape(2.);

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);

  
  IColor bg(255, 0xCE, 0xCE, 0xCE);
  IColor line(255, 0x97, 0x97, 0x97);
  IColor fill(255, 0xBB, 0xBB, 0xBB);

  pGraphics->AttachPanelBackground(&bg);
  plot = new ILevelPlotControl(this, IRECT(0,0,kWidth,kHeight), kMeter, &fill, &line, 5);
  pGraphics->AttachControl(plot);

  
  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
  
//  pGraphics->AttachControl(new MyCairoControl(this, IRECT(0, 0, 200, 200)));

  mKnob = new IKnobMultiControl(this, 50, 50, kKnob, &knob);
  pGraphics->AttachControl(mKnob);
  pGraphics->AttachControl(new IKnobMultiControl(this, 150,150, kGain, &knob));
  AttachGraphics(pGraphics);
  
  plot->setResolution(ILevelPlotControl::kMidRes);
  
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
    *out1 = *in1 * mGain;
    *out2 = *in2 * mGain;
    
    double sample = .6 * (*out1 + *out2);
    double val = env.process(sample);
    mKnob->SetValueFromPlug(val);
    plot->process(val * 10);
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
      mGain = GetParam(kGain)->Value() / 100.;
      break;

    default:
      break;
  }
}
