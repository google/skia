
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/debug/SkDebugGLContext.h"

SkDebugGLContext::SkDebugGLContext() {
    fGL.reset(GrGLCreateDebugInterface());
}

SkDebugGLContext::~SkDebugGLContext() {
    fGL.reset(NULL);
}
