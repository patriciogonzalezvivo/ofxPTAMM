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
#include "ATANCamera.h"

using namespace TooN;

class ofxATANCamera : public PTAMM::ATANCamera {
public:
		
    ofxATANCamera(string sName) : PTAMM::ATANCamera(sName) {
        msName = sName;
        //(mgvvCameraParams) = new Vector<NUMTRACKERCAMPARAMETERS>;
        *mgvvCameraParams = makeVector(0.92295, 1.28292, 0.497764, 0.490052, 0);
        mvImageSize[0] = 640.0;
        mvImageSize[1] = 480.0;
        RefreshParams();
    };
		
    void loadParameters(Vector<NUMTRACKERCAMPARAMETERS> _params);
    void loadParameters(string cfgFile);
    bool testParameters();
    
    void updateParameters(Vector<NUMTRACKERCAMPARAMETERS> vUpdate);
    
    void disableRadialDistortion(){ DisableRadialDistortion();};
    
    static const Vector<5> mvDefaultParams;
};

