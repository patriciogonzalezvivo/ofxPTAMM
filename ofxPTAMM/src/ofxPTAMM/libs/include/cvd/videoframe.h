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
//////////////////////////////////////////////////////////////////////
//                                                                  //
//   VideoFrame - An image with a timestamp, like from a video      //
//                source                                            //
//                                                                  //
//   Tom Drummond     3 April 2002                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////

#ifndef CVD_VIDEOFRAME_H
#define CVD_VIDEOFRAME_H

#include <cvd/image.h>

namespace CVD {

  namespace VideoFrameFlags 
    {
      /// Fields etc
      enum FieldType{
	Top,
	Bottom,
	Both,
	Progressive,
	Unknown
      };
    }      

/// A frame from a VideoBuffer.
/// @param T The pixel type of the video frames
/// @ingroup gVideoFrame
template <class T>
class VideoFrame : public BasicImage<T> 
{
	public:
		/// (Used internally) Construct a VideoFrame around a block of memory. The
		/// memory is not managed by the VideoFrame so must me managed seperately
		/// @param t The time (in seconds) of this frame
		/// @param data The image data for this frame
		/// @param size The size of this video frame
		VideoFrame(double t, T* data, const ImageRef& size, VideoFrameFlags::FieldType f=VideoFrameFlags::Unknown) 
		  :BasicImage<T>(data, size),my_field(f),my_timestamp(t)
		{
		}

		/// (Used internally) Construct a VideoFrame from a BasicImage
		/// @param t The time (in seconds) of this frame
		/// @param im The image data for this frame. BasicImages do not manage their own
		/// memory, so this must be managed externally
		VideoFrame(double t, const BasicImage<T>& im, VideoFrameFlags::FieldType f=VideoFrameFlags::Unknown) 
		  :BasicImage<T>(im),my_field(f),my_timestamp(t)
		{
		}

		/// What is the time (since boot) of this frame?
		double timestamp() const
		{
			return my_timestamp;
		}

		VideoFrameFlags::FieldType field() const
		{
			return my_field;
		}

	protected:
		/// We don't usually <code>delete</code> video frames. Some special destruction is usually needed.
		virtual ~VideoFrame()
		{
		}

		VideoFrameFlags::FieldType my_field;  /// Type of field in this frame
		double my_timestamp;  ///< No of seconds since boot of this frame
};

}


#endif
