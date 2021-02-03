/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"

#include "emscripten/html5.h"

static GrGLFuncPtr webgl_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(nullptr == ctx);
    return (GrGLFuncPtr) emscripten_webgl_get_proc_address(name);
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    return GrGLMakeAssembledInterface(nullptr, webgl_get_gl_proc);
}

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }
