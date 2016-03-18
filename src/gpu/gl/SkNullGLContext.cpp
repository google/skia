
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/SkNullGLContext.h"
#include "gl/GrGLInterface.h"

SkNullGLContext* SkNullGLContext::Create() {
    SkNullGLContext* ctx = new SkNullGLContext;
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

SkNullGLContext::SkNullGLContext() {
    this->init(GrGLCreateNullInterface());
}

SkNullGLContext::~SkNullGLContext() {
    this->teardown();
}
