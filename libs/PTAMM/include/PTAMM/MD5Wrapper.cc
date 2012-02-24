// -*- c++ -*-
// Copyright 2009 Isis Innovation Limited
/*
 * A wrapper class for md5 to provide some simple hashing fucntions.
 * Some code originally by Benjamin Grüdelbach (Feb 2005)
 *
 * Author: Bob Castle
 */

#include <fstream>
#include <iostream>

#include "MD5Wrapper.h"
#include "MD5.h"

namespace PTAMM {

/**
 * Constructor
 */
MD5Wrapper::MD5Wrapper()
  : mMD5( new MD5() )
{
}

/**
 * Destructor
 */
MD5Wrapper::~MD5Wrapper()
{
  delete mMD5;
}


/**
 * converts the numeric hash to
 * a valid std::string.
 * (based on Jim Howard's code;
 * http://www.codeproject.com/cpp/cmd5.asp)
 * @param bytes char array
 * @return string
 */
std::string MD5Wrapper::convToString(unsigned char *bytes)
{
  char asciihash[33];

  int p = 0;
  for(int i=0; i<16; i++)
  {
    ::sprintf(&asciihash[p],"%02x",bytes[i]);
    p += 2;
  }
  asciihash[32] = '\0';
  return std::string(asciihash);
}

/**
 * Calculate the hash from a block of data
 * @param byte the data block
 * @param nBytesToRead the number of bytes to read
 * @param sMD5Hash the returned hash
 * @return success
 */
bool MD5Wrapper::getHashFromData(const unsigned char *byte, unsigned int nBytesToRead, std::string & sMD5Hash)
{
  if( byte == NULL )  {
    return "-1";
  }

  //init md5
  MD5_CTX context;
  mMD5->MD5Init( &context );
  
  mMD5->MD5Update( &context, byte, nBytesToRead );
  
  // generate hash and return the hash as std::string
  unsigned char digest[16];
  mMD5->MD5Final( digest, &context );

  sMD5Hash = convToString(digest);
  return true;
}


}
