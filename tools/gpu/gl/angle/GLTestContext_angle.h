
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLTestContext_angle_DEFINED
#define GLTestContext_angle_DEFINED

#include "gl/GLTestContext.h"

namespace sk_gpu_test {

/**
 * Creates a GrGLInterface for the currently ANGLE GL context currently bound in ANGLE's EGL
 * implementation.
 */
const GrGLInterface* CreateANGLEGLInterface();

#ifdef SK_BUILD_FOR_WIN
/** Creates a GLTestContext backed by ANGLE's Direct3D backend. */
GLTestContext* CreateANGLEDirect3DGLTestContext();
#endif

/** Creates a GLTestContext backed by ANGLE's OpenGL backend. */
GLTestContext* CreateANGLEOpenGLGLTestContext();

}  // namespace sk_gpu_test
#endif
