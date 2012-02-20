/*
 *  ofxTracker.h
 *  try_PTAM3
 *
 *  Created by Akira Asia on 10/04/12.
 *  Copyright 2010 This is Ampontang. No rights reserved.
 *
 */
#pragma once

#include "ofMain.h"

#include "ofxATANCamera.h"

#include "PTAMM/Map.h"
#include "PTAMM/Tracker.h"
#include "PTAMM/MapPoint.h"

#include "cvd/image.h"

class ofxTracker : public PTAMM::Tracker {
public:
	
    ofxTracker(CVD::ImageRef irVideoSize, const ofxATANCamera &c, std::vector<PTAMM::Map*> &maps, PTAMM::Map *m, PTAMM::MapMaker &mm) : PTAMM::Tracker(irVideoSize, c, maps, m, mm){
        mnLostFrames = 0;
        
    };
    
    void reset(){ Reset(); };
    void buildMapBegin();
    
    void draw(bool _drawPoints = false);
};