/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrDefaultPathRenderer.h"

#include "include/core/SkString.h"
#include "include/core/SkStrokeRec.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrSurfaceContextPriv.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

GrDefaultPathRenderer::GrDefaultPathRenderer() {
}

////////////////////////////////////////////////////////////////////////////////
// Helpers for drawPath

#define STENCIL_OFF     0   // Always disable stencil (even when needed)

static inline bool single_pass_shape(const GrShape& shape) {
#if STENCIL_OFF
    return true;
#else
    // Inverse fill is always two pass.
    if (shape.inverseFilled()) {
        return false;
    }
    // This path renderer only accepts simple fill paths or stroke paths that are either hairline
    // or have a stroke width small enough to treat as hairline. Hairline paths are always single
    // pass. Filled paths are single pass if they're convex.
    if (shape.style().isSimpleFill()) {
        return shape.knownToBeConvex();
    }
    return true;
#endif
}

GrPathRenderer::StencilSupport
GrDefaultPathRenderer::onGetStencilSupport(const GrShape& shape) const {
    if (single_pass_shape(shape)) {
        return GrPathRenderer::kNoRestriction_StencilSupport;
    } else {
        return GrPathRenderer::kStencilOnly_StencilSupport;
    }
}

namespace {

class PathGeoBuilder {
public:
    PathGeoBuilder(GrPrimitiveType primitiveType, GrMeshDrawOp::Target* target,
                   sk_sp<const GrGeometryProcessor> geometryProcessor)
            : fPrimitiveType(primitiveType)
            , fTarget(target)
            , fVertexStride(sizeof(SkPoint))
            , fGeometryProcessor(std::move(geometryProcessor))
            , fFirstIndex(0)
            , fIndicesInChunk(0)
            , fIndices(nullptr) {
        this->allocNewBuffers();
    }

    ~PathGeoBuilder() {
        this->emitMeshAndPutBackReserve();
    }

    /**
     *  Path verbs
     */
    void moveTo(const SkPoint& p) {
        needSpace(1);

        fSubpathIndexStart = this->currentIndex();
        *(fCurVert++) = p;
    }

    void addLine(const SkPoint& p) {
        needSpace(1, this->indexScale());

        if (this->isIndexed()) {
            uint16_t prevIdx = this->currentIndex() - 1;
            appendCountourEdgeIndices(prevIdx);
        }
        *(fCurVert++) = p;
    }

    void addQuad(const SkPoint pts[], SkScalar srcSpaceTolSqd, SkScalar srcSpaceTol) {
        this->needSpace(GrPathUtils::kMaxPointsPerCurve,
                        GrPathUtils::kMaxPointsPerCurve * this->indexScale());

        // First pt of quad is the pt we ended on in previous step
        uint16_t firstQPtIdx = this->currentIndex() - 1;
        uint16_t numPts = (uint16_t)GrPathUtils::generateQuadraticPoints(
                pts[0], pts[1], pts[2], srcSpaceTolSqd, &fCurVert,
                GrPathUtils::quadraticPointCount(pts, srcSpaceTol));
        if (this->isIndexed()) {
            for (uint16_t i = 0; i < numPts; ++i) {
                appendCountourEdgeIndices(firstQPtIdx + i);
            }
        }
    }

    void addConic(SkScalar weight, const SkPoint pts[], SkScalar srcSpaceTolSqd,
                  SkScalar srcSpaceTol) {
        SkAutoConicToQuads converter;
        const SkPoint* quadPts = converter.computeQuads(pts, weight, srcSpaceTol);
        for (int i = 0; i < converter.countQuads(); ++i) {
            this->addQuad(quadPts + i * 2, srcSpaceTolSqd, srcSpaceTol);
        }
    }

    void addCubic(const SkPoint pts[], SkScalar srcSpaceTolSqd, SkScalar srcSpaceTol) {
        this->needSpace(GrPathUtils::kMaxPointsPerCurve,
                        GrPathUtils::kMaxPointsPerCurve * this->indexScale());

        // First pt of cubic is the pt we ended on in previous step
        uint16_t firstCPtIdx = this->currentIndex() - 1;
        uint16_t numPts = (uint16_t) GrPathUtils::generateCubicPoints(
                pts[0], pts[1], pts[2], pts[3], srcSpaceTolSqd, &fCurVert,
                GrPathUtils::cubicPointCount(pts, srcSpaceTol));
        if (this->isIndexed()) {
            for (uint16_t i = 0; i < numPts; ++i) {
                appendCountourEdgeIndices(firstCPtIdx + i);
            }
        }
    }

    void addPath(const SkPath& path, SkScalar srcSpaceTol) {
        SkScalar srcSpaceTolSqd = srcSpaceTol * srcSpaceTol;

        SkPath::Iter iter(path, false);
        SkPoint pts[4];

        bool done = false;
        while (!done) {
            SkPath::Verb verb = iter.next(pts, false);
            switch (verb) {
                case SkPath::kMove_Verb:
                    this->moveTo(pts[0]);
                    break;
                case SkPath::kLine_Verb:
                    this->addLine(pts[1]);
                    break;
                case SkPath::kConic_Verb:
                    this->addConic(iter.conicWeight(), pts, srcSpaceTolSqd, srcSpaceTol);
                    break;
                case SkPath::kQuad_Verb:
                    this->addQuad(pts, srcSpaceTolSqd, srcSpaceTol);
                    break;
                case SkPath::kCubic_Verb:
                    this->addCubic(pts, srcSpaceTolSqd, srcSpaceTol);
                    break;
                case SkPath::kClose_Verb:
                    break;
                case SkPath::kDone_Verb:
                    done = true;
            }
        }
    }

    static bool PathHasMultipleSubpaths(const SkPath& path) {
        bool first = true;

        SkPath::Iter iter(path, false);
        SkPath::Verb verb;

        SkPoint pts[4];
        while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
            if (SkPath::kMove_Verb == verb && !first) {
                return true;
            }
            first = false;
        }
        return false;
    }

private:
    /**
     *  Derived properties
     *  TODO: Cache some of these for better performance, rather than re-computing?
     */
    bool isIndexed() const {
        return GrPrimitiveType::kLines == fPrimitiveType ||
               GrPrimitiveType::kTriangles == fPrimitiveType;
    }
    bool isHairline() const {
        return GrPrimitiveType::kLines == fPrimitiveType ||
               GrPrimitiveType::kLineStrip == fPrimitiveType;
    }
    int indexScale() const {
        switch (fPrimitiveType) {
            case GrPrimitiveType::kLines:
                return 2;
            case GrPrimitiveType::kTriangles:
                return 3;
            default:
                return 0;
        }
    }

    uint16_t currentIndex() const { return fCurVert - fVertices; }

    // Allocate vertex and (possibly) index buffers
    void allocNewBuffers() {
        // Ensure that we always get enough verts for a worst-case quad/cubic, plus leftover points
        // from previous mesh piece (up to two verts to continue fanning). If we can't get that
        // many, ask for a much larger number. This needs to be fairly big to handle  quads/cubics,
        // which have a worst-case of 1k points.
        static const int kMinVerticesPerChunk = GrPathUtils::kMaxPointsPerCurve + 2;
        static const int kFallbackVerticesPerChunk = 16384;

        fVertices = static_cast<SkPoint*>(fTarget->makeVertexSpaceAtLeast(fVertexStride,
                                                                          kMinVerticesPerChunk,
                                                                          kFallbackVerticesPerChunk,
                                                                          &fVertexBuffer,
                                                                          &fFirstVertex,
                                                                          &fVerticesInChunk));

        if (this->isIndexed()) {
            // Similar to above: Ensure we get enough indices for one worst-case quad/cubic.
            // No extra indices are needed for stitching, though. If we can't get that many, ask
            // for enough to match our large vertex request.
            const int kMinIndicesPerChunk = GrPathUtils::kMaxPointsPerCurve * this->indexScale();
            const int kFallbackIndicesPerChunk = kFallbackVerticesPerChunk * this->indexScale();

            fIndices = fTarget->makeIndexSpaceAtLeast(kMinIndicesPerChunk, kFallbackIndicesPerChunk,
                                                      &fIndexBuffer, &fFirstIndex,
                                                      &fIndicesInChunk);
        }

        fCurVert = fVertices;
        fCurIdx = fIndices;
        fSubpathIndexStart = 0;
    }

    void appendCountourEdgeIndices(uint16_t edgeV0Idx) {
        // When drawing lines we're appending line segments along the countour. When applying the
        // other fill rules we're drawing triangle fans around the start of the current (sub)path.
        if (!this->isHairline()) {
            *(fCurIdx++) = fSubpathIndexStart;
        }
        *(fCurIdx++) = edgeV0Idx;
        *(fCurIdx++) = edgeV0Idx + 1;
    }

    // Emits a single draw with all accumulated vertex/index data
    void emitMeshAndPutBackReserve() {
        int vertexCount = fCurVert - fVertices;
        int indexCount = fCurIdx - fIndices;
        SkASSERT(vertexCount <= fVerticesInChunk);
        SkASSERT(indexCount <= fIndicesInChunk);

        if (this->isIndexed() ? SkToBool(indexCount) : SkToBool(vertexCount)) {
            GrMesh* mesh = fTarget->allocMesh(fPrimitiveType);
            if (!this->isIndexed()) {
                mesh->setNonIndexedNonInstanced(vertexCount);
            } else {
                mesh->setIndexed(std::move(fIndexBuffer), indexCount, fFirstIndex, 0,
                                 vertexCount - 1, GrPrimitiveRestart::kNo);
            }
            mesh->setVertexData(std::move(fVertexBuffer), fFirstVertex);
            fTarget->recordDraw(fGeometryProcessor, mesh);
        }

        fTarget->putBackIndices((size_t)(fIndicesInChunk - indexCount));
        fTarget->putBackVertices((size_t)(fVerticesInChunk - vertexCount), fVertexStride);
    }

    void needSpace(int vertsNeeded, int indicesNeeded = 0) {
        if (fCurVert + vertsNeeded > fVertices + fVerticesInChunk ||
            fCurIdx + indicesNeeded > fIndices + fIndicesInChunk) {
            // We are about to run out of space (possibly)

            // To maintain continuity, we need to remember one or two points from the current mesh.
            // Lines only need the last point, fills need the first point from the current contour.
            // We always grab both here, and append the ones we need at the end of this process.
            SkPoint lastPt = *(fCurVert - 1);
            SkASSERT(fSubpathIndexStart < fVerticesInChunk);
            SkPoint subpathStartPt = fVertices[fSubpathIndexStart];

            // Draw the mesh we've accumulated, and put back any unused space
            this->emitMeshAndPutBackReserve();

            // Get new buffers
            this->allocNewBuffers();

            // Append copies of the points we saved so the two meshes will weld properly
            if (!this->isHairline()) {
                *(fCurVert++) = subpathStartPt;
            }
            *(fCurVert++) = lastPt;
        }
    }

    GrPrimitiveType fPrimitiveType;
    GrMeshDrawOp::Target* fTarget;
    size_t fVertexStride;
    sk_sp<const GrGeometryProcessor> fGeometryProcessor;

    sk_sp<const GrBuffer> fVertexBuffer;
    int fFirstVertex;
    int fVerticesInChunk;
    SkPoint* fVertices;
    SkPoint* fCurVert;

    sk_sp<const GrBuffer> fIndexBuffer;
    int fFirstIndex;
    int fIndicesInChunk;
    uint16_t* fIndices;
    uint16_t* fCurIdx;
    uint16_t fSubpathIndexStart;
};

class DefaultPathOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkPath& path,
                                          SkScalar tolerance,
                                          uint8_t coverage,
                                          const SkMatrix& viewMatrix,
                                          bool isHairline,
                                          GrAAType aaType,
                                          const SkRect& devBounds,
                                          const GrUserStencilSettings* stencilSettings) {
        return Helper::FactoryHelper<DefaultPathOp>(context, std::move(paint), path, tolerance,
                                                    coverage, viewMatrix, isHairline, aaType,
                                                    devBounds, stencilSettings);
    }

    const char* name() const override { return "DefaultPathOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fHelper.visitProxies(func);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        string.appendf("Color: 0x%08x Count: %d\n", fColor.toBytes_RGBA(), fPaths.count());
        for (const auto& path : fPaths) {
            string.appendf("Tolerance: %.2f\n", path.fTolerance);
        }
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }
#endif

    DefaultPathOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color, const SkPath& path,
                  SkScalar tolerance, uint8_t coverage, const SkMatrix& viewMatrix, bool isHairline,
                  GrAAType aaType, const SkRect& devBounds,
                  const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID())
            , fHelper(helperArgs, aaType, stencilSettings)
            , fColor(color)
            , fCoverage(coverage)
            , fViewMatrix(viewMatrix)
            , fIsHairline(isHairline) {
        fPaths.emplace_back(PathData{path, tolerance});

        this->setBounds(devBounds, HasAABloat::kNo,
                        isHairline ? IsZeroArea::kYes : IsZeroArea::kNo);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrFSAAType fsaaType, GrClampType clampType) override {
        GrProcessorAnalysisCoverage gpCoverage =
                this->coverage() == 0xFF ? GrProcessorAnalysisCoverage::kNone
                                         : GrProcessorAnalysisCoverage::kSingleChannel;
        // This Op uses uniform (not vertex) color, so doesn't need to track wide color.
        return fHelper.finalizeProcessors(
                caps, clip, fsaaType, clampType, gpCoverage, &fColor, nullptr);
    }

private:
    void onPrepareDraws(Target* target) override {
        sk_sp<GrGeometryProcessor> gp;
        {
            using namespace GrDefaultGeoProcFactory;
            Color color(this->color());
            Coverage coverage(this->coverage());
            LocalCoords localCoords(fHelper.usesLocalCoords() ? LocalCoords::kUsePosition_Type
                                                              : LocalCoords::kUnused_Type);
            gp = GrDefaultGeoProcFactory::Make(target->caps().shaderCaps(),
                                               color,
                                               coverage,
                                               localCoords,
                                               this->viewMatrix());
        }

        SkASSERT(gp->vertexStride() == sizeof(SkPoint));

        int instanceCount = fPaths.count();

        // We avoid indices when we have a single hairline contour.
        bool isIndexed = !this->isHairline() || instanceCount > 1 ||
                         PathGeoBuilder::PathHasMultipleSubpaths(fPaths[0].fPath);

        // determine primitiveType
        GrPrimitiveType primitiveType;
        if (this->isHairline()) {
            primitiveType = isIndexed ? GrPrimitiveType::kLines : GrPrimitiveType::kLineStrip;
        } else {
            primitiveType = GrPrimitiveType::kTriangles;
        }
        PathGeoBuilder pathGeoBuilder(primitiveType, target, std::move(gp));

        // fill buffers
        for (int i = 0; i < instanceCount; i++) {
            const PathData& args = fPaths[i];
            pathGeoBuilder.addPath(args.fPath, args.fTolerance);
        }
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        fHelper.executeDrawsAndUploads(this, flushState, chainBounds);
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        DefaultPathOp* that = t->cast<DefaultPathOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return CombineResult::kCannotCombine;
        }

        if (this->color() != that->color()) {
            return CombineResult::kCannotCombine;
        }

        if (this->coverage() != that->coverage()) {
            return CombineResult::kCannotCombine;
        }

        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return CombineResult::kCannotCombine;
        }

        if (this->isHairline() != that->isHairline()) {
            return CombineResult::kCannotCombine;
        }

        fPaths.push_back_n(that->fPaths.count(), that->fPaths.begin());
        return CombineResult::kMerged;
    }

    const SkPMColor4f& color() const { return fColor; }
    uint8_t coverage() const { return fCoverage; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool isHairline() const { return fIsHairline; }

    struct PathData {
        SkPath fPath;
        SkScalar fTolerance;
    };

    SkSTArray<1, PathData, true> fPaths;
    Helper fHelper;
    SkPMColor4f fColor;
    uint8_t fCoverage;
    SkMatrix fViewMatrix;
    bool fIsHairline;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

bool GrDefaultPathRenderer::internalDrawPath(GrRenderTargetContext* renderTargetContext,
                                             GrPaint&& paint,
                                             GrAAType aaType,
                                             const GrUserStencilSettings& userStencilSettings,
                                             const GrClip& clip,
                                             const SkMatrix& viewMatrix,
                                             const GrShape& shape,
                                             bool stencilOnly) {
    auto context = renderTargetContext->surfPriv().getContext();

    SkASSERT(GrAAType::kCoverage != aaType);
    SkPath path;
    shape.asPath(&path);

    SkScalar hairlineCoverage;
    uint8_t newCoverage = 0xff;
    bool isHairline = false;
    if (IsStrokeHairlineOrEquivalent(shape.style(), viewMatrix, &hairlineCoverage)) {
        newCoverage = SkScalarRoundToInt(hairlineCoverage * 0xff);
        isHairline = true;
    } else {
        SkASSERT(shape.style().isSimpleFill());
    }

    int                          passCount = 0;
    const GrUserStencilSettings* passes[2];
    bool                         reverse = false;
    bool                         lastPassIsBounds;

    if (isHairline) {
        passCount = 1;
        if (stencilOnly) {
            passes[0] = &gDirectToStencil;
        } else {
            passes[0] = &userStencilSettings;
        }
        lastPassIsBounds = false;
    } else {
        if (single_pass_shape(shape)) {
            passCount = 1;
            if (stencilOnly) {
                passes[0] = &gDirectToStencil;
            } else {
                passes[0] = &userStencilSettings;
            }
            lastPassIsBounds = false;
        } else {
            switch (path.getFillType()) {
                case SkPath::kInverseEvenOdd_FillType:
                    reverse = true;
                    // fallthrough
                case SkPath::kEvenOdd_FillType:
                    passes[0] = &gEOStencilPass;
                    if (stencilOnly) {
                        passCount = 1;
                        lastPassIsBounds = false;
                    } else {
                        passCount = 2;
                        lastPassIsBounds = true;
                        if (reverse) {
                            passes[1] = &gInvEOColorPass;
                        } else {
                            passes[1] = &gEOColorPass;
                        }
                    }
                    break;

                case SkPath::kInverseWinding_FillType:
                    reverse = true;
                    // fallthrough
                case SkPath::kWinding_FillType:
                    passes[0] = &gWindStencilPass;
                    passCount = 2;
                    if (stencilOnly) {
                        lastPassIsBounds = false;
                        --passCount;
                    } else {
                        lastPassIsBounds = true;
                        if (reverse) {
                            passes[passCount-1] = &gInvWindColorPass;
                        } else {
                            passes[passCount-1] = &gWindColorPass;
                        }
                    }
                    break;
                default:
                    SkDEBUGFAIL("Unknown path fFill!");
                    return false;
            }
        }
    }

    SkScalar tol = GrPathUtils::kDefaultTolerance;
    SkScalar srcSpaceTol = GrPathUtils::scaleToleranceToSrc(tol, viewMatrix, path.getBounds());

    SkRect devBounds;
    GetPathDevBounds(path,
                     renderTargetContext->asRenderTargetProxy()->worstCaseWidth(),
                     renderTargetContext->asRenderTargetProxy()->worstCaseHeight(),
                     viewMatrix, &devBounds);

    for (int p = 0; p < passCount; ++p) {
        if (lastPassIsBounds && (p == passCount-1)) {
            SkRect bounds;
            SkMatrix localMatrix = SkMatrix::I();
            if (reverse) {
                // draw over the dev bounds (which will be the whole dst surface for inv fill).
                bounds = devBounds;
                SkMatrix vmi;
                // mapRect through persp matrix may not be correct
                if (!viewMatrix.hasPerspective() && viewMatrix.invert(&vmi)) {
                    vmi.mapRect(&bounds);
                } else {
                    if (!viewMatrix.invert(&localMatrix)) {
                        return false;
                    }
                }
            } else {
                bounds = path.getBounds();
            }
            const SkMatrix& viewM = (reverse && viewMatrix.hasPerspective()) ? SkMatrix::I() :
                                                                               viewMatrix;
            // This is a non-coverage aa rect op since we assert aaType != kCoverage at the start
            assert_alive(paint);
            renderTargetContext->priv().stencilRect(clip, passes[p], std::move(paint),
                    GrAA(aaType == GrAAType::kMSAA), viewM, bounds, &localMatrix);
        } else {
            bool stencilPass = stencilOnly || passCount > 1;
            std::unique_ptr<GrDrawOp> op;
            if (stencilPass) {
                GrPaint stencilPaint;
                stencilPaint.setXPFactory(GrDisableColorXPFactory::Get());
                op = DefaultPathOp::Make(context, std::move(stencilPaint), path, srcSpaceTol,
                                         newCoverage, viewMatrix, isHairline, aaType, devBounds,
                                         passes[p]);
            } else {
                assert_alive(paint);
                op = DefaultPathOp::Make(context, std::move(paint), path, srcSpaceTol, newCoverage,
                                         viewMatrix, isHairline, aaType, devBounds, passes[p]);
            }
            renderTargetContext->addDrawOp(clip, std::move(op));
        }
    }
    return true;
}

GrPathRenderer::CanDrawPath
GrDefaultPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    bool isHairline = IsStrokeHairlineOrEquivalent(
            args.fShape->style(), *args.fViewMatrix, nullptr);
    // If we aren't a single_pass_shape or hairline, we require stencil buffers.
    if (!(single_pass_shape(*args.fShape) || isHairline) &&
        (args.fCaps->avoidStencilBuffers() || args.fTargetIsWrappedVkSecondaryCB)) {
        return CanDrawPath::kNo;
    }
    // If antialiasing is required, we only support MSAA.
    if (AATypeFlags::kNone != args.fAATypeFlags && !(AATypeFlags::kMSAA & args.fAATypeFlags)) {
        return CanDrawPath::kNo;
    }
    // This can draw any path with any simple fill style.
    if (!args.fShape->style().isSimpleFill() && !isHairline) {
        return CanDrawPath::kNo;
    }
    // This is the fallback renderer for when a path is too complicated for the others to draw.
    return CanDrawPath::kAsBackup;
}

bool GrDefaultPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrDefaultPathRenderer::onDrawPath");
    GrAAType aaType = (AATypeFlags::kNone != args.fAATypeFlags)
            ? GrAAType::kMSAA
            : GrAAType::kNone;

    return this->internalDrawPath(
            args.fRenderTargetContext, std::move(args.fPaint), aaType, *args.fUserStencilSettings,
            *args.fClip, *args.fViewMatrix, *args.fShape, false);
}

void GrDefaultPathRenderer::onStencilPath(const StencilPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrDefaultPathRenderer::onStencilPath");
    SkASSERT(!args.fShape->inverseFilled());

    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Get());

    auto aaType = (GrAA::kYes == args.fDoStencilMSAA) ? GrAAType::kMSAA : GrAAType::kNone;

    this->internalDrawPath(
            args.fRenderTargetContext, std::move(paint), aaType, GrUserStencilSettings::kUnused,
            *args.fClip, *args.fViewMatrix, *args.fShape, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(DefaultPathOp) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);

    // For now just hairlines because the other types of draws require two ops.
    // TODO we should figure out a way to combine the stencil and cover steps into one op.
    GrStyle style(SkStrokeRec::kHairline_InitStyle);
    SkPath path = GrTest::TestPath(random);

    // Compute srcSpaceTol
    SkRect bounds = path.getBounds();
    SkScalar tol = GrPathUtils::kDefaultTolerance;
    SkScalar srcSpaceTol = GrPathUtils::scaleToleranceToSrc(tol, viewMatrix, bounds);

    viewMatrix.mapRect(&bounds);
    uint8_t coverage = GrRandomCoverage(random);
    GrAAType aaType = GrAAType::kNone;
    if (GrFSAAType::kUnifiedMSAA == fsaaType && random->nextBool()) {
        aaType = GrAAType::kMSAA;
    }
    return DefaultPathOp::Make(context, std::move(paint), path, srcSpaceTol, coverage, viewMatrix,
                               true, aaType, bounds, GrGetRandomStencil(random, context));
}

#endif
