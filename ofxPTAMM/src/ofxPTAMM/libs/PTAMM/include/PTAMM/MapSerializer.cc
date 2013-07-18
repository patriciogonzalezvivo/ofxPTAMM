// -*- c++ -*-
// Copyright 2009 Isis Innovation Limited

/********************************************************************

  A Class to serialize and deserialize maps.

  Author: Robert Castle, 2009, bob@robots.ox.ac.uk

********************************************************************/

#include "MapSerializer.h"
#include "Map.h"
#include "MapPoint.h"
#include "KeyFrame.h"
#include "ATANCamera.h"
#include <gvars3/GStringUtil.h>
#include "LevelHelpers.h"
#include "MD5Wrapper.h"
//#include "Games.h"
#include "Utils.h"

#include <iostream>
#include <iomanip>
#include <cvd/vision.h>
#include <cvd/image_io.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#include "direct.h"
#endif

namespace PTAMM {

using namespace std;
using namespace GVars3;



/**
 * Constructor
 */
MapSerializer::MapSerializer( std::vector<Map*> &maps )
  : mpMap(NULL),
    mvpMaps(maps)
{
}

/**
 * Destructor
 */
MapSerializer::~MapSerializer()
{
}

/**
 * Lock the current map
 * @return success
 */
bool MapSerializer::_LockMap()
{
  return mpMap->mapLockManager.LockMap( this );
}


/**
 * Unlock the current map
 */
void MapSerializer::_UnlockMap()
{
  mpMap->mapLockManager.UnlockMap( this );
}

/**
 * Call this to register the thread with a map and
 * assign pMap to mpMap.
 * @param pMap the map to register with
 */
void MapSerializer::_RegisterWithMap(Map * pMap)
{
  if( pMap != NULL ) {
    mpMap = pMap;
    mpMap->mapLockManager.Register( this );
  }
}

/**
 * Call this to remove the thread from a map and set mpMap to NULL
 */
void MapSerializer::_UnRegisterWithMap()
{
  if( mpMap != NULL )  {
    mpMap->mapLockManager.UnRegister( this );
    mpMap = NULL;
  }
}

/**
 * Initialize the map serializer with the command and parameters required.
 * The current map is also passed in case this needs to be known.
 * @param sCommand The serialization command
 * @param sParams The parameters, such as map number and/or path
 * @param currentMap the active map
 */
bool MapSerializer::Init( std::string sCommand, std::string sParams, Map &currentMap )
{
  if( isRunning() ) {
    cout << "Serialization is currently running. Please try again in a moment." << endl;
    return false;
  }
  
  mbOK = false;

  msCommand = sCommand;
  msParams = sParams;
  mpInitMap = &currentMap;
  
  mbOK = true;

  return true;
}



/**
 * Clear out all of the temp variables used when loading and saving.
 * should be called either before or after each map serialization
 */
void MapSerializer::_CleanUp()
{
  mmMapPointSaveLUT.clear();
  mmKeyFrameSaveLUT.clear();
  mmMapPointLoadLUT.clear();
  mmKeyFrameLoadLUT.clear();

  mmMeasCrossRef.clear();
  mvFailureQueueUIDs.clear();
}


/**
 * Finish off the keyframe crossreferencing now that all
 * keyframes and map points have been loaded.
 * @param hRoot root XML handle
 * @return success
 */
bool MapSerializer::_CrossReferencing(TiXmlHandle &hRoot)
{
  map< KeyFrame*, vector< std::pair< int, Measurement > > >::iterator i;
  vector< std::pair< int, Measurement > >::iterator j;

  //for each keyframe link up the map points to measurments
  for( i = mmMeasCrossRef.begin(); i != mmMeasCrossRef.end(); i++ )
  {
    KeyFrame * k = (*i).first;
    vector< std::pair< int, Measurement > > & vMeas = (*i).second;
    
    for( j = vMeas.begin(); j != vMeas.end(); j++ )
    {
      MapPoint * mp = _LookupMapPoint( (*j).first );
      if( mp == NULL )  {
        cerr << "ERROR: Could not find the matching mappoint for the meas cross ref: " << (*j).first << ". continuing" << endl;
      }
      else  {
        k->mMeasurements[ mp ] = (*j).second;
      }
    }
  }

  vector<std::pair<int, int> >::iterator fq;
  KeyFrame * k = NULL;
  MapPoint * m = NULL;
  
  for( fq = mvFailureQueueUIDs.begin(); fq != mvFailureQueueUIDs.end(); fq++ )
  {
    k = _LookupKeyFrame( (*fq).first );
    m = _LookupMapPoint( (*fq).second );
    if( m != NULL && k != NULL )  {
      mpMap->vFailureQueue.push_back( std::pair<KeyFrame *, MapPoint *>( k, m ) );
    }
  }
  
  return true;
}
  

/**
 * Load a map by loading the map.xml file and the keyframe
 * images in sDirName.
 * @param sDirName map directory
 * @return success
 */
MapSerializer::MapStatus MapSerializer::_LoadMap( std::string sDirName )
{
  TiXmlDocument mXMLDoc;                                 //XML file
  string sMapFileName = sDirName + "/map.xml";

  //load the XML file
  if( !mXMLDoc.LoadFile( sMapFileName ) )  {
    cerr << "Failed to load " << sMapFileName << ". Aborting." << endl;
    return MAP_FAILED;
  }
  
  TiXmlHandle hDoc(&mXMLDoc);
  TiXmlElement* pElem;
  TiXmlHandle hRoot(0);
  

  pElem = hDoc.FirstChildElement().Element();
  // should always have a valid root but handle gracefully if it does not
  if (!pElem)
  {
    cerr << "No root handle in XML file " << sMapFileName << endl;
    return MAP_FAILED;
  }

  string sID( MAP_XML_ID );
  string sVersion( MAP_VERSION );
  string sFileVersion = pElem->Attribute("version");  
  
  if( ( sID.compare( pElem->Value() ) != 0 ) &&
        ( sVersion.compare( sFileVersion ) != 0 ) )
  {
    cerr << "Invalid XML file. Need a version " << sVersion << " " << sID
         << " XML file. Not a version " << sFileVersion << " " << pElem->Value() << " file." << endl;
    return MAP_FAILED;
  }

  // save this for later
  hRoot = TiXmlHandle(pElem);

  ////////////  Get map lock  ////////////
  if( !_LockMap() ) {
    cerr << "Failed to get map lock" << endl;
    return MAP_FAILED;
  }

  mpMap->Reset();     // Wipe them out. All of them!

  // load the keyframes
  bool bOK = _LoadKeyFrames( hRoot, sDirName );
  if( !bOK )  {
    mpMap->Reset();
    _UnlockMap();
    return MAP_FAILED;
  }

  // load map points
  bOK = _LoadMapPoints( hRoot );
  if( !bOK ) {
    mpMap->Reset();
    _UnlockMap();
    return MAP_FAILED;
  }
  

  ////////////  load the uids for the keyframes and map points in the failure queue  ////////////
  ///@TODO   load the mvFailureQueueUIDs

  ///// cross ref
  bOK = _CrossReferencing( hRoot );
  if( !bOK )  {
    mpMap->Reset();
    _UnlockMap();
    return MAP_FAILED;
  }
  

  ////////////  load the game data  ////////////
  //string sGame = hRoot.FirstChild( "Game" ).Element()->Attribute("type");
  //cout << "Game = " << sGame << endl;
  //if( sGame != "None" )
  //{
  //  string sGameDataFile = hRoot.FirstChild( "Game" ).Element()->Attribute("path");
  //  mpMap->pGame = LoadAGame(sGame, sDirName + "/" + sGameDataFile);
  //}

  //all loaded so set map as good.
  mpMap->bGood = true;

  ////////////  relase map lock  ////////////
  _UnlockMap();
  
  return MAP_OK;

}



/**
 * Load a map specified in the sDirName.
 * @param pMap the map to load into 
 * @param sDirName the directory containing the map
 * @return success
 */
MapSerializer::MapStatus MapSerializer::LoadMap( Map * pMap, std::string sDirName )
{
  MapStatus ms = MAP_OK;

  _CleanUp();

  if( pMap == NULL ) {
    cerr << "LoadMap: got a NULL map pointer. abort" << endl;
    return MAP_FAILED;
  }

  _RegisterWithMap( pMap );

  if( sDirName.empty() ) {
    cerr << "sDirName is empty" << endl;
    ostringstream os;
    os << "map" << setfill('0') << setw(6) << mpMap->MapID();
    sDirName = os.str();
  }

  cout << "   Attempting to load map directory " << sDirName << "..." << endl;

  //check if there is a map currenly in mpMap.
  if( ( mpMap->IsGood()  || !mpMap->vpKeyFrames.empty() || !mpMap->vpPoints.empty() ))
  {
    cerr << "Map already exists! Reset the map first." << endl;
    _UnRegisterWithMap();  
    return MAP_EXISTS;
  }

  //does it exist and is it a dir
#ifdef WIN32
  // Get the current working directory:
  char* cwdBuffer = _getcwd( NULL, 0 );
  if( cwdBuffer == NULL )
  {
    perror( "Error getting the current working directory: Error with _getcwd in MapSerializer." );
    _UnRegisterWithMap();
    return MAP_FAILED;    
  }
  
  //attempt to change working directory
  int chdirReturn = _chdir( sDirName.c_str() ); //returns 0 if change was successful, -1 if it dosen't exist
  if( chdirReturn == 0 )
  {
    //restore previous working directory
    _chdir( cwdBuffer );
  }
  else
  {
#else
  struct stat st;
  if( stat( sDirName.c_str(),&st ) != 0 || !S_ISDIR(st.st_mode) )  {
#endif
    _UnRegisterWithMap();
    return MAP_FAILED;
  }

  // load the map file and hence the rest of the map
  ms = _LoadMap( sDirName );
  if( MAP_OK != ms )  {
    _UnRegisterWithMap();
    return ms;
  }

  _UnRegisterWithMap();
  _CleanUp();
  return MAP_OK;
}


/**
 * Load a keyframe from the map.xml file and its associated png image.
 * @param phKF the XML node
 * @param sPath the diectory path containing the keyframe image file
 * @param bQueueFrame is it in the queue list
 * @return success
 */
bool MapSerializer::_LoadAKeyFrame( TiXmlHandle &phKF, const std::string & sPath, bool bQueueFrame )
{
  int uid = -1;
  TiXmlElement* kfe = phKF.ToElement();
  
  kfe->QueryIntAttribute("id", &uid);
  if( uid == -1 ) {
    cerr << " keyframe has no ID. aborting" << endl;
    return false;
  }
  

  TiXmlElement * pElem = phKF.FirstChild("Image").ToElement();
  if( pElem == NULL ) {
    return false;
  }
  
  //load the image
  ostringstream os;
  os << sPath << "/" << pElem->Attribute("file");
  string sImgFileName = os.str();
  CVD::Image<CVD::byte> im;
  
  try {
    CVD::img_load( im, sImgFileName );
  }
  catch(CVD::Exceptions::All err) {
    cerr << "Failed to load image " << sImgFileName << ": " << err.what << endl;
    return false;
  }

  //////////////// Read MD5 Hash
  string sMD5, sMD5new;
  MD5Wrapper md5w;

  sMD5 = pElem->Attribute("md5");

  //verify the image
  md5w.getHashFromData( im.data(), im.totalsize(), sMD5new );

  if( sMD5.compare( sMD5new ) != 0 )
  {
    cerr << "ERROR: the MD5 hash of " << sImgFileName << " does not match the one in xml file" << endl;
    cerr << "ERROR: " << sMD5 << " != " << sMD5new << endl;
    return false;
  }


  /////////////// read camera info and set up
  pElem = phKF.FirstChild("Camera").ToElement();
  if( pElem == NULL ) {
    return false;
  }

  string sCamName = pElem->Attribute("name");
  string tmp;

  CVD::ImageRef irCamSize;
  tmp = pElem->Attribute("size");
  sscanf(tmp.c_str(), "%d %d", &irCamSize.x, &irCamSize.y);

  ///@TODO this is not very robust to changes in the number of parameters
  tmp = pElem->Attribute("params");
  Vector<NUMTRACKERCAMPARAMETERS> vCamParams;
  sscanf(tmp.c_str(), "%lf %lf %lf %lf %lf", &vCamParams[0], &vCamParams[1], &vCamParams[2], &vCamParams[3], &vCamParams[4]);

  ATANCamera Camera(sCamName, irCamSize, vCamParams);

  KeyFrame * kf = new KeyFrame(Camera);

  ////////////// create keyframe
  kf->MakeKeyFrame_Lite( im );
  if(!bQueueFrame)  {
    //if the frame is in the queue it has not had the rest of the data made.
    kf->MakeKeyFrame_Rest();
  }

  ////////////// Populate with stored data
  Vector<6> v6KFPose;
  tmp = kfe->Attribute("pose");
  sscanf(tmp.c_str(), "%lf %lf %lf %lf %lf %lf", &v6KFPose[0], &v6KFPose[1], &v6KFPose[2],
                                                 &v6KFPose[3], &v6KFPose[4], &v6KFPose[5]);

  kf->se3CfromW = SE3<>::exp( v6KFPose );

  int nTmp = 0;
  kfe->QueryIntAttribute("fixed", &nTmp);
  kf->bFixed = ( nTmp != 0 ? true : false );


  pElem = phKF.FirstChild("SceneDepth").ToElement();
  pElem->QueryDoubleAttribute("mean", &kf->dSceneDepthMean);
  pElem->QueryDoubleAttribute("sigma", &kf->dSceneDepthSigma);


  ///////////////////// load in cross ref data to temp vars

  //measurements
  {
    TiXmlHandle hMeas = phKF.FirstChild("Measurements");

    int nTmp = -1;
    string sTmp;
    vector< std::pair< int, Measurement > > vMapPointMeas;
    
    int nSize = -1;
    hMeas.ToElement()->QueryIntAttribute("size", &nSize);

    for(TiXmlElement* pNode = hMeas.FirstChild().Element();
        pNode != NULL;
        pNode = pNode->NextSiblingElement() )
    {
      Measurement meas;
      int nId = -1;
      
      pNode->QueryIntAttribute("id", &nId);
      pNode->QueryIntAttribute("nLevel", &meas.nLevel);
      pNode->QueryIntAttribute("bSubPix", &nTmp);
      meas.bSubPix = ( nTmp != 0 ? true : false );
      
      sTmp = pNode->Attribute("v2RootPos");
      sscanf( sTmp.c_str(), "%lf %lf", &meas.v2RootPos[0], &meas.v2RootPos[1] );
      
      sTmp = pNode->Attribute("v2ImplanePos");
      sscanf( sTmp.c_str(), "%lf %lf", &meas.v2ImplanePos[0], &meas.v2ImplanePos[1] );

      sTmp = pNode->Attribute("m2CamDerivs");
      sscanf( sTmp.c_str(), "%lf %lf %lf %lf", &meas.m2CamDerivs(0,0), &meas.m2CamDerivs(0,1),
                                               &meas.m2CamDerivs(1,0), &meas.m2CamDerivs(1,1));

      pNode->QueryIntAttribute("source", (int*)&meas.Source );

      vMapPointMeas.push_back( std::pair< int, Measurement >( nId, meas ) );
    }
      
    mmMeasCrossRef[ kf ] = vMapPointMeas;
  }

  /////// now add to map vector. but still need a cross ref
  mmKeyFrameLoadLUT[ uid ] = kf;

  if( bQueueFrame ) {
    mpMap->vpKeyFrameQueue.push_back( kf );
  }
  else  {
    mpMap->vpKeyFrames.push_back( kf );
  }
  
  return true;
}


/**
 * Recursively load each keyframe
 * @param hRoot XML node
 * @param sPath path where keyframe dir is located
 * @return success
 */
bool MapSerializer::_LoadKeyFrames( TiXmlHandle &hRoot, const std::string & sPath )
{
  {
    int nSize = -1;
    TiXmlHandle pNode = hRoot.FirstChild( "KeyFrames" );
    pNode.ToElement()->QueryIntAttribute("size", &nSize);

    for(TiXmlElement* pElem = pNode.FirstChild().Element();
        pElem != NULL;
        pElem = pElem->NextSiblingElement() )
    {
      ///@TODO should do empty entry checking
      TiXmlHandle hkf( pElem );
      if( !_LoadAKeyFrame( hkf, sPath ) ) {
        cerr << "Failed to Load keyframe " <<  pElem->Attribute("id")  << ". Abort." << endl;
        return false;
      }
    }

    if( (int)mpMap->vpKeyFrames.size() != nSize ) {
      cerr << "Loaded the wrong number of keyframes. " << mpMap->vpKeyFrames.size()
          << " instead of " << nSize << ". Aborting" << endl;
      return false;
    }
  }

  
  ////////////  load the keyframe queue  ////////////
  {
    int nSize = -1;
    TiXmlHandle pNode = hRoot.FirstChild( "KeyFrameQueue" );
    pNode.ToElement()->QueryIntAttribute("size", &nSize);

    for(TiXmlElement* pElem = pNode.FirstChild().Element();
        pElem != NULL;
        pElem = pElem->NextSiblingElement() )
    {
      TiXmlHandle hkf( pElem );
      if( !_LoadAKeyFrame( hkf, sPath, true ) ) {
        cerr << "Failed to Load queue keyframe " <<  pElem->Attribute("id")  << ". Abort." << endl;
        return false;
      }
    }

    if( (int)mpMap->vpKeyFrameQueue.size() != nSize ) {
      cerr << "Loaded the wrong number of queue keyframes. " << mpMap->vpKeyFrameQueue.size()
          << " instead of " << nSize << ". Aborting" << endl;
      return false;
    }
  }
  return true;
}


/**
 * Load a map point
 * @param hMP XML node
 * @param bQueuePoint is it a main point or a queue point
 * @return success
 */
bool MapSerializer::_LoadAMapPoint( TiXmlHandle &hMP, bool bQueuePoint )
{
  int uid = -1;
  hMP.ToElement()->QueryIntAttribute("id", &uid);
  if( uid == -1 ) {
    cerr << " keyframe has no ID. aborting" << endl;
    return false;
  }

  //////////////////// fill in map point
  
  MapPoint * mp = new MapPoint();

  string sTmp = hMP.ToElement()->Attribute( "position" );
  sscanf( sTmp.c_str(), "%lf %lf %lf", &mp->v3WorldPos[0], &mp->v3WorldPos[1], &mp->v3WorldPos[2]);

  hMP.ToElement()->QueryIntAttribute( "outlierCount", &mp->nMEstimatorOutlierCount );
  hMP.ToElement()->QueryIntAttribute( "inlierCount", &mp->nMEstimatorInlierCount );

  TiXmlElement * pElem = hMP.FirstChild("SourceKF").ToElement();
  if( pElem == NULL ) {
    return false;
  }

  int nPatchSource = -1;
  pElem->QueryIntAttribute( "id", &nPatchSource );
  mp->pPatchSourceKF = _LookupKeyFrame( nPatchSource );  //this can be done only if keyframes loaded first

  pElem->QueryIntAttribute( "level", &mp->nSourceLevel );
  pElem->QueryIntAttribute( "x", &mp->irCenter.x );
  pElem->QueryIntAttribute( "y", &mp->irCenter.y );

  string sCenter = pElem->Attribute( "Center_NC" );
  sscanf( sCenter.c_str(), "%lf %lf %lf", &mp->v3Center_NC[0], &mp->v3Center_NC[1], &mp->v3Center_NC[2]);

  string sOneDown = pElem->Attribute( "OneDownFromCenter_NC" );
  sscanf( sOneDown.c_str(), "%lf %lf %lf", &mp->v3OneDownFromCenter_NC[0], &mp->v3OneDownFromCenter_NC[1], &mp->v3OneDownFromCenter_NC[2]);

  string sOneRight = pElem->Attribute( "OneRightFromCenter_NC" );
  sscanf( sOneRight.c_str(), "%lf %lf %lf", &mp->v3OneRightFromCenter_NC[0], &mp->v3OneRightFromCenter_NC[1], &mp->v3OneRightFromCenter_NC[2]);

  string sNormal = pElem->Attribute( "Normal_NC" );
  sscanf( sNormal.c_str(), "%lf %lf %lf", &mp->v3Normal_NC[0], &mp->v3Normal_NC[1], &mp->v3Normal_NC[2]);

  string sPixDown = pElem->Attribute( "PixelDown_W" );
  sscanf( sPixDown.c_str(), "%lf %lf %lf", &mp->v3PixelDown_W[0], &mp->v3PixelDown_W[1], &mp->v3PixelDown_W[2]);

  string sPixRight = pElem->Attribute( "PixelRight_W" );
  sscanf( sPixRight.c_str(), "%lf %lf %lf", &mp->v3PixelRight_W[0], &mp->v3PixelRight_W[1], &mp->v3PixelRight_W[2]);

  
  //////////// cross ref data - as all keyframes are loaded we can cross ref with them.

  //MapMakerData
  {
    std::vector<string> vsUids;
    std::vector<int> vnUids;
    KeyFrame * k = NULL;
    std::vector<int>::iterator it;
    int nSize = -1;
    string sTmp;

    TiXmlElement * pElem = hMP.FirstChild("MapMakerData").FirstChild("MeasurementKFs").ToElement();
    if( pElem == NULL ) {
      return false;
    }

    pElem->QueryIntAttribute( "size", &nSize );
    const char * c = pElem->GetText();
    if(c != NULL )  {
      sTmp = c;
    }
    else  {
      sTmp = "";
    }

    vsUids = ChopAndUnquoteString(sTmp);
    for(size_t i = 0; i < vsUids.size(); i++)
    {
      int *pN = ParseAndAllocate<int>(vsUids[i]);
      if( pN )
      {
        vnUids.push_back( *pN );
        delete pN;
      }
    }
    
    

    if( (int)vnUids.size() != nSize ) {
      cerr << "Warning: MapMakerData:MeasurementKFs size mismatch for map point "
          << uid << " : " << vnUids.size() << " != " << nSize << endl;
    }

    mp->pMMData = new MapMakerData();

    for( it =  vnUids.begin(); it != vnUids.end(); it++ )
    {
      k = _LookupKeyFrame( (*it) );
      if( k != NULL ) {
        mp->pMMData->sMeasurementKFs.insert( k );
      }
      else  {
        cerr << "Warning: keyframe " << (*it) << " not found for MapMakerData:MeasurementKFs cross ref" << endl;
      }
    }

    vnUids.clear();
    vsUids.clear();

    
    pElem = hMP.FirstChild("MapMakerData").FirstChild("sNeverRetryKFs").ToElement();
    if( pElem == NULL ) {
      return false;
    }


    pElem->QueryIntAttribute( "size", &nSize );
    
    c = pElem->GetText();
    if(c != NULL )  {
      sTmp = c;
    }
    else  {
      sTmp = "";
    }

    vsUids = ChopAndUnquoteString(sTmp);
    for(size_t i = 0; i < vsUids.size(); i++)
    {
      int *pN = ParseAndAllocate<int>(vsUids[i]);
      if( pN )
      {
        vnUids.push_back( *pN );
        delete pN;
      }
    }

    if( (int)vnUids.size() != nSize ) {
      cerr << "Warning: MapMakerData:sNeverRetryKFs size mismatch for map point " << uid << endl;
    }
    
    for( it =  vnUids.begin(); it != vnUids.end(); it++ )
    {
      k = _LookupKeyFrame( (*it) );
      if( k != NULL ) {
        mp->pMMData->sNeverRetryKFs.insert( k );
      }
      else  {
        cerr << "Warning: keyframe " << (*it) << " not found for MapMakerData:sNeverRetryKFs cross ref" << endl;
      }
    }

    vnUids.clear();
    vsUids.clear();

    mmMapPointLoadLUT[ uid ] = mp;

    if( bQueuePoint ) {
      mpMap->qNewQueue.push_back( mp );
    }
    else  {
//       mp->RefreshPixelVectors();
      mpMap->vpPoints.push_back( mp );
    }
  }

  return true;

}


/**
 * Load a list of mappoints recursivly
 * @param hRoot XML node
 * @return success
 */
bool MapSerializer::_LoadMapPoints( TiXmlHandle &hRoot )
{
  {
    int nSize = -1;
    TiXmlHandle pNode = hRoot.FirstChild( "MapPoints" );
    pNode.ToElement()->QueryIntAttribute("size", &nSize);

    for(TiXmlElement* pElem = pNode.FirstChild().Element();
        pElem != NULL;
        pElem = pElem->NextSiblingElement() )
    {
      TiXmlHandle hmp( pElem );
      if( !_LoadAMapPoint( hmp ) ) {
        cerr << "Failed to Load point " << pElem->Attribute("id") << ". Abort." << endl;
        return false;
      }
    }

    if( (int)mpMap->vpPoints.size() != nSize ) {
      cerr << "Loaded the wrong number of mappoints. " << mpMap->vpPoints.size()
          << " instead of " << nSize << ". Aborting" << endl;
      return false;
    }
  }
  
  ////////////  load the new pending map points  ////////////
  {
    int nSize = -1;
    TiXmlHandle pNode = hRoot.FirstChild( "NewMapPoints" );
    pNode.ToElement()->QueryIntAttribute("size", &nSize);

    for(TiXmlElement* pElem = pNode.FirstChild().Element();
        pElem != NULL;
        pElem = pElem->NextSiblingElement() )
    {
      TiXmlHandle hmp( pElem );
      if( !_LoadAMapPoint( hmp, true ) ) {
        cerr << "Failed to Load queue point " << pElem->Attribute("id") << ". Abort." << endl;
        return false;
      }
    }

    if( (int)mpMap->qNewQueue.size() != nSize ) {
      cerr << "Loaded the wrong number of new mappoints. " << mpMap->qNewQueue.size()
          << " instead of " << nSize << ". Aborting" << endl;
      return false;
    }
  }
  
  return true;
}



/**
 * save the map class
 * @param sPath the root folder path for the map
 * @return status
 */
MapSerializer::MapStatus MapSerializer::_SaveMap( std::string sPath )
{
  ////////////  Get map lock  ////////////
  if( !_LockMap() ) {
    cerr << "Failed to get map lock" << endl;
    return MAP_FAILED;
  }
  
  TiXmlDocument xmlDoc;     //XML file

  string sMapFileName = sPath + "/map.xml";

  TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
  xmlDoc.LinkEndChild( decl );

  TiXmlElement * rootNode = new TiXmlElement(MAP_XML_ID);
  xmlDoc.LinkEndChild( rootNode );
  rootNode->SetAttribute("version", MAP_VERSION);
  
  ////////////  save the keyframes and map points  ////////////
  bool bOK = false;
  _CreateSaveLUTs();                      // create lookup tables for the mappoints and keyframes
  
  bOK = _SaveKeyFrames( sPath, rootNode );   // recursively save each keyframe
  if( !bOK )  {
    _UnlockMap();
    return MAP_FAILED;
  }
  bOK = _SaveMapPoints( rootNode );   // recursively save each map point
  if( !bOK )  {
    _UnlockMap();
    return MAP_FAILED;
  }

  ////////////  save the uids for the keyframes and map points in the failure queue  ////////////
  {
    TiXmlElement * failElem = new TiXmlElement( "FailureQueue" );
    rootNode->LinkEndChild( failElem );
    failElem->SetAttribute( "size", mpMap->vFailureQueue.size() );

    int k = -1, m = -1;
    
    vector<std::pair<KeyFrame*, MapPoint*> >::iterator fq;
    for( fq = mpMap->vFailureQueue.begin(); fq != mpMap->vFailureQueue.end(); fq++ )
    {
      k = _LookupKeyFrame( (*fq).first );
      m = _LookupMapPoint( (*fq).second );
      if( m != -1 && k != -1 )  {
        TiXmlElement * kmElem = new TiXmlElement( "Failure" );
        failElem->LinkEndChild( kmElem );
        kmElem->SetAttribute( "kf", k );
        kmElem->SetAttribute( "mp", m );
      }
    }
  }  


 
  ////////////  save the game data  ////////////
  TiXmlElement * game = new TiXmlElement( "Game" );
  rootNode->LinkEndChild( game );

  //if( mpMap->pGame )
  //{
  //  game->SetAttribute("type", mpMap->pGame->Name() );
  //  string sFile = mpMap->pGame->Save( sPath );

  //  if( !sFile.empty() ) {
  //    game->SetAttribute("path", sFile );
  //  }
  //}
  //else  {
  //  game->SetAttribute("type", "None");
  //}

  ////////////  relase map lock  ////////////
  _UnlockMap();

  xmlDoc.SaveFile(sMapFileName);
  return MAP_OK;
}

/**
 * create a look up table for all the pointers to keyframes and map points
 */
void MapSerializer::_CreateSaveLUTs()
{
  mmKeyFrameSaveLUT.clear();
  mmMapPointSaveLUT.clear();
  
  for( size_t i = 0; i < mpMap->vpKeyFrames.size(); i++ )  {
    mmKeyFrameSaveLUT[ mpMap->vpKeyFrames[i] ] = i;
  }
  
  int nSize = mpMap->vpKeyFrames.size();
  
  for( size_t i = 0; i < mpMap->vpKeyFrameQueue.size(); i++ )  {
    mmKeyFrameSaveLUT[ mpMap->vpKeyFrames[i] ] = nSize + i;
  }

  for( size_t i = 0; i < mpMap->vpPoints.size(); i++ )  {
      mmMapPointSaveLUT[ mpMap->vpPoints[i] ] = i;
  }
  
  nSize = mpMap->vpPoints.size();
  
  for( size_t i = 0; i < mpMap->qNewQueue.size(); i++ )  {
      mmMapPointSaveLUT[ mpMap->qNewQueue[i] ] = nSize + i;
  }
}

/**
 * Find a keyframe in the save lookup table and return its uid.
 * -1 if it does not exist
 * @param k the keyframe to find
 * @return the UID
 */
int MapSerializer::_LookupKeyFrame( KeyFrame * k )
{
  map< const KeyFrame *, int >::iterator i;
  i = mmKeyFrameSaveLUT.find( k );
  if( i != mmKeyFrameSaveLUT.end() ) {
    return (*i).second;
  }
  else {
    return -1;
  }
}

/**
 * find a map point in the save lookup table and return its uid. -1 if it does not exist
 * @param m the map point to find
 * @return the UID
 */
int MapSerializer::_LookupMapPoint( MapPoint * m )
{
  map< const MapPoint *, int >::iterator i;
  i = mmMapPointSaveLUT.find( m );
  if( i != mmMapPointSaveLUT.end() ) {
    return (*i).second;
  }
  else {
    return -1;
  }
}


/**
 * Find a keyframe uid in the loading lookup table and return pointer to it.
 * Null if it does not exist
 * @param uid the keyframe to find
 * @return pointer
 */
KeyFrame * MapSerializer::_LookupKeyFrame( int uid )
{
  map< int, KeyFrame * >::iterator i;
  i = mmKeyFrameLoadLUT.find( uid );
  if( i != mmKeyFrameLoadLUT.end() ) {
    return (*i).second;
  }
  else {
    return NULL;
  }
}

/**
 * Find a map point uid in the loading lookup table and return pointer to it.
 * Null if it does not exist
 * @param uid the map point to find
 * @return pointer
 */
MapPoint * MapSerializer::_LookupMapPoint( int uid )
{
  map< int, MapPoint * >::iterator i;
  i = mmMapPointLoadLUT.find( uid );
  if( i != mmMapPointLoadLUT.end() ) {
    return (*i).second;
  }
  else {
    return NULL;
  }
}


/**
 * Save an idividual keyframe out as a file and an associated png file
 * @param kf the keyframe to save
 * @param sPath the path to save to
 * @param keyFramesNode XML node
 * @return success or failure
 */
bool MapSerializer::_SaveAKeyFrame( KeyFrame * kf, const std::string & sPath, TiXmlElement * keyFramesNode )
{
  int uid = _LookupKeyFrame( kf );
  if(uid == -1 )  {
    return false;
  }
  
  TiXmlElement* kfe = new TiXmlElement("KeyFrame");
  keyFramesNode->LinkEndChild( kfe );

  //save keyframe attributes
  {
    ostringstream os;
    string s;

    kfe->SetAttribute( "id", uid );
    
    os << kf->se3CfromW.ln();
    s = os.str();
    PruneWhiteSpace( s );
    kfe->SetAttribute("pose", s );

    kfe->SetAttribute("fixed", kf->bFixed );
  }
  
  //save image
  {
    //create base filename and path for within the file structure.
    ostringstream os;
    os << "KeyFrames/" << setfill('0') << setw(6) << uid << ".png";
    string sFileBaseName = os.str();
    
    //save the image out as a png
    os.str("");
    os << sPath << "/" << sFileBaseName;
    try {
      img_save(kf->aLevels[0].im, os.str());
    }
    catch(CVD::Exceptions::All err) {
      cerr << " Failed to save image " <<  os.str() << ": " << err.what << endl;
      return false;
    }

    TiXmlElement* imageElem = new TiXmlElement("Image");
    kfe->LinkEndChild( imageElem );
    imageElem->SetAttribute("file", sFileBaseName );

    //save a MD5 hash of the image data to ensure the same data is loaded
    MD5Wrapper md5w;
    string sMD5;
    md5w.getHashFromData( kf->aLevels[0].im.data(), kf->aLevels[0].im.totalsize(), sMD5 );

    imageElem->SetAttribute("md5", sMD5 );
  }
  
  //camera model
  {
    ostringstream os;
    TiXmlElement* camElem = new TiXmlElement("Camera");
    kfe->LinkEndChild( camElem );
    
    camElem->SetAttribute("name", kf->Camera.Name() );
    os << kf->Camera.ImageSize()[0] << " " << kf->Camera.ImageSize()[1];
    camElem->SetAttribute("size", os.str() );

    os.str("");
    os << kf->Camera.GetParams()[0];
    for(int i = 1; i < NUMTRACKERCAMPARAMETERS; i++)  {
      os << " " << kf->Camera.GetParams()[i] ;
    }
    camElem->SetAttribute("params", os.str() );
  }

  ////save the rest of the keyframe info.
  {
    TiXmlElement* depthElem = new TiXmlElement("SceneDepth");
    kfe->LinkEndChild( depthElem );
      
    depthElem->SetDoubleAttribute("mean", kf->dSceneDepthMean );
    depthElem->SetDoubleAttribute("sigma", kf->dSceneDepthSigma );
  
  }
  
  //measurements_SaveKeyFrames
  {
    ostringstream os;
    string s;
    TiXmlElement* measurementsElem = new TiXmlElement("Measurements");
    kfe->LinkEndChild( measurementsElem );
      
    //find out how many are in the points list (not in the trash), so know how many will be saved
    int size = 0;
    map<MapPoint*, Measurement>::iterator mit;
    for(mit = kf->mMeasurements.begin(); mit != kf->mMeasurements.end(); mit++)
    {
      if( _LookupMapPoint( (*mit).first ) != -1 ) {
        size++;
      }
    }
    measurementsElem->SetAttribute( "size", size );

    int m = -1;
    for(mit = kf->mMeasurements.begin(); mit != kf->mMeasurements.end(); mit++)
    {
      MapPoint * mp = (*mit).first;
      m =  _LookupMapPoint( mp );
      if( m != -1 )
      {
        TiXmlElement* measElem = new TiXmlElement("Measurement");
        measurementsElem->LinkEndChild( measElem);
        ostringstream os;
        measElem->SetAttribute("id", m);
        measElem->SetAttribute("nLevel", (*mit).second.nLevel);
        measElem->SetAttribute("bSubPix", (*mit).second.bSubPix);
        
        os << (*mit).second.v2RootPos[0] << " " << (*mit).second.v2RootPos[1];
        measElem->SetAttribute("v2RootPos", os.str() );
        
        os.str("");
        os << (*mit).second.v2ImplanePos[0] << " " << (*mit).second.v2ImplanePos[1];
        measElem->SetAttribute("v2ImplanePos",os.str());
        
        os.str("");
        os << (*mit).second.m2CamDerivs[0] << " " << (*mit).second.m2CamDerivs[1];
        s = os.str();
        PruneWhiteSpace( s );
        measElem->SetAttribute("m2CamDerivs", s);

        measElem->SetAttribute("source", (*mit).second.Source);
      }
    }
  }

  return true;
}


/**
 * Save all of the keyframes out to the XML file and as image files.
 * @param sPath the map directory
 * @param rootNode XML node
 * @return success
 */
bool MapSerializer::_SaveKeyFrames( const std::string &sPath, TiXmlElement * rootNode )
{
  string sKeyFramePath = sPath + "/KeyFrames";

  //create the dir if not there
   struct stat st;
  if( stat( sKeyFramePath.c_str(),&st ) != 0  )
  {
#ifdef WIN32
    int err = _mkdir( sKeyFramePath.c_str());
#else
    int err = mkdir( sKeyFramePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif
    if( err != 0) {
      cerr << "Failed to make dir " << sKeyFramePath << endl;
      return false;
    }
  }
  
  TiXmlElement * keyFramesNode = new TiXmlElement( "KeyFrames" );
  rootNode->LinkEndChild( keyFramesNode );
  keyFramesNode->SetAttribute( "size", mpMap->vpKeyFrames.size() );

  vector<KeyFrame *>::iterator kf;
  int k = -1;

  for( kf = mpMap->vpKeyFrames.begin(); kf != mpMap->vpKeyFrames.end(); kf++ )
  {
    if( _SaveAKeyFrame ( (*kf), sPath, keyFramesNode) )  {
      k = _LookupKeyFrame( (*kf) );
      if( k == -1 ) {
        return false;
      }
    }
    else  {
      return false;
    }
  }
  
  ////////////  save the keyframe queue  ////////////
  TiXmlElement * keyFrameQNode = new TiXmlElement( "KeyFrameQueue" );
  rootNode->LinkEndChild( keyFrameQNode );
  keyFrameQNode->SetAttribute( "size", mpMap->vpKeyFrameQueue.size() );

  for( kf = mpMap->vpKeyFrameQueue.begin(); kf != mpMap->vpKeyFrameQueue.end(); kf++ )
  {
    if( !_SaveAKeyFrame ( (*kf), sPath, keyFrameQNode) )  {
      k = _LookupKeyFrame( (*kf) );
      if( k == -1 ) {
        return false;
      }
    }
    else  {
      return false;
    }
  }


  return true;
}


/**
 * save a map point 
 * @param mp the map point to save
 * @param mapPointsNode  XML node
 * @return success
 */
bool MapSerializer::_SaveAMapPoint( MapPoint * mp, TiXmlElement * mapPointsNode )
{
  int uid = _LookupMapPoint( mp );
  if(uid == -1 )  {
    cerr << " map point not in the LUT" << endl;
    return false;
  }

  
  TiXmlElement* mpe = new TiXmlElement("MapPoint");
  mapPointsNode->LinkEndChild( mpe );
  
  mpe->SetAttribute( "id", uid );

  string s;
  ostringstream os;

  os << mp->v3WorldPos;
  s = os.str();
  PruneWhiteSpace( s );
  mpe->SetAttribute("position", s );

  mpe->SetAttribute( "outlierCount", mp->nMEstimatorOutlierCount );
  mpe->SetAttribute( "inlierCount", mp->nMEstimatorInlierCount );


  //source kf
  {
    TiXmlElement* srcElem = new TiXmlElement("SourceKF");
    mpe->LinkEndChild( srcElem );

    int k = -1;
    if( mp->pPatchSourceKF != NULL )  {
      k = _LookupKeyFrame( mp->pPatchSourceKF );
    }
    if( k != -1 ) {
      srcElem->SetAttribute("id", k);
    }
    else  {
      cerr << uid << " map point needs to have a source kf!" << endl;
      return false;
    }

    srcElem->SetAttribute("level", mp->nSourceLevel );
    srcElem->SetAttribute("x", mp->irCenter.x );
    srcElem->SetAttribute("y", mp->irCenter.y );
  
    ostringstream os2;
    string s2;
    
    os2.str("");
    os2 << mp->v3Center_NC;
    s2 = os2.str();
    PruneWhiteSpace( s2 );
    srcElem->SetAttribute("Center_NC", s2 );

    os2.str("");
    os2 << mp->v3OneDownFromCenter_NC;
    s2 = os2.str();
    PruneWhiteSpace( s2 );
    srcElem->SetAttribute("OneDownFromCenter_NC", s2 );

    os2.str("");
    os2 << mp->v3OneRightFromCenter_NC;
    s2 = os2.str();
    PruneWhiteSpace( s2 );
    srcElem->SetAttribute("OneRightFromCenter_NC", s2 );

    os2.str("");
    os2 << mp->v3Normal_NC;
    s2 = os2.str();
    PruneWhiteSpace( s2 );
    srcElem->SetAttribute("Normal_NC", s2 );

    os2.str("");
    os2 << mp->v3PixelDown_W;
    s2 = os2.str();
    PruneWhiteSpace( s2 );
    srcElem->SetAttribute("PixelDown_W", s2 );

    os2.str("");
    os2 << mp->v3PixelRight_W;
    s2 = os2.str();
    PruneWhiteSpace( s2 );
    srcElem->SetAttribute("PixelRight_W", s2 );
  }

    
  ///@TODO are these needed?
//   nMEstimatorOutlierCount);
//   nMEstimatorInlierCount);
//   dCreationTime);

  

  //MapMakerData
  {
    TiXmlElement* mmDataElem = new TiXmlElement("MapMakerData");
    mpe->LinkEndChild( mmDataElem );
    
    vector<int> uids;
    int k = -1;
    std::set<KeyFrame*>::iterator sit;
  
    for( sit =  mp->pMMData->sMeasurementKFs.begin();
        sit != mp->pMMData->sMeasurementKFs.end(); sit++)
    {
      k = _LookupKeyFrame( (*sit) );
      if( k != -1 ) {
        uids.push_back( k );
      }
    }

    TiXmlElement* mkfsElem = new TiXmlElement("MeasurementKFs");
    mmDataElem->LinkEndChild( mkfsElem );

    mkfsElem->SetAttribute("size", uids.size() );
    
    ostringstream os;
    os << uids;
    
    mkfsElem->LinkEndChild( new TiXmlText(os.str() ) );

    uids.clear();

    for( sit =  mp->pMMData->sNeverRetryKFs.begin();
        sit != mp->pMMData->sNeverRetryKFs.end(); sit++)
    {
      k = _LookupKeyFrame( (*sit) );
      if( k != -1 ) {
        uids.push_back( k );
      }
    }
    

    TiXmlElement* nrkfsElem = new TiXmlElement("sNeverRetryKFs");
    mmDataElem->LinkEndChild( nrkfsElem );

    nrkfsElem->SetAttribute("size", uids.size() );
    
    os.str("");
    os << uids;
    
    nrkfsElem->LinkEndChild( new TiXmlText(os.str() ) );
    
  }
  
  return true;
}

/**
 * save all the mappoints in a map.
 * @param rootNode The root node to write to in the XML file
 * @return success.
 */
bool MapSerializer::_SaveMapPoints( TiXmlElement * rootNode )
{
  vector<MapPoint *>::iterator mp;
  vector<int> uids;
  int m = -1;

  TiXmlElement * mapPointsNode = new TiXmlElement( "MapPoints" );
  rootNode->LinkEndChild( mapPointsNode );
  mapPointsNode->SetAttribute( "size", mpMap->vpPoints.size() );


  //all points are saved, bad or not. don't save trash though
  for( mp = mpMap->vpPoints.begin(); mp != mpMap->vpPoints.end(); mp++)
  {
    if( _SaveAMapPoint( (*mp), mapPointsNode) )  {
      m = _LookupMapPoint( (*mp) );
      if( m == -1 ) {
        cerr << " could not find map point " << endl;
        return false;
      }
    }
    else  {
      cerr << " could not save map point " << endl;
      return false;
    }
  }

  ////////////  save the new pending map points  ////////////
  TiXmlElement * mapPointsNewNode = new TiXmlElement( "NewMapPoints" );
  rootNode->LinkEndChild( mapPointsNewNode );
  mapPointsNewNode->SetAttribute( "size", mpMap->qNewQueue.size() );
  
  std::deque<MapPoint*>::iterator nq;

  for( nq = mpMap->qNewQueue.begin(); nq != mpMap->qNewQueue.end(); nq++ )
  {
    if( _SaveAMapPoint( (*nq), mapPointsNewNode) )  {
      m = _LookupMapPoint( (*nq) );
      if( m == -1 ) {
        cerr << " could not find map point " << endl;
        return false;
      }
    }
    else  {
      cerr << " could not save map point " << endl;
      return false;
    }
  }

  return true;
}



/**
 * Save the map out.
 * if no sFileName is provided then the map number will be used.
 * otherwise map will be saved to path/to/sFileName
 * 
 * @param pMap the map to save
 * @param sDirName the filename ("mapname" or "path/to/mapname" or "")
 * @return status
 */
MapSerializer::MapStatus MapSerializer::SaveMap( Map * pMap, std::string sDirName )
{
  MapStatus ms = MAP_OK;
  _CleanUp();

  cerr << " Saving Map " << sDirName << endl;
  
  if( pMap == NULL ) {
    cerr << "SaveMap: got a NULL map pointer. abort" << endl;
    return MAP_FAILED;
  }
  if( !pMap->IsGood() ) {
    cerr << "SaveMap: Bad Map. abort" << endl;
    return MAP_FAILED;
  }

  //add ourselves to the map lock list
  _RegisterWithMap( pMap );

  //make dir and filenames

  //If the map was saved before, do we use the last location or the one specified?
  bool bUsePrevSaveLocation = !mpMap->sSaveFile.empty() && sDirName.empty();

  if( bUsePrevSaveLocation ) {
    sDirName = mpMap->sSaveFile;
  }
  else if( sDirName.empty() ){
    ostringstream os;
    os << "map" << setfill('0') << setw(6) << mpMap->MapID();
    sDirName = os.str();
  }
    
  //does the dir exists
  struct stat st;
  if( stat( sDirName.c_str(), &st ) == 0 )
  {
    //is it a dir  
#ifdef WIN32
    // Get the current working directory:
    char* cwdBuffer = _getcwd( NULL, 0 );
    if( cwdBuffer == NULL )
    {
      perror( "Error getting the current working directory: Error with _getcwd in MapSerializer." );
      _UnRegisterWithMap();
      return MAP_EXISTS;    
    }
    
    //attempt to change working directory
    int chdirReturn = _chdir(sDirName.c_str()); //returns 0 if change was successful, -1 if it dosen't exist
    if( chdirReturn == 0 )
    {
      //restore previous working directory
      _chdir( cwdBuffer );
    }
    else {
#else
    if( !S_ISDIR(st.st_mode) ) {
#endif
      cerr << " A file called " << sDirName << " already exists. Aborting" << endl;
      _UnRegisterWithMap();
      return MAP_EXISTS;
    }
    
    // are we using this location anyway?
    if( !bUsePrevSaveLocation ) {
      cerr << "ERROR: Map directory " << sDirName << " already exists. Aborting. " << endl;
      _UnRegisterWithMap();
      return MAP_EXISTS;
    }      
  }  
  else
  {
#ifdef WIN32
    int err = _mkdir( sDirName.c_str());
#else
    int err = mkdir( sDirName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif
    if( err != 0 )
    {
      cerr << "Failed to make dir " << sDirName << ". Aborting." << endl;
      _UnRegisterWithMap();
      return MAP_FAILED;
    }
  }

  //recusively save the map and all its elements
  ms = _SaveMap( sDirName );
  if( MAP_OK != ms )
  {
    if( MAP_FAILED )
    {
      ///@TODO this will fail if the dir has contents.
#ifdef WIN32
      _rmdir( sDirName.c_str() );
#else
      rmdir( sDirName.c_str() );
#endif
    }
    _UnRegisterWithMap();
    return ms;
  }

  //record where map was saved to
  pMap->sSaveFile = sDirName;

  _UnRegisterWithMap();
  _CleanUp();
  cerr << " Map " << pMap->MapID() << " saved to " << sDirName << endl;
  return MAP_OK;
}


/**
 * Save all the maps out.
 * if no basename is provided then the map numbers will be used.
 * otherwise maps will be saved to path/to/basename/map000001/ etc.
 * @param maps maps to save
 * @param sBaseName base dir for all maps
 */
void MapSerializer::SaveMaps( std::vector<Map*> & vpMaps,  std::string sBaseName )
{
  //is there a base name. if not just let savemap create the default name.
  bool bBase = !sBaseName.empty();
  string sDirName = "";
  MapStatus ms;

  //does the dir exist?
  if( bBase ) {
    struct stat st;
    if( stat( sBaseName.c_str(),&st ) != 0  )
    {
#ifdef WIN32
      if( _mkdir( sBaseName.c_str()) != 0 ) {
#else
      if( mkdir( sBaseName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) != 0 ) {
#endif 
        cerr << "Failed to make dir " << sBaseName << endl;
        return;
      }
    }
  }
  
  //iteratively call save map
  std::vector<Map*>::iterator i;
  for( i = vpMaps.begin(); i != vpMaps.end(); i++ )
  {
    if( bBase ) {
      ostringstream os;
      os << sBaseName << "/map" << setfill( '0' ) << setw( 6 ) << (*i)->MapID();
      sDirName = os.str();
    }

    ms = SaveMap( (*i), sDirName );
  }
}


/**
 * Called when the thread is run.
 * It calls the appropiate function in the MapSerializer based on the input options
 */
void MapSerializer::run()
{
  if( !mbOK ) {
    cerr << " Call Init() before running the thread!" << endl;
    return;
  }

  Map * pMap = _ParseCommandAndParameters();

  //if a map was specified then it is either load or save a map
  if( pMap ) {
    
    if( msCommand == "LoadMap" )  {
      LoadMap( pMap, msDirName );
    }
    else if( msCommand == "SaveMap" )  {
      SaveMap( pMap, msDirName );
    }
  }
  //otherwise it is save all maps or an error
  else {

    if( msCommand == "SaveMaps" )  {
      SaveMaps( mvpMaps, msDirName );
    }
  }
  
  _CleanUp();
  
  mbOK = false;
}


/**
 * Print out the different serialization options,
 * so users know what they can do to load and save maps.
 */
void MapSerializer::PrintOptions()
{
  std::cout << "=== Map Serialization Options ===" << std::endl;
  std::cout << "   Action      |      DirName          \n"
            << "----------------------------------------\n"
            << "  LoadMap      |                        \n"
            << "  LoadMap      | mapNum                 \n"
            << "  LoadMap      | path/to/dirname       \n"
            << std::endl
            << "  SaveMap      |                        \n"
            << "  SaveMap      | mapNum                 \n"
            << "  SaveMap      | path/to/dirname       \n"
            << "  SaveMap      | mapNum path/to/dirname\n"
            << "  SaveMaps     |                        \n"
            << "  SaveMaps     | path/to/basename       \n"
            << "==========================================" << std::endl;
}




/**
 * Figure out what the user has asked for.
 */
Map * MapSerializer::_ParseCommandAndParameters()
{

  if( msCommand != "SaveMap" && msCommand != "LoadMap" && msCommand != "SaveMaps") {
    cerr << "Invalid serialization command." << endl;
    PrintOptions();
    return NULL;
  }

  

  /*  What has the user asked for?
      No args = current map to default location
      1 arg = map num to default or current to specified path
      2 args = specified map num to specified location
  */
  
  Map * pMap = NULL;
  msDirName = "";
  
  vector<string> vs = ChopAndUnquoteString(msParams);

  if( msCommand == "SaveMap" ) {

    switch( vs.size() ) {
      case 0:
        //nothing specified, so save current map
        pMap = mpInitMap;
        msDirName = "";
        break;
        
      case 1:
        pMap = _FindTheMap( vs[0] );
        if( pMap == NULL ) {
          //assume it is a path.
          pMap = mpInitMap;
          msDirName = vs[0];
        }
        break;

      case 2:
      pMap = _FindTheMap( vs[0] );

      if(pMap == NULL) {
        cerr << " Incorrect parameters." << endl;
        PrintOptions();
        return NULL;
      }

      msDirName = vs[1];
      break;
      
      default:
      cerr << " Incorrect parameters." << endl;
      PrintOptions();
      return NULL;
    }
  }
  else if( msCommand == "SaveMaps" ) {
    if(vs.size() == 1) {
      msDirName = vs[0];
      pMap = NULL;
    }
  }
  else if(  msCommand == "LoadMap" )
  {
    //always load into the current map.
    pMap = mpInitMap;
    int *pN = NULL;
    ostringstream os;
    
    if( vs.size() == 0 )
    {
        //nothing specified, so load based on current map number
        msDirName = "";
    }
    else if( vs.size() == 1 )
    {
      //is param a number or a path?
      bool bIsNum = true;
      for( size_t i = 0; i < vs[0].size(); i++ ) {
        bIsNum = isdigit( vs[0][i] ) && bIsNum;
      }
      
      if( bIsNum )
      {
        pN = ParseAndAllocate<int>(vs[0]);
        if( pN )
        {
          os << "map" << setfill('0') << setw(6) << *pN;
          msDirName = os.str();
          cerr << "msDirName = " << msDirName << endl;
          delete pN;
        }
        else {
          cerr << " Incorrect parameters." << endl;
        PrintOptions();
        return NULL;
        }
      }
      else {
        msDirName = vs[0];
      }
    }
  }
  else {
    PrintOptions();
    return NULL;
  }

  return pMap;
}


/**
 * Attempt to extract a map number from a string parameter
 * and then locate the map in the list of maps.
 * @param sParam the parameter string
 * @return the found map. Null is not found
 */
Map * MapSerializer::_FindTheMap( std::string sParam )
{
  //is param a number or a path?
  bool bIsNum = true;
  for( size_t i = 0; i < sParam.size(); i++ ) {
    bIsNum = isdigit( sParam[i] ) && bIsNum;
  }
      
  if( bIsNum )
  {
    int *pN = ParseAndAllocate<int>(sParam);
    if( pN )
    {
      //find the map
      for( size_t ii = 0; ii < mvpMaps.size(); ii++ )
      {
        if( mvpMaps[ ii ]->MapID() == *pN ) {
          delete pN;
          return mvpMaps[ ii ];
        }
      }

      delete pN;
    }
  }

  return NULL;
}

}
