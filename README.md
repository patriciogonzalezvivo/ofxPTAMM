#ofxPTAMM#

This addon it´s based on @Akira_At_Asia ofxPTAM addon ( https://github.com/Akira-Hayasaka/ofxPTAM ). Insted of using PTAM developed by Georg Klein it use PTAMM by Robert Castle ( http://www.robots.ox.ac.uk/~bob/research/research_ptamm.html ) witch manage multiple maps and let you save/load them.

##Build your own addon libraries##
This addon it have been build and tested over MacOSX 10.6, MacOSX 10.7 and iOS 5. If you have another OS like Linux, or the provide one´s doesn´t work for you here it´s an explanation on how to build by your own.
 
- First you need to be able to compile PTAMM sources. I had a detailed tutorial for MacOSX 10.6 and 10.7 at: http://www.patriciogonzalezvivo.com/blog/?p=547 . Doing some google research you probably found what you need´s to compile it on your OS

- Once you know that you can compile and run it, let´s move the following PTAMM sources to the ```ofxPTAMM/include/PTAMM/``` ( if you don´t have one, just make it).

  - ATANCamera.cc
  - ATANCamera.h
  - Bundle.cc
  - Bundle.h
  - CalibCornerPatch.cc
  - CalibCornerPatch.h
  - HomographyIni.cc
  - HomographyIni.h
  - KeyFrame.cc
  - KeyFrame.h
  - LevelHelpers.h
  - LICENSE.txt
  - Map.cc
  - Map.h
  - MapLockManager.cc
  - MapLockManager.h
  - MapMaker.cc
  - MapMaker.h
  - MapPoint.cc
  - MapPoint.h
  - MapSerializer.cc
  - MapSerializar.h
  - MD5.cc
  - MD5.h
  - MD5Wrapper.cc
  - MD5Wrapper.h
  - MEstimator.h
  - MiniPatch.cc
  - MiniPatch.h
  - OpenGL.h
  - PatchFinder.cc
  - PatchFinder.h
  - Relocaliser.cc
  - Relocalizer.h
  - ShiTomasi.cc
  - ShiTomasi.h
  - SmallBlurryImage.cc
  - SmallBlurryImage.h
  - SmallMatrixOpts.h
  - tinyxml_license.txt
  - tinyxml.cc
  - tinyxml.h
  - tinyxmlerror.cc
  - tinyxmlparser.cc
  - Tracker.cc
  - Tracker.h
  - TrackerData.h
  - Utils.cc
  - Utils.h

- After that you need to copy the requiered libraries inside it´s own ```include``` directory. Note: That gvars3 it´s changed from the original. I take out some GUI classes and add it some definition sources in order to make it smaller and portable to iOS. If you are going to use the compiled libraries. Just copy them from your local ```include``` directory and do the same with the compiled libraries from the ```lib``` directory. Probably you want to do some thing like this:

<pre>
cd ofxPTAMM
cp -r /usr/local/include/cvd include/
cp -r /usr/local/include/gvars3 include/
cp -r /usr/local/include/TooN include/
cp -r /usr/local/lib/libcvd-0.8.dylib libs/osx/
cp -r /usr/local/lib/libGVars3-0.6.dylib libs/osx/
</pre>

- Probably you want to delet soureces form other platforms like ```ofxPTAMM/include/cvd/Build```, ```ofxPTAMM/include/cvd/Linux``` . Any way if you leave sources from other platforms and they are giving you a headache you coud use pre-compilers definitions to look the up. For example on  ```ofxPTAMM/include/cvd/Linux/capture_logic.cxx``` you can put the  code between:

```c++
#ifdef LINUX

    while(vd.pending())
    
    // ... all the code
    
    }

#endif
```

- I don´t know exactly way but sometimes it´s need to add ```#undef``` check on the begining of ```ofxPTAMM/include/TooN/TooN.h```. So at the end will look something like this

```c++
#ifdef check
#undef check
#endif

#ifndef TOON_INCLUDE_TOON_H
#define TOON_INCLUDE_TOON_H
#include <iostream>
// ...
```

- Change the path to ```gl.h``` and ```glext.h``` headers files on both ```ofxPTAMM/include/PTAMM/OpenGL.h``` and on ```ofxPTAMM/include/cvd/gl_helpers.h``` files in order to match with your OS specifications.

```c++
#ifdef _LINUX
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#ifdef TARGET_OSX
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#endif

#ifdef TARGET_OPENGLES
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <GL/glew.h>
#endif
```

- Replace all Point type calls for BPoint on ```ofxPTAM/include/Bundle.h``` and ```ofxPTAMM/include/Bundle.cc```
		
##Adding ofxPTAMM addon to a oF project##

- Add the `includes` on the `libs` directory at the Project.xcconfig

```c++
//THE PATH TO THE ROOT OF OUR OF PATH RELATIVE TO THIS PROJECT.
//THIS NEEDS TO BE DEFINED BEFORE CoreOF.xcconfig IS INCLUDED
OF_PATH = ../../..

//THIS HAS ALL THE HEADER AND LIBS FOR OF CORE
#include "../../../libs/openFrameworksCompiled/project/osx/CoreOF.xcconfig"

OTHER_LDFLAGS = $(OF_CORE_LIBS)
HEADER_SEARCH_PATHS = $(OF_CORE_HEADERS) $(OF_PATH)/addons/ofxPTAMM/libs/cvd/include $(OF_PATH)/addons/ofxPTAMM/libs/gvars3/include $(OF_PATH)/addons/ofxPTAMM/libs/PTAMM/include $(OF_PATH)/addons/ofxPTAMM/libs/TooN/include
```

- Add vecLib.framework. Here is how to add new frameworks http://meandmark.com/blog/2011/03/xcode-4-adding-a-framework-to-your-project/

- Finaly you need to copy ```ofxPTAMM/include/camera.cfg``` to ```bin/data``` directory. Needless to say that you need to calibrate your camera using PTAMM CameraCalibration. New ofxPTAMM versions have a camera-calibration class. After optimizing you need to manualy copy the results parameters to the camera.cfg   