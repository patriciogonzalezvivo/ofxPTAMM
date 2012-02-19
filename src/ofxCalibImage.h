//
//  ofxCalibImage.h
//  emptyExample
//
//  Created by Patricio González Vivo on 2/14/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//
#include "ofMain.h"

#include "ofxATANCamera.h"
#include "CalibCornerPatch.h"

#include "cvd/utility.h"
#include "cvd/convolution.h"
#include "cvd/fast_corner.h"
#include "cvd/vector_image_ref.h"
#include "cvd/image_interpolate.h"
#include "cvd/image.h"

#include "TooN/se3.h"
#include "TooN/SVD.h"
#include "TooN/wls.h"

const int N_NOT_TRIED=-1;
const int N_FAILED=-2;

struct ofxDataPackege{
    double  MaxStepDistFraction;
    double  BlurSigma;
    int     CornerPatchSize;
    int     MinCornersForGrabbedImage;
    int     MeanGate;
    bool    DisableDistortion;
    bool    Optimizing;
    
};

struct ofxCalibGridCorner{
    struct NeighborState{
        NeighborState() {val = N_NOT_TRIED;}
        int val;
    };
    
    PTAMM::CalibCornerPatch::Params Params;
    CVD::ImageRef irGridPos;
    NeighborState aNeighborStates[4];
    
    TooN::Matrix<2> GetSteps(std::vector<ofxCalibGridCorner> &vgc); 
    TooN::Matrix<2> mInheritedSteps;
    
    void draw();
    
    double expansionPotential();
};

class ofxCalibImage {
public:
    bool makeFromImage(CVD::Image<CVD::byte> &im, ofxDataPackege * _data);
    void drawImageGrid();
    void draw3DGrid(ofxATANCamera &Camera, bool bDrawErrors);
    void guessInitialPose(ofxATANCamera &Camera);
    
    struct ErrorAndJacobians{
        TooN::Vector<2> v2Error;
        TooN::Matrix<2,6> m26PoseJac;
        TooN::Matrix<2,NUMTRACKERCAMPARAMETERS> m2NCameraJac;
    };
    
    std::vector<ErrorAndJacobians> project(ofxATANCamera &Camera);
    CVD::Image<CVD::byte> mim;
    TooN::SE3<> mse3CamFromWorld;
    
protected:
    std::vector<CVD::ImageRef> mvCorners;
    std::vector<ofxCalibGridCorner> mvGridCorners;
    
    ofxDataPackege *data;
    
    bool        expandByAngle(int nSrc, int nDirn);
    int         nextToExpand();
    void        expandByStep(int n);
    CVD::ImageRef ir_from_dirn(int nDirn);
};
