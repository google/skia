
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLContext_angle_DEFINED
#define GLContext_angle_DEFINED

#include "gl/GLContext.h"

namespace sk_gpu_test {

/**
 * Creates a GrGLInterface for the currently ANGLE GL context currently bound in ANGLE's EGL
 * implementation.
 */
const GrGLInterface* CreateANGLEGLInterface();

#ifdef SK_BUILD_FOR_WIN
/** Creates a GLContext backed by ANGLE's Direct3D backend. */
GLContext* CreateANGLEDirect3DGLContext();
#endif

/** Creates a GLContext backed by ANGLE's OpenGL backend. */
GLContext* CreateANGLEOpenGLGLContext();

}  // namespace sk_gpu_test
#endif
