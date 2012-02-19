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

#include "ofMain.h"

#include "ofxTracker.h"
#include "ofxATANCamera.h"
#include "ofxCameraCalibrator.h"

#include "cvd/image.h"
#include "cvd/rgb.h"
#include "cvd/byte.h"
#include "cvd/colourspace.h"

#include "gvars3/instances.h"

#include "Map.h"
#include "MapPoint.h"
#include "MapMaker.h"
#include "MapSerializer.h"

#include "OpenGL.h"

#ifndef TARGET_OF_IPHONE
#include <cvd/gl_helpers.h>
#endif

class ofxPTAMM {
public:
    ofxPTAMM();
    ~ofxPTAMM();
    
    void    init(int imgW, int imgH, string configFile = "camera.cfg");
    bool    update(unsigned char * _pixels, int _width = -1, int _height = -1, ofImageType _type = OF_IMAGE_COLOR );
    void	draw();
    
    void    buildMap();
    void    newMap();                                  // Create a new map and move all elements to it
    bool    switchMap( int nMapNum, bool bForce=false);// Switch to a particular map.
    bool    deleteMap( int nMapNum );                  // Delete a specified map
    void    resetAll();
    void    saveMap();
    void    saveMaps();
    void    loadMap();
    
    int     getTotalMaps() const {return vMaps.size();};
    int     getActualMap() const {return map->MapID();};
    bool    isMapPresent() const;
    bool    isMapBuild() const { return bMapBuildComplete; };
    void    moveCamera() const;
    
    ofVec2f     getScreenPosition() const;
    ofMatrix4x4 getCameraMatrix() const;
    ofVec3f     getTranslation() const;
    ofMatrix4x4 getRotationMatrix() const;
    
    bool bDebug;
    
private:    
    CVD::Image<CVD::byte>       mimFrameBW;
    std::vector<PTAMM::Map*>    vMaps;            // The set of maps
    PTAMM::Map                  *map;             // The current map
    PTAMM::MapMaker             *mapMaker;        // The map maker
    ofxTracker                  *tracker;         // The tracker
    ofxATANCamera               *camera;          // The camera model
    PTAMM::MapSerializer        *mapSerializer;   // The map serializer for saving and loading maps
	
    int     imgWidth, imgHeight;
    bool    *bLockMap;    // Stop a map being edited - i.e. keyframes added, points updated
    bool	bMapBuildComplete;
};		
		

		