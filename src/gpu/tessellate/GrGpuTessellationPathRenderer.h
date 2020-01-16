/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuTessellationPathRenderer_DEFINED
#define GrGpuTessellationPathRenderer_DEFINED

#include "src/gpu/GrPathRenderer.h"

// This is the tie-in point for path rendering via GrTessellatePathOp.
class GrGpuTessellationPathRenderer : public GrPathRenderer {
    StencilSupport onGetStencilSupport(const GrShape& shape) const override {
        // TODO: Single-pass (e.g., convex) paths can have full support.
        return kStencilOnly_StencilSupport;
    }

    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;
    bool onDrawPath(const DrawPathArgs&) override;
    void onStencilPath(const StencilPathArgs&) override;
};

#endif
