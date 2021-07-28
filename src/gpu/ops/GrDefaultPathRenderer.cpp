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
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrSimpleMesh.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrUtil.h"
#include "src/gpu/effects/GrDisableColorXP.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelperWithStencil.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

GrDefaultPathRenderer::GrDefaultPathRenderer() {
}

////////////////////////////////////////////////////////////////////////////////
// Helpers for drawPath

#define STENCIL_OFF     0   // Always disable stencil (even when needed)

static inline bool single_pass_shape(const GrStyledShape& shape) {
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
GrDefaultPathRenderer::onGetStencilSupport(const GrStyledShape& shape) const {
    if (single_pass_shape(shape)) {
        return GrPathRenderer::kNoRestriction_StencilSupport;
    } else {
        return GrPathRenderer::kStencilOnly_StencilSupport;
    }
}

namespace {

class PathGeoBuilder {
public:
    PathGeoBuilder(GrPrimitiveType primitiveType,
                   GrMeshDrawTarget* target,
                   SkTDArray<GrSimpleMesh*>* meshes)
            : fPrimitiveType(primitiveType)
            , fTarget(target)
            , fVertexStride(sizeof(SkPoint))
            , fFirstIndex(0)
            , fIndicesInChunk(0)
            , fIndices(nullptr)
            , fMeshes(meshes) {
        this->allocNewBuffers();
    }

    ~PathGeoBuilder() {
        this->createMeshAndPutBackReserve();
    }

    /**
     *  Path verbs
     */
    void moveTo(const SkPoint& p) {
        if (!this->ensureSpace(1)) {
            return;
        }

        if (!this->isHairline()) {
            fSubpathIndexStart = this->currentIndex();
            fSubpathStartPoint = p;
        }
        *(fCurVert++) = p;
    }

    void addLine(const SkPoint pts[]) {
        if (!this->ensureSpace(1, this->indexScale(), &pts[0])) {
            return;
        }

        if (this->isIndexed()) {
            uint16_t prevIdx = this->currentIndex() - 1;
            this->appendCountourEdgeIndices(prevIdx);
        }
        *(fCurVert++) = pts[1];
    }

    void addQuad(const SkPoint pts[], SkScalar srcSpaceTolSqd, SkScalar srcSpaceTol) {
        if (!this->ensureSpace(GrPathUtils::kMaxPointsPerCurve,
                             GrPathUtils::kMaxPointsPerCurve * this->indexScale(),
                             &pts[0])) {
            return;
        }

        // First pt of quad is the pt we ended on in previous step
        uint16_t firstQPtIdx = this->currentIndex() - 1;
        uint16_t numPts = (uint16_t)GrPathUtils::generateQuadraticPoints(
                pts[0], pts[1], pts[2], srcSpaceTolSqd, &fCurVert,
                GrPathUtils::quadraticPointCount(pts, srcSpaceTol));
        if (this->isIndexed()) {
            for (uint16_t i = 0; i < numPts; ++i) {
                this->appendCountourEdgeIndices(firstQPtIdx + i);
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
        if (!this->ensureSpace(GrPathUtils::kMaxPointsPerCurve,
                             GrPathUtils::kMaxPointsPerCurve * this->indexScale(),
                             &pts[0])) {
            return;
        }

        // First pt of cubic is the pt we ended on in previous step
        uint16_t firstCPtIdx = this->currentIndex() - 1;
        uint16_t numPts = (uint16_t) GrPathUtils::generateCubicPoints(
                pts[0], pts[1], pts[2], pts[3], srcSpaceTolSqd, &fCurVert,
                GrPathUtils::cubicPointCount(pts, srcSpaceTol));
        if (this->isIndexed()) {
            for (uint16_t i = 0; i < numPts; ++i) {
                this->appendCountourEdgeIndices(firstCPtIdx + i);
            }
        }
    }

    void addPath(const SkPath& path, SkScalar srcSpaceTol) {
        SkScalar srcSpaceTolSqd = srcSpaceTol * srcSpaceTol;

        SkPath::Iter iter(path, false);
        SkPoint pts[4];

        bool done = false;
        while (!done) {
            SkPath::Verb verb = iter.next(pts);
            switch (verb) {
                case SkPath::kMove_Verb:
                    this->moveTo(pts[0]);
                    break;
                case SkPath::kLine_Verb:
                    this->addLine(pts);
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
        while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
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
        SkASSERT(fValid);

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
        if (!fVertices) {
            SkDebugf("WARNING: Failed to allocate vertex buffer for GrDefaultPathRenderer.\n");
            fCurVert = nullptr;
            fCurIdx = fIndices = nullptr;
            fSubpathIndexStart = 0;
            fValid = false;
            return;
        }

        if (this->isIndexed()) {
            // Similar to above: Ensure we get enough indices for one worst-case quad/cubic.
            // No extra indices are needed for stitching, though. If we can't get that many, ask
            // for enough to match our large vertex request.
            const int kMinIndicesPerChunk = GrPathUtils::kMaxPointsPerCurve * this->indexScale();
            const int kFallbackIndicesPerChunk = kFallbackVerticesPerChunk * this->indexScale();

            fIndices = fTarget->makeIndexSpaceAtLeast(kMinIndicesPerChunk, kFallbackIndicesPerChunk,
                                                      &fIndexBuffer, &fFirstIndex,
                                                      &fIndicesInChunk);
            if (!fIndices) {
                SkDebugf("WARNING: Failed to allocate index buffer for GrDefaultPathRenderer.\n");
                fVertices = nullptr;
                fValid = false;
            }
        }

        fCurVert = fVertices;
        fCurIdx = fIndices;
        fSubpathIndexStart = 0;
    }

    void appendCountourEdgeIndices(uint16_t edgeV0Idx) {
        SkASSERT(fCurIdx);

        // When drawing lines we're appending line segments along the countour. When applying the
        // other fill rules we're drawing triangle fans around the start of the current (sub)path.
        if (!this->isHairline()) {
            *(fCurIdx++) = fSubpathIndexStart;
        }
        *(fCurIdx++) = edgeV0Idx;
        *(fCurIdx++) = edgeV0Idx + 1;
    }

    // Emits a single draw with all accumulated vertex/index data
    void createMeshAndPutBackReserve() {
        if (!fValid) {
            return;
        }

        int vertexCount = fCurVert - fVertices;
        int indexCount = fCurIdx - fIndices;
        SkASSERT(vertexCount <= fVerticesInChunk);
        SkASSERT(indexCount <= fIndicesInChunk);

        GrSimpleMesh* mesh = nullptr;
        if (this->isIndexed() ? SkToBool(indexCount) : SkToBool(vertexCount)) {
            mesh = fTarget->allocMesh();
            if (!this->isIndexed()) {
                mesh->set(std::move(fVertexBuffer), vertexCount, fFirstVertex);
            } else {
                mesh->setIndexed(std::move(fIndexBuffer), indexCount, fFirstIndex, 0,
                                 vertexCount - 1, GrPrimitiveRestart::kNo, std::move(fVertexBuffer),
                                 fFirstVertex);
            }
        }

        fTarget->putBackIndices((size_t)(fIndicesInChunk - indexCount));
        fTarget->putBackVertices((size_t)(fVerticesInChunk - vertexCount), fVertexStride);

        if (mesh) {
            fMeshes->push_back(mesh);
        }
    }

    bool ensureSpace(int vertsNeeded, int indicesNeeded = 0, const SkPoint* lastPoint = nullptr) {
        if (!fValid) {
            return false;
        }

        if (fCurVert + vertsNeeded > fVertices + fVerticesInChunk ||
            fCurIdx + indicesNeeded > fIndices + fIndicesInChunk) {
            // We are about to run out of space (possibly)

#ifdef SK_DEBUG
            // To maintain continuity, we need to remember one or two points from the current mesh.
            // Lines only need the last point, fills need the first point from the current contour.
            // We always grab both here, and append the ones we need at the end of this process.
            SkASSERT(fSubpathIndexStart < fVerticesInChunk);
            // This assert is reading from the gpu buffer fVertices and will be slow, but for debug
            // that is okay.
            if (!this->isHairline()) {
                SkASSERT(fSubpathStartPoint == fVertices[fSubpathIndexStart]);
            }
            if (lastPoint) {
                SkASSERT(*(fCurVert - 1) == *lastPoint);
            }
#endif

            // Draw the mesh we've accumulated, and put back any unused space
            this->createMeshAndPutBackReserve();

            // Get new buffers
            this->allocNewBuffers();
            if (!fValid) {
                return false;
            }

            // On moves we don't need to copy over any points to the new buffer and we pass in a
            // null lastPoint.
            if (lastPoint) {
                // Append copies of the points we saved so the two meshes will weld properly
                if (!this->isHairline()) {
                    *(fCurVert++) = fSubpathStartPoint;
                }
                *(fCurVert++) = *lastPoint;
            }
        }

        return true;
    }

    GrPrimitiveType fPrimitiveType;
    GrMeshDrawTarget* fTarget;
    size_t fVertexStride;

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
    SkPoint fSubpathStartPoint;

    bool fValid = true;
    SkTDArray<GrSimpleMesh*>* fMeshes;
};

class DefaultPathOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context,
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

    void visitProxies(const GrVisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

    DefaultPathOp(GrProcessorSet* processorSet, const SkPMColor4f& color, const SkPath& path,
                  SkScalar tolerance, uint8_t coverage, const SkMatrix& viewMatrix, bool isHairline,
                  GrAAType aaType, const SkRect& devBounds,
                  const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID())
            , fHelper(processorSet, aaType, stencilSettings)
            , fColor(color)
            , fCoverage(coverage)
            , fViewMatrix(viewMatrix)
            , fIsHairline(isHairline) {
        fPaths.emplace_back(PathData{path, tolerance});

        HasAABloat aaBloat = (aaType == GrAAType::kNone) ? HasAABloat ::kNo : HasAABloat::kYes;
        this->setBounds(devBounds, aaBloat,
                        isHairline ? IsHairline::kYes : IsHairline::kNo);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        GrProcessorAnalysisCoverage gpCoverage =
                this->coverage() == 0xFF ? GrProcessorAnalysisCoverage::kNone
                                         : GrProcessorAnalysisCoverage::kSingleChannel;
        // This Op uses uniform (not vertex) color, so doesn't need to track wide color.
        return fHelper.finalizeProcessors(caps, clip, clampType, gpCoverage, &fColor, nullptr);
    }

private:
    GrPrimitiveType primType() const {
        if (this->isHairline()) {
            int instanceCount = fPaths.count();

            // We avoid indices when we have a single hairline contour.
            bool isIndexed = instanceCount > 1 ||
                                PathGeoBuilder::PathHasMultipleSubpaths(fPaths[0].fPath);

            return isIndexed ? GrPrimitiveType::kLines : GrPrimitiveType::kLineStrip;
        }

        return GrPrimitiveType::kTriangles;
    }

    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&& appliedClip,
                             const GrDstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        GrGeometryProcessor* gp;
        {
            using namespace GrDefaultGeoProcFactory;
            Color color(this->color());
            Coverage coverage(this->coverage());
            LocalCoords localCoords(fHelper.usesLocalCoords() ? LocalCoords::kUsePosition_Type
                                                              : LocalCoords::kUnused_Type);
            gp = GrDefaultGeoProcFactory::Make(arena,
                                               color,
                                               coverage,
                                               localCoords,
                                               this->viewMatrix());
        }

        SkASSERT(gp->vertexStride() == sizeof(SkPoint));

        fProgramInfo =  fHelper.createProgramInfoWithStencil(caps, arena, writeView,
                                                             std::move(appliedClip),
                                                             dstProxyView, gp, this->primType(),
                                                             renderPassXferBarriers, colorLoadOp);

    }

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        PathGeoBuilder pathGeoBuilder(this->primType(), target, &fMeshes);

        // fill buffers
        for (int i = 0; i < fPaths.count(); i++) {
            const PathData& args = fPaths[i];
            pathGeoBuilder.addPath(args.fPath, args.fTolerance);
        }
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo) {
            this->createProgramInfo(flushState);
        }

        if (!fProgramInfo || !fMeshes.count()) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
        for (int i = 0; i < fMeshes.count(); ++i) {
            flushState->drawMesh(*fMeshes[i]);
        }
    }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override {
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

        if (!SkMatrixPriv::CheapEqual(this->viewMatrix(), that->viewMatrix())) {
            return CombineResult::kCannotCombine;
        }

        if (this->isHairline() != that->isHairline()) {
            return CombineResult::kCannotCombine;
        }

        fPaths.push_back_n(that->fPaths.count(), that->fPaths.begin());
        return CombineResult::kMerged;
    }

#if GR_TEST_UTILS
    SkString onDumpInfo() const override {
        SkString string = SkStringPrintf("Color: 0x%08x Count: %d\n",
                                         fColor.toBytes_RGBA(), fPaths.count());
        for (const auto& path : fPaths) {
            string.appendf("Tolerance: %.2f\n", path.fTolerance);
        }
        string += fHelper.dumpInfo();
        return string;
    }
#endif

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

    SkTDArray<GrSimpleMesh*> fMeshes;
    GrProgramInfo* fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};

}  // anonymous namespace

bool GrDefaultPathRenderer::internalDrawPath(skgpu::v1::SurfaceDrawContext* sdc,
                                             GrPaint&& paint,
                                             GrAAType aaType,
                                             const GrUserStencilSettings& userStencilSettings,
                                             const GrClip* clip,
                                             const SkMatrix& viewMatrix,
                                             const GrStyledShape& shape,
                                             bool stencilOnly) {
    auto context = sdc->recordingContext();

    SkASSERT(GrAAType::kCoverage != aaType);
    SkPath path;
    shape.asPath(&path);

    SkScalar hairlineCoverage;
    uint8_t newCoverage = 0xff;
    bool isHairline = false;
    if (GrIsStrokeHairlineOrEquivalent(shape.style(), viewMatrix, &hairlineCoverage)) {
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
                case SkPathFillType::kInverseEvenOdd:
                    reverse = true;
                    [[fallthrough]];
                case SkPathFillType::kEvenOdd:
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

                case SkPathFillType::kInverseWinding:
                    reverse = true;
                    [[fallthrough]];
                case SkPathFillType::kWinding:
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
    GetPathDevBounds(path, sdc->asRenderTargetProxy()->backingStoreDimensions(),
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
            sdc->stencilRect(clip, passes[p], std::move(paint),
                             GrAA(aaType == GrAAType::kMSAA), viewM, bounds,
                             &localMatrix);
        } else {
            bool stencilPass = stencilOnly || passCount > 1;
            GrOp::Owner op;
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
            sdc->addDrawOp(clip, std::move(op));
        }
    }
    return true;
}

GrPathRenderer::CanDrawPath
GrDefaultPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    bool isHairline = GrIsStrokeHairlineOrEquivalent(
            args.fShape->style(), *args.fViewMatrix, nullptr);
    // If we aren't a single_pass_shape or hairline, we require stencil buffers.
    if (!(single_pass_shape(*args.fShape) || isHairline) &&
        !args.fProxy->canUseStencil(*args.fCaps)) {
        return CanDrawPath::kNo;
    }
    // If antialiasing is required, we only support MSAA.
    if (GrAAType::kNone != args.fAAType && GrAAType::kMSAA != args.fAAType) {
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
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fContext->priv().auditTrail(),
                              "GrDefaultPathRenderer::onDrawPath");
    GrAAType aaType = (GrAAType::kNone != args.fAAType) ? GrAAType::kMSAA : GrAAType::kNone;

    return this->internalDrawPath(
            args.fSurfaceDrawContext, std::move(args.fPaint), aaType, *args.fUserStencilSettings,
            args.fClip, *args.fViewMatrix, *args.fShape, false);
}

void GrDefaultPathRenderer::onStencilPath(const StencilPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fContext->priv().auditTrail(),
                              "GrDefaultPathRenderer::onStencilPath");
    SkASSERT(!args.fShape->inverseFilled());

    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Get());

    auto aaType = (GrAA::kYes == args.fDoStencilMSAA) ? GrAAType::kMSAA : GrAAType::kNone;

    this->internalDrawPath(
            args.fSurfaceDrawContext, std::move(paint), aaType, GrUserStencilSettings::kUnused,
            args.fClip, *args.fViewMatrix, *args.fShape, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(DefaultPathOp) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);

    // For now just hairlines because the other types of draws require two ops.
    // TODO we should figure out a way to combine the stencil and cover steps into one op.
    GrStyle style(SkStrokeRec::kHairline_InitStyle);
    const SkPath& path = GrTest::TestPath(random);

    // Compute srcSpaceTol
    SkRect bounds = path.getBounds();
    SkScalar tol = GrPathUtils::kDefaultTolerance;
    SkScalar srcSpaceTol = GrPathUtils::scaleToleranceToSrc(tol, viewMatrix, bounds);

    viewMatrix.mapRect(&bounds);
    uint8_t coverage = GrRandomCoverage(random);
    GrAAType aaType = GrAAType::kNone;
    if (numSamples > 1 && random->nextBool()) {
        aaType = GrAAType::kMSAA;
    }
    return DefaultPathOp::Make(context, std::move(paint), path, srcSpaceTol, coverage, viewMatrix,
                               true, aaType, bounds, GrGetRandomStencil(random, context));
}

#endif
