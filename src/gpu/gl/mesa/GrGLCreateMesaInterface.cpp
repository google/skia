
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
    SkASSERT(NULL == ctx);
    SkASSERT(OSMesaGetCurrentContext());
    return OSMesaGetProcAddress(name);
}

const GrGLInterface* GrGLCreateMesaInterface() {
    if (NULL == OSMesaGetCurrentContext()) {
        return NULL;
    }
    return GrGLAssembleInterface(NULL, osmesa_get);
}
