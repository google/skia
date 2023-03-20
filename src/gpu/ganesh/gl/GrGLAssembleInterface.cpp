/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLAssembleInterface.h"

#include "include/gpu/gl/GrGLAssembleHelpers.h"
#include "include/private/base/SkTemplates.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"

#define GET_PROC_LOCAL(F) GrGL##F##Fn* F = (GrGL##F##Fn*)get(ctx, "gl" #F)

sk_sp<const GrGLInterface> GrGLMakeAssembledInterface(void *ctx, GrGLGetProc get) {
    GET_PROC_LOCAL(GetString);
    if (nullptr == GetString) {
        return nullptr;
    }

    const char* verStr = reinterpret_cast<const char*>(GetString(GR_GL_VERSION));
    if (nullptr == verStr) {
        return nullptr;
    }

    GrGLStandard standard = GrGLGetStandardInUseFromString(verStr);
    // standard can be unused (optimzed away) if SK_ASSUME_GL_ES is set
    sk_ignore_unused_variable(standard);

    if (GR_IS_GR_GL_ES(standard)) {
        return GrGLMakeAssembledGLESInterface(ctx, get);
    } else if (GR_IS_GR_GL(standard)) {
        return GrGLMakeAssembledGLInterface(ctx, get);
    } else if (GR_IS_GR_WEBGL(standard)) {
        return GrGLMakeAssembledWebGLInterface(ctx, get);
    }
    return nullptr;
}

const GrGLInterface* GrGLAssembleInterface(void *ctx, GrGLGetProc get) {
    return GrGLMakeAssembledInterface(ctx, get).release();
}
