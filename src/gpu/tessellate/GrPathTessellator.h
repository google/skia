/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathTessellator_DEFINED
#define GrPathTessellator_DEFINED

#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

class SkPath;

// Prepares GPU data for, and then draws a path's tessellated geometry. Depending on the subclass,
// the caller may or may not be required to draw the path's inner fan separately.
class GrPathTessellator {
public:
    // Called before draw(). Creates GPU buffers containing the geometry to tessellate.
    virtual void prepare(GrMeshDrawOp::Target*, const SkMatrix&, const SkPath&) = 0;

    // Issues draw calls for the tessellated geometry. The caller is responsible for binding its
    // desired pipeline ahead of time.
    virtual void draw(GrOpFlushState*) const = 0;

    // Draws a 4-point instance for each curve. This method is used for drawing convex hulls over
    // each cubic with GrFillCubicHullShader. The caller is responsible for binding its desired
    // pipeline ahead of time. This method is not supported by every subclass.
    virtual void drawHullInstances(GrOpFlushState*) const { SK_ABORT("Not supported."); }

    virtual ~GrPathTessellator() {}
};

// Prepares and draws a list of indirect draw commands and instance data for the path's outer
// curves. Quadratics are converted to cubics. An outer curve is an independent, 4-point closed
// contour that represents either a cubic or a conic.
//
// For performance reasons we can often express triangles as an indirect curve draw and sneak them
// in alongside the other indirect draws. If emitInnerFanTriangles is true, then the path's inner
// fan is also included in the indirect draws.
class GrPathIndirectTessellator final : public GrPathTessellator {
public:
    GrPathIndirectTessellator(const SkMatrix&, const SkPath&, bool emitInnerFanTriangles);
    void prepare(GrMeshDrawOp::Target*, const SkMatrix&, const SkPath&) override;
    void draw(GrOpFlushState*) const override;
    void drawHullInstances(GrOpFlushState*) const override;

private:
    constexpr static float kLinearizationIntolerance =
            GrTessellationPathRenderer::kLinearizationIntolerance;
    constexpr static int kMaxResolveLevel = GrTessellationPathRenderer::kMaxResolveLevel;

    const bool fEmitInnerFanTriangles;
    int fResolveLevelCounts[kMaxResolveLevel + 1] = {0};
    int fOuterCurveInstanceCount = 0;

    sk_sp<const GrBuffer> fInstanceBuffer;
    int fBaseInstance;
    int fInstanceCount = 0;

    sk_sp<const GrBuffer> fIndirectDrawBuffer;
    size_t fIndirectDrawOffset;
    int fIndirectDrawCount = 0;
    sk_sp<const GrBuffer> fIndirectIndexBuffer;
};

// Base class for GrPathTessellators that draw actual hardware tessellation patches.
class GrPathHardwareTessellator : public GrPathTessellator {
public:
    void draw(GrOpFlushState*) const final;

protected:
    sk_sp<const GrBuffer> fPatchBuffer;
    int fBasePatchVertex;
    int fPatchVertexCount = 0;
};

// Draws an array of "outer curve" patches for GrCubicTessellateShader. Each patch is an independent
// 4-point curve, representing either a cubic or conic. Qudaratics are converted to cubics. The
// caller is responsible to stencil the path's inner fan along with these outer cubics.
class GrPathHWOuterCurveTessellator final : public GrPathHardwareTessellator {
public:
    void prepare(GrMeshDrawOp::Target*, const SkMatrix&, const SkPath&) override;
};

// Draws an array of "wedge" patches for GrWedgeTessellateShader. A wedge is an independent, 5-point
// closed contour consisting of 4 control points plus an anchor point fanning from the center of the
// curve's resident contour. A wedge can be ether a cubic or a conic. Qudaratics and lines are
// converted to cubics. Once stencilled, these wedges alone define the complete path.
class GrPathHWWedgeTessellator final : public GrPathHardwareTessellator {
public:
    void prepare(GrMeshDrawOp::Target*, const SkMatrix&, const SkPath&) override;
};

#endif
