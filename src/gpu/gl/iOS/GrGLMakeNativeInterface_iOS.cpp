/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"

#include <dlfcn.h>

static GrGLFuncPtr ios_get_gl_proc(void* ctx, const char name[]) {
    return (GrGLFuncPtr) dlsym(RTLD_DEFAULT, name);
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    return GrGLMakeAssembledGLESInterface(nullptr, ios_get_gl_proc);
}

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }

