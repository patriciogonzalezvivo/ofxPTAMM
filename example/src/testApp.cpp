#include "testApp.h"

void testApp::setup(){
	ofEnableAlphaBlending();
	ofBackground(255,255,255);
	ofSetVerticalSync(true);
	
	camWidth 		= 640;
	camHeight 		= 480;
	
	ptamm.init(camWidth,camHeight);
	
	grabber.initGrabber(camWidth,camHeight);
    video.allocate(camWidth, camHeight, OF_IMAGE_COLOR);
}

void testApp::update(){
    grabber.update();
    
    if (grabber.isFrameNew()){
        video.setFromPixels(grabber.getPixels(),camWidth,camHeight,OF_IMAGE_COLOR);
        video.mirror(false, true);
        video.update();
        ptamm.update( video.getPixelsRef() );
    }
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
}

void testApp::draw(){
    ofSetColor(255, 255);
    video.draw(0,0);
    
    ofSetColor(255, 255);
	ptamm.draw();
    
    ofPushMatrix();
    if ( ptamm.isMapBuild() ){
        ofTranslate(ptamm.getPosition());
        
        ofVec3f rot = ptamm.getOrientation();
        ofRotateX(rot.x);
        ofRotateY(rot.y);
        ofRotateZ(rot.z);
    }
    
    ofSetColor(0, 0, 200);
    ofBox(0,0, 80);
    ofPopMatrix();
}


void testApp::keyPressed(int key){
	switch (key){
		case ' ':
			ptamm.startBuildMap();
			break;
		case 'r':
			ptamm.resetMap();
			break;
	}
}

void testApp::keyReleased(int key){ 

}

void testApp::mouseMoved(int x, int y ){ 

}

void testApp::mouseDragged(int x, int y, int button){
    
}

void testApp::mousePressed(int x, int y, int button) {
	
}

void testApp::mouseReleased(){

}

void testApp::mouseReleased(int x, int y, int button){
    
}

void testApp::windowResized(int w, int h){
    
}

