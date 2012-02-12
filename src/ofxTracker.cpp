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

void ofxTracker::draw(){
    if(mpMap->IsGood()){
        RenderGrid();
    } else {
        
        if(mnInitialStage == TRAIL_TRACKING_STARTED) { 
            
            // Draw Trace
            
            glPointSize(5);
            glLineWidth(2);
            
            glEnable(GL_POINT_SMOOTH);
            glEnable(GL_LINE_SMOOTH);
            
            glBegin(GL_LINES);
            
            MiniPatch BackwardsPatch;
            Level &lCurrentFrame = mCurrentKF.aLevels[0];
            Level &lPreviousFrame = mPreviousFrameKF.aLevels[0];	
            
            for(list<Trail>::iterator i = mlTrails.begin(); i!=mlTrails.end();)	{
                
                list<Trail>::iterator next = i; next++;
                
                Trail &trail = *i;
                ImageRef irStart = trail.irCurrentPos;
                ImageRef irEnd = irStart;
                bool bFound = trail.mPatch.FindPatch(irEnd, lCurrentFrame.im, 10, lCurrentFrame.vCorners);
                
                if(bFound) {
                    // Also find backwards in a married-matches check
                    BackwardsPatch.SampleFromImage(irEnd, lCurrentFrame.im);
                    ImageRef irBackWardsFound = irEnd;
                    bFound = BackwardsPatch.FindPatch(irBackWardsFound, lPreviousFrame.im, 10, lPreviousFrame.vCorners);
                    if((irBackWardsFound - irStart).mag_squared() > 2)
                        bFound = false;
                    
                    trail.irCurrentPos = irEnd;
                    
                }
                
                glVertex(trail.irInitialPos);
                
                glVertex(trail.irCurrentPos);
                
                
                if(!bFound) {// Erase from list of trails if not found this frame.
                    mlTrails.erase(i);
                }
                
                i = next;
            }
            
            glEnd();
        }
    }
};