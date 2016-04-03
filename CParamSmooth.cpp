//
//  CParamSmooth.cpp
//
//  Author: alexirae@gmail.com
//  from musicdsp.org archives
//
#include "CParamSmooth.h"
#include <math.h>

CParamSmooth::CParamSmooth(){};

CParamSmooth::CParamSmooth(double smoothingTimeInMs, double samplingRate)
{
    const double c_twoPi = 6.283185307179586476925286766559f;
    
    a = exp(-c_twoPi / (smoothingTimeInMs * 0.001f * samplingRate));
    b = 1.0 - a;
    z = 0.0f;
}

void CParamSmooth::init(double smoothingTimeInMs, double samplingRate)
{
    const double c_twoPi = 6.283185307179586476925286766559f;
    
    a = exp(-c_twoPi / (smoothingTimeInMs * 0.001f * samplingRate));
    b = 1.0 - a;
    z = 0.0f;
}

double CParamSmooth::process(double in)
{
    z = (in * b) + (z * a);
    return z;
}
    
