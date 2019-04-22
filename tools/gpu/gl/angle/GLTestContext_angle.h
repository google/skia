
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLTestContext_angle_DEFINED
#define GLTestContext_angle_DEFINED

#include "tools/gpu/gl/GLTestContext.h"

namespace sk_gpu_test {

/**
 * Creates a GrGLInterface for the current ANGLE GLES Context. Here current means bound in ANGLE's
 * implementation of EGL.
 */
sk_sp<const GrGLInterface> CreateANGLEGLInterface();

enum class ANGLEBackend {
    kD3D9,
    kD3D11,
    kOpenGL
};

enum class ANGLEContextVersion {
    kES2,
    kES3
};

/** Creates a GLTestContext backed by ANGLE. */
std::unique_ptr<GLTestContext> MakeANGLETestContext(ANGLEBackend, ANGLEContextVersion,
                                                    GLTestContext* shareContext = nullptr,
                                                    void* display = nullptr);

}  // namespace sk_gpu_test
#endif
