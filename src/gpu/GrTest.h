
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTest_DEFINED
#define GrTest_DEFINED

#include "GrContext.h"
#include "GrDrawTarget.h"
#include "gl/GrGLContext.h"

/** Allows a test to temporarily draw to a GrDrawTarget owned by a GrContext. Tests that use this
    should be careful not to mix using the GrDrawTarget directly and drawing via SkCanvas or
    GrContext. In the future this object may provide some guards to prevent this. */
class GrTestTarget {
public:
    GrTestTarget() : fGLContext(NULL) {};

    void init(GrContext*, GrDrawTarget*, const GrGLContext*);

    GrDrawTarget* target() { return fDrawTarget.get(); }

    /** Returns a GrGLContext if the GrContext is backed by OpenGL. */
    const GrGLContext* glContext() { return fGLContext; }

private:
    SkAutoTUnref<GrDrawTarget>              fDrawTarget;
    SkAutoTUnref<GrContext>                 fContext;
    SkAutoTUnref<const GrGLContext>         fGLContext;
};

#endif
