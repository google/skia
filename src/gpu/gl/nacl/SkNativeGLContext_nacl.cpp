
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkNativeGLContext.h"

SkNativeGLContext::AutoContextRestore::AutoContextRestore() {
}

SkNativeGLContext::AutoContextRestore::~AutoContextRestore() {
}

SkNativeGLContext::SkNativeGLContext()
    : fContext(NULL)
    , fDisplay(NULL)
{
}

SkNativeGLContext::~SkNativeGLContext() {
    this->destroyGLContext();
}

void SkNativeGLContext::destroyGLContext() {
}

const GrGLInterface* SkNativeGLContext::createGLContext() {
    return NULL;
}

void SkNativeGLContext::makeCurrent() const {
}
