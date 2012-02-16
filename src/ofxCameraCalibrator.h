//
//  ofxCameraCalibrator.h
//  emptyExample
//
//  Created by Patricio González Vivo on 2/14/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#pragma once

#define PTAM_SCALE 0.001

#include "cvd/image.h"
#include "cvd/byte.h"
#include "ofxCalibImage.h"

class ofxATANCamera;

#include "ofMain.h"

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
    
    ofxATANCamera               *mpCamera;          // The camera model
    CVD::Image<CVD::byte>       mimFrameBW;
    std::vector<ofxCalibImage>  vCalibImgs;
    
    double  meanPixelError;
    int     nShowImage;
    int     imgWidth, imgHeight;
    
    bool    bDisableDistortion;
    bool    bGrabNextFrame;
};		