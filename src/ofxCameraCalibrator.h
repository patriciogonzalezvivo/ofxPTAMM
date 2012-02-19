//
//  ofxCameraCalibrator.h
//  emptyExample
//
//  Created by Patricio González Vivo on 2/14/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#pragma once

#define PTAM_SCALE 0.001

#include "ofMain.h"

#include "ofxCalibImage.h"
#include "ofxATANCamera.h"

#include "cvd/image.h"
#include "cvd/byte.h"
#include "TooN/SVD.h"
#include <fstream>
#include <stdlib.h>

class ofxCameraCalibrator{
public:
    ofxCameraCalibrator();
    
    void    init(int imgW, int imgH, string configFile = "camera.cfg");
    bool    update(unsigned char * _pixels, int _width = -1, int _height = -1, ofImageType _type = OF_IMAGE_COLOR );
    void    grabPicture(){ bGrabNextFrame = true; };
    
    void	draw();
    
    void    reset();
    
    int     getTotalImages(){ return vCalibImgs.size(); };
    int     getActualImage(){ return nShowImage; };
    void    viewImage(int n){ nShowImage = n; bOptimizing = true; };
    
    bool    bOptimizing;
    bool    bDebug;
    
private:    
    void    optimizeOneStep();
    
    CVD::Image<CVD::byte>       mimFrameBW;
    std::vector<ofxCalibImage>  vCalibImgs;
    ofxATANCamera               *mpCamera;          // The camera model
    ofxDataPackege              data;
    
    double  meanPixelError;
    int     nShowImage;
    int     imgWidth, imgHeight;
    
    bool    bDisableDistortion;
    bool    bGrabNextFrame;
};