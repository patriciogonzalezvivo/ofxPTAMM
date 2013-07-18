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
#ifndef CVD_QT_ARGB_H
#define CVD_QT_ARGB_H

#include <iostream>
#include <cvd/internal/is_pod.h>

namespace CVD {


//////////////////////////////
// CVD::Argb
// Template class to represent red, green, blue and alpha components
//
/// A colour consisting of red, green, blue and alpha components
/// @param T The datatype of each component
/// @ingroup gImage
template <typename T>
class Argb
{
public:
	/// Default constructor. Does nothing.
	Argb() {}
	/// Constructs a colour as specified
	/// @param r The red component
	/// @param g The green component
	/// @param b The blue component
	/// @param a The alpha component
	Argb(T a, T r, T g, T b) : red(r), green(g), blue(b), alpha(a) {}

   T blue; ///< The blue component
   T green; ///< The green component
   T red; ///< The red component
   T alpha; ///< The alpha component

   /// Assignment operator
   /// @param c The colour to copy from
   Argb<T>& operator=(const Argb<T>& c)
      {red = c.red; green = c.green; blue = c.blue; alpha = c.alpha; return *this;}

	/// Assignment operator between two different storage types, using the standard casts as necessary
	/// @param c The colour to copy from
   template <typename T2>
     Argb<T>& operator=(const Argb<T2>& c){
     red = static_cast<T>(c.red);
     green = static_cast<T>(c.green); 
     blue = static_cast<T>(c.blue); 
     alpha = static_cast<T>(c.alpha); 
     return *this;
   }

	/// Logical equals operator. Returns true if each component is the same.
	/// @param c Argb to compare with
	bool operator==(const Argb<T>& c) const
      {return red == c.red && green == c.green && blue == c.blue && alpha == c.alpha;}

	/// Logical not-equals operator. Returns true unless each component is the same.
	/// @param c Argb to compare with
	bool operator!=(const Argb<T>& c) const
      {return red != c.red || green != c.green || blue != c.blue || alpha != c.alpha;}

//   T to_grey() const {return 0.3*red + 0.6*green + 0.1*blue;}
};

/// Write the colour to a stream in the format "(red,green,blue,alpha)"
/// @param os The stream
/// @param x The colour object
/// @relates Argb
template <typename T>
std::ostream& operator <<(std::ostream& os, const Argb<T>& x)
{
   return os << "(" << x.alpha << "," << x.red << ","
             << x.green << "," << x.blue << ")";
}

/// Write the colour to a stream in the format "(red,green,blue,alpha)"
/// @param os The stream
/// @param x The colour object
/// @relates Argb
inline std::ostream& operator <<(std::ostream& os, const Argb<unsigned char>& x)
{
   return os << "(" << static_cast<unsigned int>(x.alpha) << ","
             << static_cast<unsigned int>(x.red) << ","
             << static_cast<unsigned int>(x.green) << ","
             << static_cast<unsigned int>(x.blue) << ")";
}

#ifndef DOXYGEN_IGNORE_INTERNAL
namespace Internal
{
  template<class C> struct is_POD<Argb<C> >
  {
    enum { is_pod = is_POD<C>::is_pod };
  };
}
#endif



} // end namespace 
#endif

