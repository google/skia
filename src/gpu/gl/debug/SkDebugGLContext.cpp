
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/SkDebugGLContext.h"

const GrGLInterface* SkDebugGLContext::createGLContext(GrGLStandard forcedGpuAPI) {
    if (kGLES_GrGLStandard == forcedGpuAPI) {
        return NULL;
    }

    return GrGLCreateDebugInterface();
}
