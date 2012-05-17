
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

    virtual bool canDrawPath(const SkPath& path,
                             GrPathFill fill,
                             const GrDrawTarget* target,
                             bool antiAlias) const SK_OVERRIDE;

    static bool staticCanDrawPath(bool pathIsConvex,
                                  GrPathFill fill,
                                  const GrDrawTarget* target,
                                  bool antiAlias);

protected:
    virtual bool onDrawPath(const SkPath& path,
                            GrPathFill fill,
                            const GrVec* translate,
                            GrDrawTarget* target,
                            GrDrawState::StageMask stageMask,
                            bool antiAlias) SK_OVERRIDE;
};
