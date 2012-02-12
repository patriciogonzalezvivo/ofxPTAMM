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

class ofxATANCamera : public PTAMM::ATANCamera {
public:
		
    ofxATANCamera(string sName) : PTAMM::ATANCamera(sName) { };
		
    void manualParamUpdate(string cfgFile);
		
    bool paramTest();
	
private:

};

