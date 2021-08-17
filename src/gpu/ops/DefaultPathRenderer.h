/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DefaultPathRenderer_DEFINED
#define DefaultPathRenderer_DEFINED

#include "include/core/SkTypes.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/ops/GrPathStencilSettings.h"

namespace skgpu::v1 {

/**
 *  Subclass that renders the path using the stencil buffer to resolve fill rules
 * (e.g. winding, even-odd)
 */
class DefaultPathRenderer final : public GrPathRenderer {
public:
    DefaultPathRenderer() = default;

    const char* name() const override { return "Default"; }

private:
    StencilSupport onGetStencilSupport(const GrStyledShape&) const override;

    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    void onStencilPath(const StencilPathArgs&) override;

    bool internalDrawPath(skgpu::v1::SurfaceDrawContext*,
                          GrPaint&&,
                          GrAAType,
                          const GrUserStencilSettings&,
                          const GrClip*,
                          const SkMatrix& viewMatrix,
                          const GrStyledShape&,
                          bool stencilOnly);

    using INHERITED = GrPathRenderer;
};

} // namespace skgpu::v1

#endif // DefaultPathRenderer_DEFINED
