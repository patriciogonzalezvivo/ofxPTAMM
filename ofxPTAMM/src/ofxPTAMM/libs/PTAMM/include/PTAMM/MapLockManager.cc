// -*- c++ -*-
// Copyright 2009 Isis Innovation Limited

/********************************************************************

  A Class to manage the locking and unlocking of maps

  Author: Robert Castle, 2008, bob@robots.ox.ac.uk

********************************************************************/

#include "MapLockManager.h"

#include <iostream>

#ifdef WIN32
#include <Windows.h>
#endif

namespace PTAMM {

/**
 * add thread to the usage map list
 * @param t thread to add
 */
void MapLockManager::Register( void * t )
{
  mLockRecords[ t ] = false;
}

/**
 * remove a thread from the map usage list
 * @param t thread to remove
 */
void MapLockManager::UnRegister( void * t)
{
  mLockRecords.erase( t );
}


/**
 * is a thread registered
 * @param t is thread registered
 * @return true/false
 */
bool MapLockManager::IsRegistered( void * t)
{
  std::map< void *, bool >::iterator i;
  i = mLockRecords.find( t );
  if( i != mLockRecords.end() ) {
    return true;
  }
  else {
    return false;
  }

}



/**
 * Threads call this to see if the map is locked
 * @param t checking thread
 * @param nTimeout timeout before returning
 * @return  false if no lock, or true if locked
 * @TODO implement the timeout
 */
bool MapLockManager::CheckLockAndWait( void * t, int nTimeout)
{
  //not locked continue
  if( !mbLocked ) {
    return false;
  }

  //we are locked so ack
  mLockRecords[ t ] = true;

  //if locked do we wait.
  while( mbLocked )
  {
#ifdef WIN32
    Sleep(1);
#else
    usleep(300);
#endif
  }

  mLockRecords[ t ] = false;
  
  return true;
}


/**
 * lock the map
 * @param t locing thread
 * @param nTimeout timeout before aborting
 * @return sucess
 * @TODO implement timeout
 */
bool MapLockManager::LockMap( void * t, int nTimeout )
{
  if( mbLocked && t != mLockingThread )  {
    std::cerr << "Map is already locked by another thread." << std::endl;
    return false;
  }
    
  mbLocked = true;
  mLockingThread = t;

  bool bAllAck = true;
  std::map< void *, bool >::iterator i;

  //if thread is the only user then you have got lock
  if( mLockRecords.size() == 1 )  {
    if( mLockRecords.find( t ) != mLockRecords.end() ) {
      std::cerr << "only map user. lock granted" << std::endl;
      return true;
    }
    else  {
      std::cerr << "WARNING: This thread is not registered to this map! Another thread is so waiting for them." << std::endl;
    }
  }

  //wait for other threads to ack.
  do
  {
    bAllAck = true;
    for( i = mLockRecords.begin(); i != mLockRecords.end(); i++ )
    {
      if( (*i).first == t ) {
        //locking yourself out is stupid
        continue;
      }

      //will only go true when all have ackknowledged
      bAllAck = bAllAck && (*i).second;
    }

    //wait for a bit
    if(!bAllAck)  {
#ifdef WIN32
      Sleep(1);
#else
      usleep(300);
#endif
    }
  }  while( !bAllAck );

  return true;
}

/**
 * Unlock a map
 * @param t thread
 * @return success
 */
bool MapLockManager::UnlockMap( void * t)
{
  if(t == mLockingThread) {
    mbLocked = false;
    mLockingThread = NULL;
    return true;
  }

  return false;
}


}

