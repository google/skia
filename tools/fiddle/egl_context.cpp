/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "SkRefCnt.h"
#include "gl/GrGLFunctions.h"
#include "gl/GrGLInterface.h"
#include "gl/GLTestContext.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <sstream>

// create_grcontext implementation for EGL.
sk_sp<GrContext> create_grcontext(std::ostringstream &driverinfo) {
    // We are leaking tc, but that's OK because fiddle is a short lived proces.
    sk_gpu_test::GLTestContext* tc = sk_gpu_test::CreatePlatformGLTestContext(kGLES_GrGLStandard);
    if (!tc) {
        return nullptr;
    }
    tc->makeCurrent();
    sk_sp<GrContext> result = tc->makeGrContext(GrContextOptions());
    if (!result) {
        return nullptr;
    }

    driverinfo << "GL Version: " << glGetString(GL_VERSION) << "\n";
    driverinfo << "GL Vendor: " << glGetString(GL_VENDOR) << "\n";
    driverinfo << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
    driverinfo << "GL Extensions: " << glGetString(GL_EXTENSIONS) << "\n";

    return result;
}
