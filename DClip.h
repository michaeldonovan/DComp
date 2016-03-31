#ifndef __DCLIP__
#define __DCLIP__

#include "IPlug_include_in_plug_hdr.h"
#include "EnvelopeFollower.h"
#include "ICairoControls.h"

class DClip : public IPlug
{
public:
  DClip(IPlugInstanceInfo instanceInfo);
  ~DClip();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

private:
  double scaleValue(double inValue, double inMin, double inMax, double outMin, double outMax);
  
  const int kGainMin = 0;
  const int kGainMax = 32;
  const int kCeilingMin = -32;
  const int kCeilingMax = 0;
  
  IColor plotBackgroundColor = IColor(206,206,206);
  IColor plotLineColor =  IColor(255, 151, 151, 151);
  IColor plotPreFillColor =  IColor(255, 198, 198, 198);
  IColor plotPostFillColor =  IColor(255, 187, 187, 187);
  IColor yellow = IColor(255, 255, 233, 30);
  
  double mGain, mCeiling;
  //double mCeiling;
  struct NVGcontext* vg;
  envFollower env;
  ILevelPlotControl* plot;
  
  IBitmapControl* mGainSlider;
  IFaderControl* mGainSliderHandles;
  IBitmapControl* mOutputMeter;
  IFaderControl* mCeilingSliderHandles;
  ICaptionControl* mGainCaption;
  ICaptionControl* mCeilingCaption;
};

#endif
//#endif
