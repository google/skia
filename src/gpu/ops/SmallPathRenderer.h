/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SmallPathRenderer_DEFINED
#define SmallPathRenderer_DEFINED

#include "src/gpu/v1/PathRenderer.h"

class GrStyledShape;

namespace skgpu::v1 {

class SmallPathRenderer final : public PathRenderer {
public:
    SmallPathRenderer() = default;

    const char* name() const override { return "Small"; }

private:
    StencilSupport onGetStencilSupport(const GrStyledShape&) const override {
        return PathRenderer::kNoSupport_StencilSupport;
    }

    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;
};

} // namespace skgpu::v1

#endif // SmallPathRenderer_DEFINED
