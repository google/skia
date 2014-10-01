
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLUtil.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

static GrGLFuncPtr egl_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(NULL == ctx);
    return eglGetProcAddress(name);
}

const GrGLInterface* GrGLCreateNativeInterface() {
    return GrGLAssembleInterface(NULL, egl_get_gl_proc);
}
