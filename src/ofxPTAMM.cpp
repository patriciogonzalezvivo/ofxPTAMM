/*
 *  ofxPTAM.cpp
 *  try_PTAM
 *
 *  Created by Akira Asia on 10/03/29.
 *  Copyright 2010 __This is Ampontang.__. No rights reserved.
 *
 */

#include "ofxPTAMM.h"

using namespace GVars3;

ofxPTAMM::ofxPTAMM(){
    map = new PTAMM::Map;
    map->bGood = false;
    vMaps.push_back( map );
    map->mapLockManager.Register(this);
    mapMaker = new PTAMM::MapMaker( vMaps, map );
    mapSerializer = new PTAMM::MapSerializer( vMaps );
    
    bDebug = false;
	bMapBuildComplete = false;
    bLockMap = new bool(false);
}

ofxPTAMM::~ofxPTAMM(){
    if( map != NULL )  {
        map->mapLockManager.UnRegister( this );
    }
}

void ofxPTAMM::init(int imgW, int imgH, string configFile) {
    
	imgWidth = imgW;
	imgHeight = imgH;	
    
	mimFrameBW.resize( CVD::ImageRef(imgWidth,imgHeight));
    // First, check if the camera is calibrated.
    // If not, we need to run the calibration widget.
    GV3::get<TooN::Vector<NUMTRACKERCAMPARAMETERS> >("Camera.Parameters", ofxATANCamera::mvDefaultParams, HIDDEN);
    camera = new ofxATANCamera("Camera");
    camera->loadParameters(configFile);
    
    if(!camera->testParameters()){
        cout << endl;
        cout << "! Camera.Parameters is not set, need to run the CameraCalibrator tool" << endl;
        cout << "  and/or put the Camera.Parameters= line into the appropriate .cfg file." << endl;
        exit(1);
    }
    
    tracker = new ofxTracker( CVD::ImageRef(imgWidth,imgHeight), *camera, vMaps, map, *mapMaker);		
}

bool ofxPTAMM::update(unsigned char * _pixels, int _width, int _height, ofImageType _type){    
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
        return false;
    
    //Check if the map has been locked by another thread, and wait for release.
    bool bWasLocked = map->mapLockManager.CheckLockAndWait( this, 0 );
    
    /* This is a rather hacky way of getting this feedback,
     but GVars cannot be assigned to different variables
     and each map has its own edit lock bool.
     A button could be used instead, but the visual
     feedback would not be as obvious.
     */
    map->bEditLocked = *bLockMap; //sync up the maps edit lock with the gvar bool.
    
    if(bWasLocked)
        tracker->ForceRecovery();
    
	tracker->TrackFrame(mimFrameBW, false);
    
	bMapBuildComplete = map->IsGood();
    ofLog( OF_LOG_VERBOSE , tracker->GetMessageForUser());
    return true;
}

void ofxPTAMM::draw() {
    tracker->draw(bDebug);
}

bool ofxPTAMM::isMapPresent() const {
    return ( map->IsGood() && !(tracker->IsLost()));
}

ofVec2f ofxPTAMM::getScreenPosition() const{
    ofVec2f rta = ofVec3f(0,0);
    
    if ( isMapBuild() ){
        TooN::SE3<> cvdMapMatrix = tracker->GetCurrentPose();
        TooN::Matrix<4> cvdCamMatrix = camera->MakeUFBLinearFrustumMatrix(0.005, 100);
        
        TooN::Vector<3> v3;
        v3[0] = 0.001;
        v3[1] = 0.001;
        v3[2] = 0.001;
        TooN::Vector<3> v3Cam = cvdMapMatrix * v3;
        if(v3Cam[2] < 0.001)
            v3Cam[2] = 0.001;
        TooN::Vector<2> vPos = camera->Project(project(v3Cam));
        
        rta.set(vPos[0], vPos[1]);
    }
    
    return rta;
};

void ofxPTAMM::moveCamera() const {
    // TODO:
    //  - make this works on OpenGL 1.0 in order make camera translations and rotatios
    //    on iphone
    //
    camera->SetImageSize( CVD::ImageRef(imgWidth,imgHeight) );
    
    TooN::SE3<> cvdMatrix = tracker->GetCurrentPose();
	
    glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
    
    TooN::Matrix<4> mC = camera->MakeUFBLinearFrustumMatrix(0.005, 100);
    GLdouble glmC[16];
    glmC[0] = mC[0][0]; glmC[1] = mC[1][0]; glmC[2] = mC[2][0]; glmC[3] = mC[3][0];
    glmC[4] = mC[0][1]; glmC[5] = mC[1][1]; glmC[6] = mC[2][1]; glmC[7] = mC[3][1];
    glmC[8] = mC[0][2]; glmC[9] = mC[1][2]; glmC[10] = mC[2][2]; glmC[11] = mC[3][2];
    glmC[12] = mC[0][3]; glmC[13] = mC[1][3]; glmC[14] = mC[2][3]; glmC[15] = mC[3][3];
    glMultMatrixd(glmC);
	
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
	//glMultMatrix( TooN::SE3<>() );
    
    ofScale(PTAM_SCALE, PTAM_SCALE, PTAM_SCALE);
    ofRotate(180, 0, 1, 0);
};

ofMatrix4x4 ofxPTAMM::getCameraMatrix() const{
    TooN::Matrix<4> m = camera->MakeUFBLinearFrustumMatrix(0.005, 100);
    /*GLdouble glm[16];
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
    TooN::SE3<> cvdMatrix = tracker->GetCurrentPose();
    //glTranslated( cvdMatrix.get_translation()[0], cvdMatrix.get_translation()[1], cvdMatrix.get_translation()[2]);
    return ofVec3f(cvdMatrix.get_translation()[0], cvdMatrix.get_translation()[1], cvdMatrix.get_translation()[2]);
};

ofMatrix4x4 ofxPTAMM::getRotationMatrix() const{
    TooN::SE3<> cvdMatrix = tracker->GetCurrentPose();
    
    TooN::Matrix<3> m = cvdMatrix.get_rotation().get_matrix();
    /*GLdouble glm[16];
     glm[0] = m[0][0]; glm[1] = m[1][0]; glm[2] = m[2][0]; glm[3] = 0;
     glm[4] = m[0][1]; glm[5] = m[1][1]; glm[6] = m[2][1]; glm[7] = 0;
     glm[8] = m[0][2]; glm[9] = m[1][2]; glm[10] = m[2][2]; glm[11] = 0;
     glm[12] = 0; glm[13] = 0; glm[14] = 0; glm[15] = 1;
     glMultMatrixd(glm);*/
    return ofMatrix4x4(m[0][0], m[1][0], m[2][0], 0, 
                       m[0][1], m[1][1], m[2][1], 0, 
                       m[0][2], m[1][2], m[2][2], 0, 
                       0,       0,       0,       1);
};

void ofxPTAMM::buildMap(){
    tracker->buildMapBegin();
};

/**
 * Switch to the map with ID nMapNum
 * @param  nMapNum Map ID
 * @param bForce This is only used by DeleteMap and ResetAll, and is
 * to ensure that MapViewer is looking at a safe map.
 */
bool ofxPTAMM::switchMap( int nMapNum, bool bForce ){
    
    //same map, do nothing. This should not actually occur
    if(map->MapID() == nMapNum) {
        return true;
    }
    
    if( (nMapNum < 0) ){
        cerr << "Invalid map number: " << nMapNum << ". Not changing." << endl;
        return false;
    }
    
    
    for( size_t ii = 0; ii < vMaps.size(); ii++ ){
        PTAMM::Map * pcMap = vMaps[ ii ];
        if( pcMap->MapID() == nMapNum ) {
            map->mapLockManager.UnRegister( this );
            map = pcMap;
            map->mapLockManager.Register( this );
        }
    }
    
    if(map->MapID() != nMapNum){
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
    
    *bLockMap = map->bEditLocked;
    
    
    //update the map maker thread
    if( !mapMaker->RequestSwitch( map ) ) {
        return false;
    }
    
    while( !mapMaker->SwitchDone() ) {
#ifdef WIN32
        Sleep(1);
#else
        usleep(10);
#endif
    }
    
    //update the map viewer object
    //mpMapViewer->SwitchMap(mpMap, bForce);
    
    if( !tracker->SwitchMap( map ) ) {
        return false;
    }
    
    return true;
}

/**
 * Create a new map and switch all
 * threads and objects to it.
 */
void ofxPTAMM::newMap(){
    *bLockMap = false;
    map->mapLockManager.UnRegister( this );
    map = new PTAMM::Map();
    map->mapLockManager.Register( this );
    vMaps.push_back( map );
    
    //update the map maker thread
    mapMaker->RequestReInit( map );
    while( !mapMaker->ReInitDone() ) {
#ifdef WIN32
        Sleep(1);
#else
        usleep(10);
#endif
    }
    
    //update the map viewer object
    //mapViewer->SwitchMap(map);
    
    tracker->SetNewMap( map );
    
    cout << "New map created (" << map->MapID() << ")" << endl;
}


/**
 * Moves all objects and threads to the first map, and resets it.
 * Then deletes the rest of the maps, placing PTAMM in its
 * original state.
 * This reset ignores the edit lock status on all maps
 */
void ofxPTAMM::resetAll(){
    
    //move all maps to first map.
    if( map != vMaps.front() ){
        if( !switchMap( vMaps.front()->MapID(), true ) ) {
            cerr << "Reset All: Failed to switch to first map" << endl;
        }
    }
    map->bEditLocked = false;
    
    //reset map.
    tracker->Reset();
    
    //lock and delete all remaining maps
    while( vMaps.size() > 1 ){
        deleteMap( vMaps.back()->MapID() );
    }
    
}

/**
 * Delete a specified map.
 * @param nMapNum map to delete
 */
bool ofxPTAMM::deleteMap( int nMapNum ){
    if( vMaps.size() <= 1 ){
        cout << "Cannot delete the only map. Use Reset instead." << endl;
        return false;
    }
    
    
    //if the specified map is the current map, move threads to another map
    if( nMapNum == map->MapID() ){
        int nNewMap = -1;
        
        if( map == vMaps.front() ) {
            nNewMap = vMaps.back()->MapID();
        } else {
            nNewMap = vMaps.front()->MapID();
        }
        
        // move the current map users elsewhere
        if( !switchMap( nNewMap, true ) ) {
            cerr << "Delete Map: Failed to move threads to another map." << endl;
            return false;
        }
    }
    
    // find and delete the map
    for( size_t ii = 0; ii < vMaps.size(); ii++ ){
        PTAMM::Map * pDelMap = vMaps[ ii ];
        if( pDelMap->MapID() == nMapNum ) {
            
            pDelMap->mapLockManager.Register( this );
            pDelMap->mapLockManager.LockMap( this );
            delete pDelMap;
            vMaps.erase( vMaps.begin() + ii );
            
            ///@TODO Possible bug. If another thread (eg serialization) was using this
            /// and waiting for unlock, would become stuck or seg fault.
        }
    }
    
    return true;
}


void ofxPTAMM::saveMap(){
    if( mapSerializer->Init( "SaveMap", "", *map) ) {
        mapSerializer->start();
    }
}

void ofxPTAMM::saveMaps(){
    if( mapSerializer->Init( "SaveMaps", "", *map) ) {
        mapSerializer->start();
    }
}

void ofxPTAMM::loadMap(){
    if( mapSerializer->Init( "LoadMap", "", *map) ) {
        mapSerializer->start();
    }
}
