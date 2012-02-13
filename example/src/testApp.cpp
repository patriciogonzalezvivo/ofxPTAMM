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
    
    // ofBox uses texture coordinates from 0-1, so you can load whatever
	// sized images you want and still use them to texture your box
	// but we have to explicitly normalize our tex coords here
	ofEnableNormalizedTexCoords();
	
	// loads the OF logo from disk
    logo.loadImage("of.png");
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
    
    if ( ptamm.isMapBuild() ){
        cam.begin();
        ofPushMatrix();
		
        ptamm.moveCamera();
    
        logo.bind();
        ofFill();
        ofSetColor(255);
        ofBox(100);
        logo.unbind();
    
        ofNoFill();
        ofSetColor(128);
        ofBox(100 * 1.1f);
		
        ofPopMatrix();
        cam.end();
    }
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

