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
#ifndef CVD_COLOURSPACES_H
#define CVD_COLOURSPACES_H

#include <cvd/internal/is_pod.h>
#include <cvd/internal/builtin_components.h>
#include <cvd/internal/pixel_traits.h>
#include <cvd/internal/name_builtin_types.h>

namespace CVD
{

	/// Bayer datatype representing the colour filter pattern BGGR
	/// @ingroup gVideoBuffer
	struct bayer_bggr
	{
		unsigned char val;
		operator unsigned char() const { return val; }
		bayer_bggr() {}
		bayer_bggr(unsigned char v) : val(v) {}
	};

	/// Bayer datatype representing the colour filter pattern GBRG
	/// @ingroup gVideoBuffer
	struct bayer_gbrg
	{
		unsigned char val;
		operator unsigned char() const { return val; }
		bayer_gbrg() {}
		bayer_gbrg(unsigned char v) : val(v) {}
	};
	
	/// Bayer datatype representing the colour filter pattern GRBG
	/// @ingroup gVideoBuffer
	struct bayer_grbg
	{
		unsigned char val;
		operator unsigned char() const { return val; }
		bayer_grbg() {}
		bayer_grbg(unsigned char v) : val(v) {}
	};

	/// Bayer datatype representing the colour filter pattern RGGB
	/// @ingroup gVideoBuffer
	struct bayer_rggb
	{
		unsigned char val;
		operator unsigned char() const { return val; }
		bayer_rggb() {}
		bayer_rggb(unsigned char v) : val(v) {}
	};
	
	/// 16 bit Bayer datatype representing the colour filter pattern BGGR
	/// @ingroup gVideoBuffer
	struct bayer_bggr16
	{
		unsigned short  val;
		operator unsigned short() const { return val; }
		bayer_bggr16() {}
		bayer_bggr16(unsigned short v) : val(v) {}
	};

	/// 16bit Bayer datatype representing the colour filter pattern GBRG
	/// @ingroup gVideoBuffer
	struct bayer_gbrg16
	{
		unsigned short val;
		operator unsigned short() const { return val; }
		bayer_gbrg16() {}
		bayer_gbrg16(unsigned short v) : val(v) {}
	};
	
	/// 16bit Bayer datatype representing the colour filter pattern GRBG
	/// @ingroup gVideoBuffer
	struct bayer_grbg16
	{
		unsigned short val;
		operator unsigned short() const { return val; }
		bayer_grbg16() {}
		bayer_grbg16(unsigned short v) : val(v) {}
	};

	/// 16bit Bayer datatype representing the colour filter pattern RGGB
	/// @ingroup gVideoBuffer
	struct bayer_rggb16
	{
		unsigned short val;
		operator unsigned short() const { return val; }
		bayer_rggb16() {}
		bayer_rggb16(unsigned char v) : val(v) {}
	};

	/// 16 bit big endian Bayer datatype representing the colour filter pattern BGGR
	/// @ingroup gVideoBuffer
	struct bayer_bggr16be
	{
		unsigned short  val;
		operator unsigned short() const { return val; }
		bayer_bggr16be() {}
		bayer_bggr16be(unsigned short v) : val(v) {}
	};

	/// 16bit big endian Bayer datatype representing the colour filter pattern GBRG
	/// @ingroup gVideoBuffer
	struct bayer_gbrg16be
	{
		unsigned short val;
		operator unsigned short() const { return val; }
		bayer_gbrg16be() {}
		bayer_gbrg16be(unsigned short v) : val(v) {}
	};
	
	/// 16bit big endian Bayer datatype representing the colour filter pattern GRBG
	/// @ingroup gVideoBuffer
	struct bayer_grbg16be
	{
		unsigned short val;
		operator unsigned short() const { return val; }
		bayer_grbg16be() {}
		bayer_grbg16be(unsigned short v) : val(v) {}
	};

	/// 16bit big endian Bayer datatype representing the colour filter pattern RGGB
	/// @ingroup gVideoBuffer
	struct bayer_rggb16be
	{
		unsigned short val;
		operator unsigned short() const { return val; }
		bayer_rggb16be() {}
		bayer_rggb16be(unsigned char v) : val(v) {}
	};

	/// typedef to support old bayer datatype
	/// @ingroup gVideoBuffer
	/// @deprecated
	typedef bayer_bggr bayer;
	
	/// A datatype to represent yuv411 (uyyvyy) data, typically from firewire
	/// cameras. It can be used to configure dvbuffer to return this format.
	/// @ingroup gVideoBuffer
	struct yuv411
	{
		unsigned char val;
	};

	/// A datatype to represent yuv422 (yuyv) data.
	/// @ingroup gVideoBuffer
	struct yuv422
	{
		unsigned short val;
	};

	/// A datatype to represent yuv420p (yy...u...v) data.
	/// @ingroup gVideoBuffer
	struct yuv420p
	{
		unsigned short val;
	};
	
	/// A datatype to represent the other yuv422 (uyvy) data. This is returned
	/// by the @ref QTBuffer on Mac OSX. See the following from Apple:
	/// http://developer.apple.com/quicktime/icefloe/dispatch020.html
	/// @ingroup gVideoBuffer
	struct vuy422
	{
		unsigned short val;
	};

	namespace PNM{
		template<> struct type_name<bayer_bggr> { static std::string name(){return "bayer_bggr" ;}};
		template<> struct type_name<bayer_gbrg> { static std::string name(){return "bayer_gbrg" ;}};
		template<> struct type_name<bayer_grbg> { static std::string name(){return "bayer_grbg" ;}};
		template<> struct type_name<bayer_rggb> { static std::string name(){return "bayer_rggb" ;}};

		template<> struct type_name<bayer_bggr16> { static std::string name(){return "bayer_bggr16" ;}};
		template<> struct type_name<bayer_gbrg16> { static std::string name(){return "bayer_gbrg16" ;}};
		template<> struct type_name<bayer_grbg16> { static std::string name(){return "bayer_grbg16" ;}};
		template<> struct type_name<bayer_rggb16> { static std::string name(){return "bayer_rggb16" ;}};

		template<> struct type_name<bayer_bggr16be> { static std::string name(){return "bayer_bggr16be" ;}};
		template<> struct type_name<bayer_gbrg16be> { static std::string name(){return "bayer_gbrg16be" ;}};
		template<> struct type_name<bayer_grbg16be> { static std::string name(){return "bayer_grbg16be" ;}};
		template<> struct type_name<bayer_rggb16be> { static std::string name(){return "bayer_rggb16be" ;}};

		template<> struct type_name<yuv411> { static std::string name(){return "yuv411" ;}};
		template<> struct type_name<yuv422> { static std::string name(){return "yuv422" ;}};
		template<> struct type_name<yuv420p>{ static std::string name(){return "yuv420p";}};
		template<> struct type_name<vuy422> { static std::string name(){return "vuy422" ;}};
	}

  namespace Pixel {
        template<int LIFT> struct traits<bayer_bggr, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 8;
            static const unsigned char max_intensity=(1 << bits_used) - 1;
        };

	template<int LIFT> struct traits<bayer_rggb, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 8;
            static const unsigned char max_intensity=(1 << bits_used) - 1;
        };

	template<int LIFT> struct traits<bayer_gbrg, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 8;
            static const unsigned char max_intensity=(1 << bits_used) - 1;
        };
	
	template<int LIFT> struct traits<bayer_grbg, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 8;
            static const unsigned char max_intensity=(1 << bits_used) - 1;
        };

        template<int LIFT> struct traits<bayer_bggr16, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 16;
            static const unsigned short max_intensity=(1 << bits_used) - 1;
        };

	template<int LIFT> struct traits<bayer_rggb16, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 16;
            static const unsigned short max_intensity=(1 << bits_used) - 1;
        };

	template<int LIFT> struct traits<bayer_gbrg16, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 16;
            static const unsigned short max_intensity=(1 << bits_used) - 1;
        };
	
	template<int LIFT> struct traits<bayer_grbg16, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 16;
            static const unsigned short max_intensity=(1 << bits_used) - 1;
        };

        template<int LIFT> struct traits<bayer_bggr16be, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 16;
            static const unsigned short max_intensity=(1 << bits_used) - 1;
        };

	template<int LIFT> struct traits<bayer_rggb16be, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 16;
            static const unsigned short max_intensity=(1 << bits_used) - 1;
        };

	template<int LIFT> struct traits<bayer_gbrg16be, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 16;
            static const unsigned short max_intensity=(1 << bits_used) - 1;
        };
	
	template<int LIFT> struct traits<bayer_grbg16be, LIFT>
        {
            typedef int wider_type;
            typedef float float_type;
            static const bool integral = true;
            static const bool is_signed = false;
            static const int bits_used = 16;
            static const unsigned short max_intensity=(1 << bits_used) - 1;
        };
    }

   
#ifndef DOXYGEN_IGNORE_INTERNAL
    namespace Internal
    {
      template<> struct is_POD<bayer_bggr>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<bayer_gbrg>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<bayer_grbg>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<bayer_rggb>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<bayer_bggr16>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<bayer_gbrg16>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<bayer_grbg16>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<bayer_rggb16>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<bayer_bggr16be>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<bayer_gbrg16be>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<bayer_grbg16be>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<bayer_rggb16be>
      {
	enum { is_pod = 1 };
      };
      template<> struct is_POD<yuv411>
      {
	enum { is_pod = 1 };
      };
    }
#endif
}

#endif
