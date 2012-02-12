/*
 *  ofxPTAM.h
 *  try_PTAM
 *
 *  Created by Akira Asia on 10/03/29.
 *  Copyright 2010 __This is Ampontang.__. No rights reserved.
 *
 */

#pragma once

#define PTAM_SCALE 0.001

#include <gvars3/instances.h>

#include "cvd/image.h"
#include "cvd/rgb.h"
#include "cvd/byte.h"
//#include <vector>

#include "Map.h"
#include "MapMaker.h"
//#include "MapSerializer.h"

class ofxATANCamera;
//class MapViewer;
class ofxTracker;

#include "ofMain.h"

class ofxPTAMM {
public:
    ofxPTAMM();
    ~ofxPTAMM();
    
    void    init(int imgW, int imgH);
    void    update(ofPixelsRef _pixelsRef);
    void	draw();
    
    void	startBuildMap();	
    void	resetMap();
    
    bool    isMapBuild() const { return bMapBuildComplete; };
    
    // TODOS
    //  - HELP with scele, orientation and rotationMatrix
    
    ofVec2f     getPosition() const;
	float       getScale() const;           // NOT WORKING!!!
	ofVec3f     getOrientation() const;     // NOT WORKING!!!
	ofMatrix4x4 getRotationMatrix() const;  // NOT WORKING!!!
    
private:
    bool SwitchMap( int nMapNum, bool bForce = false );                                    // Switch to a particular map.
    void NewMap();                                  // Create a new map and move all elements to it
    bool DeleteMap( int nMapNum );                  // Delete a specified map
    void ResetAll();                                // Wipes out ALL maps, returning system to initial state
//  void StartMapSerialization(std::string sCommand, std::string sParams);   //(de)serialize a map
    
    GVars3::gvar3<int> mgvnLockMap;                 // Stop a map being edited - i.e. keyframes added, points updated
    
    CVD::Image<CVD::byte>       mimFrameBW;
    std::vector<PTAMM::Map*>    mvpMaps;            // The set of maps
    PTAMM::Map                  *mpMap;             // The current map
    PTAMM::MapMaker             *mpMapMaker;        // The map maker
    ofxTracker                  *mpTracker;         // The tracker
    ofxATANCamera               *mpCamera;          // The camera model
//  PTAMM::MapViewer            *mpMapViewer;       // The Map Viewer
//  PTAMM::MapSerializer        *mpMapSerializer;   // The map serializer for saving and loading maps
	
    int     imgWidth, imgHeight;
    
    bool	bMapBuildComplete;
};		
		

		