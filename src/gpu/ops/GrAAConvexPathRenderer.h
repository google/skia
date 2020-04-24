/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAConvexPathRenderer_DEFINED
#define GrAAConvexPathRenderer_DEFINED

#include "src/gpu/GrPathRenderer.h"

class GrAAConvexPathRenderer : public GrPathRenderer {
public:
    GrAAConvexPathRenderer();

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;
};

#endif
