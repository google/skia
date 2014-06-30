// Modified from chromium/src/webkit/glue/gl_bindings_skia_cmd_buffer.cc

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLExtensions.h"
#include "gl/GrGLUtil.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>

#define GET_PROC(F) functions->f ## F = (GrGL ## F ## Proc) get(ctx, "gl" #F)
#define GET_PROC_SUFFIX(F, S) functions->f ## F = (GrGL ## F ## Proc) get(ctx, "gl" #F #S)
#define GET_PROC_LOCAL(F) GrGL ## F ## Proc F = (GrGL ## F ## Proc) get(ctx, "gl" #F)

#define GET_LINKED(F) functions->f ## F = gl ## F
#define GET_LINKED_SUFFIX(F, S) functions->f ## F = gl ## F ##S

#include "gl/GrGLAssembleGLESInterface.h"

static GrGLFuncPtr android_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(NULL == ctx);
    return eglGetProcAddress(name);
}

const GrGLInterface* GrGLCreateNativeInterface() {

    const char* verStr = reinterpret_cast<const char*>(glGetString(GR_GL_VERSION));
    GrGLStandard standard = GrGLGetStandardInUseFromString(verStr);

    if (kGLES_GrGLStandard == standard) {
        return GrGLAssembleGLESInterface(NULL, android_get_gl_proc);
    } else if (kGL_GrGLStandard == standard) {
        return GrGLAssembleGLInterface(NULL, android_get_gl_proc);
    }

    return NULL;
}
