/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTriangulatingPathRenderer_DEFINED
#define GrTriangulatingPathRenderer_DEFINED

#include "src/gpu/GrPathRenderer.h"

/**
 *  Subclass that renders the path by converting to screen-space trapezoids plus
 *   extra 1-pixel geometry for AA.
 */
class GrTriangulatingPathRenderer : public GrPathRenderer {
public:
    GrTriangulatingPathRenderer();
#if GR_TEST_UTILS
    void setMaxVerbCount(int maxVerbCount) { fMaxVerbCount = maxVerbCount; }
#endif

    const char* name() const final { return "Triangulating"; }

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    StencilSupport onGetStencilSupport(const GrStyledShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }

    bool onDrawPath(const DrawPathArgs&) override;
    int fMaxVerbCount;

    using INHERITED = GrPathRenderer;
};

#endif
