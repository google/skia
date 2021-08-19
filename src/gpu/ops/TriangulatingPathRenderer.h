/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TriangulatingPathRenderer_DEFINED
#define TriangulatingPathRenderer_DEFINED

#include "src/gpu/v1/PathRenderer.h"

namespace skgpu::v1 {

/**
 *  Subclass that renders the path by converting to screen-space trapezoids plus
 *  extra 1-pixel geometry for AA.
 */
class TriangulatingPathRenderer final : public PathRenderer {
public:
    TriangulatingPathRenderer();
#if GR_TEST_UTILS
    void setMaxVerbCount(int maxVerbCount) { fMaxVerbCount = maxVerbCount; }
#endif

    const char* name() const override { return "Triangulating"; }

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    StencilSupport onGetStencilSupport(const GrStyledShape&) const override {
        return kNoSupport_StencilSupport;
    }

    bool onDrawPath(const DrawPathArgs&) override;

    int fMaxVerbCount;
};

} // namespace skgpu::v1

#endif // TriangulatingPathRenderer_DEFINED
