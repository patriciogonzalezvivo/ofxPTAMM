// -*- c++ -*-
// Copyright 2009 Isis Innovation Limited

/********************************************************************

  A set of utility functions
  Author: Robert Castle, 2009, bob@robots.ox.ac.uk

********************************************************************/

#ifndef __PTAMM_UTILS__
#define __PTAMM_UTILS__

#include <string>
#include <iostream>
#include <iterator>
#include <vector>

namespace PTAMM {

/**
 * Output a vector as a stream that is space separated.
 * @param os Output stream
 * @param v Vector to output
 * @return the stream output
 */
template<class T>
  std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
  std::copy(v.begin(), v.end(), std::ostream_iterator<T>(os, " "));
  return os;
}

void PruneWhiteSpace(std::string & str);

}

#endif
