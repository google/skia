/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/ganesh/gl/GrGLCoreFunctions.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"

#include <EGL/egl.h>
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2.h>

static GrGLFuncPtr egl_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(nullptr == ctx);
    #define M(X) if (0 == strcmp(#X, name)) { return (GrGLFuncPtr) X; }
    GR_GL_CORE_FUNCTIONS_EACH(M)
    #undef M
    return eglGetProcAddress(name);
}

namespace GrGLInterfaces {
sk_sp<const GrGLInterface> MakeEGL() {
    return GrGLMakeAssembledInterface(nullptr, egl_get_gl_proc);
}
}  // namespace GrGLInterfaces

#if !defined(SK_DISABLE_LEGACY_EGLINTERFACE_FACTORY)
sk_sp<const GrGLInterface> GrGLMakeEGLInterface() { return GrGLInterfaces::MakeEGL(); }
#endif
