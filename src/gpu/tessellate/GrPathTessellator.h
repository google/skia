/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathTessellator_DEFINED
#define GrPathTessellator_DEFINED

#include "src/gpu/GrInnerFanTriangulator.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

class SkPath;

// Prepares GPU data for, and then draws a path's tessellated geometry. Depending on the subclass,
// the caller may or may not be required to draw the path's inner fan separately.
class GrPathTessellator {
public:
    using BreadcrumbTriangleList = GrInnerFanTriangulator::BreadcrumbTriangleList;

    // Called before draw(). Prepares GPU buffers containing the geometry to tessellate. If the
    // given BreadcrumbTriangleList is non-null, then this class will also include the breadcrumb
    // triangles in its draw.
    virtual void prepare(GrMeshDrawOp::Target*, const SkMatrix&, const SkPath&,
                         const BreadcrumbTriangleList* = nullptr) = 0;

    // Issues draw calls for the tessellated geometry. The caller is responsible for binding its
    // desired pipeline ahead of time.
    virtual void draw(GrOpFlushState*) const = 0;

    // Draws a 4-point instance for each curve. This method is used for drawing convex hulls over
    // each cubic with GrFillCubicHullShader. The caller is responsible for binding its desired
    // pipeline ahead of time. This method is not supported by every subclass.
    virtual void drawHullInstances(GrOpFlushState*) const { SK_ABORT("Not supported."); }

    virtual ~GrPathTessellator() {}
};

// Draws tessellations of the path's outer curves using indirect draw commands. Quadratics are
// converted to cubics. An outer curve is an independent, 4-point closed contour that represents
// either a cubic or a conic.
//
// For performance reasons we can often express triangles as one of these indirect draws and sneak
// them in alongside the other curves. If DrawInnerFan is kYes, then this class also draws the
// path's inner fan along with the outer curves.
class GrPathIndirectTessellator final : public GrPathTessellator {
public:
    enum class DrawInnerFan : bool { kNo = false, kYes };
    GrPathIndirectTessellator(const SkMatrix&, const SkPath&, DrawInnerFan);

    void prepare(GrMeshDrawOp::Target*, const SkMatrix&, const SkPath&,
                 const BreadcrumbTriangleList*) override;
    void draw(GrOpFlushState*) const override;
    void drawHullInstances(GrOpFlushState*) const override;

private:
    constexpr static float kLinearizationIntolerance =
            GrTessellationPathRenderer::kLinearizationIntolerance;
    constexpr static int kMaxResolveLevel = GrTessellationPathRenderer::kMaxResolveLevel;

    const bool fDrawInnerFan;
    int fResolveLevelCounts[kMaxResolveLevel + 1] = {0};
    int fOuterCurveInstanceCount = 0;

    sk_sp<const GrBuffer> fInstanceBuffer;
    int fBaseInstance = 0;
    int fTotalInstanceCount = 0;

    sk_sp<const GrBuffer> fIndirectDrawBuffer;
    size_t fIndirectDrawOffset = 0;
    int fIndirectDrawCount = 0;
    sk_sp<const GrBuffer> fIndirectIndexBuffer;
};

// Base class for GrPathTessellators that draw actual hardware tessellation patches.
class GrPathHardwareTessellator : public GrPathTessellator {
public:
    GrPathHardwareTessellator() = default;

    void draw(GrOpFlushState*) const final;

protected:
    sk_sp<const GrBuffer> fPatchBuffer;
    int fBasePatchVertex = 0;
    int fPatchVertexCount = 0;
};

// Draws an array of "outer curve" patches for GrCubicTessellateShader. Each patch is an independent
// 4-point curve, representing either a cubic or conic. Qudaratics are converted to cubics. The
// caller is responsible to stencil the path's inner fan along with these outer cubics.
class GrPathOuterCurveTessellator final : public GrPathHardwareTessellator {
public:
    GrPathOuterCurveTessellator() = default;

    void prepare(GrMeshDrawOp::Target*, const SkMatrix&, const SkPath&,
                 const BreadcrumbTriangleList*) override;
};

// Draws an array of "wedge" patches for GrWedgeTessellateShader. A wedge is an independent, 5-point
// closed contour consisting of 4 control points plus an anchor point fanning from the center of the
// curve's resident contour. A wedge can be either a cubic or a conic. Qudaratics and lines are
// converted to cubics. Once stencilled, these wedges alone define the complete path.
class GrPathWedgeTessellator final : public GrPathHardwareTessellator {
public:
    GrPathWedgeTessellator() = default;

    void prepare(GrMeshDrawOp::Target*, const SkMatrix&, const SkPath&,
                 const BreadcrumbTriangleList*) override;
};

#endif
