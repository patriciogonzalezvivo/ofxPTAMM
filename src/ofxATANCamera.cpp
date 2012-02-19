/*
 *  ofxATANCamera.cpp
 *  try_PTAM3
 *
 *  Created by Akira Asia on 10/04/09.
 *  Copyright 2010 This is Ampontang. No rights reserved.
 *
 */

#include "ofxATANCamera.h"

void ofxATANCamera::loadParameters(string cfgFile) {
    
	ifstream stream;
	stream.open(ofToDataPath(cfgFile).c_str());
	string params;
	getline(stream, params);
	stream.close();
    
	unsigned int loc = params.find( "=", 0 );
    params.erase(0, loc+1);
	vector<char*> tmp;
	char *strchar = std::strcpy(new char[params.size() + 1], params.c_str());
	char *tp;
	tp = strtok(strchar, " ");
	tmp.push_back(tp);
	while (tp != NULL) {
        tp = strtok( NULL," " );
        tmp.push_back(tp);
	}
	
    TooN::Vector<5> vUpdate;
    
    // For some reason the first value apears as the second and so on
    // I think it´s the [ ]
    //
    unsigned int offset = 0;
    if (atof(tmp[0]) == 0)
        offset = 1;
    
	for (int i = 0; i < 5; i++) {
		vUpdate[i] = atof(tmp[i + offset]);
	}
	
	(*mgvvCameraParams) = (*mgvvCameraParams) + vUpdate;
	RefreshParams();
}

void ofxATANCamera::loadParameters(TooN::Vector<NUMTRACKERCAMPARAMETERS> _params){ 
    (*mgvvCameraParams) = _params; 
};

bool ofxATANCamera::testParameters(){ 
    if (mvFocal[0] == 320) return false; 
    else return true; 
};

void ofxATANCamera::updateParameters(TooN::Vector<NUMTRACKERCAMPARAMETERS> vUpdate){
    (*mgvvCameraParams) = (*mgvvCameraParams) + vUpdate;
    RefreshParams();
    
    cout << "New parameters: " ;
    for (int i = 0; i < 5; i++) {
        cout << (*mgvvCameraParams)[i] << " ";
    }
    cout << endl;
};

const TooN::Vector<5> ofxATANCamera::mvDefaultParams = TooN::makeVector(0.5, 0.75, 0.5, 0.5, 0.1);
