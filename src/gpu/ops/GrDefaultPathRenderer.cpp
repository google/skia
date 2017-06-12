/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDefaultPathRenderer.h"

#include "GrContext.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawOpTest.h"
#include "GrFixedClip.h"
#include "GrMesh.h"
#include "GrOpFlushState.h"
#include "GrPathUtils.h"
#include "GrPipelineBuilder.h"
#include "SkGeometry.h"
#include "SkString.h"
#include "SkStrokeRec.h"
#include "SkTLazy.h"
#include "SkTraceEvent.h"

#include "ops/GrMeshDrawOp.h"
#include "ops/GrNonAAFillRectOp.h"

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

// Needs to be large enough to handle quads/cubics, which have a worst-case of 1k points
static const int kVerticesPerChunk = 16384;

class PathGeoBuilder {
public:
    PathGeoBuilder(GrPrimitiveType primitiveType, GrMeshDrawOp::Target* target,
                   GrGeometryProcessor* geometryProcessor, const GrPipeline* pipeline)
            : fMesh(primitiveType)
            , fTarget(target)
            , fVertexStride(sizeof(SkPoint))
            , fGeometryProcessor(geometryProcessor)
            , fPipeline(pipeline)
            , fIndexBuffer(nullptr)
            , fFirstIndex(0)
            , fIndices(nullptr) {
        this->allocNewBuffers();
    }

    ~PathGeoBuilder() {
        this->emitMesh();
        this->putBackReserve();
    }

    // Called before we start each path
    void beginInstance() {
        fSubpathIndexStart = fVertexOffset;
        fCurIdx = fIndices + fIndexOffset;
        fCurVert = fVertices + fVertexOffset;
    }

    // Called after we end each path
    void endInstance() {
        fVertexOffset = fCurVert - fVertices;
        fIndexOffset = fCurIdx - fIndices;
        SkASSERT(fVertexOffset <= kVerticesPerChunk);
        SkASSERT(fIndexOffset <= this->maxIndices());
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
            SkPath::Verb verb = iter.next(pts);
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
        return kLines_GrPrimitiveType == fMesh.primitiveType() ||
               kTriangles_GrPrimitiveType == fMesh.primitiveType();
    }
    bool isHairline() const {
        return kLines_GrPrimitiveType == fMesh.primitiveType() ||
               kLineStrip_GrPrimitiveType == fMesh.primitiveType();
    }
    int indexScale() const {
        switch (fMesh.primitiveType()) {
            case kLines_GrPrimitiveType:
                return 2;
            case kTriangles_GrPrimitiveType:
                return 3;
            default:
                return 0;
        }
    }
    int maxIndices() const { return kVerticesPerChunk * this->indexScale(); }

    uint16_t currentIndex() const { return fCurVert - fVertices; }

    void putBackReserve() {
        fTarget->putBackIndices((size_t)(this->maxIndices() - fIndexOffset));
        fTarget->putBackVertices((size_t)(kVerticesPerChunk - fVertexOffset), fVertexStride);
    }

    // Allocate vertex and (possibly) index buffers
    void allocNewBuffers() {
        fVertices = static_cast<SkPoint*>(fTarget->makeVertexSpace(fVertexStride, kVerticesPerChunk,
                                                                   &fVertexBuffer, &fFirstVertex));
        if (this->isIndexed()) {
            fIndices = fTarget->makeIndexSpace(this->maxIndices(), &fIndexBuffer, &fFirstIndex);
        }
        fVertexOffset = 0;
        fIndexOffset = 0;
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
    void emitMesh() {
        if (fVertexOffset > 0) {
            if (!this->isIndexed()) {
                fMesh.setNonIndexedNonInstanced(fVertexOffset);
            } else {
                fMesh.setIndexed(fIndexBuffer, fIndexOffset, fFirstIndex, 0, fVertexOffset - 1);
            }
            fMesh.setVertexData(fVertexBuffer, fFirstVertex);
            fTarget->draw(fGeometryProcessor, fPipeline, fMesh);
        }
    }

    void needSpace(int vertsNeeded, int indicesNeeded = 0) {
        if (fCurVert + vertsNeeded > fVertices + kVerticesPerChunk ||
            fCurIdx + indicesNeeded > fIndices + this->maxIndices()) {
            // We are about to run out of space (possibly)

            // To maintain continuity, we need to remember one or two points from the current mesh.
            // Lines only need the last point, fills need the first point from the current contour.
            // We always grab both here, and append the ones we need at the end of this process.
            SkPoint lastPt = *(fCurVert - 1);
            SkPoint subpathStartPt = fVertices[fSubpathIndexStart];

            // Pretend that we've reached the end of an entire path, so our offsets are correct
            this->endInstance();

            // Draw the mesh we've accumulated
            this->emitMesh();

            // Put back any unused space, get new buffers
            this->putBackReserve();
            this->allocNewBuffers();

            // Start a "new" path, which is really just a continuation of the in-progress one
            this->beginInstance();

            // Append copies of the points we saved so the two meshes will weld properly
            if (!this->isHairline()) {
                *(fCurVert++) = subpathStartPt;
            }
            *(fCurVert++) = lastPt;
        }
    }

    GrMesh fMesh;
    GrMeshDrawOp::Target* fTarget;
    size_t fVertexStride;
    GrGeometryProcessor* fGeometryProcessor;
    const GrPipeline* fPipeline;

    const GrBuffer* fVertexBuffer;
    int fFirstVertex;
    SkPoint* fVertices;
    SkPoint* fCurVert;
    int fVertexOffset;

    const GrBuffer* fIndexBuffer;
    int fFirstIndex;
    uint16_t* fIndices;
    uint16_t* fCurIdx;
    int fIndexOffset;
    uint16_t fSubpathIndexStart;
};

class DefaultPathOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color, const SkPath& path,
                                                    SkScalar tolerance, uint8_t coverage,
                                                    const SkMatrix& viewMatrix, bool isHairline,
                                                    const SkRect& devBounds) {
        return std::unique_ptr<GrLegacyMeshDrawOp>(new DefaultPathOp(
                color, path, tolerance, coverage, viewMatrix, isHairline, devBounds));
    }

    const char* name() const override { return "DefaultPathOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf("Color: 0x%08x Count: %d\n", fColor, fPaths.count());
        for (const auto& path : fPaths) {
            string.appendf("Tolerance: %.2f\n", path.fTolerance);
        }
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    DefaultPathOp(GrColor color, const SkPath& path, SkScalar tolerance, uint8_t coverage,
                  const SkMatrix& viewMatrix, bool isHairline, const SkRect& devBounds)
            : INHERITED(ClassID())
            , fColor(color)
            , fCoverage(coverage)
            , fViewMatrix(viewMatrix)
            , fIsHairline(isHairline) {
        fPaths.emplace_back(PathData{path, tolerance});

        this->setBounds(devBounds, HasAABloat::kNo,
                        isHairline ? IsZeroArea::kYes : IsZeroArea::kNo);
    }

    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        color->setToConstant(fColor);
        *coverage = this->coverage() == 0xff ? GrProcessorAnalysisCoverage::kNone
                                             : GrProcessorAnalysisCoverage::kSingleChannel;
    }

    void applyPipelineOptimizations(const PipelineOptimizations& optimizations) override {
        optimizations.getOverrideColorIfSet(&fColor);
        fUsesLocalCoords = optimizations.readsLocalCoords();
    }

    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp;
        {
            using namespace GrDefaultGeoProcFactory;
            Color color(this->color());
            Coverage coverage(this->coverage());
            LocalCoords localCoords(this->usesLocalCoords() ? LocalCoords::kUsePosition_Type :
                                                              LocalCoords::kUnused_Type);
            gp = GrDefaultGeoProcFactory::Make(color, coverage, localCoords, this->viewMatrix());
        }

        SkASSERT(gp->getVertexStride() == sizeof(SkPoint));

        int instanceCount = fPaths.count();

        // We will use index buffers if we have multiple paths or one path with multiple contours
        bool isIndexed = instanceCount > 1;
        for (int i = 0; !isIndexed && i < instanceCount; i++) {
            const PathData& args = fPaths[i];
            isIndexed = isIndexed || PathGeoBuilder::PathHasMultipleSubpaths(args.fPath);
        }

        // determine primitiveType
        GrPrimitiveType primitiveType;
        if (this->isHairline()) {
            primitiveType = isIndexed ? kLines_GrPrimitiveType : kLineStrip_GrPrimitiveType;
        } else {
            primitiveType = isIndexed ? kTriangles_GrPrimitiveType : kTriangleFan_GrPrimitiveType;
        }

        PathGeoBuilder pathGeoBuilder(primitiveType, target, gp.get(), this->pipeline());

        // fill buffers
        for (int i = 0; i < instanceCount; i++) {
            const PathData& args = fPaths[i];

            pathGeoBuilder.beginInstance();
            pathGeoBuilder.addPath(args.fPath, args.fTolerance);
            pathGeoBuilder.endInstance();
        }
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        DefaultPathOp* that = t->cast<DefaultPathOp>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (this->color() != that->color()) {
            return false;
        }

        if (this->coverage() != that->coverage()) {
            return false;
        }

        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        if (this->isHairline() != that->isHairline()) {
            return false;
        }

        fPaths.push_back_n(that->fPaths.count(), that->fPaths.begin());
        this->joinBounds(*that);
        return true;
    }

    GrColor color() const { return fColor; }
    uint8_t coverage() const { return fCoverage; }
    bool usesLocalCoords() const { return fUsesLocalCoords; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool isHairline() const { return fIsHairline; }

    struct PathData {
        SkPath fPath;
        SkScalar fTolerance;
    };

    GrColor fColor;
    uint8_t fCoverage;
    SkMatrix fViewMatrix;
    bool fUsesLocalCoords;
    bool fIsHairline;
    SkSTArray<1, PathData, true> fPaths;

    typedef GrLegacyMeshDrawOp INHERITED;
};

bool GrDefaultPathRenderer::internalDrawPath(GrRenderTargetContext* renderTargetContext,
                                             GrPaint&& paint,
                                             GrAAType aaType,
                                             const GrUserStencilSettings& userStencilSettings,
                                             const GrClip& clip,
                                             const SkMatrix& viewMatrix,
                                             const GrShape& shape,
                                             bool stencilOnly) {
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
    GetPathDevBounds(path, renderTargetContext->width(), renderTargetContext->height(), viewMatrix,
                     &devBounds);

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
            renderTargetContext->addDrawOp(
                    clip,
                    GrNonAAFillRectOp::Make(std::move(paint), viewM, bounds, nullptr, &localMatrix,
                                            aaType, passes[p]));
        } else {
            std::unique_ptr<GrLegacyMeshDrawOp> op =
                    DefaultPathOp::Make(paint.getColor(), path, srcSpaceTol, newCoverage,
                                        viewMatrix, isHairline, devBounds);
            bool stencilPass = stencilOnly || passCount > 1;
            GrPaint::MoveOrNew passPaint(paint, stencilPass);
            if (stencilPass) {
                passPaint.paint().setXPFactory(GrDisableColorXPFactory::Get());
            }
            GrPipelineBuilder pipelineBuilder(std::move(passPaint), aaType);
            pipelineBuilder.setUserStencil(passes[p]);
            renderTargetContext->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip,
                                                     std::move(op));
        }
    }
    return true;
}

bool GrDefaultPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    bool isHairline = IsStrokeHairlineOrEquivalent(args.fShape->style(), *args.fViewMatrix, nullptr);
    // If we aren't a single_pass_shape or hairline, we require stencil buffers.
    if (!(single_pass_shape(*args.fShape) || isHairline) && args.fCaps->avoidStencilBuffers()) {
        return false;
    }
    // This can draw any path with any simple fill style but doesn't do coverage-based antialiasing.
    return GrAAType::kCoverage != args.fAAType &&
           (args.fShape->style().isSimpleFill() || isHairline);
}

bool GrDefaultPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrDefaultPathRenderer::onDrawPath");
    return this->internalDrawPath(args.fRenderTargetContext,
                                  std::move(args.fPaint),
                                  args.fAAType,
                                  *args.fUserStencilSettings,
                                  *args.fClip,
                                  *args.fViewMatrix,
                                  *args.fShape,
                                  false);
}

void GrDefaultPathRenderer::onStencilPath(const StencilPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrDefaultPathRenderer::onStencilPath");
    SkASSERT(!args.fShape->inverseFilled());

    GrPaint paint;
    paint.setXPFactory(GrDisableColorXPFactory::Get());

    this->internalDrawPath(args.fRenderTargetContext, std::move(paint), args.fAAType,
                           GrUserStencilSettings::kUnused, *args.fClip, *args.fViewMatrix,
                           *args.fShape, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_LEGACY_MESH_DRAW_OP_TEST_DEFINE(DefaultPathOp) {
    GrColor color = GrRandomColor(random);
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
    return DefaultPathOp::Make(color, path, srcSpaceTol, coverage, viewMatrix, true, bounds);
}

#endif
