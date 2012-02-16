//
//  ofxCalibImage.h
//  emptyExample
//
//  Created by Patricio González Vivo on 2/14/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//
#include "CalibImage.h"
#include "ofMain.h"

using namespace PTAMM;

class ofxCalibImage: public CalibImage {
public:
	
    void drawImageGrid();
    void draw3DGrid(ATANCamera &Camera, bool bDrawErrors);
    
};
