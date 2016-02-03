
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLTypes_DEFINED
#define GrGLTypes_DEFINED

#include "GrGLConfig.h"

/**
 * Classifies GL contexts by which standard they implement (currently as OpenGL vs. OpenGL ES).
 */
enum GrGLStandard {
    kNone_GrGLStandard,
    kGL_GrGLStandard,
    kGLES_GrGLStandard,
};
static const int kGrGLStandardCnt = 3;

///////////////////////////////////////////////////////////////////////////////

/**
 * Declares typedefs for all the GL functions used in GrGLInterface
 */

typedef unsigned int GrGLenum;
typedef unsigned char GrGLboolean;
typedef unsigned int GrGLbitfield;
typedef signed char GrGLbyte;
typedef char GrGLchar;
typedef short GrGLshort;
typedef int GrGLint;
typedef int GrGLsizei;
typedef int64_t GrGLint64;
typedef unsigned char GrGLubyte;
typedef unsigned short GrGLushort;
typedef unsigned int GrGLuint;
typedef uint64_t GrGLuint64;
typedef float GrGLfloat;
typedef float GrGLclampf;
typedef double GrGLdouble;
typedef double GrGLclampd;
typedef void GrGLvoid;
#ifndef SK_IGNORE_64BIT_OPENGL_CHANGES
#ifdef _WIN64
typedef signed long long int GrGLintptr;
typedef signed long long int GrGLsizeiptr;
#else
typedef signed long int GrGLintptr;
typedef signed long int GrGLsizeiptr;
#endif
#else
typedef signed long int GrGLintptr;
typedef signed long int GrGLsizeiptr;
#endif
typedef void* GrGLeglImage;
/**
 * EGL types.
 */

typedef void* GrEGLImage;
typedef void* GrEGLDisplay;
typedef void* GrEGLContext;
typedef void* GrEGLClientBuffer;
typedef unsigned int GrEGLenum;
typedef int32_t GrEGLint;
typedef unsigned int GrEGLBoolean;

///////////////////////////////////////////////////////////////////////////////
/**
 * Types for interacting with GL resources created externally to Skia. GrBackendObjects for GL
 * textures are really const GrGLTexture*
 */

struct GrGLTextureInfo {
    GrGLenum fTarget;
    GrGLuint fID;
};

GR_STATIC_ASSERT(sizeof(GrBackendObject) >= sizeof(const GrGLTextureInfo*));

#endif
