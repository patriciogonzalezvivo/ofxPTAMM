/*
 *  ofxATANCamera.h
 *  try_PTAM3
 *
 *  Created by Akira Asia on 10/04/09.
 *  Copyright 2010 This is Ampontang. No rights reserved.
 *
 */

#pragma once

#include "ofMain.h"

#include "PTAMM/ATANCamera.h"

//#include <fstream>

class ofxATANCamera : public PTAMM::ATANCamera {
public:
    
    ofxATANCamera(string sName) : PTAMM::ATANCamera(sName) {
        
#ifdef TARGET_OF_IPHONE
        /*
        *mgvvCameraParams = TooN::makeVector(0.98065,1.31844,0.373223,0.666443,0);
        mvImageSize[0] = 480.0;
        mvImageSize[1] = 640.0;*/
        *mgvvCameraParams = TooN::makeVector(0.488953,0.657065,0.186756,0.333456,0);
        mvImageSize[0] = 240.0;
        mvImageSize[1] = 320.0;
#else
        *mgvvCameraParams = TooN::makeVector(0.92295, 1.28292, 0.497764, 0.490052, 0);
        mvImageSize[0] = 640.0;
        mvImageSize[1] = 480.0;
#endif
        RefreshParams();
    };
    
    void loadParameters(TooN::Vector<NUMTRACKERCAMPARAMETERS> _params);
    void loadParameters(string cfgFile);
    bool testParameters();
    
    void updateParameters(TooN::Vector<NUMTRACKERCAMPARAMETERS> vUpdate);
    
    void disableRadialDistortion(){ DisableRadialDistortion();};
    
    inline TooN::Vector<NUMTRACKERCAMPARAMETERS> getParams()  { return *mgvvCameraParams; }
    TooN::Matrix<2, NUMTRACKERCAMPARAMETERS> getCameraParameterDerivs() { return GetCameraParameterDerivs();};
    
    static const TooN::Vector<5> mvDefaultParams;
};

