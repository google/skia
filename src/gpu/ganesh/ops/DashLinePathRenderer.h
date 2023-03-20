/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ganesh_DashLinePathRenderer_DEFINED
#define skgpu_ganesh_DashLinePathRenderer_DEFINED

#include "src/gpu/ganesh/PathRenderer.h"

class GrGpu;

namespace skgpu::ganesh {

class DashLinePathRenderer final : public skgpu::v1::PathRenderer {
public:
    DashLinePathRenderer() = default;

    const char* name() const override { return "DashLine"; }

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    StencilSupport onGetStencilSupport(const GrStyledShape&) const override {
        return kNoSupport_StencilSupport;
    }

    bool onDrawPath(const DrawPathArgs&) override;

    sk_sp<GrGpu> fGpu;
};

} // namespace skgpu::ganesh

#endif // skgpu_ganesh_DashLinePathRenderer_DEFINED
