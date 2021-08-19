/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DefaultPathRenderer_DEFINED
#define DefaultPathRenderer_DEFINED

#include "src/gpu/v1/PathRenderer.h"

namespace skgpu::v1 {

/**
 *  Subclass that renders the path using the stencil buffer to resolve fill rules
 * (e.g. winding, even-odd)
 */
class DefaultPathRenderer final : public PathRenderer {
public:
    DefaultPathRenderer() = default;

    const char* name() const override { return "Default"; }

private:
    StencilSupport onGetStencilSupport(const GrStyledShape&) const override;

    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    void onStencilPath(const StencilPathArgs&) override;

    bool internalDrawPath(SurfaceDrawContext*,
                          GrPaint&&,
                          GrAAType,
                          const GrUserStencilSettings&,
                          const GrClip*,
                          const SkMatrix& viewMatrix,
                          const GrStyledShape&,
                          bool stencilOnly);
};

} // namespace skgpu::v1

#endif // DefaultPathRenderer_DEFINED
