/*                       
	This file is part of the CVD Library.

	Copyright (C) 2005 The Authors

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef CVD_INCLUDE_BGRX_H
#define CVD_INCLUDE_BGRX_H

#include <iostream>
#include <cvd/internal/is_pod.h>

namespace CVD {


//////////////////////////////
// CVD::Bgrx
/// A colour consisting of red, green, blue and dummy components,
/// in the order bgr dummy in memory.
/// @param T The datatype of each component
/// @ingroup gImage
template <typename T>
class Bgrx
{
public:
	/// Default constructor. Does nothing.
	Bgrx() {}
	/// Constructs a colour as specified
	/// @param r The red component
	/// @param g The green component
	/// @param b The blue component
	Bgrx(T b, T g, T r) : blue(b), green(g), red(r) {}

   T blue; ///< The blue component
   T green; ///< The green component
   T red; ///< The red component
   T dummy; ///< The dummy

	/// Assignment operator between two different storage types, using the standard casts as necessary
	/// @param c The colour to copy from
   template <typename T2>
     Bgrx<T>& operator=(const Bgrx<T2>& c){
     blue = static_cast<T>(c.blue); 
     green = static_cast<T>(c.green); 
     red = static_cast<T>(c.red);
     return *this;
   }

	/// Logical equals operator. Returns true if each component is the same.
	/// @param c Bgrx to compare with
	bool operator==(const Bgrx<T>& c) const
      {return red == c.red && green == c.green && blue == c.blue;}

	/// Logical not-equals operator. Returns true unless each component is the same.
	/// @param c Bgrx to compare with
	bool operator!=(const Bgrx<T>& c) const
      {return red != c.red || green != c.green || blue != c.blue;}

//   T to_grey() const {return 0.3*red + 0.6*green + 0.1*blue;}
};

/// Write the colour to a stream in the format "(blue,green,red)"
/// @param os The stream
/// @param x The colour object
/// @relates Bgrx
template <typename T>
std::ostream& operator <<(std::ostream& os, const Bgrx<T>& x)
{
   return os << "(" << x.blue << ","
             << x.green << "," << x.red << ")";
}

/// Write the colour to a stream in the format "(blue,green,red)"
/// @param os The stream
/// @param x The colour object
/// @relates Bgrx
inline std::ostream& operator <<(std::ostream& os, const Bgrx<unsigned char>& x)
{
   return os << "(" 
             << static_cast<unsigned int>(x.blue) << ")"
             << static_cast<unsigned int>(x.green) << ","
             << static_cast<unsigned int>(x.red) << ",";
}

#ifndef DOXYGEN_IGNORE_INTERNAL
namespace Internal
{
  template<class C> struct is_POD<Bgrx<C> >
  {
    enum { is_pod = is_POD<C>::is_pod };
  };
}
#endif



} // end namespace 
#endif

