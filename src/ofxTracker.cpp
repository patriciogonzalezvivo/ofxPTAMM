/*
 *  ofxTracker.cpp
 *  try_PTAM3
 *
 *  Created by Akira Asia on 10/04/12.
 *  Copyright 2010 This is Ampontang. No rights reserved.
 *
 */

#include "ofxTracker.h"

void ofxTracker::buildMapBegin() {
	mbUserPressedSpacebar = true;
}

void ofxTracker::draw(bool _drawPoints){
    //By now it draw debug information like the trail and the grid
    
    if( mpMap->IsGood() & !IsLost() ){
        // The colour of the ref grid shows if the coarse stage of tracking was used
        // (it's turned off when the camera is sitting still to reduce jitter.)
        
        if(mbDidCoarse)
            ofSetColor(0, 255*0.5, 255, 255*0.6);
        else
            ofSetColor(0, 0, 255, 255*0.6);
        
        // The grid is projected manually, i.e. GL receives projected 2D coords to draw.
        int nHalfCells = 8;
        int nTot = nHalfCells * 2 + 1;
        CVD::Image< TooN::Vector<2,GLfloat> >  imVertices( CVD::ImageRef(nTot,nTot));
        for(int i=0; i<nTot; i++)
            for(int j=0; j<nTot; j++){
                TooN::Vector<3> v3;
                v3[0] = (i - nHalfCells) * 0.1;
                v3[1] = (j - nHalfCells) * 0.1;
                v3[2] = 0.0;
                TooN::Vector<3> v3Cam = mse3CamFromWorld * v3;
                
                if(v3Cam[2] < 0.001)
                    v3Cam[2] = 0.001;
                imVertices[i][j] = mCamera.Project( PTAMM::project(v3Cam) );
            }
        
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(2);
        
        glEnableClientState(GL_VERTEX_ARRAY);
        for(int i=0; i<nTot; i++){
            glVertexPointer(2, GL_FLOAT, 0, &imVertices[i][0]);
            glDrawArrays(GL_LINE_STRIP, 0, nTot);
            
            glVertexPointer(2, GL_FLOAT,sizeof(GLfloat)*2*nTot, &imVertices[0][i]);
            glDrawArrays(GL_LINE_STRIP, 0, nTot);
        };
        glDisableClientState(GL_VERTEX_ARRAY);
        glLineWidth(1);
        glColor4f(1,0,0,1);
        
        if (_drawPoints){
            std::vector<PTAMM::MapPoint*> points = mpMap->vpPoints;
            int totalPoints = points.size();
            
            for (int i = 0; i < totalPoints; i++){
                
                const TooN::Vector<3> v3 = points.at(i)->v3WorldPos;
                const int level = points.at(i)->nSourceLevel;
                
                TooN::Vector<3> v3Cam = mse3CamFromWorld * v3;
                
                if(v3Cam[2] < 0.001)
                    v3Cam[2] = 0.001;
                
                TooN::Vector<2> pos = mCamera.Project( PTAMM::project(v3Cam) );
                
                if (level == 0)
                    ofSetColor(255, 0, 0);
                else if (level == 1)
                    ofSetColor(0,0, 255);
                else if (level == 2)
                    ofSetColor(0,255, 0);
                
                ofCircle(pos[0],pos[1], 4);
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
        
        PTAMM::MiniPatch BackwardsPatch;
        PTAMM::Level &lCurrentFrame = mCurrentKF.aLevels[0];
        PTAMM::Level &lPreviousFrame = mPreviousFrameKF.aLevels[0];
        
        for(list<PTAMM::Trail>::iterator i = mlTrails.begin(); i!=mlTrails.end();){
            list<PTAMM::Trail>::iterator next = i; next++;
            
            PTAMM::Trail &trail = *i;
            CVD::ImageRef irStart = trail.irCurrentPos;
            CVD::ImageRef irEnd = irStart;
            bool bFound = trail.mPatch.FindPatch(irEnd, lCurrentFrame.im, 10, lCurrentFrame.vCorners);
            if(bFound){
                // Also find backwards in a married-matches check
                BackwardsPatch.SampleFromImage(irEnd, lCurrentFrame.im);
                CVD::ImageRef irBackWardsFound = irEnd;
                bFound = BackwardsPatch.FindPatch(irBackWardsFound, lPreviousFrame.im, 10, lPreviousFrame.vCorners);
                if((irBackWardsFound - irStart).mag_squared() > 2)
                    bFound = false;
                
                trail.irCurrentPos = irEnd;
            }
            
            if(!bFound){
                tempTrailColours[0]=0;
                tempTrailColours[1]=1.0; 
                tempTrailColours[2]=1.0;
                tempTrailColours[3]=1.0;
            } else {
                tempTrailColours[0]=1.0;
                tempTrailColours[1]=1.0; 
                tempTrailColours[2]=0;
                tempTrailColours[3]=1.0;
            }
            
            
            tempTrailVertices[0]=trail.irInitialPos.x;
            tempTrailVertices[1]=trail.irInitialPos.y;
            
            if(bFound){
                tempTrailColours[4]=1.0;
                tempTrailColours[5]=0; 
                tempTrailColours[6]=0;
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
            
            if(!bFound){ // Erase from list of trails if not found this frame.
                mlTrails.erase(i);
            }
            i = next;
        }
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        
        mPreviousFrameKF = mCurrentKF;
        
    }
}