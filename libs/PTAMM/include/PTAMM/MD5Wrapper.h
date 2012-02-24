// -*- c++ -*-
// Copyright 2009 Isis Innovation Limited
/*
 * A wrapper class for md5 to provide some simple hashing fucntions.
 * Some code originally by Benjamin Grüdelbach (Feb 2005)
 *
 * Author: Bob Castle
 */

#ifndef MD5WRAPPER_H
#define MD5WRAPPER_H

#include <string>

namespace PTAMM {

class MD5;

class MD5Wrapper
{
  public:
     MD5Wrapper();
    ~MD5Wrapper();
  
    bool getHashFromData(const unsigned char *byte, unsigned int nBytesToRead, std::string & sMD5Hash);
    
  private:
    std::string convToString(unsigned char *bytes);

  private:
    MD5 *mMD5;

};

}

#endif
