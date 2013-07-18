// -*- c++ -*-
// Copyright 2009 Isis Innovation Limited

/********************************************************************

  A Class to manage the locking and unlocking of maps

  Author: Robert Castle, 2008, bob@robots.ox.ac.uk

********************************************************************/

#ifndef __MAP_LOCK_MANAGER__
#define __MAP_LOCK_MANAGER__

#include <map>
#include <cvd/thread.h>

namespace PTAMM {

/**
 * This class is to be used by any thread that want to gain a sole
 * lock on a map to use it. Typically this used by the mapserializer.
 * But other threads my need to use it in the future.
 *
 * @TODO on unregister, check for stale locks and release.
 */
class MapLockManager
{
  public:
    MapLockManager() : mLockingThread(NULL), mbLocked(false)  {}
    ~MapLockManager() {}

    void Register( void * t );                   // call when switching to a map
    void UnRegister( void * t);                  // call when leaving a map
    bool IsRegistered( void * t);                // check if thread is registered
    bool IsLocked() { return mbLocked; }                  // check if map is locked
    bool CheckLockAndWait( void * t, int nTimeout = 0);   // if locked wait for release
    bool LockMap( void * t, int nTimeout = 0 );  // lock a map. waits until all thread ack
    bool UnlockMap( void * t);                   // unlock a map. only if thread is the owner

  private:
    std::map< void *, bool > mLockRecords;       // which threads are currently using this map
    void * mLockingThread;                       // who want to lock / has locked the thread
    bool mbLocked;                               // is the map locked

};


}


#endif

