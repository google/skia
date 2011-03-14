/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrGLConfig_DEFINED
#define GrGLConfig_DEFINED

#include "GrTypes.h"
#include "GrGLInterface.h"

/**
 * The following are optional defines that can be enabled at the compiler
 * command line, in a IDE project, in a GrUserConfig.h file, or in a GL custom
 * file (if one is in use). They don't require GR_GL_CUSTOM_SETUP or
 * setup GR_GL_CUSTOM_SETUP_HEADER to be enabled:
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
 */


#if !defined(GR_GL_LOG_CALLS)
    #define GR_GL_LOG_CALLS             0
#endif

#if !defined(GR_GL_LOG_CALLS_START)
    #define GR_GL_LOG_CALLS_START       0
#endif

#if !defined(GR_GL_CHECK_ERROR)
    #define GR_GL_CHECK_ERROR           GR_DEBUG
#endif

#if !defined(GR_GL_CHECK_ERROR_START)
    #define GR_GL_CHECK_ERROR_START     1
#endif

////////////////////////////////////////////////////////////////////////////////

#if GR_SCALAR_IS_FIXED
    #define GrGLType   GL_FIXED
#elif GR_SCALAR_IS_FLOAT
    #define GrGLType   GL_FLOAT
#else
    #error "unknown GR_SCALAR type"
#endif

#if GR_TEXT_SCALAR_IS_USHORT
    #define GrGLTextType                    GL_UNSIGNED_SHORT
    #define GR_GL_TEXT_TEXTURE_NORMALIZED   1
#elif GR_TEXT_SCALAR_IS_FLOAT
    #define GrGLTextType                    GL_FLOAT
    #define GR_GL_TEXT_TEXTURE_NORMALIZED   0
#elif GR_TEXT_SCALAR_IS_FIXED
    #define GrGLTextType                    GL_FIXED
    #define GR_GL_TEXT_TEXTURE_NORMALIZED   0
#else
    #error "unknown GR_TEXT_SCALAR type"
#endif

// Pick a pixel config for 32bit bitmaps. Our default is GL_RGBA (except on
// Windows where we match GDI's order).
#ifndef GR_GL_32BPP_COLOR_FORMAT
    #if GR_WIN32_BUILD
        #define GR_GL_32BPP_COLOR_FORMAT    GR_BGRA //use GR prefix because this
    #else                                           //may be an extension.
        #define GR_GL_32BPP_COLOR_FORMAT    GL_RGBA
    #endif
#endif



// BGRA format
#define GR_BGRA                     0x80E1

// FBO / stencil formats
#define GR_FRAMEBUFFER              0x8D40
#define GR_FRAMEBUFFER_COMPLETE     0x8CD5
#define GR_COLOR_ATTACHMENT0        0x8CE0
#define GR_FRAMEBUFFER_BINDING      0x8CA6
#define GR_RENDERBUFFER             0x8D41
#define GR_STENCIL_ATTACHMENT       0x8D20
#define GR_STENCIL_INDEX4           0x8D47
#define GR_STENCIL_INDEX8           0x8D48
#define GR_STENCIL_INDEX16          0x8D49
#define GR_DEPTH24_STENCIL8         0x88F0
#define GR_MAX_RENDERBUFFER_SIZE    0x84E8
#define GR_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GR_DEPTH_STENCIL            0x84F9
#define GR_RGBA8                    0x8058
#define GR_RGB565                   0x8D62


// Multisampling

// IMG MAX_SAMPLES uses a different value than desktop, Apple ES extension.
#define GR_MAX_SAMPLES              0x8D57
#define GR_MAX_SAMPLES_IMG          0x9135
#define GR_READ_FRAMEBUFFER         0x8CA8
#define GR_DRAW_FRAMEBUFFER         0x8CA9

// Buffer mapping
#define GR_WRITE_ONLY               0x88B9
#define GR_BUFFER_MAPPED            0x88BC

// Palette texture
#define GR_PALETTE8_RGBA8           0x8B91

////////////////////////////////////////////////////////////////////////////////

extern void GrGLCheckErr(const char* location, const char* call);

static inline void GrGLClearErr() {
    while (GL_NO_ERROR != GrGLGetGLInterface()->fGetError()) {}
}

#if GR_GL_CHECK_ERROR
    extern bool gCheckErrorGL;
    #define GR_GL_CHECK_ERROR_IMPL(X) if (gCheckErrorGL) GrGLCheckErr(GR_FILE_AND_LINE_STR, #X)
#else
    #define GR_GL_CHECK_ERROR_IMPL(X)
#endif

#if GR_GL_LOG_CALLS
    extern bool gLogCallsGL;
    #define GR_GL_LOG_CALLS_IMPL(X) if (gLogCallsGL) GrPrintf(GR_FILE_AND_LINE_STR "GL: " #X "\n")
#else
    #define GR_GL_LOG_CALLS_IMPL(X)
#endif

#define GR_GL(X)                 GrGLGetGLInterface()->f##X;; GR_GL_LOG_CALLS_IMPL(X); GR_GL_CHECK_ERROR_IMPL(X);
#define GR_GL_NO_ERR(X)          GrGLGetGLInterface()->f##X;; GR_GL_LOG_CALLS_IMPL(X); GR_GL_CHECK_ERROR_IMPL(X);

////////////////////////////////////////////////////////////////////////////////

/**
 *  GrGL_RestoreResetRowLength() will reset GL_UNPACK_ROW_LENGTH to 0. We write
 *  this wrapper, since GL_UNPACK_ROW_LENGTH is not available on all GL versions
 */
#if GR_SUPPORT_GLDESKTOP
    static inline void GrGL_RestoreResetRowLength() {
        GR_GL(PixelStorei(GL_UNPACK_ROW_LENGTH, 0));
    }
#else
    #define GrGL_RestoreResetRowLength()
#endif

////////////////////////////////////////////////////////////////////////////////

/**
 *  Some drivers want the var-int arg to be zero-initialized on input.
 */
#define GR_GL_INIT_ZERO     0
#define GR_GL_GetIntegerv(e, p)     \
    do {                            \
        *(p) = GR_GL_INIT_ZERO;     \
        GR_GL(GetIntegerv(e, p));   \
    } while (0)

////////////////////////////////////////////////////////////////////////////////

#endif
