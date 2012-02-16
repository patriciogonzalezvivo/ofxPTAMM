//
//  ofxCalibImage.cpp
//  emptyExample
//
//  Created by Patricio González Vivo on 2/14/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#include "ofxCalibImage.h"
#include "cvd/image.h"

void ofxCalibImage::drawImageGrid(){
    ofPushStyle();
    ofPushView();
    
    ofSetLineWidth(2); //glLineWidth(2);
    ofSetColor(0, 0, 255);// glColor3f(0,0,1);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    
    for(int i=0; i< (int) mvGridCorners.size(); i++){
        for(int dirn=0; dirn<4; dirn++)
            if(mvGridCorners[i].aNeighborStates[dirn].val > i){
                ofLine( mvGridCorners[i].Params.v2Pos[0], mvGridCorners[i].Params.v2Pos[1],
                       mvGridCorners[mvGridCorners[i].aNeighborStates[dirn].val].Params.v2Pos[0],
                       mvGridCorners[mvGridCorners[i].aNeighborStates[dirn].val].Params.v2Pos[1]); 
            }
    }
    
    ofSetColor(255, 255, 0);
    for(unsigned int i=0; i<mvGridCorners.size(); i++)
        ofCircle(mvGridCorners[i].Params.v2Pos[0], mvGridCorners[i].Params.v2Pos[1], 3);
    
    ofPopStyle();
    ofPopView();
};

void ofxCalibImage::draw3DGrid(ATANCamera &Camera, bool bDrawErrors){
    
    ofPushStyle();
    ofPushView();
    ofSetLineWidth(2);
    ofSetColor(0, 0, 255);
    
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    
    for(int i=0; i< (int) mvGridCorners.size(); i++){
        for(int dirn=0; dirn<4; dirn++)
            if(mvGridCorners[i].aNeighborStates[dirn].val > i){
                Vector<3> v3; v3[2] = 0.0;
                v3.slice<0,2>() = vec(mvGridCorners[i].irGridPos);
                Vector<2> A = Camera.Project(project(mse3CamFromWorld * v3));
                
                v3.slice<0,2>() = vec(mvGridCorners[mvGridCorners[i].aNeighborStates[dirn].val].irGridPos);
                Vector<2> B = Camera.Project(project(mse3CamFromWorld * v3));
                ofLine(A[0], A[1], B[0], B[1]);
            }
    }
    
    if(bDrawErrors){
        ofSetColor(255,0, 255);
        ofSetLineWidth(1);
        for(int i=0; i< (int) mvGridCorners.size(); i++){
            Vector<3> v3; v3[2] = 0.0;
            v3.slice<0,2>() = vec(mvGridCorners[i].irGridPos);
            Vector<2> v2Pixels_Projected = Camera.Project(project(mse3CamFromWorld * v3));
            Vector<2> v2Error = mvGridCorners[i].Params.v2Pos - v2Pixels_Projected;
            Vector<2> B = v2Pixels_Projected + 10.0 * v2Error;
            ofLine(v2Pixels_Projected[0], v2Pixels_Projected[1], 
                   B[0], B[1]);
        }
    }
};