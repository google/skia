/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SmallPathRenderer_DEFINED
#define SmallPathRenderer_DEFINED

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)

#include "src/gpu/ganesh/PathRenderer.h"

class GrStyledShape;

namespace skgpu::ganesh {

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

}  // namespace skgpu::ganesh

#endif // SK_ENABLE_OPTIMIZE_SIZE

#endif // SmallPathRenderer_DEFINED
