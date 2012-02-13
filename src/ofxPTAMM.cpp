/*
 *  ofxPTAM.cpp
 *  try_PTAM
 *
 *  Created by Akira Asia on 10/03/29.
 *  Copyright 2010 __This is Ampontang.__. No rights reserved.
 *
 */

#include "ofxPTAMM.h"
#include "ofxATANCamera.h"

#include "cvd/colourspace.h"
#include "gvars3/instances.h"
#include "MapMaker.h"
#include "ofxTracker.h"
#include "OpenGL.h"
#include "MapPoint.h"

using namespace CVD;
using namespace GVars3;

ofxPTAMM::ofxPTAMM(){
    mpMap = new PTAMM::Map;
    mvpMaps.push_back( mpMap );
    mpMap->mapLockManager.Register(this);
    mpMapMaker = new PTAMM::MapMaker( mvpMaps, mpMap );
    //mpMapViewer = new MapViewer(mvpMaps, mpMap, mGLWindow);
    //mpMapSerializer = new PTAMM::MapSerializer( mvpMaps );
    
	bMapBuildComplete = false;
}

ofxPTAMM::~ofxPTAMM(){
    if( mpMap != NULL )  {
        mpMap->mapLockManager.UnRegister( this );
    }
}

void ofxPTAMM::init(int imgW, int imgH) {

	imgWidth = imgW;
	imgHeight = imgH;	
		
	mimFrameBW.resize(ImageRef(imgWidth,imgHeight));
    // First, check if the camera is calibrated.
    // If not, we need to run the calibration widget.
    GV3::get<Vector<NUMTRACKERCAMPARAMETERS> >("Camera.Parameters", ATANCamera::mvDefaultParams, HIDDEN);
    mpCamera = new ofxATANCamera("Camera");
    mpCamera->manualParamUpdate("camera.cfg");	
    //mpCamera->SetImageSize(mVideoSource.Size());
    
    if(!mpCamera->paramTest())
    {
        cout << endl;
        cout << "! Camera.Parameters is not set, need to run the CameraCalibrator tool" << endl;
        cout << "  and/or put the Camera.Parameters= line into the appropriate .cfg file." << endl;
        exit(1);
    }
    
    mpTracker = new ofxTracker(ImageRef(imgWidth,imgHeight), *mpCamera, mvpMaps, mpMap, *mpMapMaker);	
}

void ofxPTAMM::update(ofPixelsRef _pixelsRef){
    // TODOs
    //    - some checks here in order to avoid SEGMENTATION_FALT
    
    int nChannels = _pixelsRef.getNumChannels();
    unsigned char * pixels = _pixelsRef.getPixels();

    ImageRef mirSize = ImageRef(imgWidth,imgHeight);
    BasicImage<CVD::byte> imCaptured(pixels, mirSize);
    mimFrameBW.resize(mirSize);
    
    if (nChannels == 1){
        for (int y=0; y<mirSize.y; y++) {
            for (int x=0; x<mirSize.x; x++) {
                mimFrameBW[y][x]        = *pixels;
                pixels++;
            }
        }
    } else if (nChannels > 1){
        for (int y=0; y<mirSize.y; y++) {
            for (int x=0; x<mirSize.x; x++) {
                pixels++;
                mimFrameBW[y][x]        = *pixels;
                pixels++;
                pixels++;
            }
        }
    } else if (nChannels == 4){
        for (int y=0; y<mirSize.y; y++) {
            for (int x=0; x<mirSize.x; x++) {
                pixels++;
                mimFrameBW[y][x]        = *pixels;
                pixels++;
                pixels++;
                pixels++;
            }
        }
    }
    
    //Check if the map has been locked by another thread, and wait for release.
    bool bWasLocked = mpMap->mapLockManager.CheckLockAndWait( this, 0 );
    /* This is a rather hacky way of getting this feedback,
     but GVars cannot be assigned to different variables
     and each map has its own edit lock bool.
     A button could be used instead, but the visual
     feedback would not be as obvious.
     */ 
    
    // TODO: - not ready for multi-thread
    
    //mpMap->bEditLocked = *mgvnLockMap; //sync up the maps edit lock with the gvar bool.
    
    if(bWasLocked)
        mpTracker->ForceRecovery();
    
	mpTracker->TrackFrame(mimFrameBW, false);
	bMapBuildComplete = mpMap->IsGood();
    ofLog( ofLogLevel(OF_LOG_VERBOSE) , mpTracker->GetMessageForUser());
}

void ofxPTAMM::draw() {
    mpTracker->draw();
}

ofVec2f ofxPTAMM::getScreenPosition() const{
    ofVec2f rta = ofVec3f(0,0);
    
    if ( isMapBuild() ){
        SE3<> cvdMapMatrix = mpTracker->GetCurrentPose();
        Matrix<4> cvdCamMatrix = mpCamera->MakeUFBLinearFrustumMatrix(0.005, 100);
        
        Vector<3> v3;
        v3[0] = 0.001;
        v3[1] = 0.001;
        v3[2] = 0.001;
        Vector<3> v3Cam = cvdMapMatrix * v3;
        if(v3Cam[2] < 0.001)
            v3Cam[2] = 0.001;
        Vector<2> vPos = mpCamera->Project(project(v3Cam));
        
        rta.set(vPos[0], vPos[1]);
    }
    
    return rta;
};

void ofxPTAMM::moveCamera(){
    mpCamera->SetImageSize(ImageRef(imgWidth,imgHeight));
    
    SE3<> cvdMatrix = mpTracker->GetCurrentPose();
	
    glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
    
	//glMultMatrix(mpCamera->MakeUFBLinearFrustumMatrix(0.005, 100));
    TooN::Matrix<4> mC = mpCamera->MakeUFBLinearFrustumMatrix(0.005, 100);
    GLdouble glmC[16];
    glmC[0] = mC[0][0]; glmC[1] = mC[1][0]; glmC[2] = mC[2][0]; glmC[3] = mC[3][0];
    glmC[4] = mC[0][1]; glmC[5] = mC[1][1]; glmC[6] = mC[2][1]; glmC[7] = mC[3][1];
    glmC[8] = mC[0][2]; glmC[9] = mC[1][2]; glmC[10] = mC[2][2]; glmC[11] = mC[3][2];
    glmC[12] = mC[0][3]; glmC[13] = mC[1][3]; glmC[14] = mC[2][3]; glmC[15] = mC[3][3];
    
    glMultMatrixd(glmC);
	
    //glMultMatrix( cvdMatrix );
    glTranslated( cvdMatrix.get_translation()[0], cvdMatrix.get_translation()[1], cvdMatrix.get_translation()[2]);
    TooN::Matrix<3> m = cvdMatrix.get_rotation().get_matrix();
    GLdouble glm[16];
    
    glm[0] = m[0][0]; glm[1] = m[1][0]; glm[2] = m[2][0]; glm[3] = 0;
    glm[4] = m[0][1]; glm[5] = m[1][1]; glm[6] = m[2][1]; glm[7] = 0;
    glm[8] = m[0][2]; glm[9] = m[1][2]; glm[10] = m[2][2]; glm[11] = 0;
    glm[12] = 0; glm[13] = 0; glm[14] = 0; glm[15] = 1;

    glMultMatrixd(glm);
     
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrix(SE3<>());
    
    glScaled(PTAM_SCALE, PTAM_SCALE, PTAM_SCALE);
    //ofRotate(180, 0, 1, 0);
    glRotatef(180, 0, 1, 0);
};

ofMatrix4x4 ofxPTAMM::getCameraMatrix() const{
    TooN::Matrix<4> m = mpCamera->MakeUFBLinearFrustumMatrix(0.005, 100);
    /*
    GLdouble glm[16];
    glm[0] = m[0][0]; glm[1] = m[1][0]; glm[2] = m[2][0]; glm[3] = m[3][0];
    glm[4] = m[0][1]; glm[5] = m[1][1]; glm[6] = m[2][1]; glm[7] = m[3][1];
    glm[8] = m[0][2]; glm[9] = m[1][2]; glm[10] = m[2][2]; glm[11] = m[3][2];
    glm[12] = m[0][3]; glm[13] = m[1][3]; glm[14] = m[2][3]; glm[15] = m[3][3];
    glMultMatrixd(glm);*/
    return ofMatrix4x4(m[0][0], m[1][0], m[2][0], m[3][0], 
                       m[0][1], m[1][1], m[2][1], m[3][1], 
                       m[0][2], m[1][2], m[2][2], m[3][2], 
                       m[0][3], m[1][3], m[2][3], m[3][3]);
};

ofVec3f ofxPTAMM:: getTranslation() const{
    SE3<> cvdMatrix = mpTracker->GetCurrentPose();
    //glTranslated( cvdMatrix.get_translation()[0], cvdMatrix.get_translation()[1], cvdMatrix.get_translation()[2]);
    return ofVec3f(cvdMatrix.get_translation()[0], cvdMatrix.get_translation()[1], cvdMatrix.get_translation()[2]);
};

ofMatrix4x4 ofxPTAMM::getRotationMatrix() const{
    SE3<> cvdMatrix = mpTracker->GetCurrentPose();
    
    TooN::Matrix<3> m = cvdMatrix.get_rotation().get_matrix();
    /*
    GLdouble glm[16];
    glm[0] = m[0][0]; glm[1] = m[1][0]; glm[2] = m[2][0]; glm[3] = 0;
    glm[4] = m[0][1]; glm[5] = m[1][1]; glm[6] = m[2][1]; glm[7] = 0;
    glm[8] = m[0][2]; glm[9] = m[1][2]; glm[10] = m[2][2]; glm[11] = 0;
    glm[12] = 0; glm[13] = 0; glm[14] = 0; glm[15] = 1;
    glMultMatrixd(glm);
    */
    return ofMatrix4x4(m[0][0], m[1][0], m[2][0], 0, 
                       m[0][1], m[1][1], m[2][1], 0, 
                       m[0][2], m[1][2], m[2][2], 0, 
                       0,       0,       0,       1);
};

void ofxPTAMM::resetMap() {
	mpTracker->reset();
}
void ofxPTAMM::startBuildMap() {
	mpTracker->buildMapBegin();
}

/**
 * Switch to the map with ID nMapNum
 * @param  nMapNum Map ID
 * @param bForce This is only used by DeleteMap and ResetAll, and is
 * to ensure that MapViewer is looking at a safe map.
 */
bool ofxPTAMM::SwitchMap( int nMapNum, bool bForce ){
    
    //same map, do nothing. This should not actually occur
    if(mpMap->MapID() == nMapNum) {
        return true;
    }
    
    if( (nMapNum < 0) ){
        cerr << "Invalid map number: " << nMapNum << ". Not changing." << endl;
        return false;
    }
    
    
    for( size_t ii = 0; ii < mvpMaps.size(); ii++ ){
        PTAMM::Map * pcMap = mvpMaps[ ii ];
        if( pcMap->MapID() == nMapNum ) {
            mpMap->mapLockManager.UnRegister( this );
            mpMap = pcMap;
            mpMap->mapLockManager.Register( this );
        }
    }
    
    if(mpMap->MapID() != nMapNum){
        cerr << "Failed to switch to " << nMapNum << ". Does not exist." << endl;
        return false;
    }
    
    /*  Map was found and switched to for system.
     Now update the rest of the system.
     Order is important. Do not want keyframes added or
     points deleted from the wrong map.
     
     MapMaker is in its own thread.
     System,Tracker, and MapViewer are all in this thread.
     */
    
    *mgvnLockMap = mpMap->bEditLocked;
    
    
    //update the map maker thread
    if( !mpMapMaker->RequestSwitch( mpMap ) ) {
        return false;
    }
    
    while( !mpMapMaker->SwitchDone() ) {
#ifdef WIN32
        Sleep(1);
#else
        usleep(10);
#endif
    }
    
    //update the map viewer object
    //mpMapViewer->SwitchMap(mpMap, bForce);
    
    if( !mpTracker->SwitchMap( mpMap ) ) {
        return false;
    }
    
    return true;
}

/**
 * Create a new map and switch all
 * threads and objects to it.
 */
void ofxPTAMM::NewMap(){
    
    *mgvnLockMap = false;
    mpMap->mapLockManager.UnRegister( this );
    mpMap = new Map();
    mpMap->mapLockManager.Register( this );
    mvpMaps.push_back( mpMap );
    
    //update the map maker thread
    mpMapMaker->RequestReInit( mpMap );
    while( !mpMapMaker->ReInitDone() ) {
#ifdef WIN32
        Sleep(1);
#else
        usleep(10);
#endif
    }
    
    //update the map viewer object
    //mpMapViewer->SwitchMap(mpMap);
    
    mpTracker->SetNewMap( mpMap );
    
    cout << "New map created (" << mpMap->MapID() << ")" << endl;
}


/**
 * Moves all objects and threads to the first map, and resets it.
 * Then deletes the rest of the maps, placing PTAMM in its
 * original state.
 * This reset ignores the edit lock status on all maps
 */
void ofxPTAMM::ResetAll(){
    
    //move all maps to first map.
    if( mpMap != mvpMaps.front() ){
        if( !SwitchMap( mvpMaps.front()->MapID(), true ) ) {
            cerr << "Reset All: Failed to switch to first map" << endl;
        }
    }
    mpMap->bEditLocked = false;
    
    //reset map.
    mpTracker->Reset();
    
    //lock and delete all remaining maps
    while( mvpMaps.size() > 1 ){
        DeleteMap( mvpMaps.back()->MapID() );
    }
    
}

/**
 * Delete a specified map.
 * @param nMapNum map to delete
 */
bool ofxPTAMM::DeleteMap( int nMapNum ){
    if( mvpMaps.size() <= 1 ){
        cout << "Cannot delete the only map. Use Reset instead." << endl;
        return false;
    }
    
    
    //if the specified map is the current map, move threads to another map
    if( nMapNum == mpMap->MapID() ){
        int nNewMap = -1;
        
        if( mpMap == mvpMaps.front() ) {
            nNewMap = mvpMaps.back()->MapID();
        } else {
            nNewMap = mvpMaps.front()->MapID();
        }
        
        // move the current map users elsewhere
        if( !SwitchMap( nNewMap, true ) ) {
            cerr << "Delete Map: Failed to move threads to another map." << endl;
            return false;
        }
    }
    
    
    
    // find and delete the map
    for( size_t ii = 0; ii < mvpMaps.size(); ii++ ){
        Map * pDelMap = mvpMaps[ ii ];
        if( pDelMap->MapID() == nMapNum ) {
            
            pDelMap->mapLockManager.Register( this );
            pDelMap->mapLockManager.LockMap( this );
            delete pDelMap;
            mvpMaps.erase( mvpMaps.begin() + ii );
            
            ///@TODO Possible bug. If another thread (eg serialization) was using this
            /// and waiting for unlock, would become stuck or seg fault.
        }
    }
    
    return true;
}


/**
 * Set up the map serialization thread for saving/loading and the start the thread
 * @param sCommand the function that was called (eg. SaveMap)
 * @param sParams the params string, which may contain a filename and/or a map number
 */
/*
void ofxPTAMM::StartMapSerialization(std::string sCommand, std::string sParams) {
    if( mpMapSerializer->Init( sCommand, sParams, *mpMap) ) {
        mpMapSerializer->start();
    }
}*/

