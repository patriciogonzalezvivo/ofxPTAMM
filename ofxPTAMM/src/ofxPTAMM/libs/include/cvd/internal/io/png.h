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
#ifndef CVD_INTERNAL_IO_PNG_H
#define CVD_INTERNAL_IO_PNG_H
#include <iostream>
#include <vector>
#include <string>

#include <cvd/image.h>
#include <cvd/internal/load_and_save.h>
#include <cvd/internal/convert_pixel_types.h>

struct png_struct_def;
struct png_info_struct;


namespace CVD{
namespace PNG{

using CVD::Internal::TypeList;
using CVD::Internal::Head;



class png_reader
{
	public:
		png_reader(std::istream&);
		~png_reader();

		ImageRef size();

		void get_raw_pixel_line(bool*);
		void get_raw_pixel_line(unsigned char*);
		void get_raw_pixel_line(unsigned short*);
		void get_raw_pixel_line(Rgb<unsigned char>*);
		void get_raw_pixel_line(Rgb<unsigned short>*);
		void get_raw_pixel_line(Rgba<unsigned char>*);
		void get_raw_pixel_line(Rgba<unsigned short>*);

		std::string datatype();
		std::string name();

		typedef TypeList<bool,
				TypeList<byte,
		        TypeList<unsigned short,
		        TypeList<Rgb<byte>,
		        TypeList<Rgb<unsigned short>,
		        TypeList<Rgba<byte>,
		        TypeList<Rgba<unsigned short>,
				                              Head> > > > > > > Types;

	private:
		
		std::istream& i;
		std::string type;
		unsigned long row;
		png_struct_def* png_ptr;
		png_info_struct* info_ptr, *end_info;

		std::string error_string;
		ImageRef my_size;

		template<class C> void read_pixels(C*);

};


////////////////////////////////////////////////////////////////////////////////
//
// How to convert Misc types in to PNG compatible types
//

//The range is encoded un unary notation. The range is on some integer, x.
//g1 is set if x > 1. g8 is set if x > 8 and so on.
//This allows us to choose a type with a reasonable number of bits.
template<int g1, int g8> struct IntMapper    { typedef unsigned short type;};
template<>               struct IntMapper<1, 0> { typedef unsigned char type; };
template<>               struct IntMapper<0, 0> { typedef bool type; };


//Mapping for integral types
template<class ComponentIn, int is_integral> struct ComponentMapper_
{
	typedef typename IntMapper<
								(Pixel::traits<ComponentIn>::bits_used > 1),
								(Pixel::traits<ComponentIn>::bits_used > 8)
								>::type type;
};


//Mapping for non integral types
template<class ComponentIn> struct ComponentMapper_<ComponentIn, 0> { typedef unsigned short type; };

template<class ComponentIn> struct ComponentMapper
{
	typedef typename ComponentMapper_<ComponentIn, Pixel::traits<ComponentIn>::integral>::type type;
};


//Mapping for Rgbish types
template<class ComponentIn> struct ComponentMapper<Rgb<ComponentIn> >
{
	typedef Rgb<typename ComponentMapper_<ComponentIn, Pixel::traits<ComponentIn>::integral>::type> type;
};

template<class ComponentIn> struct ComponentMapper<Rgba<ComponentIn> >
{
	typedef Rgba<typename ComponentMapper_<ComponentIn, Pixel::traits<ComponentIn>::integral>::type> type;
};

template<> struct ComponentMapper<Rgb8>
{
	typedef Rgb8 type;
};




class png_writer
{
	public:
		png_writer(std::ostream&, ImageRef size, const std::string& type, const std::map<std::string, Parameter<> >& p);
		~png_writer();

		void write_raw_pixel_line(const bool*);
		void write_raw_pixel_line(const unsigned char*);
		void write_raw_pixel_line(const unsigned short*);
		void write_raw_pixel_line(const Rgb<unsigned char>*);
		void write_raw_pixel_line(const Rgb8*);
		void write_raw_pixel_line(const Rgb<unsigned short>*);
		void write_raw_pixel_line(const Rgba<unsigned char>*);
		void write_raw_pixel_line(const Rgba<unsigned short>*);

		template<class Incoming> struct Outgoing
		{		
			typedef typename ComponentMapper<Incoming>::type type;
		};		
	private:

		template<class P> void write_line(const P*);

	long row;
	std::ostream& o;
	ImageRef size;
	std::string type;
	std::string error_string;

	png_struct_def* png_ptr;
	png_info_struct* info_ptr, *end_info;

};


}}
#endif
