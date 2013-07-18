#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

#include <cvd/config.h>

#include <cvd/colourspacebuffer.h>
#include <cvd/colourspaces.h>
#include <cvd/videobufferwithdata.h>
#include <cvd/readaheadvideobuffer.h>

#include <cvd/diskbuffer2.h>
#include <cvd/serverpushjpegbuffer.h>

namespace CVD {
	struct ParseException : public Exceptions::All
	{
		ParseException(const std::string& what_) { what = what_; }
	};
	
	struct VideoSourceException : public Exceptions::All
	{
		VideoSourceException(const std::string& what_) { what = what_; }
	};

	struct VideoSource 
	{
		std::string protocol;
		std::string identifier;
		typedef std::vector<std::pair<std::string,std::string> > option_list;
		option_list options;
	};

	std::ostream& operator<<(std::ostream& out, const VideoSource& vs);

	void parse(std::istream& in, VideoSource& vs);
	template <class T> VideoBuffer<T>* open_video_source(const std::string& src);

	////////////////////////////////////////////////////////////////////////////////
	//
	// Server push JPEG buffer
	//

	template<class T, bool Implemented = Pixel::DefaultConvertible<T>::is> struct makeJPEGStream
	{
		static VideoBuffer<T>* make(const std::string& filename)
		{
			using std::auto_ptr;
			using std::ifstream;

			auto_ptr<std::ifstream> stream(new ifstream(filename.c_str()));
			
			auto_ptr<VideoBuffer<T> > buf(static_cast<VideoBuffer<T>*>(new ServerPushJpegBuffer<T>(*stream)));
			return new VideoBufferWithData<T, std::ifstream>(buf, stream);
		}
	};

	template<class T> struct makeJPEGStream<T, false>
	{
		static VideoBuffer<T>* make(const std::string&)
		{
			throw VideoSourceException("ServerPushJpegBuffer cannot handle type " + PNM::type_name<T>::name());
		}
	};

	void get_jpegstream_options(const VideoSource& vs, int& fps);

 
    ////////////////////////////////////////////////////////////////////////////////
	//
	// Colourspace conversion buffer
	//
    
	void get_colourspace_options(const VideoSource& vs, std::string& from);

	template<class Out, class In, bool can_convert> struct MakeConverter{
		static VideoBuffer<Out>* make(const std::string& r)
		{
			std::auto_ptr<VideoBuffer<In> > buf  = std::auto_ptr<VideoBuffer<In> > (static_cast<VideoBuffer<In>*>(open_video_source<In>(r)));
			std::auto_ptr<VideoBuffer<Out> > cvt = std::auto_ptr<VideoBuffer<Out> >(static_cast<VideoBuffer<Out>*>( new ColourspaceBuffer<Out, In>(*buf)));
			return new VideoBufferWithData<Out, VideoBuffer<In> >(cvt, buf);
		}
	};

	template<class Out, class In> struct MakeConverter<Out, In, false>
	{
		static VideoBuffer<Out>* make(const std::string&)
		{
			throw VideoSourceException("ColorspaceBuffer cannot convert from " + PNM::type_name<In>::name() + " to " + PNM::type_name<Out>::name());
		}
	};

	template<class T> struct MakeConverter<T, T, true>{
		static VideoBuffer<T>* make(const std::string& r)
		{
			return open_video_source<T>(r);
		}
	};

	template<class Out, class In> VideoBuffer<Out>* makeConvertBufferBit(const std::string& r)
	{
		return MakeConverter<Out, In, IsConvertible<In, Out>::is >::make(r);
	};

	template<class T> VideoBuffer<T>* makeColourspaceBuffer(const std::string& c, const std::string& r)
	{

		if(c == "byte" || c == "mono" || c == "gray" || c == "grey")
			return makeConvertBufferBit<T, byte>(r);
		else if(c == "rgb<byte>" || c == "rgb")
			return makeConvertBufferBit<T, Rgb<byte> >(r);
		else if(c == "yuv411")
			return makeConvertBufferBit<T, yuv411>(r);
		else if(c == "yuv422")
			return makeConvertBufferBit<T, yuv422>(r);
		else if(c == "yuv420p")
			return makeConvertBufferBit<T, yuv420p>(r);
		else if(c == "vuy422")
			return makeConvertBufferBit<T, vuy422>(r);
		else if(c == "bayer_bggr")
			return makeConvertBufferBit<T, bayer_bggr>(r);
		else if(c == "bayer_gbrg")
			return makeConvertBufferBit<T, bayer_gbrg>(r);
		else if(c == "bayer_grbg")
			return makeConvertBufferBit<T, bayer_grbg>(r);
		else if(c == "bayer_rggb")
			return makeConvertBufferBit<T, bayer_rggb>(r);
		else if(c == "bayer_bggr16be")
			return makeConvertBufferBit<T, bayer_bggr16be>(r);
		else if(c == "bayer_gbrg16be")
			return makeConvertBufferBit<T, bayer_gbrg16be>(r);
		else if(c == "bayer_grbg16be")
			return makeConvertBufferBit<T, bayer_grbg16be>(r);
		else if(c == "bayer_rggb16be")
			return makeConvertBufferBit<T, bayer_rggb16be>(r);
		else
			throw  VideoSourceException("ColorspaceBuffer cannot handle type " + c);
	}

	////////////////////////////////////////////////////////////////////////////////
	//
	// DiskBuffer2 buffer
	//

#ifdef CVD_HAVE_GLOB
	template<class T, bool Implemented = Pixel::DefaultConvertible<T>::is> struct makeDiskBuffer2
	{
		static VideoBuffer<T>* make(const std::vector<std::string>& files, double fps, VideoBufferFlags::OnEndOfBuffer eob)
		{
			return new DiskBuffer2<T>(files, fps, eob);    
		}
	};

	template<class T> struct makeDiskBuffer2<T, false>
	{
		static VideoBuffer<T>* make(const std::vector<std::string>& , double , VideoBufferFlags::OnEndOfBuffer)
		{
			throw VideoSourceException("DiskBuffer2 cannot handle type " + PNM::type_name<T>::name());
		}
	};

#endif

	void get_files_options(const VideoSource& vs, int& fps, int& ra_frames, VideoBufferFlags::OnEndOfBuffer& eob);
	
	////////////////////////////////////////////////////////////////////////////////
	//
	// v4l1 buffer
	//

	template <class T> VideoBuffer<T>* makeV4L1Buffer(const std::string&, const ImageRef& )
	{
		throw VideoSourceException("V4L1Buffer cannot handle types other than byte, bayer, yuv422, Rgb<byte>");
	}

	template <> VideoBuffer<byte>* makeV4L1Buffer(const std::string& dev, const ImageRef& size);
	template <> VideoBuffer<bayer>* makeV4L1Buffer(const std::string& dev, const ImageRef& size);
	template <> VideoBuffer<yuv422>* makeV4L1Buffer(const std::string& dev, const ImageRef& size);
	template <> VideoBuffer<Rgb<byte> >* makeV4L1Buffer(const std::string& dev, const ImageRef& size);

	void get_v4l1_options(const VideoSource& vs, ImageRef& size);

	////////////////////////////////////////////////////////////////////////////////
	//
	// v4l buffer
	//

	template <class T> VideoBuffer<T>* makeV4LBuffer(const std::string&, const ImageRef&, int, bool, bool)
	{
		throw VideoSourceException("V4LBuffer cannot handle types other than byte, bayer, yuv422, vuy422, Rgb<byte>");
	}

	template <> VideoBuffer<byte>* makeV4LBuffer(const std::string& dev, const ImageRef& size, int input, bool interlaced, bool verbose);
	template <> VideoBuffer<bayer_grbg>* makeV4LBuffer(const std::string& dev, const ImageRef& size, int input, bool interlaced, bool verbose);
	template <> VideoBuffer<yuv422>* makeV4LBuffer(const std::string& dev, const ImageRef& size, int input, bool interlaced, bool verbose);
	template <> VideoBuffer<vuy422>* makeV4LBuffer(const std::string& dev, const ImageRef& size, int input, bool interlaced, bool verbose);
	template <> VideoBuffer<Rgb<byte> >* makeV4LBuffer(const std::string& dev, const ImageRef& size, int input, bool interlaced, bool verbose);
	template <> VideoBuffer<Rgb8>* makeV4LBuffer(const std::string& dev, const ImageRef& size, int input, bool interlaced, bool verbose);

	void get_v4l2_options(const VideoSource& vs, ImageRef& size, int& input, bool& interlaced, bool& verbose);


	////////////////////////////////////////////////////////////////////////////////
	//
	// video file buffer
	//

	template <class T> VideoBuffer<T>* makeVideoFileBuffer(const std::string& , VideoBufferFlags::OnEndOfBuffer )
	{
		throw VideoSourceException("VideoFileBuffer cannot handle types other than byte, Rgb<byte>");
	}
	
	template <> VideoBuffer<byte>* makeVideoFileBuffer(const std::string& file, VideoBufferFlags::OnEndOfBuffer eob);
	template <> VideoBuffer<Rgb<byte> >* makeVideoFileBuffer(const std::string& file, VideoBufferFlags::OnEndOfBuffer eob);

	void get_file_options(const VideoSource& vs, int& ra_frames, VideoBufferFlags::OnEndOfBuffer& eob);

	////////////////////////////////////////////////////////////////////////////////
	//
	// DC1394 buffer
	//
	template <class T> VideoBuffer<T>* makeDVBuffer2(int , ImageRef , float , ImageRef, bool, bool, int)
	{
		throw VideoSourceException("DVBuffer2 cannot handle " + PNM::type_name<T>::name());
	}
	
	template <> VideoBuffer<byte>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<unsigned short>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<yuv411>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<yuv422>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<Rgb<byte> >* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<bayer_bggr>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<bayer_gbrg>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<bayer_grbg>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<bayer_rggb>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<bayer_bggr16be>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<bayer_gbrg16be>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<bayer_grbg16be>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);
	template <> VideoBuffer<bayer_rggb16be>* makeDVBuffer2(int cam, ImageRef size, float fps, ImageRef offset, bool verbose, bool bus_reset, int format7_mode);

	void get_dc1394_options(const VideoSource& vs, ImageRef& size, float& fps, ImageRef& offset, bool& verbose, bool& bus_reset, int& format7_mode);

	////////////////////////////////////////////////////////////////////////////////
	//
	// QuickTime buffer
	//

	template <class T> VideoBuffer<T> * makeQTBuffer( const ImageRef & , int , bool, bool )
	{
		throw VideoSourceException("QTBuffer cannot handle " + PNM::type_name<T>::name());
	}
	template <> VideoBuffer<vuy422> * makeQTBuffer( const ImageRef & size, int input, bool showsettings, bool verbose);
	template <> VideoBuffer<yuv422> * makeQTBuffer( const ImageRef & size, int input, bool showsettings, bool verbose);
	template <> VideoBuffer<Rgb<byte> > * makeQTBuffer( const ImageRef & size, int input, bool showsettings, bool verbose);
	
	void get_qt_options(const VideoSource & vs, ImageRef & size, bool & showsettings, bool & verbose);


	////////////////////////////////////////////////////////////////////////////////
	//
	// video source handler
	//

	template <class T> VideoBuffer<T>* open_video_source(const VideoSource& vs)
	{
		using std::auto_ptr;
		if(vs.protocol == "jpegstream")
		{
			int ra_frames=0;
			get_jpegstream_options(vs, ra_frames);

			auto_ptr<VideoBuffer<T> > jpeg_buffer(makeJPEGStream<T>::make(vs.identifier));

			if(ra_frames == 0)
				return jpeg_buffer.release();
			else
			{
				auto_ptr<VideoBuffer<T> > b(new ReadAheadVideoBuffer<T>(*(jpeg_buffer.get()), ra_frames));
				return new VideoBufferWithData<T, VideoBuffer<T> >(b, jpeg_buffer);
			}
		}
		else if(vs.protocol == "colourspace")
		{
			std::string from = "byte";
			get_colourspace_options(vs, from);

			return makeColourspaceBuffer<T>(from, vs.identifier);
		}
		else if (vs.protocol == "files") {
			int fps, ra_frames=0;
			VideoBufferFlags::OnEndOfBuffer eob;
			get_files_options(vs, fps, ra_frames, eob);
			VideoBuffer<T>* vb = makeDiskBuffer2<T>::make(globlist(vs.identifier), fps, eob);
			if (ra_frames)
				vb = new ReadAheadVideoBuffer<T>(*vb, ra_frames);
			return vb;
		}
		else if (vs.protocol == "v4l1") {
			ImageRef size;
			get_v4l1_options(vs, size);
			return makeV4L1Buffer<T>(vs.identifier, size);
		} 
		else if (vs.protocol == "v4l2") {
			ImageRef size;
			int input;
			bool interlaced, verbose;
			get_v4l2_options(vs, size, input, interlaced, verbose);
			return makeV4LBuffer<T>(vs.identifier, size, input, interlaced, verbose);	
		} 
		else if (vs.protocol == "dc1394") {
			int cam_no = atoi(vs.identifier.c_str());
			ImageRef size, offset;
			float fps;
			bool verbose;
			bool bus_reset;
			int  format7_mode;
			get_dc1394_options(vs, size, fps, offset, verbose, bus_reset, format7_mode);
			return makeDVBuffer2<T>(cam_no, size, fps, offset, verbose, bus_reset, format7_mode);
		} 
		else if (vs.protocol == "file") {
			int ra_frames = 0;
			VideoBufferFlags::OnEndOfBuffer eob;
			get_file_options(vs, ra_frames, eob);
			VideoBuffer<T>* vb = makeVideoFileBuffer<T>(vs.identifier, eob);
			if (ra_frames)
				vb = new ReadAheadVideoBuffer<T>(*vb, ra_frames);
			return vb;
		} 
	else if (vs.protocol == "qt") {
		ImageRef size;
		bool showsettings, verbose;
		int input = atoi(vs.identifier.c_str());
		get_qt_options(vs, size, showsettings, verbose);
		return makeQTBuffer<T>(size, input, showsettings, verbose);
	}
		else
			throw VideoSourceException("undefined video source protocol: '" + vs.protocol + "'\n\t valid protocols: "
									   "colourspace, jpegstream, "
									   "file, "
									   "v4l2, "
									   "v4l1, "
									   "dc1394, "
									   "qt, "
									   "files"
									   );
	}

/**
opens a video device described by a video source url given from an input stream. See
@ref open_video_source(const std::string &) for details on the url syntax.

@ingroup gVideo
*/
	template <class T> VideoBuffer<T>* open_video_source(std::istream& in)
	{
		VideoSource vs;
		parse(in, vs);
		return open_video_source<T>(vs);
	}

/**
opens a video device described by a video source url. This allows to decide at
runtime what kind of video input your program is using. Basic use is to call
open_video_source<T>(url) to get a VideoBuffer<T>*.


In many cases, the VideoBuffer returned by open_video_source() is a wrapper
around the video buffer dealing with the hardware and so does not provide 
access to the controls. The underlying buffer can be accessed with 
VideoBuffer::root_buffer().

The url syntax is the following:
@verbatim
url		 := protocol ':' [ '[' options ']' ] // identifier
protocol := "files" | "file" | "v4l2" | "v4l1" | "jpegstream" | "dc1394" | "qt" | "colourspace"
options  := option [ ',' options ]
option	 := name [ '=' value ]
@endverbatim

identifier and values can be quoted literals with escapes, all other text is unquoted.
Some Examples:

Open a DiskBuffer2 for *.pgm in /local/capture/:
@verbatim
files:///local/capture/ *.pgm
@endverbatim

Open a DiskBuffer2 that loops and uses a ReadAheadVideoBuffer wrapper with 40 frame buffer, with 30 fps:
@verbatim
files:[read_ahead=40, fps=30, on_end=loop]///local/capture/ *.pgm
@endverbatim

Open a V4L2 device at /dev/video0:
@verbatim
v4l2:///dev/video0
@endverbatim
   
Open a V4L2 device with fields on input 2:
@verbatim
v4l2:[input=2,fields]///dev/video0
@endverbatim

Open firewire camera 1 with the default fps:
@verbatim
dc1394://1
@endverbatim

Open firewire camera 1, capturing in YUV411, at 30fps:
@verbaitm
colourspace:[from=yuv411]//dc1394:[fps=30]//1
@endverbatim

Open an avi file relative to the current directory:
@verbatim
file://../../stuff/movie.avi
@endverbatim

Open the first QuickTime camera and show the settings dialog
@verbatim
qt:[showsettings=1]//0
@endverbatim

Open an HTTP camera. First create a named pipe from the shell, 
and start grabbing video:
@verbatim
mkfifo /tmp/video
wget http//my.camera/file_representing_video -O /tmp/video
@endverbatim
then open a source with:
@verbatim
jpegstream:///tmp/video
@endverbatim
If the argument is provided from a shell such as BASH, then then
redirection can be used:
@verbatim
jpegstream://<(wget http//my.camera/file_representing_video -O - )
@endverbatim

Fields are:
bool = true | yes | 1 ! false | no | 0
offset = <width>x<height>
size = <offset> | qvga | vga | pal | ntsc | xga

Options supported by the various protocols are:
@verbatim
'files' protocol (DiskBuffer2):  identifier is glob pattern
		fps = <number>
		read_ahead [= <number>] (default is 50 if specified without value)
		on_end = repeat_last | unset_pending | loop (default is repeat_last)

'file' protocol (VideoFileBuffer): identifier is path to file
	   read_ahead  [= <number>] (default is 50 if specified without value)
	   on_end = repeat_last | unset_pending | loop (default is repeat_last)

'v4l1' protocol (V4L1Buffer): identifier is device name
	   size = <size>  (default is 0x0)

'v4l2' protocol (V4LBuffer): identifier is device name
	   size = <size> (default vga)
	   input = <number>
	   interlaced | fields [= <bool> ]
		   verbose [ = <bool> ]

'dc1394' protocol (DVBuffer): identifier is camera number
	   fps = <number> (default is camera default)
	   size = <size>
	   offset = <offset>
	   verbose [ = <bool> ]
	   reset [ = <bool> ]
	   mode | format7 | format7_mode = <number>

'qt' protocol (QTBuffer): identifier is camera number
	  size = vga | qvga | <width>x<height>	(default vga)
	  showsettings [ = <bool> ] (default 0)
	  verbose [ = <bool> ] (default 0)

'jpegstream' protocol (ServerPushJpegBuffer): identifier is path to file
	  read_ahead  [= <number>] (default is 50 if specified without value)

'colourspace' protcol (ColourspaceBuffer): identifier is a video URL
      from = byte | mono | gray | grey | yuv411 | yuv422 | rgb<byte> 
	         | rgb | bayer_bggr | bayer_gbrg | bayer_grbg | bayer_rggb  (default mono)

@endverbatim

@ingroup gVideo
*/
	template <class T> VideoBuffer<T>* open_video_source(const std::string& src)
	{
		std::istringstream in(src);
		return open_video_source<T>(in);
	}	 
}

#endif
