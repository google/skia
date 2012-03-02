
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrTesselatedPathRenderer_DEFINED
#define GrTesselatedPathRenderer_DEFINED

#include "GrPathRenderer.h"

class GrTesselatedPathRenderer : public GrPathRenderer {
public:
    GrTesselatedPathRenderer();

    virtual bool canDrawPath(const SkPath& path,
                             GrPathFill fill,
                             const GrDrawTarget* target,
                             bool antiAlias) const SK_OVERRIDE;

    virtual bool onDrawPath(const SkPath& path,
                            GrPathFill fill,
                            const GrVec* translate,
                            GrDrawTarget* target,
                            GrDrawState::StageMask stageMask,
                            bool antiAlias) SK_OVERRIDE;
};

#endif
