/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLAssembleHelpers.h"
#include "gl/GrGLUtil.h"

#define GET_PROC_LOCAL(F) GrGL##F##Fn* F = (GrGL##F##Fn*)get(ctx, "gl" #F)

sk_sp<const GrGLInterface> GrGLMakeAssembledInterface(void *ctx, GrGLGetProc get) {
#if IS_WEBGL==1
    return GrGLMakeAssembledGLESInterface(ctx, get);
#else
    GET_PROC_LOCAL(GetString);
    if (nullptr == GetString) {
        return nullptr;
    }

    const char* verStr = reinterpret_cast<const char*>(GetString(GR_GL_VERSION));
    if (nullptr == verStr) {
        return nullptr;
    }

    GrGLStandard standard = GrGLGetStandardInUseFromString(verStr);

    if (kGLES_GrGLStandard == standard) {
        return GrGLMakeAssembledGLESInterface(ctx, get);
    } else if (kGL_GrGLStandard == standard) {
        return GrGLMakeAssembledGLInterface(ctx, get);
    }
    return nullptr;
#endif
}

SK_API const GrGLInterface* GrGLAssembleInterface(void *ctx, GrGLGetProc get) {
    return GrGLMakeAssembledInterface(ctx, get).release();
}
