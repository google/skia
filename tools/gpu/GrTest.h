/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTest_DEFINED
#define GrTest_DEFINED

#include "GrContext.h"
#include "GrDrawContext.h"

namespace GrTest {
    /**
     * Forces the GrContext to use a small atlas which only has room for one plot and will thus
     * constantly be evicting entries
     */
    void SetupAlwaysEvictAtlas(GrContext*);
};

/** TODO Please do not use this if you can avoid it.  We are in the process of deleting it.
    Allows a test to temporarily draw to a GrDrawTarget owned by a GrContext. Tests that use this
    should be careful not to mix using the GrDrawTarget directly and drawing via SkCanvas or
    GrContext. In the future this object may provide some guards to prevent this. */
class GrTestTarget {
public:
    GrTestTarget() {};

    void init(GrContext*, sk_sp<GrDrawContext>);

    GrResourceProvider* resourceProvider() { return fContext->resourceProvider(); }

private:
    SkAutoTUnref<GrContext>                 fContext;
    sk_sp<GrDrawContext>                    fDrawContext;
};

#endif
