
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef NullGLContext_DEFINED
#define NullGLContext_DEFINED

#include "gl/GLTestContext.h"

namespace sk_gpu_test {
GLTestContext* CreateNullGLTestContext(bool enableNVPR, GLTestContext* shareContext);
}  // namespace sk_gpu_test

#endif
