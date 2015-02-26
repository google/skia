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

    bool canDrawPath(const GrDrawTarget*,
                     const GrPipelineBuilder*,
                     const SkMatrix&,
                     const SkPath&,
                     const SkStrokeRec&,
                     bool antiAlias) const SK_OVERRIDE;
protected:

    StencilSupport onGetStencilSupport(const GrDrawTarget*,
                                       const GrPipelineBuilder*,
                                       const SkPath&,
                                       const SkStrokeRec&) const SK_OVERRIDE;

    bool onDrawPath(GrDrawTarget*,
                    GrPipelineBuilder*,
                    GrColor,
                    const SkMatrix& viewMatrix,
                    const SkPath&,
                    const SkStrokeRec&,
                    bool antiAlias) SK_OVERRIDE;

    typedef GrPathRenderer INHERITED;
};

#endif
