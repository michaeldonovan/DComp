//
//  envFollower.h
//
//
//
//

#ifndef envFollower_h
#define envFollower_h

class envFollower{
public:
    envFollower(int attackMS, int releaseMS, double SampleRate){
        attack = pow(0.01, 1.0/(attackMS * SampleRate * 0.001));
        release = pow(0.01, 1.0/(releaseMS * SampleRate * 0.001));
        env = 0.0;
    }
    
    double process(double sample){
        double mag = fabs(sample);
        if(mag > env){
            env = attack * (env - mag) + mag;
        }
        else{
            env = release * (env - mag) + mag;
        }
        
        return env;
    }
    
protected:
    double attack, release, env;
};
#endif /* envFollower_h */
