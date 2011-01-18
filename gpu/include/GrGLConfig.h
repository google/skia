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

#if GR_WIN32_BUILD
    // glew has to be included before gl
    #define GR_INCLUDE_GLDESKTOP    <GL/glew.h>
    #define GR_INCLUDE_GLDESKTOPext <GL/gl.h>
    #define GR_GL_FUNC __stdcall
    // undo stupid windows defines
    #undef near
    #undef far
#elif GR_MAC_BUILD
    #define GR_INCLUDE_GLDESKTOP    <OpenGL/gl.h>
    #define GR_INCLUDE_GLDESKTOPext <OpenGL/glext.h>
    #define GR_GL_FUNC
#elif GR_IOS_BUILD
    #define GR_INCLUDE_GLES1       <OpenGLES/ES1/gl.h>
    #define GR_INCLUDE_GLES1ext    <OpenGLES/ES1/glext.h>
    #define GR_INCLUDE_GLES2       <OpenGLES/ES2/gl.h>
    #define GR_INCLUDE_GLES2ext    <OpenGLES/ES2/glext.h>
    #define GR_GL_FUNC
#elif GR_ANDROID_BUILD
    #ifndef GL_GLEXT_PROTOTYPES
        #define GL_GLEXT_PROTOTYPES
    #endif
    #define GR_INCLUDE_GLES2        <GLES2/gl2.h>
    #define GR_INCLUDE_GLES2ext     <GLES2/gl2ext.h>
    #define GR_GL_FUNC
#elif GR_LINUX_BUILD
    // need to distinguish between ES and Deskop versions for linux
    #ifndef GL_GLEXT_PROTOTYPES
        #define GL_GLEXT_PROTOTYPES
    #endif
    #define GR_INCLUDE_GLDESKTOP    <GL/gl.h>
    #define GR_INCLUDE_GLDESKTOPext <GL/glext.h>
//    #define GR_INCLUDE_GLES1        <GLES/gl.h>
//    #define GR_INCLUDE_GLES1ext     <GLES/glext.h>
//    #define GR_INCLUDE_GLES2        <GLES2/gl2.h>
//    #define GR_INCLUDE_GLES2ext     <GLES2/gl2ext.h>
    #define GR_GL_FUNC
#elif GR_QNX_BUILD
    #ifndef GL_GLEXT_PROTOTYPES
        #define GL_GLEXT_PROTOTYPES
    #endif
    // This is needed by the QNX GLES2 headers
    #define GL_API_EXT
    #define GR_INCLUDE_GLES2        <GLES2/gl2.h>
    #define GR_INCLUDE_GLES2ext     <GLES2/gl2ext.h>
    #define GR_INCLUDE_EGL          <EGL/egl.h>
    #define GR_GL_FUNC
#else
    #error "unsupported GR_???_BUILD"
#endif

// Ensure we're at least defined
//

#ifndef GR_SUPPORT_GLES1
    #if defined(GR_INCLUDE_GLES1)
        #define GR_SUPPORT_GLES1        1
    #else
        #define GR_SUPPORT_GLES1        0
    #endif
#endif

#ifndef GR_SUPPORT_GLES2
    #if defined(GR_INCLUDE_GLES2)
        #define GR_SUPPORT_GLES2        1
    #else
        #define GR_SUPPORT_GLES2        0
    #endif
#endif

#define GR_SUPPORT_GLES (GR_SUPPORT_GLES1 || GR_SUPPORT_GLES2)

#ifndef GR_SUPPORT_GLDESKTOP
    #if defined(GR_INCLUDE_GLDESKTOP)
        #define GR_SUPPORT_GLDESKTOP    1
    #else
        #define GR_SUPPORT_GLDESKTOP    0
    #endif
#endif

#ifndef GR_SUPPORT_EGL
    #if defined(GR_INCLUDE_EGL)
        #define GR_SUPPORT_EGL          1
    #else
        #define GR_SUPPORT_EGL          0
    #endif
#endif
// Filter the includes based on what we support
//

#if !GR_SUPPORT_GLES1
    #undef GR_INCLUDE_GLES1
    #undef GR_INCLUDE_GLES1ext
#endif

#if !GR_SUPPORT_GLES2
    #undef GR_INCLUDE_GLES2
    #undef GR_INCLUDE_GLES2ext
#endif

#if !GR_SUPPORT_GLDESKTOP
    #undef GR_INCLUDE_GLDESKTOP
    #undef GR_INCLUDE_GLDESKTOPext
#endif

#if !GR_SUPPORT_EGL
    #undef GR_INCLUDE_EGL
#endif

// Begin including GL headers
//

#ifdef GR_INCLUDE_GLES1
    #include GR_INCLUDE_GLES1
#endif
#ifdef GR_INCLUDE_GLES1ext
    #include GR_INCLUDE_GLES1ext
#endif
#ifdef GR_INCLUDE_GLES2
    #include GR_INCLUDE_GLES2
#endif
#ifdef GR_INCLUDE_GLES2ext
    #include GR_INCLUDE_GLES2ext
#endif
#ifdef GR_INCLUDE_GLDESKTOP
    #include GR_INCLUDE_GLDESKTOP
#endif
#ifdef GR_INCLUDE_GLDESKTOPext
    #include GR_INCLUDE_GLDESKTOPext
#endif
#ifdef GR_INCLUDE_EGL
    #include GR_INCLUDE_EGL
#endif

//
// End including GL headers

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
// setup for opengl ES/desktop extensions
// we make a struct of function pointers so that each GL context
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

// GR_FORCE_GLCHECKERR can be defined by GrUserConfig.h
#if defined(GR_FORCE_GLCHECKERR)
    #define GR_LOCAL_CALL_CHECKERR GR_FORCE_GLCHECKERR
#else
    #define GR_LOCAL_CALL_CHECKERR GR_DEBUG
#endif
static inline void GrDebugGLCheckErr(const char* location, const char* call) {
#if GR_LOCAL_CALL_CHECKERR
    GrGLCheckErr(location, call);
#endif
}
#undef GR_LOCAL_CALL_CHECKERR

#if GR_GL_LOG_CALLS
    extern bool gPrintGL;
    #define GR_GL(X)                 gl ## X; GrDebugGLCheckErr(GR_FILE_AND_LINE_STR, #X); if (gPrintGL) GrPrintf(GR_FILE_AND_LINE_STR "GL: " #X "\n")
    #define GR_GL_NO_ERR(X)          GrGLClearErr(); gl ## X; if (gPrintGL) GrPrintf(GR_FILE_AND_LINE_STR "GL: " #X "\n")
    #define GR_GLEXT(exts, X)        exts. X; GrDebugGLCheckErr(GR_FILE_AND_LINE_STR, #X); if (gPrintGL) GrPrintf(GR_FILE_AND_LINE_STR "GL: " #X "\n")
    #define GR_GLEXT_NO_ERR(exts, X) GrGLClearErr(); exts. X; if (gPrintGL) GrPrintf(GR_FILE_AND_LINE_STR "GL: " #X "\n")
#else
    #define GR_GL(X)                 gl ## X; GrDebugGLCheckErr(GR_FILE_AND_LINE_STR, #X)
    #define GR_GL_NO_ERR(X)          GrGLClearErr(); gl ## X
    #define GR_GLEXT(exts, X)        exts. X; GrDebugGLCheckErr(GR_FILE_AND_LINE_STR, #X)
    #define GR_GLEXT_NO_ERR(exts, X) GrGLClearErr(); exts. X
#endif

#endif

