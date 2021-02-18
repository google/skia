/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/gl/GrGLUtil.h"

// Define this to get a prototype for glXGetProcAddress on some systems
#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glx.h>

static GrGLFuncPtr glx_get(void* ctx, const char name[]) {
    // Avoid calling glXGetProcAddress() for EGL procs.
    // We don't expect it to ever succeed, but somtimes it returns non-null anyway.
    if (0 == strncmp(name, "egl", 3)) {
        return nullptr;
    }

    SkASSERT(nullptr == ctx);
    SkASSERT(glXGetCurrentContext());
    return glXGetProcAddress(reinterpret_cast<const GLubyte*>(name));
}

sk_sp<const GrGLInterface> GrGLMakeGLXInterface() {
    if (nullptr == glXGetCurrentContext()) {
        return nullptr;
    }

    return GrGLMakeAssembledInterface(nullptr, glx_get);
}
