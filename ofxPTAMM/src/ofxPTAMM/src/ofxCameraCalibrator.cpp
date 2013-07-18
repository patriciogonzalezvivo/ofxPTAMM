//
//  ofxCameraCalibrator.cpp
//  emptyExample
//
//  Created by Patricio González Vivo on 2/14/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//
#include "ofxCameraCalibrator.h"

ofxCameraCalibrator::ofxCameraCalibrator(){
    bDebug = false;
    nShowImage = 0;
    bDisableDistortion = false;
    
    data.CornerPatchSize = 20;
    data.DisableDistortion = false;
    data.Optimizing = false;
    data.MaxStepDistFraction = 0.3;
    data.BlurSigma = 1.0;
    data.MinCornersForGrabbedImage = 20;
    data.MeanGate = 10;
    
    reset();
}

void ofxCameraCalibrator::init(int imgW, int imgH, string configFile) {
    
	imgWidth = imgW;
	imgHeight = imgH;	
    
	mimFrameBW.resize( CVD::ImageRef(imgWidth,imgHeight) );
    // First, check if the camera is calibrated.
    // If not, we need to run the calibration widget.
    mpCamera = new ofxATANCamera("Camera");	
}

bool ofxCameraCalibrator::update(unsigned char * _pixels, int _width, int _height, ofImageType _type){
    bool rta = true;
    
    if (_width == -1)
        _width = imgWidth;
    
    if (_height == -1)
        _height = imgHeight;
    
    CVD::ImageRef mirSize = CVD::ImageRef(_width,_height);
    mimFrameBW.resize(mirSize);
    
    if (_type == OF_IMAGE_GRAYSCALE){
        for (int y=0; y<mirSize.y; y++) {
            for (int x=0; x<mirSize.x; x++) {
                mimFrameBW[y][x]        = *_pixels;
                _pixels++;
            }
        }
    } else if (_type == OF_IMAGE_COLOR){
        for (int y=0; y<mirSize.y; y++) {
            for (int x=0; x<mirSize.x; x++) {
                _pixels++;
                mimFrameBW[y][x]        = *_pixels;
                _pixels++;
                _pixels++;
            }
        }
    } else if (_type == OF_IMAGE_COLOR_ALPHA ){
        for (int y=0; y<mirSize.y; y++) {
            for (int x=0; x<mirSize.x; x++) {
                _pixels++;
                mimFrameBW[y][x]        = *_pixels;
                _pixels++;
                _pixels++;
                _pixels++;
            }
        }
    } else
        rta = false;

    return rta;
};

void ofxCameraCalibrator::draw(){
    if(vCalibImgs.size() < 1)
        bOptimizing = 0;
    
    if(!bOptimizing){
        ofxCalibImage c;
        /*
        ofImage a;
        a.allocate(mimFrameBW.size().x, mimFrameBW.size().y, OF_IMAGE_GRAYSCALE);
        a.setFromPixels(mimFrameBW.data(), mimFrameBW.size().x, mimFrameBW.size().y, OF_IMAGE_GRAYSCALE);
        a.draw(0,0);
        */
        if(c.makeFromImage(mimFrameBW, &data) ){
            if(bGrabNextFrame){
                cout << "Grabbed" << endl;
                vCalibImgs.push_back(c);
                vCalibImgs.back().guessInitialPose(*mpCamera);
                vCalibImgs.back().draw3DGrid(*mpCamera, false);
                bGrabNextFrame = false;
            };
        }
    } else {
        optimizeOneStep();
        int nToShow = nShowImage - 1;
        if(nToShow < 0)
            nToShow = 0;
        if(nToShow >= (int) vCalibImgs.size())
            nToShow = static_cast<int>(vCalibImgs.size())-1;
        nShowImage = nToShow + 1;
    
        ofImage img;
        img.allocate(vCalibImgs[nToShow].mim.size().x, vCalibImgs[nToShow].mim.size().y, OF_IMAGE_GRAYSCALE);
        img.setFromPixels(vCalibImgs[nToShow].mim.data(), vCalibImgs[nToShow].mim.size().x, vCalibImgs[nToShow].mim.size().y, OF_IMAGE_GRAYSCALE);
        img.draw(0,0);
        
        vCalibImgs[nToShow].draw3DGrid(*mpCamera,true);
    }
}

void ofxCameraCalibrator::reset(){
    //mpCamera->manualParamUpdate(TooN::makeVector(0.92295, 1.28292, 0.497764, 0.490052, 0)); //ATANCamera::mvDefaultParams;
    if ( bDisableDistortion ) mpCamera->disableRadialDistortion();
    
    //mpCamera->SetImageSize(mVideoSource.Size());
    //CVD::ImageRef mirSize = CVD::ImageRef(imgWidth,imgHeight);
    //mimFrameBW.resize(mirSize);
    
    bGrabNextFrame = false;
    bOptimizing = false;
    vCalibImgs.clear();
}


void ofxCameraCalibrator::optimizeOneStep(){
    int nViews = static_cast<int>(vCalibImgs.size());
    int nDim = 6 * nViews + NUMTRACKERCAMPARAMETERS;
    
    int nCamParamBase = nDim - NUMTRACKERCAMPARAMETERS;
    
    TooN::Matrix<> mJTJ(nDim, nDim);
    TooN::Vector<> vJTe(nDim);
    mJTJ = TooN::Identity; // Weak stabilizing prior
    vJTe = TooN::Zeros;
    
    if( bDisableDistortion ) mpCamera->disableRadialDistortion();
    
    
    double dSumSquaredError = 0.0;
    int nTotalMeas = 0;
    
    for(int n=0; n<nViews; n++){
        int nMotionBase = n*6;
        vector<ofxCalibImage::ErrorAndJacobians> vEAJ = vCalibImgs[n].project(*mpCamera);
        for(unsigned int i=0; i<vEAJ.size(); i++){
            ofxCalibImage::ErrorAndJacobians &EAJ = vEAJ[i];
            // All the below should be +=, but the MSVC compiler doesn't seem to understand that. :(
            mJTJ.slice(nMotionBase, nMotionBase, 6, 6) = 
            mJTJ.slice(nMotionBase, nMotionBase, 6, 6) + EAJ.m26PoseJac.T() * EAJ.m26PoseJac;
            mJTJ.slice(nCamParamBase, nCamParamBase, NUMTRACKERCAMPARAMETERS, NUMTRACKERCAMPARAMETERS) = 
            mJTJ.slice(nCamParamBase, nCamParamBase, NUMTRACKERCAMPARAMETERS, NUMTRACKERCAMPARAMETERS) + EAJ.m2NCameraJac.T() * EAJ.m2NCameraJac;
            mJTJ.slice(nMotionBase, nCamParamBase, 6, NUMTRACKERCAMPARAMETERS) =
            mJTJ.slice(nMotionBase, nCamParamBase, 6, NUMTRACKERCAMPARAMETERS) + EAJ.m26PoseJac.T() * EAJ.m2NCameraJac;
            mJTJ.T().slice(nMotionBase, nCamParamBase, 6, NUMTRACKERCAMPARAMETERS) = 
            mJTJ.T().slice(nMotionBase, nCamParamBase, 6, NUMTRACKERCAMPARAMETERS) + EAJ.m26PoseJac.T() * EAJ.m2NCameraJac;
            // Above does twice the work it needs to, but who cares..
            
            vJTe.slice(nMotionBase,6) = 
            vJTe.slice(nMotionBase,6) + EAJ.m26PoseJac.T() * EAJ.v2Error;
            vJTe.slice(nCamParamBase,NUMTRACKERCAMPARAMETERS) = 
            vJTe.slice(nCamParamBase,NUMTRACKERCAMPARAMETERS) + EAJ.m2NCameraJac.T() * EAJ.v2Error;
            
            dSumSquaredError += EAJ.v2Error * EAJ.v2Error;
            ++nTotalMeas;
        }
    };
    
    meanPixelError = std::sqrt(dSumSquaredError / nTotalMeas);
    
    TooN::SVD<> svd(mJTJ);
    TooN::Vector<> vUpdate(nDim);
    vUpdate = svd.backsub(vJTe);
    vUpdate *= 0.1; // Slow down because highly nonlinear...
    for(int n=0; n<nViews; n++)
        vCalibImgs[n].mse3CamFromWorld = PTAMM::SE3<>::exp(vUpdate.slice(n * 6, 6)) * vCalibImgs[n].mse3CamFromWorld;
    
    mpCamera->updateParameters( vUpdate.slice(nCamParamBase, NUMTRACKERCAMPARAMETERS) );
};

