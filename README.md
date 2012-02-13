#ofxPTAMM#

This addon it´s based on @Akira_At_Asia ofxPTAM addon. Insted of using PTAM developed by Georg Klein it use PTAMM by Robert Castle.

First you need to be able to compile PTAMM sources. I had a detailed tutorail for MacOSX 10.6 and 10.7 at: http://www.patriciogonzalezvivo.com/blog/?p=547

##Prepare everything for seting your addon##

- Move PTAMM directory inside ofxPTAMM addon and rename it ```include```. So at the end every PTAMM source could be found at ```ofxPTAMM/include/```

- Copy need headers into ```include``` directory and replace your compiled libraries.

<pre>
cd ofxPTAMM
cp -r /usr/local/include/cvd include/
cp -r /usr/local/include/gvars3 include/
cp -r /usr/local/include/TooN include/
cp -r /usr/local/lib/libcvd-0.8.dylib libs/osx/
cp -r /usr/local/lib/libGVars3-0.6.dylib libs/osx/
</pre>

- Add #undef check on the begining of ```ofxPTAMM/include/TooN/TooN.h```

```c++
#ifdef check
#undef check
#endif

#ifndef TOON_INCLUDE_TOON_H
#define TOON_INCLUDE_TOON_H
#include <iostream>
// ...

```

- On OSX replace ```GL/*.h``` for ```OpenGL/*.h``` on ```ofxPTAMM/include/cvd/gl_helpers.h```

```c++
//#include <GL/gl.h>
//#include <GL/glu.h>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

```

- Put ```ofxPTAMM/include/cvd/Linux/capture_logic.cxx``` code between:

```c++
#ifdef LINUX

    while(vd.pending())
    
    // ... all the code
    
    }

#endif
```

- Remove references to ```ofxPTAMM/include/cvd/Linux``` and ```ofxPTAMM/include/Build``` directories

- Replace all Point type calls for BPoint on ```ofxPTAM/include/Bundle.h``` and ```ofxPTAMM/include/Bundle.cc```
		
- Delete reference for main() 

<pre>
cd ofxPTAMM/include
rm main.cc
rm CammeraCalibration.h
rm CammeraCalibration.h
</pre>

##Adding ofxPTAMM addon to a oF project##

- Add the include and lib directory at the Project.xcconfig

```c++
//THE PATH TO THE ROOT OF OUR OF PATH RELATIVE TO THIS PROJECT.
//THIS NEEDS TO BE DEFINED BEFORE CoreOF.xcconfig IS INCLUDED
OF_PATH = ../../..

//THIS HAS ALL THE HEADER AND LIBS FOR OF CORE
#include "../../../libs/openFrameworksCompiled/project/osx/CoreOF.xcconfig"

OTHER_LDFLAGS = $(OF_CORE_LIBS)
HEADER_SEARCH_PATHS = $(OF_CORE_HEADERS) $(OF_PATH)/addons/ofxPTAMM/include
```

- Add vecLib.framework. Here is how to add new frameworks http://meandmark.com/blog/2011/03/xcode-4-adding-a-framework-to-your-project/

- Copy ```ofxPTAMM/include/camera.cfg``` to ```bin/data``` directory 