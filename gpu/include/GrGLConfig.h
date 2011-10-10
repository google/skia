
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGLConfig_DEFINED
#define GrGLConfig_DEFINED

#include "GrTypes.h"
#include "GrGLDefines.h"

/**
 * Optional GL config file.
 */
#ifdef GR_GL_CUSTOM_SETUP_HEADER
    #include GR_GL_CUSTOM_SETUP_HEADER
#endif

#if !defined(GR_GL_FUNCTION_TYPE)
    #define GR_GL_FUNCTION_TYPE
#endif

/**
 * The following are optional defines that can be enabled at the compiler
 * command line, in a IDE project, in a GrUserConfig.h file, or in a GL custom
 * file (if one is in use). If a GR_GL_CUSTOM_SETUP_HEADER is used they can
 * also be placed there.
 *
 * GR_GL_LOG_CALLS: if 1 Gr can print every GL call using GrPrintf. Defaults to
 * 0. Logging can be enabled and disabled at runtime using a debugger via to
 * global gLogCallsGL. The initial value of gLogCallsGL is controlled by
 * GR_GL_LOG_CALLS_START.
 *
 * GR_GL_LOG_CALLS_START: controls the initial value of gLogCallsGL when
 * GR_GL_LOG_CALLS is 1. Defaults to 0.
 *
 * GR_GL_CHECK_ERROR: if enabled Gr can do a glGetError() after every GL call.
 * Defaults to 1 if GR_DEBUG is set, otherwise 0. When GR_GL_CHECK_ERROR is 1
 * this can be toggled in a debugger using the gCheckErrorGL global. The initial
 * value of gCheckErrorGL is controlled by by GR_GL_CHECK_ERROR_START.
 *
 * GR_GL_CHECK_ERROR_START: controls the initial value of gCheckErrorGL
 * when GR_GL_CHECK_ERROR is 1.  Defaults to 1.
 *
 * GR_GL_NO_CONSTANT_ATTRIBUTES: if this evaluates to true then the GL backend
 * will use uniforms instead of attributes in all cases when there is not
 * per-vertex data. This is important when the underlying GL implementation
 * doesn't actually support immediate style attribute values (e.g. when 
 * the GL stream is converted to DX as in ANGLE on Chrome). Defaults to 0.
 *
 * GR_GL_ATTRIBUTE_MATRICES: If changing uniforms is very expensive it may be
 * faster to use vertex attributes for matrices (set via glVertexAttrib3fv). 
 * Setting this build flag enables this behavior. GR_GL_NO_CONSTANT_ATTRIBUTES
 * must not be set since this uses constant attributes for the matrices. 
 * Defaults to 0.
 *
 * GR_GL_USE_BUFFER_DATA_NULL_HINT: When specifing new data for a vertex/index
 * buffer that replaces old data Ganesh can give a hint to the driver that the
 * previous data will not be used in future draws like this:
 *  glBufferData(GL_..._BUFFER, size, NULL, usage);       //<--hint, NULL means
 *  glBufferSubData(GL_..._BUFFER, 0, lessThanSize, data) //   old data can't be
 *                                                        //   used again.
 * However, this can be an unoptimization on some platforms, esp. Chrome.
 * Chrome's cmd buffer will create a new allocation and memset the whole thing
 * to zero (for security reasons). Defaults to 1 (enabled).
 *
 * GR_GL_PER_GL_FUNC_CALLBACK: When set to 1 the GrGLInterface object provides
 * a function pointer that is called just before every gl function. The ptr must
 * be valid (i.e. there is no NULL check). However, by default the callback will
 * be set to a function that does nothing. The signature of the function is:
 *    void function(const GrGLInterface*)
 * It is not extern "C".
 * The GrGLInterface field fCallback specifies the function ptr and there is an
 * additional field fCallbackData of type intptr_t for client data.
 */

#if !defined(GR_GL_LOG_CALLS)
    #define GR_GL_LOG_CALLS                     GR_DEBUG
#endif

#if !defined(GR_GL_LOG_CALLS_START)
    #define GR_GL_LOG_CALLS_START               0
#endif

#if !defined(GR_GL_CHECK_ERROR)
    #define GR_GL_CHECK_ERROR                   GR_DEBUG
#endif

#if !defined(GR_GL_CHECK_ERROR_START)
    #define GR_GL_CHECK_ERROR_START             1
#endif

#if !defined(GR_GL_NO_CONSTANT_ATTRIBUTES)
    #define GR_GL_NO_CONSTANT_ATTRIBUTES        0
#endif

#if !defined(GR_GL_ATTRIBUTE_MATRICES)
    #define GR_GL_ATTRIBUTE_MATRICES            0
#endif

#if !defined(GR_GL_USE_BUFFER_DATA_NULL_HINT)
    #define GR_GL_USE_BUFFER_DATA_NULL_HINT     1
#endif

#if !defined(GR_GL_PER_GL_FUNC_CALLBACK)
    #define GR_GL_PER_GL_FUNC_CALLBACK          0
#endif

#if(GR_GL_NO_CONSTANT_ATTRIBUTES) && (GR_GL_ATTRIBUTE_MATRICES)
    #error "Cannot combine GR_GL_NO_CONSTANT_ATTRIBUTES and GR_GL_ATTRIBUTE_MATRICES"
#endif

////////////////////////////////////////////////////////////////////////////////

/**
 * The following macros are used to staticlly configure the default
 * GrGLInterface, but should not be used outside of the GrGLInterface
 * scaffolding.  Undefine here to prevent accidental use.
 */
#undef GR_SUPPORT_GLDESKTOP
#undef GR_SUPPORT_GLES1
#undef GR_SUPPORT_GLES2
#undef GR_SUPPORT_GLES

////////////////////////////////////////////////////////////////////////////////

#if GR_SCALAR_IS_FIXED
    #define GrGLType   GL_FIXED
#elif GR_SCALAR_IS_FLOAT
    #define GrGLType   GR_GL_FLOAT
#else
    #error "unknown GR_SCALAR type"
#endif

#if GR_TEXT_SCALAR_IS_USHORT
    #define GrGLTextType                    GR_GL_UNSIGNED_SHORT
    #define GR_GL_TEXT_TEXTURE_NORMALIZED   1
#elif GR_TEXT_SCALAR_IS_FLOAT
    #define GrGLTextType                    GR_GL_FLOAT
    #define GR_GL_TEXT_TEXTURE_NORMALIZED   0
#elif GR_TEXT_SCALAR_IS_FIXED
    #define GrGLTextType                    GR_GL_FIXED
    #define GR_GL_TEXT_TEXTURE_NORMALIZED   0
#else
    #error "unknown GR_TEXT_SCALAR type"
#endif

// Pick a pixel config for 32bit bitmaps. Our default is GL_RGBA (except on
// Windows where we match GDI's order).
#ifndef GR_GL_32BPP_COLOR_FORMAT
    #if GR_WIN32_BUILD || GR_LINUX_BUILD
        #define GR_GL_32BPP_COLOR_FORMAT    GR_GL_BGRA
    #else
        #define GR_GL_32BPP_COLOR_FORMAT    GR_GL_RGBA
    #endif
#endif

////////////////////////////////////////////////////////////////////////////////

struct GrGLInterface;

extern void GrGLCheckErr(const GrGLInterface* gl,
                         const char* location,
                         const char* call);

extern void GrGLClearErr(const GrGLInterface* gl);

#if GR_GL_CHECK_ERROR
    extern bool gCheckErrorGL;
    #define GR_GL_CHECK_ERROR_IMPL(IFACE, X)                    \
        if (gCheckErrorGL)                                      \
            GrGLCheckErr(IFACE, GR_FILE_AND_LINE_STR, #X)
#else
    #define GR_GL_CHECK_ERROR_IMPL(IFACE, X)
#endif

#if GR_GL_LOG_CALLS
    extern bool gLogCallsGL;
    #define GR_GL_LOG_CALLS_IMPL(X)                             \
        if (gLogCallsGL)                                        \
            GrPrintf(GR_FILE_AND_LINE_STR "GL: " #X "\n")
#else
    #define GR_GL_LOG_CALLS_IMPL(X)
#endif

#if GR_GL_PER_GL_FUNC_CALLBACK
    #define GR_GL_CALLBACK_IMPL(IFACE) (IFACE)->fCallback(IFACE)
#else
    #define GR_GL_CALLBACK_IMPL(IFACE)
#endif

#define GR_GL_CALL(IFACE, X)                                    \
    do {                                                        \
        GR_GL_CALL_NOERRCHECK(IFACE, X);                        \
        GR_GL_CHECK_ERROR_IMPL(IFACE, X);                       \
    } while (false)

#define GR_GL_CALL_NOERRCHECK(IFACE, X)                         \
    do {                                                        \
        GR_GL_CALLBACK_IMPL(IFACE);                             \
        (IFACE)->f##X;                                          \
        GR_GL_LOG_CALLS_IMPL(X);                                \
    } while (false)

#define GR_GL_CALL_RET(IFACE, RET, X)                           \
    do {                                                        \
        GR_GL_CALL_RET_NOERRCHECK(IFACE, RET, X);               \
        GR_GL_CHECK_ERROR_IMPL(IFACE, X);                       \
    } while (false)

#define GR_GL_CALL_RET_NOERRCHECK(IFACE, RET, X)                \
    do {                                                        \
        GR_GL_CALLBACK_IMPL(IFACE);                             \
        (RET) = (IFACE)->f##X;                                  \
        GR_GL_LOG_CALLS_IMPL(X);                                \
    } while (false)

#define GR_GL_GET_ERROR(IFACE) (IFACE)->fGetError()

////////////////////////////////////////////////////////////////////////////////

/**
 *  GrGLResetRowLength() will reset GL_UNPACK_ROW_LENGTH to 0. We write
 *  this wrapper, since GL_UNPACK_ROW_LENGTH is not available on all GL versions
 */
extern void GrGLResetRowLength(const GrGLInterface*);

////////////////////////////////////////////////////////////////////////////////

/**
 *  Some drivers want the var-int arg to be zero-initialized on input.
 */
#define GR_GL_INIT_ZERO     0
#define GR_GL_GetIntegerv(gl, e, p)     \
    do {                            \
        *(p) = GR_GL_INIT_ZERO;     \
        GR_GL_CALL(gl, GetIntegerv(e, p));   \
    } while (0)

#define GR_GL_GetFramebufferAttachmentParameteriv(gl, t, a, pname, p)           \
    do {                                                                        \
        *(p) = GR_GL_INIT_ZERO;                                                 \
        GR_GL_CALL(gl, GetFramebufferAttachmentParameteriv(t, a, pname, p));    \
    } while (0)

#define GR_GL_GetRenderbufferParameteriv(gl, t, pname, p)                       \
    do {                                                                        \
        *(p) = GR_GL_INIT_ZERO;                                                 \
        GR_GL_CALL(gl, GetRenderbufferParameteriv(t, pname, p));                \
    } while (0)

#define GR_GL_GetTexLevelParameteriv(gl, t, l, pname, p)                        \
    do {                                                                        \
        *(p) = GR_GL_INIT_ZERO;                                                 \
        GR_GL_CALL(gl, GetTexLevelParameteriv(t, l, pname, p));                 \
    } while (0)

////////////////////////////////////////////////////////////////////////////////

#endif
