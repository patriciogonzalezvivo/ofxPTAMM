// -*- c++ -*-
// Copyright 2009 Isis Innovation Limited

/********************************************************************

  A Class to serialize and deserialize maps.

  Author: Robert Castle, 2009, bob@robots.ox.ac.uk

********************************************************************/

#ifndef __MAP_SERIALIZER__
#define __MAP_SERIALIZER__


#include <fstream>
#include <vector>
#include <map>
#include <cvd/thread.h>

#include "tinyxml.h"

namespace PTAMM {

class Map;
class KeyFrame;
struct MapPoint;
struct Measurement;

#define MAP_XML_ID "PTAMM_Map"
#define MAP_VERSION "1.0"

/**
 * This class is used for serializing and deserializing maps into a xml format
 */
class MapSerializer : public CVD::Thread
{
  public:
    enum MapStatus { MAP_OK, MAP_FAILED, MAP_EXISTS };
    
    MapSerializer( std::vector<Map*> &maps );
    ~MapSerializer();

    bool Init(  std::string sCommand, std::string sParams, Map &currentMap );
    void PrintOptions();
    
    virtual void run();

  private:
    Map * _ParseCommandAndParameters();
    
    //The actual map saving and loading functions
    MapStatus LoadMap( Map * pMap, std::string sDirName );
    MapStatus SaveMap( Map * pMap, std::string sDirName );
    void SaveMaps( std::vector<Map*> & vpMaps,  std::string sBaseName );
        
    //saving
    MapStatus _SaveMap( std::string sPath );
    
    bool _SaveAKeyFrame( KeyFrame * kf, const std::string & sPath, TiXmlElement * keyFramesNode );
    bool _SaveKeyFrames( const std::string & sPath, TiXmlElement * rootNode );
    
    bool _SaveAMapPoint( MapPoint * mp, TiXmlElement * mapPointsNode );
    bool _SaveMapPoints( TiXmlElement * rootNode );
    
    void _CreateSaveLUTs();
    int _LookupKeyFrame( KeyFrame * k );
    int _LookupMapPoint( MapPoint * m );

    //loading
    MapStatus _LoadMap( std::string sDirName );
    
    bool _LoadAKeyFrame( TiXmlHandle &hKF, const std::string & sPath, bool bQueueFrame = false );
    bool _LoadKeyFrames( TiXmlHandle &hRoot, const std::string & sPath );
    
    bool _LoadAMapPoint( TiXmlHandle &hMP, bool bQueuePoint = false );
    bool _LoadMapPoints( TiXmlHandle &hRoot );

    KeyFrame * _LookupKeyFrame( int uid );
    MapPoint * _LookupMapPoint( int uid );
    bool _CrossReferencing(TiXmlHandle &hRoot);

    //other
    bool _LockMap();
    void _UnlockMap();
    void _UnRegisterWithMap();
    void _RegisterWithMap( Map * map );
    void _CleanUp();
    Map * _FindTheMap( std::string sParam );
    
  private:
    std::string msDirName;                                  // The directory to save the map(s) to
    Map * mpMap;                                            // the map currently be (de)serialized
    std::vector<Map*> & mvpMaps;                            // The set of maps
    bool mbOK;                                              // Init() has been run.

    std::map< const MapPoint *, int > mmMapPointSaveLUT;    // lookup table for assigning uid to mappoints
    std::map< const KeyFrame *, int > mmKeyFrameSaveLUT;    // lookup table for assigning uid to keyframes
    std::map< int, MapPoint * >       mmMapPointLoadLUT;    // lookup table for assigning uid to mappoints
    std::map< int, KeyFrame * >       mmKeyFrameLoadLUT;    // lookup table for assigning uid to keyframes
    //look up table for cross referencing keyframes with map points. need this as load keyframes then map points. then cross ref.
    std::map< KeyFrame*, std::vector< std::pair< int, Measurement > > > mmMeasCrossRef;
    std::vector< std::pair< int, int > > mvFailureQueueUIDs;

    std::string msCommand;                                  // The command passed to Init()
    std::string msParams;                                   // The params passed to Init()
    Map * mpInitMap;                                        // The map passed to Init(). The current map.
};

}

#endif
