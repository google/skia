/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellatingPathRenderer_DEFINED
#define GrTessellatingPathRenderer_DEFINED

#include "GrPathRenderer.h"

/**
 *  Subclass that renders the path by converting to screen-space trapezoids plus
 *   extra 1-pixel geometry for AA.
 */
class SK_API GrTessellatingPathRenderer : public GrPathRenderer {
public:
    GrTessellatingPathRenderer();

private:
    bool onCanDrawPath(const CanDrawPathArgs& ) const override;

    StencilSupport onGetStencilSupport(const GrDrawTarget*,
                                       const GrPipelineBuilder*,
                                       const SkPath&,
                                       const GrStrokeInfo&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    typedef GrPathRenderer INHERITED;
};

#endif
