/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellatePathOp_DEFINED
#define GrTessellatePathOp_DEFINED

#include "src/gpu/ops/GrMeshDrawOp.h"

class GrAppliedHardClip;
class GrStencilPathShader;

// Renders paths using a hybrid Red Book "stencil, then cover" method. Curves get linearized by
// GPU tessellation shaders. This Op doesn't apply analytic AA, so it requires a render target that
// supports either MSAA or mixed samples if AA is desired.
class GrTessellatePathOp : public GrDrawOp {
public:
    enum class Flags {
        kNone = 0,
        kStencilOnly = (1 << 0),
        kWireframe = (1 << 1)
    };

private:
    DEFINE_OP_CLASS_ID

    GrTessellatePathOp(const SkMatrix& viewMatrix, const SkPath& path, GrPaint&& paint,
                       GrAAType aaType, Flags flags = Flags::kNone)
            : GrDrawOp(ClassID())
            , fFlags(flags)
            , fViewMatrix(viewMatrix)
            , fPath(path)
            , fAAType(aaType)
            , fColor(paint.getColor4f())
            , fProcessors(std::move(paint)) {
        SkRect devBounds;
        fViewMatrix.mapRect(&devBounds, path.getBounds());
        this->setBounds(devBounds, HasAABloat(GrAAType::kCoverage == fAAType), IsHairline::kNo);
    }

    const char* name() const override { return "GrTessellatePathOp"; }
    void visitProxies(const VisitProxyFunc& fn) const override { fProcessors.visitProxies(fn); }
    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      bool hasMixedSampledCoverage,
                                      GrClampType clampType) override {
        return fProcessors.finalize(
                fColor, GrProcessorAnalysisCoverage::kNone, clip, &GrUserStencilSettings::kUnused,
                hasMixedSampledCoverage, caps, clampType, &fColor);
    }

    FixedFunctionFlags fixedFunctionFlags() const override;
    void onPrePrepare(GrRecordingContext*, const GrSurfaceProxyView*, GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&) override;
    void onPrepare(GrOpFlushState* state) override;

    // Produces a non-overlapping triangulation of the path's inner polygon(s). The inner polygons
    // connect the endpoints of each verb. (i.e., they are the path that would result from
    // collapsing all curves to single lines.) If this succeeds, then we will be able to fill the
    // triangles directly and bypass stencilling them.
    //
    // Returns false if the inner triangles do not form a simple polygon (e.g., self intersection,
    // double winding). Non-simple polygons would need to split edges in order to avoid overlap,
    // and this is not an option as it would introduce T-junctions with the outer cubics.
    bool prepareNonOverlappingInnerTriangles(GrMeshDrawOp::Target*, int* numCountedCurves);

    // Produces a "Red Book" style triangulation of the SkPath's inner polygon(s). The inner
    // polygons connect the endpoints of each verb. (i.e., they are the path that would result from
    // collapsing all curves to single lines.) Stencilled together with the outer cubics, these
    // define the complete path.
    //
    // This method emits the inner triangles with a "middle-out" topology. Middle-out can reduce
    // the load on the rasterizer by a great deal as compared to a linear triangle strip or fan.
    // See GrMiddleOutPolygonTriangulator.
    void prepareMiddleOutInnerTriangles(GrMeshDrawOp::Target*, int* numCountedCurves);

    enum class CubicDataAlignment : bool {
        kVertexBoundary,
        kInstanceBoundary
    };

    // Writes an array of "outer" cubics from each bezier in the SkPath, converting any quadratics
    // to cubics. An outer cubic is an independent, 4-point closed contour consisting of a single
    // cubic curve. Stencilled together with the inner triangles, these define the complete path.
    void prepareOuterCubics(GrMeshDrawOp::Target*, int numCountedCurves, CubicDataAlignment);

    // Writes an array of cubic "wedges" from the SkPath, converting any lines or quadratics to
    // cubics. A wedge is an independent, 5-point closed contour consisting of 4 cubic control
    // points plus an anchor point fanning from the center of the curve's resident contour. Once
    // stencilled, these wedges alone define the complete path.
    //
    // TODO: Eventually we want to use rational cubic wedges in order to support conics.
    void prepareCubicWedges(GrMeshDrawOp::Target*);

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;
    void drawStencilPass(GrOpFlushState*);
    void drawCoverPass(GrOpFlushState*);

    const Flags fFlags;
    const SkMatrix fViewMatrix;
    const SkPath fPath;
    const GrAAType fAAType;
    SkPMColor4f fColor;
    GrProcessorSet fProcessors;

    sk_sp<const GrBuffer> fTriangleBuffer;
    int fBaseTriangleVertex;
    int fTriangleVertexCount;

    // These switches specify how the above fTriangleBuffer should be drawn (if at all).
    //
    // If stencil=true and fill=false:
    //
    //     We just stencil the triangles (with cubics) during the stencil step. The path gets filled
    //     later using a simple bounding box if needed.
    //
    // If stencil=true and fill=true:
    //
    //     We still stencil the triangles and cubics normally, but during the *fill* step we fill
    //     the triangles plus local convex hulls around each cubic instead of a bounding box.
    //
    // If stencil=false and fill=true:
    //
    //     This means that fTriangleBuffer contains non-overlapping geometry that can be filled
    //     directly to the final render target. We only need to stencil *curves*, and during the
    //     fill step we draw the triangles directly with a stencil test that accounts for curves
    //     (see drawCoverPass()), and then finally fill the curves with local convex hulls.
    bool fDoStencilTriangleBuffer = false;
    bool fDoFillTriangleBuffer = false;

    // The cubic buffer defines either standalone cubics or wedges. These are stencilled by
    // tessellation shaders, and may also be used do fill local convex hulls around each cubic.
    sk_sp<const GrBuffer> fCubicBuffer;
    int fBaseCubicVertex;
    int fCubicVertexCount;
    GrStencilPathShader* fStencilCubicsShader = nullptr;

    friend class GrOpMemoryPool;  // For ctor.

public:
    // This serves as a base class for benchmarking individual methods on GrTessellatePathOp.
    class TestingOnly_Benchmark;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrTessellatePathOp::Flags);

#endif
