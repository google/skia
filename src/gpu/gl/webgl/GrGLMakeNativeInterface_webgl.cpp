/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"

#include <emscripten/html5.h>
#include <webgl/webgl1.h>
#include <webgl/webgl1_ext.h>
#include <webgl/webgl2.h>
#include <webgl/webgl2_ext.h>

static GrGLFuncPtr webgl_get_gl_proc(void* ctx, const char name[]) {

    #define M(X) if (0 == strcmp(#X, name)) { return (GrGLFuncPtr) emscripten_##X; }
    M(glGetString)
    #undef M

    // We explicitly do not use GetProcAddress or something similar because
    // its code size is quite large. We shouldn't need GetProcAddress
    // because emscripten provides us all the valid function pointers
    // for WebGL via the included headers.
    // https://github.com/emscripten-core/emscripten/blob/7ba7700902c46734987585409502f3c63beb650f/system/include/emscripten/html5_webgl.h#L93
    SkASSERTF(false, "Can't lookup fn %s\n", name);
    return nullptr;
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    return GrGLMakeAssembledInterface(nullptr, webgl_get_gl_proc);
}

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }
