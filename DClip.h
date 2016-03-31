#ifndef __DCLIP__
#define __DCLIP__

#include "IPlug_include_in_plug_hdr.h"
#include "EnvelopeFollower.h"
#include "ICairoControls.h"
#include "CParamSmooth.h"
#include "IPopupMenuControl.h"
#include <ctime>

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
  
  clock_t start_time;
  clock_t stop_time;
  
  const int kGainMin = 0;
  const int kGainMax = 32;
  const int kCeilingMin = -32;
  const int kCeilingMax = 2;
  const double frameTime = 1/20.;
  
  IColor plotBackgroundColor = IColor(206,206,206);
  IColor plotPreLineColor =  IColor(170, 151, 151, 151);
  IColor plotPostLineColor =  IColor(255, 120, 120, 120);
  IColor plotPreFillColor =  IColor(30, 198, 198, 198);
  IColor plotPostFillColor =  IColor(255, 187, 187, 187);
  IColor yellow = IColor(255, 255, 233, 30);
  IColor grLineColor = IColor(120, 250, 0,0);
  IColor grFillColor = IColor(15, 250, 0,0);

  double mGain, mCeiling, duration;
  //double mCeiling;
  struct NVGcontext* vg;
  envFollower envPlotIn;
  envFollower envPlotOut;
  envFollower envMeter;
  envFollower envGR;

  CParamSmooth mGainSmoother;
  CParamSmooth mCeilingSmoother;
  
  ILevelPlotControl* plot;
  ILevelPlotControl* plotOut;
  ILevelPlotControl* GRplot;

  IBitmapControl* mDBMeter;
  IFaderControl* mGainSliderHandles;
  IBitmapControl* mGRMeter;
  IFaderControl* mCeilingSliderHandles;
  ICaptionControl* mGainCaption;
  ICaptionControl* mCeilingCaption;
  
  IBitmapControl* mShadow;
};

#endif
//#endif
