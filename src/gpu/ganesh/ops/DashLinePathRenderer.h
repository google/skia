/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ganesh_DashLinePathRenderer_DEFINED
#define skgpu_ganesh_DashLinePathRenderer_DEFINED

#include "src/gpu/ganesh/PathRenderer.h"

namespace skgpu::ganesh {

class DashLinePathRenderer final : public skgpu::ganesh::PathRenderer {
public:
    DashLinePathRenderer() = default;

    const char* name() const override { return "DashLine"; }

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    StencilSupport onGetStencilSupport(const GrStyledShape&) const override {
        return kNoSupport_StencilSupport;
    }

    bool onDrawPath(const DrawPathArgs&) override;
};

} // namespace skgpu::ganesh

#endif // skgpu_ganesh_DashLinePathRenderer_DEFINED
