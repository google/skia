/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPathRenderer.h"

// This path renderer is made to create geometry (i.e. primitives) from the original path (before
// the path is stroked) and render using the GPU directly rather than using any software rendering
// step. It can be rendered in a single pass for simple cases and use multiple passes for features
// like AA or opacity support.

class GrStrokePathRenderer : public GrPathRenderer {

public:
    GrStrokePathRenderer();

    virtual bool canDrawPath(const SkPath& path,
                             const SkStrokeRec& stroke,
                             const GrDrawTarget* target,
                             bool antiAlias) const SK_OVERRIDE;

protected:
    virtual bool onDrawPath(const SkPath& path,
                            const SkStrokeRec& stroke,
                            GrDrawTarget* target,
                            bool antiAlias) SK_OVERRIDE;
};
