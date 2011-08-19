
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkEGLContext.h"

SkEGLContext::SkEGLContext()
    : fFBO(0) {
}

SkEGLContext::~SkEGLContext() {
}

bool SkEGLContext::init(int width, int height) {
    return false;
}
