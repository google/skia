
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrGLConfig_chrome_DEFINED
#define GrGLConfig_chrome_DEFINED

// glGetError() forces a sync with gpu process on chrome
#define GR_GL_CHECK_ERROR_START                     0

// cmd buffer allocates memory and memsets it to zero when it sees glBufferData
// with NULL.
#define GR_GL_USE_BUFFER_DATA_NULL_HINT             0

// chrome uses this to set the context on each GL call.
#define GR_GL_PER_GL_FUNC_CALLBACK                  1

// Check error is even more expensive in chrome (cmd buffer flush). The
// compositor also doesn't check its allocations.
#define GR_GL_CHECK_ALLOC_WITH_GET_ERROR            0

// CheckFramebufferStatus in chrome synchronizes the gpu and renderer processes.
#define GR_GL_CHECK_FBO_STATUS_ONCE_PER_FORMAT      1

// Non-VBO vertices and indices are not allowed in Chromium.
#define GR_GL_MUST_USE_VBO                          1

// Use updated Khronos signature for glShaderSource
// (const char* const instead of char**).
#define GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE       1

#if !defined(GR_GL_IGNORE_ES3_MSAA)
    #define GR_GL_IGNORE_ES3_MSAA 1
#endif

#endif
