/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLFunctions.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "tools/gpu/gl/GLTestContext.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <sstream>

// create_direct_context implementation for EGL.
sk_sp<GrDirectContext> create_direct_context(
        std::ostringstream& driverinfo,
        std::unique_ptr<sk_gpu_test::GLTestContext>* glContext) {
    glContext->reset(sk_gpu_test::CreatePlatformGLTestContext(kGLES_GrGLStandard));
    if (!glContext) {
        return nullptr;
    }
    (*glContext)->makeCurrent();
    sk_sp<GrDirectContext> result = (*glContext)->makeContext(GrContextOptions());
    if (!result) {
        glContext->reset();
        return nullptr;
    }

    driverinfo << "GL Version: " << glGetString(GL_VERSION) << "\n";
    driverinfo << "GL Vendor: " << glGetString(GL_VENDOR) << "\n";
    driverinfo << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
    driverinfo << "GL Extensions: " << glGetString(GL_EXTENSIONS) << "\n";

    return result;
}
