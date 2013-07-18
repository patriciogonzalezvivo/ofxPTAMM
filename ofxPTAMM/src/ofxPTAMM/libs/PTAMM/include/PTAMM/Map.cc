// Copyright 2009 Isis Innovation Limited
#include "Map.h"
#include "MapPoint.h"
#include "KeyFrame.h"
//#include "Game.h"

namespace PTAMM {

/**
 * Constructor. Calls reset and sets the map ID
 */
Map::Map()
  : pGame( NULL )
{
  static int nMapCounter = 0;
  mnMapNum = nMapCounter++;
  
  Reset();
}

/**
 * Destructor
 */
Map::~Map()
{
  Reset();
}

/**
 * Reset the map
 */
void Map::Reset()
{
  //clear map points
  for(unsigned int i = 0; i < vpPoints.size(); i++) {
    delete vpPoints.at(i);
  }
  vpPoints.clear();

  //clear trash points
  EmptyTrash();

  //clear keyframes
  for(unsigned int i = 0; i < vpKeyFrames.size(); i++) {
    delete vpKeyFrames.at(i);
  }
  vpKeyFrames.clear();

  bGood = false;         //no longer good
  bEditLocked = false;   //make editable
  sSaveFile = "";        //not associated with a file on disk

  //clear queued keyframes.
  for(unsigned int i = 0; i < vpKeyFrameQueue.size(); i++) {
    delete vpKeyFrameQueue.at(i);
  }
  vpKeyFrameQueue.clear();

  //clear the failure queue
  vFailureQueue.clear();

  //clear queued new observations.
  while(!qNewQueue.empty())
  {
//     delete qNewQueue.front();
    qNewQueue.pop_front();
  }

  //remove any associated game
  if( pGame != NULL )  {
    delete pGame;
    pGame = NULL;
  }

  bBundleConverged_Full = true;
  bBundleConverged_Recent = true;

  // mnMapNum is not reset as this is constant for a map.
}

/**
 * Move any points marked as bad to the trash
 */
void Map::MoveBadPointsToTrash()
{
  int nBad = 0;
  for(int i = vpPoints.size()-1; i>=0; i--)
    {
      if(vpPoints[i]->bBad)
	{
	  vpPointsTrash.push_back(vpPoints[i]);
	  vpPoints.erase(vpPoints.begin() + i);
	  nBad++;
	}
    };
}


/**
 * Delete of the points in the trash
 */
void Map::EmptyTrash()
{
  for(unsigned int i=0; i<vpPointsTrash.size(); i++)
    delete vpPointsTrash[i];
  vpPointsTrash.clear();
}


}

