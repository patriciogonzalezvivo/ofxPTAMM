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

#ifndef CVD_INC_INTEGRAL_IMAGE_H
#define CVD_INC_INTEGRAL_IMAGE_H

#include <cvd/image.h>
#include <cvd/vision.h>
#include <cvd/internal/pixel_operations.h>

namespace CVD
{
	///Compute an integral image. In an integral image, pixel (x,y) is equal to the sum of all the pixels 
	///in the rectangle from (0,0) to (x,y) in the original image.
	/// and reallocation is not performed if <code>b</code> is unique and of the correct size.
	/// @param D The destination image pixel type
	/// @param S The source image pixel type
	/// @param in The source image.
	/// @param out The source image.
	/// @ingroup gVision
		
	template<class S, class D> void integral_image(const SubImage<S>& in, SubImage<D>& out)
	{
		if( in.size() != out.size())
			throw Exceptions::Vision::IncompatibleImageSizes("integral_image");

		Pixel::operations<D>::assign(out[0][0], in[0][0]);
		//Do the first row. 
		for(int x=1; x < in.size().x; x++)
			out[0][x] =out[0][x-1] + in[0][x];

		//Do the first column. 
		for(int y=1; y < in.size().y; y++)
			out[y][0] =out[y-1][0] + in[y][0];

		//Do the remainder of the image
		for(int y=1; y < in.size().y; y++)
		{
			D sum;
			Pixel::operations<D>::assign(sum,in[y][0]);

			for(int x=1; x < in.size().x; x++)
			{
				sum+= in[y][x];
				Pixel::operations<D>::assign(out[y][x],sum + out[y-1][x]);
			}
		}
	}
	#ifndef DOXYGEN_IGNORE_INTERNAL
		namespace Internal
		{
			template<class C> class IntegralImage{};

			template<class C>  struct ImagePromise<IntegralImage<C> >
			{
				ImagePromise(const SubImage<C>& im)
				:i(im)
				{}

				const SubImage<C>& i;
				template<class D> void execute(Image<D>& j)
				{
					j.resize(i.size());
					integral_image<C,D>(i, j);
				}
			};
		};

		template<class C> Internal::ImagePromise<Internal::IntegralImage<C> > integral_image(const SubImage<C>& c)
		{
			return Internal::ImagePromise<Internal::IntegralImage<C> >(c);
		}
	#else
		///Compute an integral image. In an integral image, pixel (x,y) is equal to the sum of all the pixels 
		///in the rectangle from (0,0) to (x,y) in the original image.
		///Type deduction is automatic, and D can not be specified. The following usage will work:
		///
		/// \code
		/// Image<byte> a;
		/// Image<int> b;
		/// ...
		/// b = integral_image(a);
		/// \endcode
		/// Note that this is performed using lazy evaluation, so convertion happens on evaluation of assignment,
		/// and reallocation is not performed if <code>b</code> is unique and of the correct size.
		/// @param D The destination image pixel type
		/// @param S The source image pixel type
		/// @param from The source image.
		/// @return The integral image
		/// @ingroup gVision
		template<class S, class D> Image<D> integral_image(const BasicImage<S>& from);

	#endif

}


#endif

