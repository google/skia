
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/SkNullGLContext.h"

SkNullGLContext::SkNullGLContext() {
    fGL.reset(GrGLCreateNullInterface());
}

SkNullGLContext::~SkNullGLContext() {
    fGL.reset(NULL);
}

