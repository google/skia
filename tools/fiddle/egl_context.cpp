/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/gl/GrGLFunctions.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "tools/gpu/gl/GLTestContext.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <sstream>

// create_grcontext implementation for EGL.
sk_sp<GrContext> create_grcontext(std::ostringstream& driverinfo,
                                  std::unique_ptr<sk_gpu_test::GLTestContext>* glContext) {
    // We are leaking tc, but that's OK because fiddle is a short lived proces.
    glContext->reset(sk_gpu_test::CreatePlatformGLTestContext(kGLES_GrGLStandard));
    if (!glContext) {
        return nullptr;
    }
    (*glContext)->makeCurrent();
    sk_sp<GrContext> result = (*glContext)->makeGrContext(GrContextOptions());
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
