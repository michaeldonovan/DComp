#ifndef __DCOMP__
#define __DCOMP__

#include "IPlug_include_in_plug_hdr.h"
#include "IPopupMenuControl.h"

class DComp : public IPlug
{
public:
  DComp(IPlugInstanceInfo instanceInfo);
  ~DComp();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

private:
  char* versionString = "v0.1.0";
  
  double scaleValue(double inValue, double inMin, double inMax, double outMin, double outMax);
  

  
  const int kGainMin = 0;
  const int kGainMax = 32;
  const int kThresholdMin = -32;
  const int kThresholdMax = 2;
  const double frameTime = 1/20.;
  
  IColor plotBackgroundColor = IColor(206,206,206);
  IColor plotPreLineColor =  IColor(170, 151, 151, 151);
  IColor plotPostLineColor =  IColor(123, 200, 200, 200);
  IColor plotCompLineColor =  IColor(255, 180, 180, 180);
  IColor plotCompFillColor =  IColor(90, 198, 198, 198);

  IColor plotPreFillColor =  IColor(110, 198, 198, 198);
 // IColor plotPostFillColor =  IColor(125, 255, 233, 30);
  IColor plotPostFillColor =  IColor(150, 255, 196, 0);

  IColor yellow = IColor(255, 255, 233, 30);
  IColor grLineColor = IColor(180, 250, 125,90);
  IColor grFillColor = IColor(50, 250, 0,0);
  IColor threshLineColor = IColor(180, 240, 240, 240);
  
  IColor modeColor = IColor(240, 255, 196, 0);

  
  IText caption = IText(14, &COLOR_WHITE, "Futura", IText::kStyleNormal, IText::kAlignCenter);
  IText cutoffCaption = IText(11, &COLOR_WHITE, "Futura", IText::kStyleNormal, IText::kAlignCenter);
  IText sliderCaption = IText(12, &COLOR_WHITE, "Futura", IText::kStyleNormal, IText::kAlignCenter);
  IText popUpLabel = IText(18, &COLOR_WHITE, "Futura", IText::kStyleNormal, IText::kAlignCenter);
  IText versionText = IText(9, &threshLineColor, "Futura", IText::kStyleNormal, IText::kAlignNear);
  
  double mGain, mThreshold, mAttack, mHold, mRelease, mRatio, mKnee, mMix, mCuttoffLP, mCuttoffHP;
  int mMode;
  bool mSidechainEnable, mSCAudition, mLPEnable, mHPEnable;
  envFollower envPlotIn;
  envFollower envPlotOut;
  compressor mComp;
  
  VAStateVariableFilter mLowpass;
  VAStateVariableFilter mHighpass;
  
  CParamSmooth mGainSmoother;
  CParamSmooth mThresholdSmoother;
  CParamSmooth mAttackSmoother;
  CParamSmooth mReleaseSmoother;
  CParamSmooth mHoldSmoother;
  CParamSmooth mRatioSmoother;
  CParamSmooth mKneeSmoother;
  CParamSmooth mMixSmoother;
  CParamSmooth mHPSmoother;
  CParamSmooth mLPSmoother;

  ILevelPlotControl* plot;
  ILevelPlotControl* plotOut;
  ILevelPlotControl* GRplot;
  ICompressorPlotControl* compPlot;
  IThresholdPlotControl* threshPlot;

  IBitmapControl* mShadow;
};

#endif
