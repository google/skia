/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellationPathRenderer_DEFINED
#define GrTessellationPathRenderer_DEFINED

#include "include/gpu/GrTypes.h"

class GrCaps;

#if SK_GPU_V1

#include "src/gpu/GrPathRenderer.h"

// This is the tie-in point for path rendering via GrPathTessellateOp. This path renderer draws
// paths using a hybrid Red Book "stencil, then cover" method. Curves get linearized by GPU
// tessellation shaders. This path renderer doesn't apply analytic AA, so it requires MSAA if AA is
// desired.
class GrTessellationPathRenderer : public GrPathRenderer {
public:
    // We send these flags to the internal path filling Ops to control how a path gets rendered.
    enum class PathFlags {
        kNone = 0,
        kStencilOnly = (1 << 0),
        kWireframe = (1 << 1)
    };

    static bool IsSupported(const GrCaps&);

    const char* name() const final { return "GrTessellationPathRenderer"; }
    StencilSupport onGetStencilSupport(const GrStyledShape&) const override;
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;
    bool onDrawPath(const DrawPathArgs&) override;
    void onStencilPath(const StencilPathArgs&) override;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrTessellationPathRenderer::PathFlags)

#else // SK_GPU_V1

class GrTessellationPathRenderer {
public:
    // We send these flags to the internal path filling Ops to control how a path gets rendered.
    enum class PathFlags {
        kNone = 0,
        kStencilOnly = (1 << 0),
        kWireframe = (1 << 1)
    };

    static bool IsSupported(const GrCaps&) { return false; }

};

GR_MAKE_BITFIELD_CLASS_OPS(GrTessellationPathRenderer::PathFlags)

#endif // SK_GPU_V1

#endif
