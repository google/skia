
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPathRenderer.h"


class GrAAConvexPathRenderer : public GrPathRenderer {
public:
    GrAAConvexPathRenderer();
    bool canDrawPath(const GrDrawTarget::Caps& targetCaps,
                                       const SkPath& path,
                                       GrPathFill fill,
                                       bool antiAlias) const;
    void drawPath(GrDrawState::StageMask stageMask);
};
