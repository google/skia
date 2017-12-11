/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLUtil.h"

#include <GLFW/glfw3.h>

static GrGLFuncPtr glfw_get(void* ctx, const char name[]) {
    SkASSERT(nullptr == ctx);
    SkASSERT(glfwGetCurrentContext());
    return glfwGetProcAddress(name);
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    if (nullptr == glfwGetCurrentContext()) {
        return nullptr;
    }

    return GrGLMakeAssembledInterface(nullptr, glfw_get);
}

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }
