//
//  CParamSmooth.h
//
//  Author: alexirae@gmail.com
//  from musicdsp.org archives
//

#ifndef CParamSmooth_hpp
#define CParamSmooth_hpp

class CParamSmooth
{
public:
    CParamSmooth(double smoothingTimeInMs, double samplingRate);

    CParamSmooth();
    
    ~CParamSmooth(){};
    
    void init(double smoothingTimeInMs, double samplingRate);

    double process(double in);

private:
    double a, b, z;
};

#endif /* CParamSmooth_hpp */
