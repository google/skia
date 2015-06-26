
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
#include "gl/GrGLInterface.h"

/** Allows a test to temporarily draw to a GrDrawTarget owned by a GrContext. Tests that use this
    should be careful not to mix using the GrDrawTarget directly and drawing via SkCanvas or
    GrContext. In the future this object may provide some guards to prevent this. */
class GrTestTarget {
public:
    GrTestTarget() {};

    void init(GrContext*, GrDrawTarget*, const GrGLInterface*);

    GrDrawTarget* target() { return fDrawTarget.get(); }

    /** Returns a GrGLInterface if the GrContext is backed by OpenGL. */
    const GrGLInterface* glInterface() { return fGLInterface.get(); }

private:
    SkAutoTUnref<GrDrawTarget>              fDrawTarget;
    SkAutoTUnref<GrContext>                 fContext;
    SkAutoTUnref<const GrGLInterface>       fGLInterface;
};

#endif
