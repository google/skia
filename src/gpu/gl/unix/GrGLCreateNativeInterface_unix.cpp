/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLUtil.h"

#include <GL/glx.h>

static GrGLFuncPtr glx_get(void* ctx, const char name[]) {
    SkASSERT(NULL == ctx);
    SkASSERT(glXGetCurrentContext());
    return glXGetProcAddress(reinterpret_cast<const GLubyte*>(name));
}

const GrGLInterface* GrGLCreateNativeInterface() {
    if (NULL == glXGetCurrentContext()) {
        return NULL;
    }

    return GrGLAssembleInterface(NULL, glx_get);
}
