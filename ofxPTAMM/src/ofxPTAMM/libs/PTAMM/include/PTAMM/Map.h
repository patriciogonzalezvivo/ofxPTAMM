// -*- c++ -*-
// Copyright 2009 Isis Innovation Limited
//
// This header declares the Map class.
// This is pretty light-weight: All it contains is
// a vector of MapPoints and a vector of KeyFrames.
//
// N.b. since I don't do proper thread safety,
// everything is stored as lists of pointers,
// and map points are not erased if they are bad:
// they are moved to the trash list. That way
// old pointers which other threads are using are not 
// invalidated!

#ifndef __MAP_H
#define __MAP_H
#include <vector>
#include <TooN/se3.h>
#include <cvd/image.h>
#include <deque>

#include "MapLockManager.h"

namespace PTAMM {

struct MapPoint;
class KeyFrame;
class Game;

class Map
{
  public:
    Map();
    ~Map();
    inline bool IsGood() {return bGood;}
    void Reset();
  
    void MoveBadPointsToTrash();
    void EmptyTrash();

    int  QueueSize() { return static_cast<int>(vpKeyFrameQueue.size()); } // How many KFs in the queue waiting to be added?
    
    int MapID() { return mnMapNum; }

  public:
    std::vector<MapPoint*> vpPoints;
    std::vector<MapPoint*> vpPointsTrash;
    std::vector<KeyFrame*> vpKeyFrames;

    // These have been moved from MapMaker, as now have multiple maps
    std::vector<KeyFrame*> vpKeyFrameQueue;  // Queue of keyframes from the tracker waiting to be processed
    std::vector<std::pair<KeyFrame*, MapPoint*> > vFailureQueue; // Queue of failed observations to re-find
    std::deque<MapPoint*> qNewQueue;   // Queue of newly-made map points to re-find in other KeyFrames

    bool bBundleConverged_Full;                      // Has global bundle adjustment converged?
    bool bBundleConverged_Recent;                    // Has local bundle adjustment converged?

  
    bool bGood;                                   // Is the map valid (has content)?
    bool bEditLocked;                                // Is the map locked from being edited?

    //PTAMM Additions
    Game * pGame;                                    // The AR Game for this map

    MapLockManager mapLockManager;                   // All threads must register to this and
                                                     // use when need complete control of a map

    std::string sSaveFile;                           // where the map was loaded from

  private:
    int mnMapNum;                                    // The map number

};


}

#endif

