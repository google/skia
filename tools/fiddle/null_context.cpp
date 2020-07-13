/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/fiddle/fiddle_main.h"

// create_direct_context for when neither Mesa nor EGL are available.
sk_sp<GrDirectContext> create_direct_context(std::ostringstream& driverinfo,
                                             std::unique_ptr<sk_gpu_test::GLTestContext>*) {
    driverinfo << "(no GL driver available)";
    return nullptr;
}
