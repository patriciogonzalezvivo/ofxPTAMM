/*
 *  ofxTracker.h
 *  try_PTAM3
 *
 *  Created by Akira Asia on 10/04/12.
 *  Copyright 2010 This is Ampontang. No rights reserved.
 *
 */
#pragma once

#include "MapSerializer.h"
#include "Tracker.h"
#include "Map.h"

#include "ofMain.h"

using namespace PTAMM;

class ofxTracker : public Tracker {
public:
	
    ofxTracker(CVD::ImageRef irVideoSize, const ATANCamera &c, std::vector<Map*> &maps, Map *m, MapMaker &mm) : Tracker(irVideoSize, c, maps, m, mm){ };
    void reset(){ Reset(); };
    void buildMapBegin();
    
    void draw(bool _drawPoints = false);
};