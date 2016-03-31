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
    envFollower(int attackMS, int releaseMS, int holdMS, double SampleRate) : env(0.), timer(0.){
        attack = pow(0.01, 1.0/(attackMS * SampleRate * 0.001));
        release = pow(0.01, 1.0/(releaseMS * SampleRate * 0.001));
        hold = holdMS / 1000. * SampleRate;
    }
    
    double process(double sample){
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
#endif /* envFollower_h */
