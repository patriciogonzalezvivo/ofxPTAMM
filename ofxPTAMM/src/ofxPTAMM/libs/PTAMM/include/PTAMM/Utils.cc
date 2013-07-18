// -*- c++ -*-
// Copyright 2009 Isis Innovation Limited

/********************************************************************

  A set of utility functions
  Author: Robert Castle, 2009, bob@robots.ox.ac.uk

********************************************************************/

#include "Utils.h"

#include <sstream>
#include <gvars3/GStringUtil.h>

namespace PTAMM {


/**
 * Edit a string so that the only whitespace is a single space between elements.
 * @param str the string to edit. This gets replaced.
 */
void PruneWhiteSpace(std::string & str)
{
  std::vector< std::string > tokens = GVars3::ChopAndUnquoteString(str);
  std::ostringstream os;
  os << tokens;

  str = os.str();

  if(str.at( str.size() - 1 ) == ' ' ) {
    str.erase(  str.end() - 1 );
  }
}

}
