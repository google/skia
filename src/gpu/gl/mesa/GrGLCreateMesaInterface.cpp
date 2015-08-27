
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLAssembleInterface.h"
#include "../GrGLUtil.h"

#include "osmesa_wrapper.h"

static GrGLFuncPtr osmesa_get(void* ctx, const char name[]) {
    SkASSERT(nullptr == ctx);
    SkASSERT(OSMesaGetCurrentContext());
    return OSMesaGetProcAddress(name);
}

const GrGLInterface* GrGLCreateMesaInterface() {
    if (nullptr == OSMesaGetCurrentContext()) {
        return nullptr;
    }
    return GrGLAssembleInterface(nullptr, osmesa_get);
}
