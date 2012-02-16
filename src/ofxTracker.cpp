/*
 *  ofxTracker.cpp
 *  try_PTAM3
 *
 *  Created by Akira Asia on 10/04/12.
 *  Copyright 2010 This is Ampontang. No rights reserved.
 *
 */

#include "ofxTracker.h"
#include "cvd/image.h"
#include "OpenGL.h"
#include "MapPoint.h"

using namespace CVD;

void ofxTracker::buildMapBegin() {
	mbUserPressedSpacebar = true;
}

void ofxTracker::draw(bool _drawPoints){
    //By now it draw debug information like the trail and the grid
    ofPushView();
    ofPushMatrix();
    ofPushStyle();
    if(mpMap->IsGood() && !IsLost() ){
        RenderGrid();
        
        if (_drawPoints){
            std::vector<MapPoint*> points = mpMap->vpPoints;
            int totalPoints = points.size();
            
            for (int i = 0; i < totalPoints; i++){
                
                const Vector<3> v3 = points.at(i)->v3WorldPos;
                const int level = points.at(i)->nSourceLevel;
                
                Vector<3> v3Cam = mse3CamFromWorld * v3;
                
                if(v3Cam[2] < 0.001)
                    v3Cam[2] = 0.001;
                
                Vector<2> pos = mCamera.Project( project(v3Cam) );
                
                int size = 0;
                
                if (level == 0){
                    ofSetColor(161, 28, 109);
                    size = 1;
                } else if (level == 1){
                    ofSetColor(120,106, 164);
                    size = 2;
                } else if (level == 2){
                    ofSetColor(86,74, 176);
                    size = 3;
                } else if (level == 3){
                    ofSetColor(18,255, 249);
                    size = 4;
                }
                
                ofNoFill();
                ofCircle(pos[0]-size*0.5,pos[1]-size*0.5, size);
            }
        }
        
    } else if(mnInitialStage == TRAIL_TRACKING_STARTED) { 
        GLfloat tempTrailVertices[4];
        GLfloat tempTrailColours[8];
        
        glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLineWidth(2);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
        
        MiniPatch BackwardsPatch;
        Level &lCurrentFrame = mCurrentKF.aLevels[0];
        Level &lPreviousFrame = mPreviousFrameKF.aLevels[0];
        
        for(list<Trail>::iterator i = mlTrails.begin(); i!=mlTrails.end();)
        {
            list<Trail>::iterator next = i; next++;
            
            Trail &trail = *i;
            ImageRef irStart = trail.irCurrentPos;
            ImageRef irEnd = irStart;
            bool bFound = trail.mPatch.FindPatch(irEnd, lCurrentFrame.im, 10, lCurrentFrame.vCorners);
            if(bFound)
            {
                // Also find backwards in a married-matches check
                BackwardsPatch.SampleFromImage(irEnd, lCurrentFrame.im);
                ImageRef irBackWardsFound = irEnd;
                bFound = BackwardsPatch.FindPatch(irBackWardsFound, lPreviousFrame.im, 10, lPreviousFrame.vCorners);
                if((irBackWardsFound - irStart).mag_squared() > 2)
                    bFound = false;
                
                trail.irCurrentPos = irEnd;
            }
            
            if(!bFound){
                tempTrailColours[0]=0.0; 
                tempTrailColours[1]=0.631;
                tempTrailColours[2]=0.435;
                tempTrailColours[3]=1.0;
            } else {
                tempTrailColours[0]=0.945; 
                tempTrailColours[1]=0.928;
                tempTrailColours[2]=0.082;
                tempTrailColours[3]=1.0;
            }
            
            
            tempTrailVertices[0]=trail.irInitialPos.x;
            tempTrailVertices[1]=trail.irInitialPos.y;
            
            if(bFound){
                tempTrailColours[4]=1.0; 
                tempTrailColours[5]=0.204;
                tempTrailColours[6]=0.553;
                tempTrailColours[7]=1.0;
            } else {
                tempTrailColours[4]=tempTrailColours[0];
                tempTrailColours[5]=tempTrailColours[1];
                tempTrailColours[6]=tempTrailColours[2];
                tempTrailColours[7]=tempTrailColours[3];
            }
            tempTrailVertices[2]=trail.irCurrentPos.x;
            tempTrailVertices[3]=trail.irCurrentPos.y;
            
            glVertexPointer(2,GL_FLOAT,0,&tempTrailVertices);
            glColorPointer(4, GL_FLOAT, 0, &tempTrailColours);
            glDrawArrays(GL_LINE_STRIP, 0, 2);
            
            if(!bFound) // Erase from list of trails if not found this frame.
                mlTrails.erase(i);
            
            i = next;
        }
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        mPreviousFrameKF = mCurrentKF;
    }
    ofPopView();
    ofPopMatrix();
    ofPopStyle();
}