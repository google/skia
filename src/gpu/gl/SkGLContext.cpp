
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLContext.h"
#include "GrGLUtil.h"

SkGLContext::SkGLContext() {
}

SkGLContext::~SkGLContext() {
    SkASSERT(NULL == fGL.get());  // Subclass should destroy the interface.
}

void SkGLContext::testAbandon() {
    if (fGL) {
        fGL->abandon();
    }
}
