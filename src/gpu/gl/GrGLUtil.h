/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLUtil_DEFINED
#define GrGLUtil_DEFINED

#include "gl/GrGLInterface.h"

////////////////////////////////////////////////////////////////////////////////

typedef uint32_t GrGLVersion;
typedef uint32_t GrGLSLVersion;

#define GR_GL_VER(major, minor) ((static_cast<int>(major) << 16) | \
                                 static_cast<int>(minor))
#define GR_GLSL_VER(major, minor) ((static_cast<int>(major) << 16) | \
                                   static_cast<int>(minor))

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

////////////////////////////////////////////////////////////////////////////////

/**
 * Helpers for glGetString()
 */

// these variants assume caller already has a string from glGetString()
GrGLVersion GrGLGetVersionFromString(const char* versionString);
GrGLBinding GrGLGetBindingInUseFromString(const char* versionString);
GrGLSLVersion GrGLGetGLSLVersionFromString(const char* versionString);
bool GrGLHasExtensionFromString(const char* ext, const char* extensionString);

// these variants call glGetString()
bool GrGLHasExtension(const GrGLInterface*, const char* ext);
GrGLBinding GrGLGetBindingInUse(const GrGLInterface*);
GrGLVersion GrGLGetVersion(const GrGLInterface*);
GrGLSLVersion GrGLGetGLSLVersion(const GrGLInterface*);

/**
 * Helpers for glGetError()
 */

extern void GrGLCheckErr(const GrGLInterface* gl,
                         const char* location,
                         const char* call);

extern void GrGLClearErr(const GrGLInterface* gl);

#endif
