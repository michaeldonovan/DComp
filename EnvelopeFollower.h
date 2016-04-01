//
//  envFollower.h
//
//
//
//

#ifndef envFollower_h
#define envFollower_h

#include <algorithm>

class envFollower{
public:
    envFollower(){
        init(5, 50, 0, 44100);
    }
    
    envFollower(double attackMS, double releaseMS, double holdMS, double SampleRate){
        init(attackMS, releaseMS, holdMS, SampleRate);
    }
    
    virtual void init(double attackMS, double releaseMS, double holdMS, double SampleRate){
        attack = pow(0.01, 1.0/(attackMS * SampleRate * 0.001));
        release = pow(0.01, 1.0/(releaseMS * SampleRate * 0.001));
        hold = holdMS / 1000. * SampleRate;
        env = 0;
        timer = 0;
    }
    
    void setAttack(double attackMS){
        attack = attackMS;
    }
    
    void setRelease(double releaseMS){
        release = releaseMS;
    }
    
    void setHold(double holdMS){
        hold = holdMS;
    }
    
    virtual double process(double sample){
        double mag = fabs(sample);
        if(mag > env){
            env = attack * (env - mag) + mag;
            timer=0;
        }
        else if(timer<hold){
            timer++;
        }
        else{
            env = release * (env - mag) + mag;
        }
        
        return env;
    }
    
protected:
    double attack, release, env;
    int timer, hold;
};


class compressor : public envFollower{
public:
    enum kMode{
        kCompressor,
        kLimiter
    };
    
    compressor(){
        init(5, 50, 0, 4, 0, 44100);
    }
    
    compressor(double attackMS, double releaseMS, double holdMS, double ratio, double knee, double SampleRate){
        init(attackMS, releaseMS, holdMS, ratio, knee, SampleRate);
    }
    
    void init(double attackMS, double releaseMS, double holdMS, double ratio, double knee, double SampleRate){
        envFollower::init(attackMS, releaseMS, holdMS, SampleRate);
        gainReduction = 0;
        mKnee = knee;
        mRatio = ratio;
        mThreshold = 0.;
        calcKnee();
        calcSlope();
    }
    
    void setKnee(double knee){
        mKnee = knee;
        calcKnee();
        calcSlope();
    }
    
    void setRatio(double ratio){
        mRatio = ratio;
        calcKnee();
        calcSlope();
    }
    
    void setThreshold(double threshold){
        mThreshold = threshold;
        calcKnee();
        calcSlope();
    }
    
    void setMode(int mode){
        mMode = mode;
        calcKnee();
        calcSlope();
    }
    
    double process(double sample){
        double e;
        e = envFollower::process(sample);
        calcSlope();
        
        if(kneeWidth > 0. && e > kneeBoundL && e < kneeBoundU){
            slope *= ((e - kneeBoundL) / kneeWidth) * 0.5;
            gainReduction = slope * (kneeBoundL  - e);
        }
        else{
            gainReduction = slope * (mThreshold - e);
            gainReduction = std::min(0., gainReduction);
        }
        
        return sample * DBToAmp(gainReduction);
    }
    
    double getGainReductionDB(){
        return gainReduction;
    }
    
private:
    double gainReduction, mKnee, mRatio, mThreshold, kneeWidth, kneeBoundL, kneeBoundU, slope;
    int mMode;
    
    inline void calcKnee(){
        kneeWidth = mThreshold * mKnee * -1.;
        kneeBoundL = mThreshold - (kneeWidth / 2.);
        kneeBoundU = mThreshold + (kneeWidth / 2.);
    }
    
    inline void calcSlope(){
        if(mMode == kCompressor){
            slope = 1 - (1 / mRatio);
        }
        else{
            slope = 1;
        }
    }
};
#endif /* envFollower_h */
