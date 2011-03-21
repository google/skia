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


#ifndef GrGLPlatformIncludes_DEFINED
#define GrGLPlatformIncludes_DEFINED

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
 *      Optionally, define GR_GL_FUNCTION_TYPE to any qualifier needed on GL
 *      function pointer declarations (e.g. __stdcall).
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
 *      2. Specify all of the necessary GL include headers in the following
 *         macros:
 *              GR_GL_PLATFORM_HEADER_SUPPORT:  Header required before GL
 *                                              includes.
 *              GR_GL_PLATFORM_HEADER:  GL header location.
 *              GR_GL_PLATFORM_HEADER_EXT:  (Optional)  Header for extension
 *                                          definitions.
 *      3. Optionally define GR_GL_FUNCTION_TYPE.
 *      4. Define GR_GL_PROC_ADDRESS.
 *      5. Optionally define GR_GL_PROC_ADDRESS_HEADER
 */

#if GR_GL_CUSTOM_SETUP

    #ifdef GR_SUPPORT_GLES1
        #define GR_GL_PLATFORM_HEADER GR_INCLUDE_GLES1
        #if defined(GR_INCLUDE_GLES1ext)
            #define GR_GL_PLATFORM_HEADER_EXT GR_INCLUDE_GLES1ext
        #endif
    #endif

    #ifdef GR_SUPPORT_GLES2
        #define GR_GL_PLATFORM_HEADER GR_INCLUDE_GLES2
        #if defined(GR_INCLUDE_GLES2ext)
            #define GR_GL_PLATFORM_HEADER_EXT GR_INCLUDE_GLES2ext
        #endif
    #endif

    #ifdef GR_SUPPORT_GLDESKTOP
        #define GR_GL_PLATFORM_HEADER GR_INCLUDE_GLDESKTOP
        #if defined(GR_INCLUDE_GLDESKTOPext)
            #define GR_GL_PLATFORM_HEADER_EXT GR_INCLUDE_GLDESKTOPext
        #endif
    #endif

#elif defined(GR_GL_CUSTOM_SETUP_HEADER)

    #include GR_GL_CUSTOM_SETUP_HEADER

#else
    #include "GrConfig.h"

    #undef GR_GL_FUNCTION_TYPE
    #undef GR_GL_PROC_ADDRESS
    #undef GR_GL_PROC_ADDRESS_HEADER

    #if GR_WIN32_BUILD
        #define GR_SUPPORT_GLDESKTOP        1

        #define GR_GL_PLATFORM_HEADER_SUPPORT <Windows.h>
        #define GR_GL_PLATFORM_HEADER <GL/gl.h>

        #define GR_GL_FUNCTION_TYPE         __stdcall
        #define GR_GL_PROC_ADDRESS(X)       wglGetProcAddress(#X)
        #define GR_GL_PROC_ADDRESS_HEADER   <windows.h>

        // Force querying for the existence of these extensions on Windows
        // builds.
        #define GL_APPLE_framebuffer_multisample        1
        #define GL_EXT_framebuffer_object               1
        #define GL_OES_mapbuffer                        1
        #define GL_OES_mapbuffer                        1
    #elif GR_MAC_BUILD
        #define GR_SUPPORT_GLDESKTOP        1

        #define GR_GL_PLATFORM_HEADER       <OpenGL/gl.h>
        #define GR_GL_PLATFORM_HEADER_EXT   <OpenGL/glext.h>

        #define GR_GL_PROC_ADDRESS(X)       &X
    #elif GR_IOS_BUILD
        #define GR_SUPPORT_GLES1            1
        #define GR_SUPPORT_GLES2            1

        #define GR_GL_PLATFORM_HEADER       <OpenGLES/ES1/gl.h>
        #define GR_GL_PLATFORM_HEADER_EXT   <OpenGLES/ES1/glext.h>

        #define GR_GL_PLATFORM_HEADER2      <OpenGLES/ES2/gl.h>
        #define GR_GL_PLATFORM_HEADER_EXT2  <OpenGLES/ES2/glext.h>
        #define GR_GL_PROC_ADDRESS(X)       &X
    #elif GR_ANDROID_BUILD
        #ifndef GL_GLEXT_PROTOTYPES
            #define GL_GLEXT_PROTOTYPES
        #endif
        #define GR_SUPPORT_GLES2            1

        #define GR_GL_PLATFORM_HEADER       <GLES2/gl2.h> 
        #define GR_GL_PLATFORM_HEADER_EXT   <GLES2/gl2ext.h>

        #define GR_GL_PROC_ADDRESS(X)       eglGetProcAddress(#X)
        #define GR_GL_PROC_ADDRESS_HEADER   <EGL/egl.h>
    #elif GR_QNX_BUILD
        #ifndef GL_GLEXT_PROTOTYPES
            #define GL_GLEXT_PROTOTYPES
        #endif
         #define GR_SUPPORT_GLES2           1
        // This is needed by the QNX GLES2 headers
        #define GL_API_EXT

        #define GR_GL_PLATFORM_HEADER       <GLES2/gl2.h> 
        #define GR_GL_PLATFORM_HEADER_EXT   <GLES2/gl2ext.h>

        #define GR_GL_PROC_ADDRESS(X)       eglGetProcAddress(#X)
        #define GR_GL_PROC_ADDRESS_HEADER   <EGL/egl.h>
    #elif GR_LINUX_BUILD
        #ifndef GL_GLEXT_PROTOTYPES
            #define GL_GLEXT_PROTOTYPES
        #endif

        #define GR_GL_PLATFORM_HEADER       <GL/gl.h> 
        #define GR_GL_PLATFORM_HEADER_EXT   <GL/glext.h>

        #define GR_GL_PROC_ADDRESS(X)       glXGetProcAddress(reinterpret_cast<const GLubyte*>(#X))
        #define GR_SUPPORT_GLDESKTOP        1
        #define GR_GL_PROC_ADDRESS_HEADER   <GL/glx.h>
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

#endif
