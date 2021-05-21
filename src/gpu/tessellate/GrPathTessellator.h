/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathTessellator_DEFINED
#define GrPathTessellator_DEFINED

#include "src/gpu/GrInnerFanTriangulator.h"
#include "src/gpu/GrVertexChunkArray.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

class SkPath;

// Prepares GPU data for, and then draws a path's tessellated geometry. Depending on the subclass,
// the caller may or may not be required to draw the path's inner fan separately.
class GrPathTessellator {
public:
    using BreadcrumbTriangleList = GrInnerFanTriangulator::BreadcrumbTriangleList;

    // For subclasses that use this enum, if DrawInnerFan is kNo, the class only emits the path's
    // outer curves. In that case the caller is responsible to handle the path's inner fan.
    enum class DrawInnerFan : bool {
        kNo = false,
        kYes
    };

    // Called before draw(). Prepares GPU buffers containing the geometry to tessellate. If the
    // given BreadcrumbTriangleList is non-null, then this class will also include the breadcrumb
    // triangles in its draw.
    virtual void prepare(GrMeshDrawOp::Target*, const SkRect& cullBounds, const SkMatrix&,
                         const SkPath&, const BreadcrumbTriangleList* = nullptr) = 0;

    // Issues draw calls for the tessellated geometry. The caller is responsible for binding its
    // desired pipeline ahead of time.
    virtual void draw(GrOpFlushState*) const = 0;

    // Draws a 4-point instance for each curve. This method is used for drawing convex hulls over
    // each cubic with GrFillCubicHullShader. The caller is responsible for binding its desired
    // pipeline ahead of time. This method is not supported by every subclass.
    virtual void drawHullInstances(GrOpFlushState*) const { SK_ABORT("Not supported."); }

    virtual ~GrPathTessellator() {}
};

// Draws tessellations of the path's outer curves and, optionally, inner fan triangles using
// indirect draw commands. Quadratics are converted to cubics and triangles are converted to conics
// with w=Inf. An outer curve is an independent, 4-point closed contour that represents either a
// cubic or a conic.
class GrPathIndirectTessellator final : public GrPathTessellator {
public:
    GrPathIndirectTessellator(const SkMatrix&, const SkPath&, DrawInnerFan);

    void prepare(GrMeshDrawOp::Target*, const SkRect& cullBounds, const SkMatrix&, const SkPath&,
                 const BreadcrumbTriangleList*) override;
    void draw(GrOpFlushState*) const override;
    void drawHullInstances(GrOpFlushState*) const override;

private:
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
};

// Base class for GrPathTessellators that draw actual hardware tessellation patches.
class GrPathHardwareTessellator : public GrPathTessellator {
public:
    GrPathHardwareTessellator(int numVerticesPerPatch)
            : fNumVerticesPerPatch(numVerticesPerPatch) {}

    void draw(GrOpFlushState*) const final;

protected:
    GrVertexChunkArray fVertexChunkArray;
    int fNumVerticesPerPatch;
};

// Draws an array of "outer curve" patches and, optionally, inner fan triangles for
// GrCubicTessellateShader. Each patch is an independent 4-point curve, representing either a cubic
// or a conic. Quadratics are converted to cubics and triangles are converted to conics with w=Inf.
class GrPathOuterCurveTessellator final : public GrPathHardwareTessellator {
public:
    GrPathOuterCurveTessellator(DrawInnerFan drawInnerFan)
            : GrPathHardwareTessellator(4), fDrawInnerFan(drawInnerFan == DrawInnerFan::kYes) {}

    void prepare(GrMeshDrawOp::Target*, const SkRect& cullBounds, const SkMatrix&, const SkPath&,
                 const BreadcrumbTriangleList*) override;

private:
    const bool fDrawInnerFan;
};

// Draws an array of "wedge" patches for GrWedgeTessellateShader. A wedge is an independent, 5-point
// closed contour consisting of 4 control points plus an anchor point fanning from the center of the
// curve's resident contour. A wedge can be either a cubic or a conic. Quadratics and lines are
// converted to cubics. Once stencilled, these wedges alone define the complete path.
class GrPathWedgeTessellator final : public GrPathHardwareTessellator {
public:
    GrPathWedgeTessellator() : GrPathHardwareTessellator(5) {}

    void prepare(GrMeshDrawOp::Target*, const SkRect& cullBounds, const SkMatrix&, const SkPath&,
                 const BreadcrumbTriangleList*) override;
};

#endif
