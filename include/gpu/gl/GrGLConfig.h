
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGLConfig_DEFINED
#define GrGLConfig_DEFINED

#include "GrTypes.h"

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
 * GR_GL_LOG_CALLS: if 1 Gr can print every GL call using SkDebugf. Defaults to
 * 0. Logging can be enabled and disabled at runtime using a debugger via to
 * global gLogCallsGL. The initial value of gLogCallsGL is controlled by
 * GR_GL_LOG_CALLS_START.
 *
 * GR_GL_LOG_CALLS_START: controls the initial value of gLogCallsGL when
 * GR_GL_LOG_CALLS is 1. Defaults to 0.
 *
 * GR_GL_CHECK_ERROR: if enabled Gr can do a glGetError() after every GL call.
 * Defaults to 1 if SK_DEBUG is set, otherwise 0. When GR_GL_CHECK_ERROR is 1
 * this can be toggled in a debugger using the gCheckErrorGL global. The initial
 * value of gCheckErrorGL is controlled by by GR_GL_CHECK_ERROR_START.
 *
 * GR_GL_CHECK_ERROR_START: controls the initial value of gCheckErrorGL
 * when GR_GL_CHECK_ERROR is 1.  Defaults to 1.
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
 * GR_GL_CHECK_ALLOC_WITH_GET_ERROR: If set to 1 this will then glTexImage,
 * glBufferData, glRenderbufferStorage, etc will be checked for errors. This
 * amounts to ensuring the error is GL_NO_ERROR, calling the allocating
 * function, and then checking that the error is still GL_NO_ERROR. When the
 * value is 0 we will assume no error was generated without checking.
 *
 * GR_GL_CHECK_FBO_STATUS_ONCE_PER_FORMAT: We will normally check the FBO status
 * every time we bind a texture or renderbuffer to an FBO. However, in some
 * environments CheckFrameBufferStatus is very expensive. If this is set we will
 * check the first time we use a color format or a combination of color /
 * stencil formats as attachments. If the FBO is complete we will assume
 * subsequent attachments with the same formats are complete as well.
 *
 * GR_GL_MUST_USE_VBO: Indicates that all vertices and indices must be rendered
 * from VBOs. Chromium's command buffer doesn't allow glVertexAttribArray with
 * ARARY_BUFFER 0 bound or glDrawElements with ELEMENT_ARRAY_BUFFER 0 bound.
 *
 * GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE is for compatibility with the new version
 * of the OpenGLES2.0 headers from Khronos.  glShaderSource now takes a const char * const *,
 * instead of a const char
 */

#if !defined(GR_GL_LOG_CALLS)
    #ifdef SK_DEBUG
        #define GR_GL_LOG_CALLS 1
    #else
        #define GR_GL_LOG_CALLS 0
    #endif
#endif

#if !defined(GR_GL_LOG_CALLS_START)
    #define GR_GL_LOG_CALLS_START                       0
#endif

#if !defined(GR_GL_CHECK_ERROR)
    #ifdef SK_DEBUG
        #define GR_GL_CHECK_ERROR 1
    #else
        #define GR_GL_CHECK_ERROR 0
    #endif
#endif

#if !defined(GR_GL_CHECK_ERROR_START)
    #define GR_GL_CHECK_ERROR_START                     1
#endif

#if !defined(GR_GL_USE_BUFFER_DATA_NULL_HINT)
    #define GR_GL_USE_BUFFER_DATA_NULL_HINT             1
#endif

#if !defined(GR_GL_CHECK_ALLOC_WITH_GET_ERROR)
    #define GR_GL_CHECK_ALLOC_WITH_GET_ERROR            1
#endif

#if !defined(GR_GL_CHECK_FBO_STATUS_ONCE_PER_FORMAT)
    #define GR_GL_CHECK_FBO_STATUS_ONCE_PER_FORMAT      0
#endif

#if !defined(GR_GL_MUST_USE_VBO)
    #define GR_GL_MUST_USE_VBO                          0
#endif

#if !defined(GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE)
    #define GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE       0
#endif

#endif
