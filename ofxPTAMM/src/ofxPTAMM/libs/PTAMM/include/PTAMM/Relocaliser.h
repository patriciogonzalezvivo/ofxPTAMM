// -*- c++ -*- 
// Copyright 2009 Isis Innovation Limited
//
// SmallBlurryImage-based relocaliser
// 
// Each KF stores a small, blurred version of itself;
// Just compare a small, blurred version of the input frame to all the KFs,
// choose the closest match, and then estimate a camera rotation by direct image
// minimisation.
//
// This has been modified to search the keyframes of all maps to allow map switching


#ifndef __RELOCALISER_H
#define __RELOCALISER_H
#include <TooN/se2.h>
#include "ATANCamera.h"
#include "SmallBlurryImage.h"

#include "Map.h"

namespace PTAMM {


class Relocaliser
{
public:
  Relocaliser(std::vector<Map*> &maps, ATANCamera &camera);
  bool AttemptRecovery(Map & currentMap, KeyFrame &k);
  SE3<> BestPose();
  
protected:
  void ScoreKFs(Map * pMap, KeyFrame &kCurrentF);

  std::vector<Map*> & mvpMaps;                    // Reference to all of the maps
  Map * mpBestMap;                                // The map where the camera has been found
  bool mbNewRun;                                 // Is this a new search of all maps?
  
  ATANCamera mCamera;
  int mnBest;
  double mdBestScore;
  SE2<> mse2;
  SE3<> mse3Best;

};


}

#endif

