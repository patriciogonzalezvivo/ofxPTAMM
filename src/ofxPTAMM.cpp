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
    
    cout << mpTracker->GetMessageForUser() << endl;
}

void ofxPTAMM::draw() {
    mpTracker->draw();
}

ofVec2f ofxPTAMM::getPosition() const{
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

ofVec3f ofxPTAMM::getOrientation() const{
    SE3<> cvdMatrix = mpTracker->GetCurrentPose();
    /*
    float heading = 0.0;
	float attitude= 0.0;
	float bank = 0.0;
    
    //Assuming the angles are in radians.
	if (cvdMatrix.get_rotation().get_matrix()[1][0] > 0.998) { // singularity at north pole
		heading = atan2(cvdMatrix.get_rotation().get_matrix()[0][2],cvdMatrix.get_rotation().get_matrix()[2][2]);
		attitude = PI/2;
		bank = 0;
	} else if (cvdMatrix.get_rotation().get_matrix()[1][0] < -0.998) { // singularity at south pole
		heading = atan2(cvdMatrix.get_rotation().get_matrix()[0][2],cvdMatrix.get_rotation().get_matrix()[2][2]);
		attitude = -PI/2;
		bank = 0;
	}
    
	heading = atan2(-cvdMatrix.get_rotation().get_matrix()[2][0],cvdMatrix.get_rotation().get_matrix()[0][0]);
	bank = atan2(-cvdMatrix.get_rotation().get_matrix()[1][2],cvdMatrix.get_rotation().get_matrix()[1][1]);
	attitude = asin(cvdMatrix.get_rotation().get_matrix()[1][0]);
     
    return ofVec3f(attitude,heading,bank);
    */
    
	ofMatrix4x4 matrix = ofMatrix4x4(cvdMatrix.get_rotation().get_matrix()[0][0], 
                                     cvdMatrix.get_rotation().get_matrix()[0][1], 
                                     cvdMatrix.get_rotation().get_matrix()[0][2], 
                                     0, 
                                     cvdMatrix.get_rotation().get_matrix()[1][0], 
                                     cvdMatrix.get_rotation().get_matrix()[1][1], 
                                     cvdMatrix.get_rotation().get_matrix()[1][2], 
                                     0,
                                     cvdMatrix.get_rotation().get_matrix()[2][0], 
                                     cvdMatrix.get_rotation().get_matrix()[2][1], 
                                     cvdMatrix.get_rotation().get_matrix()[2][2], 
                                     0,
                                     0,0,0,1);
    
    return matrix.getRotate().getEuler();
};

ofMatrix4x4 ofxPTAMM::getRotationMatrix() const {
    ofVec3f euler = getOrientation();
	ofMatrix4x4 matrix;
	matrix.makeRotationMatrix(ofRadToDeg(euler.x), ofVec3f(1,0,0),
                              ofRadToDeg(euler.y), ofVec3f(0,1,0),
                              ofRadToDeg(euler.z), ofVec3f(0,0,1));
    return matrix;
}

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

