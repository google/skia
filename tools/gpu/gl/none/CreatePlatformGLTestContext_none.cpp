
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GLTestContext.h"

namespace sk_gpu_test {
GLTestContext* CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI,
                                           GLTestContext *shareContext) {
    return nullptr;
}
}  // namespace sk_gpu_test

