#ifndef __DCLIP__
#define __DCLIP__

#include "IPlug_include_in_plug_hdr.h"
//#include "IDClip.h"
#include "MyCairoControl.h"
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
  double mGain;
  struct NVGcontext* vg;
  envFollower env;
  ILevelPlotControl* plot;
  IKnobMultiControl* mKnob;
};

#endif
//#endif
