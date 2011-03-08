/*
    Copyright 2010 Google Inc.

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

#if !defined(GR_GL_CUSTOM_SETUP)
    #define GR_GL_CUSTOM_SETUP 0
#endif
/**
 * We need to pull in the right GL headers and determine whether we are
 * compiling for ES1, ES2, or desktop GL. (We allow ES1 and ES2 to both be
 * supported in the same build but not ESx and desktop). We also need to know
 * the platform-specific way to get extension function pointers (e.g.
 * eglGetProcAddress). The port specifies this info explicitly or we will infer
 * it from the GR_*_BUILD flag.
 *
 * To specify GL setup directly define GR_GL_CUSTOM_SETUP to 1 and define:
 *      GR_SUPPORT_GLDESKTOP or (GR_SUPPORT_GLES1 and/or GR_SUPPORT_GLES2) to 1
 *
 *      if GR_SUPPORT_GLDESKTOP is 1 then provide:
 *          1. The name of your GL header in GR_INCLUDE_GLDESKTOP
 *          2. If necessary, the name of a file that includes extension
 *             definitions in GR_INCLUDE_GLDESKTOPext.
 *      if GR_SUPPORT_GLES1 is 1 then provide:
 *          1. The name of your GL header in GR_INCLUDE_GLES1
 *          2. If necessary, the name of a file that includes extension
 *             definitions in GR_INCLUDE_GLES1ext.
 *      if GR_SUPPORT_GLES2 is 1 then provide:
 *          1. The name of your GL header in GR_INCLUDE_GLES2
 *          2. If necessary, the name of a file that includes extension
 *             definitions in GR_INCLUDE_GLES2ext.
 *
 *      Optionally, define GR_GL_FUNC to any qualifier needed on GL function
 *      pointer declarations (e.g. __stdcall).
 *
 *      Define GR_GL_PROC_ADDRESS to take a gl function and produce a
 *      function pointer. Two examples:
 *          1. Your platform doesn't require a proc address function, just take
 *             the address of the function:
 *             #define GR_GL_PROC_ADDRESS(X) &X
 *          2. Your platform uses eglGetProcAddress:
 *             #define GR_GL_PROC_ADDRESS eglGetProcAddress(#X)
 *
 *     Optionally define GR_GL_PROC_ADDRESS_HEADER to include any additional
 *     header necessary to use GR_GL_PROC_ADDRESS (e.g. <EGL/egl.h>)
 *
 * Alternatively, define GR_GL_CUSTOM_SETUP_HEADER (and not GR_GL_CUSTOM_SETUP)
 * to a header that can be included. This file should:
 *      1. Define the approprate GR_SUPPORT_GL* macro(s) to 1
 *      2. Includes all necessary GL headers.
 *      3. Optionally define GR_GL_FUNC.
 *      4. Define GR_GL_PROC_ADDRESS.
 *      5. Optionally define GR_GL_PROC_ADDRESS_HEADER
 *
 *------------------------------------------------------------------------------
 *
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

#if GR_GL_CUSTOM_SETUP

    #ifdef GR_SUPPORT_GLES1
        #include GR_INCLUDE_GLES1
        #if defined(GR_INCLUDE_GLES1ext)
            #include GR_INCLUDE_GLES1ext
        #endif
    #endif

    #ifdef GR_SUPPORT_GLES2
        #include GR_INCLUDE_GLES2
        #if defined(GR_INCLUDE_GLES2ext)
            #include GR_INCLUDE_GLES2ext
        #endif
    #endif

    #ifdef GR_SUPPORT_GLDESKTOP
        #include GR_INCLUDE_GLDESKTOP
        #if defined(GR_INCLUDE_GLDESKTOPext)
            #include GR_INCLUDE_GLDESKTOPext
        #endif
    #endif

#elif defined(GR_GL_CUSTOM_SETUP_HEADER)

    #include GR_GL_CUSTOM_SETUP_HEADER

#else
    #undef GR_GL_FUNC
    #undef GR_GL_PROC_ADDRESS
    #undef GR_GL_PROC_ADDRESS_HEADER

    #if GR_WIN32_BUILD
        #define GR_SUPPORT_GLDESKTOP        1
        // glew has to be included before gl
        #include <GL/glew.h>
        #include <GL/gl.h>
        // remove stupid windows defines
        #undef near
        #undef far
        #define GR_GL_FUNC __stdcall
        #define GR_GL_PROC_ADDRESS(X)       wglGetProcAddress(#X)
        #define GR_GL_PROC_ADDRESS_HEADER   <windows.h>
    #elif GR_MAC_BUILD
        #define GR_SUPPORT_GLDESKTOP        1
        #include <OpenGL/gl.h>
        #include <OpenGL/glext.h>
        #define GR_GL_PROC_ADDRESS(X)       &X
    #elif GR_IOS_BUILD
        #define GR_SUPPORT_GLES1            1
        #include <OpenGLES/ES1/gl.h>
        #include <OpenGLES/ES1/glext.h>
        #define GR_SUPPORT_GLES2            1
        #include <OpenGLES/ES2/gl.h>
        #include <OpenGLES/ES2/glext.h>
        #define GR_GL_PROC_ADDRESS(X)       &X
    #elif GR_ANDROID_BUILD
        #ifndef GL_GLEXT_PROTOTYPES
            #define GL_GLEXT_PROTOTYPES
        #endif
        #define GR_SUPPORT_GLES2            1
        #include <GLES2/gl2.h>
        #include <GLES2/gl2ext.h>
        #define GR_GL_PROC_ADDRESS(X)       eglGetProcAddress(#X)
        #define GR_GL_PROC_ADDRESS_HEADER   <EGL/egl.h>
    #elif GR_QNX_BUILD
        #ifndef GL_GLEXT_PROTOTYPES
            #define GL_GLEXT_PROTOTYPES
        #endif
         #define GR_SUPPORT_GLES2           1
        // This is needed by the QNX GLES2 headers
        #define GL_API_EXT
        #include <GLES2/gl2.h>
        #include <GLES2/gl2ext.h>
        #define GR_GL_PROC_ADDRESS(X)       eglGetProcAddress(#X)
        #define GR_GL_PROC_ADDRESS_HEADER   <EGL/egl.h>
    #elif GR_LINUX_BUILD
        #ifndef GL_GLEXT_PROTOTYPES
            #define GL_GLEXT_PROTOTYPES
        #endif
        #define GL_EXT_framebuffer_blit     0
        #include <GL/gl.h>
        #include <GL/glext.h>
        #define GR_GL_PROC_ADDRESS(X)       &X
        #define GR_SUPPORT_GLDESKTOP        1
    #else
        #error "unsupported GR_???_BUILD"
    #endif

#endif

#if !defined(GR_SUPPORT_GLDESKTOP)
    #define GR_SUPPORT_GLDESKTOP    0
#endif
#if !defined(GR_SUPPORT_GLES1)
    #define GR_SUPPORT_GLES1        0
#endif
#if !defined(GR_SUPPORT_GLES2)
    #define GR_SUPPORT_GLES2        0
#endif

#define GR_SUPPORT_GLES ((GR_SUPPORT_GLES1) || (GR_SUPPORT_GLES2))

#if !GR_SUPPORT_GLES && !GR_SUPPORT_GLDESKTOP
    #error "Either desktop or ES GL must be supported"
#elif GR_SUPPORT_GLES && GR_SUPPORT_GLDESKTOP
    #error "Cannot support both desktop and ES GL"
#endif

#if !defined(GR_GL_FUNC)
    #define GR_GL_FUNC
#endif

#if !defined(GR_GL_PROC_ADDRESS)
    #error "Must define GR_GL_PROC_ADDRESS"
#endif

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

// Pick a pixel config for 32bit bitmaps. Our default is GL_RGBA (expect on
// Windows where we match GDI's order).
#ifndef GR_GL_32BPP_COLOR_FORMAT
    #if GR_WIN32_BUILD
        #define GR_GL_32BPP_COLOR_FORMAT    GR_BGRA //use GR prefix because this
    #else                                           //may be an extension.
        #define GR_GL_32BPP_COLOR_FORMAT    GL_RGBA
    #endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Setup for opengl ES/desktop extensions
// We make a struct of function pointers so that each GL context
// can have it's own struct. (Some environments may have different proc
// addresses for different contexts).

extern "C" {
struct GrGLExts {
// FBO
    GLvoid (GR_GL_FUNC *GenFramebuffers)(GLsizei n, GLuint *framebuffers);
    GLvoid (GR_GL_FUNC *BindFramebuffer)(GLenum target, GLuint framebuffer);
    GLvoid (GR_GL_FUNC *FramebufferTexture2D)(GLenum target, GLenum attachment,
                                              GLenum textarget, GLuint texture,
                                              GLint level);
    GLenum (GR_GL_FUNC *CheckFramebufferStatus)(GLenum target);
    GLvoid (GR_GL_FUNC *DeleteFramebuffers)(GLsizei n, const
                                            GLuint *framebuffers);
    GLvoid (GR_GL_FUNC *RenderbufferStorage)(GLenum target,
                                             GLenum internalformat,
                                             GLsizei width, GLsizei height);
    GLvoid (GR_GL_FUNC *GenRenderbuffers)(GLsizei n, GLuint *renderbuffers);
    GLvoid (GR_GL_FUNC *DeleteRenderbuffers)(GLsizei n,
                                             const GLuint *renderbuffers);
    GLvoid (GR_GL_FUNC *FramebufferRenderbuffer)(GLenum target,
                                                 GLenum attachment,
                                                 GLenum renderbuffertarget,
                                                 GLuint renderbuffer);
    GLvoid (GR_GL_FUNC *BindRenderbuffer)(GLenum target, GLuint renderbuffer);

// Multisampling
    // same prototype for ARB_FBO, EXT_FBO, GL 3.0, & Apple ES extension
    GLvoid (GR_GL_FUNC *RenderbufferStorageMultisample)(GLenum target,
                                                        GLsizei samples,
                                                        GLenum internalformat,
                                                        GLsizei width,
                                                        GLsizei height);
    // desktop: ext_fbo_blit, arb_fbo, gl 3.0
    GLvoid (GR_GL_FUNC *BlitFramebuffer)(GLint srcX0, GLint srcY0,
                                         GLint srcX1, GLint srcY1,
                                         GLint dstX0, GLint dstY0,
                                         GLint dstX1, GLint dstY1,
                                         GLbitfield mask, GLenum filter);
    // apple's es extension
    GLvoid (GR_GL_FUNC *ResolveMultisampleFramebuffer)();

    // IMG'e es extension
    GLvoid (GR_GL_FUNC *FramebufferTexture2DMultisample)(GLenum target,
                                                         GLenum attachment,
                                                         GLenum textarget,
                                                         GLuint texture,
                                                         GLint level,
                                                         GLsizei samples);

// Buffer mapping (extension in ES).
    GLvoid* (GR_GL_FUNC *MapBuffer)(GLenum target, GLenum access);
    GLboolean (GR_GL_FUNC *UnmapBuffer)(GLenum target);
};
}

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

extern void GrGLInitExtensions(GrGLExts* exts);

////////////////////////////////////////////////////////////////////////////////

extern void GrGLCheckErr(const char* location, const char* call);

static inline void GrGLClearErr() {
    while (GL_NO_ERROR != glGetError()) {}
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

#define GR_GL(X)                 gl ## X; GR_GL_LOG_CALLS_IMPL(X); GR_GL_CHECK_ERROR_IMPL(X);
#define GR_GL_NO_ERR(X)          GrGLClearErr(); gl ## X; GR_GL_LOG_CALLS_IMPL(X); GR_GL_CHECK_ERROR_IMPL(X);
#define GR_GLEXT(exts, X)        exts. X; GR_GL_LOG_CALLS_IMPL(X); GR_GL_CHECK_ERROR_IMPL(X);
#define GR_GLEXT_NO_ERR(exts, X) GrGLClearErr(); exts. X; GR_GL_LOG_CALLS_IMPL(X); GR_GL_CHECK_ERROR_IMPL(X);

////////////////////////////////////////////////////////////////////////////////

/**
 * Helpers for glGetString()
 */
bool has_gl_extension(const char* ext);
void gl_version(int* major, int* minor);

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
