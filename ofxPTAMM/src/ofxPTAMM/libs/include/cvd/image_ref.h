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
//-*- c++ -*-
//////////////////////////////////////////////////////////////////////////
//                                                                      //
//  CVD::image.h                                                        //
//                                                                      //
//  Definitions for of template classes CVD::ImageRef and CVD::Image    //
//                                                                      //
//  derived from IPRS_* developed by Tom Drummond                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef __CVD_IMAGE_REF_H__
#define __CVD_IMAGE_REF_H__

#include <iostream>
#include <cctype>
#include <cvd/exceptions.h>

namespace CVD {

/// @ingroup gImage

//////////////////////////////////////
// CVD::ImageRef                    //
//////////////////////////////////////
/// An (x,y) image co-ordinate
class ImageRef
{
public:

	//Construction
	/// Construct an ImageRef initialised at (0,0)
	inline ImageRef();
	/// Construct an ImageRef
	/// @param xp The x co-ordinate
	/// @param yp The y co-ordinate
	inline ImageRef(int xp, int yp);
	/// Construct an ImageRef from a stream
	/// @param is The stream to read
	inline ImageRef(std::istream& is);

	//Iteration

	/// Step to the next co-ordinate in the image (in horizontal scanline order).
	/// Successive calls to this function will work along each row of the image.
	/// If the next co-ordinate is off the end of the image, it returns false.
	/// @param max The size of the image
	inline bool next(const ImageRef& max);
	/// Step to the previous co-ordinate in the image (in horizontal scanline order).
	/// Successive calls to this function will work backwards along each row of the image.
	/// If the next co-ordinate is off the start of the image, it returns false.
	/// @param max The size of the image
	inline bool prev(const ImageRef& max);
	/// Step to the next co-ordinate in the image (in horizontal scanline order),
	/// for pixels within a rectangular window
	/// Successive calls to this function will work backwards along each row of the image.
	/// If the next co-ordinate is off the start of the image, it returns false.
	/// @param min The minimum (x,y) co-ordinates in the window
	/// @param max The minimum (x,y) co-ordinates in the window
	inline bool next(const ImageRef& min, const ImageRef& max);
	/// Step to the previous co-ordinate in the image (in horizontal scanline order),
	/// for pixels within a rectangule window
	/// Successive calls to this function will work backwards along each row of the image.
	/// If the next co-ordinate is off the start of the image, it returns false.
	/// @param min The minimum (x,y) co-ordinates in the window
	/// @param max The minimum (x,y) co-ordinates in the window
	inline bool prev(const ImageRef& min, const ImageRef& max);

	/// Resets the ImageRef to (0,0)
	inline void home();
	/// Resets the ImageRef to the maximum co-ordinate in the image
	/// i.e. <code>(size.x - 1, size.y - 1)</code>
	/// @param size The size of the image
	inline void end(const ImageRef& size);


	//Operators

	/// Assigment
	/// @param ref The co-ordinate to copy
	inline ImageRef& 	operator=(const ImageRef& ref);
	/// Logical equals
	/// @param ref The co-ordinate to compare with
	inline bool 		operator==(const ImageRef& ref) const;
	/// Logical not equals
	/// @param ref The co-ordinate to compare with
	inline bool 		operator!=(const ImageRef& ref) const;
  /// Unary minus. Negates both x and y components.
  inline ImageRef  operator-() const;
  /// Multiply both x and y co-ordinates by a scalar
  /// @param scale The multiplier
  inline ImageRef&  operator*=(const double scale);
	/// Divide both x and y co-ordinates by a scalar
	/// @param scale The factor
	inline ImageRef&	operator/=(const double scale);
	/// Add an offset to the co-ordinate
	/// @param rhs The offset
	inline ImageRef& 	operator+=(const ImageRef rhs);
	/// Subtract an offset from the co-ordinate
	/// @param rhs The offset
	inline ImageRef& 	operator-=(const ImageRef rhs);
	/// Multiply both x and y co-ordinates by a scalar
	/// @param scale The multiplier
	inline ImageRef 	operator*(const double scale) const;
	/// Divide both x and y co-ordinates by a scalar
	/// @param scale The factor
	inline ImageRef 	operator/(const double scale) const;
	/// Add an offset to the co-ordinate
	/// @param rhs The offset
	inline ImageRef 	operator+(const ImageRef rhs) const;
	/// Subtract an offset from the co-ordinate
	/// @param rhs The offset
	inline ImageRef 	operator-(const ImageRef rhs) const;
	/// Bitwise left-shift operator
	/// @param i The amount to shift
	inline ImageRef& 	operator<<=(int i);
	/// Bitwise right-shift operator
	/// @param i The amount to shift
	inline ImageRef& 	operator>>=(int i);
	/// Bitwise right-shift operator
	/// @param i The amount to shift
	inline ImageRef		operator>>(int i) const;
	/// Bitwise left-shift operator
	/// @param i The amount to shift
	inline ImageRef		operator<<(int i) const;
  /** An ImageRef is less than another ImageRef if it is earlier in the
  standard horizontal scan-line order, i.e. has a smaller y-co-ordinate
  or the same y-co-ordinate but a smaller x-co-ordinate. An ordering of
  ImageRefs is needed to allow them to be used in STL for sets, maps,
  multisets, multimaps etc. **/
  inline bool operator<(const ImageRef & other) const;
  /** An ImageRef is greater than another ImageRef if it is earlier in the
  standard horizontal scan-line order, i.e. has a smaller y-co-ordinate
  or the same y-co-ordinate but a smaller x-co-ordinate. **/
  inline bool operator>(const ImageRef & other) const;

	/// Magnitude-squared (x*x + y*y)
	inline unsigned int     mag_squared() const;

	/// Area (product of x and y; signed)
	inline int area() const;

	/// The equivalent of doing .* in matlab
	inline ImageRef dot_times(const ImageRef &ref) const;

	/// Square bracket subscripts for easy loops. 0=x 1=y other=error
	inline int& operator[](int i);

	/// Square bracket const subscripts for easy loops. 0=x 1=y other=error
	inline int operator[](int i) const;

	//Why do these exist?
	///@overload
	inline ImageRef shiftl(int i) const;
	///@overload
	inline ImageRef shiftr(int i) const;

	// and now the data members (which are public!)
	int x; ///< The x co-ordinate
	int y; ///< The y co-ordinate

};

/// Left-multiply an ImageRef by a scalar. Mutiplies both x and y.
/// @param scale The multiplier
/// @param ref The ImageRef to scale
/// @relates ImageRef
inline ImageRef operator*(const int scale, const ImageRef&  ref);

namespace Exceptions
{
  /// Exception if subscript for [] is not 0 or 1
  struct BadSubscript: public CVD::Exceptions::All {BadSubscript(){};};
}


#include <cvd/internal/image_ref_implementation.hh>

// Streams stuff for ImageRef class //

/// Write an ImageRef to a stream in the format "[x y]"
/// @param os The stream
/// @param ref The co-ordinate
/// @relates ImageRef
inline std::ostream& operator<<(std::ostream& os, const ImageRef& ref)
{
	return os << "[" << ref.x << " " << ref.y << "]";
}

/// Read an ImageRef from a stream. Any format with two successive numbers will work
/// @relates ImageRef
inline std::istream& operator>>(std::istream& is, ImageRef& ref)
{
	//Full parsing for ImageRefs, to allow it to accept the
	//output produced above, as well as the older (x,y) format
	is >> std::ws;

	unsigned char c = is.get();

	if(is.eof())
		return is;

	if(c == '(' )
	{
		is >> std::ws >> ref.x >> std::ws;

		if(is.get() != ',')
			goto bad;

		is >> std::ws >> ref.y >> std::ws;

		if(is.get() != ')')
			goto bad;
	}
	else if(c == '[' )
	{
		is >> std::ws >> ref.x >> std::ws >> ref.y >> std::ws;
		if(is.get() != ']')
			goto bad;
	}
	else if(isdigit(c))
	{
		is.unget();
		is >> ref.x >> ref.y;
	}
	else
		goto bad;

	return is;

	bad:
	is.setstate(std::ios_base::badbit);

	return is;
}

/// A zero ImageRef
/// @relates ImageRef
const ImageRef ImageRef_zero(0, 0);


} //namespace CVD


#endif
