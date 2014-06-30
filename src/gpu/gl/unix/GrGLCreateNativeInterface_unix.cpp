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

#define GET_PROC(F) functions->f ## F = (GrGL ## F ## Proc) get(ctx, "gl" #F)
#define GET_PROC_SUFFIX(F, S) functions->f ## F = (GrGL ## F ## Proc) get(ctx, "gl" #F #S)
#define GET_PROC_LOCAL(F) GrGL ## F ## Proc F = (GrGL ## F ## Proc) get(ctx, "gl" #F)

#define GET_LINKED GET_PROC
#define GET_LINKED_SUFFIX GET_PROC_SUFFIX

#include "gl/GrGLAssembleGLESInterface.h"

static GrGLFuncPtr glx_get(void* ctx, const char name[]) {
    SkASSERT(NULL == ctx);
    SkASSERT(NULL != glXGetCurrentContext());
    return glXGetProcAddress(reinterpret_cast<const GLubyte*>(name));
}

const GrGLInterface* GrGLCreateNativeInterface() {
    if (NULL == glXGetCurrentContext()) {
        return NULL;
    }

    const char* verStr = reinterpret_cast<const char*>(glGetString(GR_GL_VERSION));
    GrGLStandard standard = GrGLGetStandardInUseFromString(verStr);

    if (kGLES_GrGLStandard == standard) {
        return GrGLAssembleGLESInterface(NULL, glx_get);
    } else if (kGL_GrGLStandard == standard) {
        return GrGLAssembleGLInterface(NULL, glx_get);
    }
    return NULL;
}
