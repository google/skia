/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLUtil_DEFINED
#define GrGLUtil_DEFINED

#include "gl/GrGLInterface.h"
#include "GrGLDefines.h"
#include "GrStencilSettings.h"

class SkMatrix;

////////////////////////////////////////////////////////////////////////////////

typedef uint32_t GrGLVersion;
typedef uint32_t GrGLSLVersion;
typedef uint32_t GrGLDriverVersion;

#define GR_GL_VER(major, minor) ((static_cast<int>(major) << 16) | \
                                 static_cast<int>(minor))
#define GR_GLSL_VER(major, minor) ((static_cast<int>(major) << 16) | \
                                   static_cast<int>(minor))
#define GR_GL_DRIVER_VER(major, minor) ((static_cast<int>(major) << 16) | \
                                        static_cast<int>(minor))

#define GR_GL_INVALID_VER GR_GL_VER(0, 0)
#define GR_GLSL_INVALID_VER GR_GLSL_VER(0, 0)
#define GR_GL_DRIVER_UNKNOWN_VER GR_GL_DRIVER_VER(0, 0)

/**
 * The Vendor and Renderer enum values are lazily updated as required.
 */
enum GrGLVendor {
    kARM_GrGLVendor,
    kImagination_GrGLVendor,
    kIntel_GrGLVendor,
    kQualcomm_GrGLVendor,
    kNVIDIA_GrGLVendor,
    kATI_GrGLVendor,

    kOther_GrGLVendor
};

enum GrGLRenderer {
    kTegra2_GrGLRenderer,
    kTegra3_GrGLRenderer,
    kPowerVR54x_GrGLRenderer,
    kPowerVRRogue_GrGLRenderer,
    kAdreno3xx_GrGLRenderer,
    kAdreno4xx_GrGLRenderer,
    kOSMesa_GrGLRenderer,
    kOther_GrGLRenderer
};

enum GrGLDriver {
    kMesa_GrGLDriver,
    kChromium_GrGLDriver,
    kNVIDIA_GrGLDriver,
    kIntel_GrGLDriver,
    kANGLE_GrGLDriver,
    kUnknown_GrGLDriver
};

////////////////////////////////////////////////////////////////////////////////

/**
 *  Some drivers want the var-int arg to be zero-initialized on input.
 */
#define GR_GL_INIT_ZERO     0
#define GR_GL_GetIntegerv(gl, e, p)                                            \
    do {                                                                       \
        *(p) = GR_GL_INIT_ZERO;                                                \
        GR_GL_CALL(gl, GetIntegerv(e, p));                                     \
    } while (0)

#define GR_GL_GetFramebufferAttachmentParameteriv(gl, t, a, pname, p)          \
    do {                                                                       \
        *(p) = GR_GL_INIT_ZERO;                                                \
        GR_GL_CALL(gl, GetFramebufferAttachmentParameteriv(t, a, pname, p));   \
    } while (0)

#define GR_GL_GetNamedFramebufferAttachmentParameteriv(gl, fb, a, pname, p)          \
    do {                                                                             \
        *(p) = GR_GL_INIT_ZERO;                                                      \
        GR_GL_CALL(gl, GetNamedFramebufferAttachmentParameteriv(fb, a, pname, p));   \
    } while (0)

#define GR_GL_GetRenderbufferParameteriv(gl, t, pname, p)                      \
    do {                                                                       \
        *(p) = GR_GL_INIT_ZERO;                                                \
        GR_GL_CALL(gl, GetRenderbufferParameteriv(t, pname, p));               \
    } while (0)

#define GR_GL_GetTexLevelParameteriv(gl, t, l, pname, p)                       \
    do {                                                                       \
        *(p) = GR_GL_INIT_ZERO;                                                \
        GR_GL_CALL(gl, GetTexLevelParameteriv(t, l, pname, p));                \
    } while (0)

#define GR_GL_GetShaderPrecisionFormat(gl, st, pt, range, precision)           \
    do {                                                                       \
        (range)[0] = GR_GL_INIT_ZERO;                                          \
        (range)[1] = GR_GL_INIT_ZERO;                                          \
        (*precision) = GR_GL_INIT_ZERO;                                        \
        GR_GL_CALL(gl, GetShaderPrecisionFormat(st, pt, range, precision));    \
    } while (0)

////////////////////////////////////////////////////////////////////////////////

/**
 * Helpers for glGetString()
 */

// these variants assume caller already has a string from glGetString()
GrGLVersion GrGLGetVersionFromString(const char* versionString);
GrGLStandard GrGLGetStandardInUseFromString(const char* versionString);
GrGLSLVersion GrGLGetGLSLVersionFromString(const char* versionString);
GrGLVendor GrGLGetVendorFromString(const char* vendorString);
GrGLRenderer GrGLGetRendererFromString(const char* rendererString);

void GrGLGetDriverInfo(GrGLStandard standard,
                       GrGLVendor vendor,
                       const char* rendererString,
                       const char* versionString,
                       GrGLDriver* outDriver,
                       GrGLDriverVersion* outVersion);

// these variants call glGetString()
GrGLVersion GrGLGetVersion(const GrGLInterface*);
GrGLSLVersion GrGLGetGLSLVersion(const GrGLInterface*);
GrGLVendor GrGLGetVendor(const GrGLInterface*);
GrGLRenderer GrGLGetRenderer(const GrGLInterface*);


/**
 * Helpers for glGetError()
 */

void GrGLCheckErr(const GrGLInterface* gl,
                  const char* location,
                  const char* call);

void GrGLClearErr(const GrGLInterface* gl);

////////////////////////////////////////////////////////////////////////////////

/**
 * Macros for using GrGLInterface to make GL calls
 */

// internal macro to conditionally call glGetError based on compile-time and
// run-time flags.
#if GR_GL_CHECK_ERROR
    extern bool gCheckErrorGL;
    #define GR_GL_CHECK_ERROR_IMPL(IFACE, X)                    \
        if (gCheckErrorGL)                                      \
            GrGLCheckErr(IFACE, GR_FILE_AND_LINE_STR, #X)
#else
    #define GR_GL_CHECK_ERROR_IMPL(IFACE, X)
#endif

// internal macro to conditionally log the gl call using SkDebugf based on
// compile-time and run-time flags.
#if GR_GL_LOG_CALLS
    extern bool gLogCallsGL;
    #define GR_GL_LOG_CALLS_IMPL(X)                             \
        if (gLogCallsGL)                                        \
            SkDebugf(GR_FILE_AND_LINE_STR "GL: " #X "\n")
#else
    #define GR_GL_LOG_CALLS_IMPL(X)
#endif

// makes a GL call on the interface and does any error checking and logging
#define GR_GL_CALL(IFACE, X)                                    \
    do {                                                        \
        GR_GL_CALL_NOERRCHECK(IFACE, X);                        \
        GR_GL_CHECK_ERROR_IMPL(IFACE, X);                       \
    } while (false)

// Variant of above that always skips the error check. This is useful when
// the caller wants to do its own glGetError() call and examine the error value.
#define GR_GL_CALL_NOERRCHECK(IFACE, X)                         \
    do {                                                        \
        (IFACE)->fFunctions.f##X;                               \
        GR_GL_LOG_CALLS_IMPL(X);                                \
    } while (false)

// same as GR_GL_CALL but stores the return value of the gl call in RET
#define GR_GL_CALL_RET(IFACE, RET, X)                           \
    do {                                                        \
        GR_GL_CALL_RET_NOERRCHECK(IFACE, RET, X);               \
        GR_GL_CHECK_ERROR_IMPL(IFACE, X);                       \
    } while (false)

// same as GR_GL_CALL_RET but always skips the error check.
#define GR_GL_CALL_RET_NOERRCHECK(IFACE, RET, X)                \
    do {                                                        \
        (RET) = (IFACE)->fFunctions.f##X;                       \
        GR_GL_LOG_CALLS_IMPL(X);                                \
    } while (false)

// call glGetError without doing a redundant error check or logging.
#define GR_GL_GET_ERROR(IFACE) (IFACE)->fFunctions.fGetError()

GrGLenum GrToGLStencilFunc(GrStencilTest test);


#endif
