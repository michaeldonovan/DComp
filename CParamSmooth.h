//
//  CParamSmooth.h
//
//  Author: alexirae@gmail.com
//  from musicdsp.org archives
//

#ifndef CParamSmooth_hpp
#define CParamSmooth_hpp

#include <math.h>

class CParamSmooth
{
public:
    CParamSmooth(float smoothingTimeInMs, float samplingRate)
    {
        const float c_twoPi = 6.283185307179586476925286766559f;
        
        a = exp(-c_twoPi / (smoothingTimeInMs * 0.001f * samplingRate));
        b = 1.0f - a;
        z = 0.0f;
    }
    
    ~CParamSmooth(){};
                 
    double process(double in)
    {
        z = (in * b) + (z * a);
        return z;
    }
    
private:
    float a;
    float b;
    float z;
};

#endif /* CParamSmooth_hpp */
