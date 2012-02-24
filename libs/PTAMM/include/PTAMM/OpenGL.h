// Copyright 2008 Isis Innovation Limited
#ifndef __OPENGL_INCLUDES_H
#define __OPENGL_INCLUDES_H

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

#endif