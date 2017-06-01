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

static inline void append_countour_edge_indices(bool hairLine,
                                                uint16_t fanCenterIdx,
                                                uint16_t edgeV0Idx,
                                                uint16_t** indices) {
    // when drawing lines we're appending line segments along
    // the contour. When applying the other fill rules we're
    // drawing triangle fans around fanCenterIdx.
    if (!hairLine) {
        *((*indices)++) = fanCenterIdx;
    }
    *((*indices)++) = edgeV0Idx;
    *((*indices)++) = edgeV0Idx + 1;
}

static inline void add_quad(SkPoint** vert, const SkPoint* base, const SkPoint pts[],
                            SkScalar srcSpaceTolSqd, SkScalar srcSpaceTol, bool indexed,
                            bool isHairline, uint16_t subpathIdxStart, int offset, uint16_t** idx) {
    // first pt of quad is the pt we ended on in previous step
    uint16_t firstQPtIdx = (uint16_t)(*vert - base) - 1 + offset;
    uint16_t numPts =  (uint16_t)
        GrPathUtils::generateQuadraticPoints(
            pts[0], pts[1], pts[2],
            srcSpaceTolSqd, vert,
            GrPathUtils::quadraticPointCount(pts, srcSpaceTol));
    if (indexed) {
        for (uint16_t i = 0; i < numPts; ++i) {
            append_countour_edge_indices(isHairline, subpathIdxStart,
                                         firstQPtIdx + i, idx);
        }
    }
}

// Needs to be large enough to handle quads/cubics, which have a worst-case of 1k points
static const int kVerticesPerChunk = 16384;
// See GrPathUtils.cpp: MAX_POINTS_PER_CURVE
static const int kMaxVerticesPerCurve = 1 << 10;

struct PathGeoHelper {
    PathGeoHelper(GrPrimitiveType primitiveType, GrMeshDrawOp::Target* target, int indexScale,
                  GrGeometryProcessor* geometryProcessor, const GrPipeline* pipeline)
            : fMesh(primitiveType)
            , fTarget(target)
            , fVertexStride(sizeof(SkPoint))
            , fIndexScale(indexScale)
            , fGeometryProcessor(geometryProcessor)
            , fPipeline(pipeline)
            , fIndexBuffer(nullptr)
            , fFirstIndex(0)
            , fIndices(nullptr) {
        this->allocNewBuffers();
    }

    ~PathGeoHelper() {
        this->putBackReserves();
    }

    bool isIndexed() const { return fIndexScale > 0; }
    int maxIndices() const { return fIndexScale * kVerticesPerChunk; }

    void allocNewBuffers() {
        fVertices = static_cast<SkPoint*>(fTarget->makeVertexSpace(fVertexStride, kVerticesPerChunk,
                                                                   &fVertexBuffer, &fFirstVertex));
        if (this->isIndexed()) {
            fIndices = fTarget->makeIndexSpace(this->maxIndices(), &fIndexBuffer, &fFirstIndex);
        }
        fVertexOffset = 0;
        fIndexOffset = 0;
    }

    void beginInstance() {
        fVertexCount = 0;
        fIndexCount = 0;
        fSubpathIndexStart = fVertexOffset;
    }

    void endInstance() {
        fVertexOffset += fVertexCount;
        fIndexOffset += fIndexCount;
        SkASSERT(fVertexOffset <= kVerticesPerChunk);
        SkASSERT(fIndexOffset <= this->maxIndices());
    }

    void emitMesh() {
        if (!this->isIndexed()) {
            fMesh.setNonIndexedNonInstanced(fVertexOffset);
        } else {
            fMesh.setIndexed(fIndexBuffer, fIndexOffset, fFirstIndex, 0, fVertexOffset - 1);
        }
        fMesh.setVertexData(fVertexBuffer, fFirstVertex);
        fTarget->draw(fGeometryProcessor, fPipeline, fMesh);
    }

    void putBackReserves() {
        fTarget->putBackIndices((size_t)(this->maxIndices() - fIndexOffset));
        fTarget->putBackVertices((size_t)(kVerticesPerChunk - fVertexOffset), fVertexStride);
    }

    uint16_t* indexBase() { return fIndices + fIndexOffset; }
    SkPoint* vertexBase() { return fVertices + fVertexCount; }

    GrMesh fMesh;
    GrMeshDrawOp::Target* fTarget;
    size_t fVertexStride;
    int fIndexScale;
    GrGeometryProcessor* fGeometryProcessor;
    const GrPipeline* fPipeline;

    const GrBuffer* fVertexBuffer;
    int fFirstVertex;
    SkPoint* fVertices;
    int fVertexOffset;
    int fVertexCount;

    const GrBuffer* fIndexBuffer;
    int fFirstIndex;
    uint16_t* fIndices;
    int fIndexOffset;
    int fIndexCount;
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

        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(SkPoint));

        int instanceCount = fPaths.count();

        // We will use index buffers if we have multiple paths or one path with multiple contours
        bool isIndexed = instanceCount > 1;
        for (int i = 0; !isIndexed && i < instanceCount; i++) {
            const PathData& args = fPaths[i];
            isIndexed = isIndexed || GrPathUtils::hasMultipleSubpaths(args.fPath);
        }

        // determine primitiveType
        GrPrimitiveType primitiveType;
        int indexScale = 0;
        if (this->isHairline()) {
            if (isIndexed) {
                indexScale = 2;
                primitiveType = kLines_GrPrimitiveType;
            } else {
                primitiveType = kLineStrip_GrPrimitiveType;
            }
        } else {
            if (isIndexed) {
                indexScale = 3;
                primitiveType = kTriangles_GrPrimitiveType;
            } else {
                primitiveType = kTriangleFan_GrPrimitiveType;
            }
        }

        // Our geometry emission code will keep re-allocating buffers and re-using this one mesh
        PathGeoHelper pathGeoHelper(primitiveType, target, indexScale, gp.get(), this->pipeline());

        // fill buffers
        int vertexOffset = 0;
        int indexOffset = 0;
        for (int i = 0; i < instanceCount; i++) {
            const PathData& args = fPaths[i];

            pathGeoHelper.beginInstance();

            if (!this->createGeom(pathGeoHelper, args.fPath, args.fTolerance)) {
                return;
            }

            pathGeoHelper.endInstance();
        }

        pathGeoHelper.emitMesh();
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

    bool createGeom(PathGeoHelper& helper, const SkPath& path, SkScalar srcSpaceTol) const {
        SkScalar srcSpaceTolSqd = srcSpaceTol * srcSpaceTol;

        uint16_t* idxBase = helper.indexBase();
        uint16_t* idx = idxBase;

        SkPoint* base = helper.vertexBase();
        SkPoint* vert = base;

        SkPoint pts[4];

        bool first = true;

        SkPath::Iter iter(path, false);

        bool done = false;
        while (!done) {
            SkPath::Verb verb = iter.next(pts);
            switch (verb) {
                case SkPath::kMove_Verb:
                    if (!first) {
                        uint16_t currIdx = (uint16_t) (vert - base + helper.fVertexOffset);
                        helper.fSubpathIndexStart = currIdx;
                    }
                    *vert = pts[0];
                    vert++;
                    break;
                case SkPath::kLine_Verb:
                    if (helper.isIndexed()) {
                        uint16_t prevIdx = (uint16_t)(vert - base - 1 + helper.fVertexOffset);
                        append_countour_edge_indices(this->isHairline(), helper.fSubpathIndexStart,
                                                     prevIdx, &idx);
                    }
                    *(vert++) = pts[1];
                    break;
                case SkPath::kConic_Verb: {
                    SkScalar weight = iter.conicWeight();
                    SkAutoConicToQuads converter;
                    const SkPoint* quadPts = converter.computeQuads(pts, weight, srcSpaceTol);
                    for (int i = 0; i < converter.countQuads(); ++i) {
                        add_quad(&vert, base, quadPts + i*2, srcSpaceTolSqd, srcSpaceTol,
                                 helper.isIndexed(), this->isHairline(), helper.fSubpathIndexStart,
                                 helper.fVertexOffset, &idx);
                    }
                    break;
                }
                case SkPath::kQuad_Verb:
                    add_quad(&vert, base, pts, srcSpaceTolSqd, srcSpaceTol, helper.isIndexed(),
                             this->isHairline(), helper.fSubpathIndexStart, helper.fVertexOffset,
                             &idx);
                    break;
                case SkPath::kCubic_Verb: {
                    // first pt of cubic is the pt we ended on in previous step
                    uint16_t firstCPtIdx = (uint16_t)(vert - base - 1 + helper.fVertexOffset);
                    uint16_t numPts = (uint16_t) GrPathUtils::generateCubicPoints(
                                    pts[0], pts[1], pts[2], pts[3],
                                    srcSpaceTolSqd, &vert,
                                    GrPathUtils::cubicPointCount(pts, srcSpaceTol));
                    if (helper.isIndexed()) {
                        for (uint16_t i = 0; i < numPts; ++i) {
                            append_countour_edge_indices(this->isHairline(),
                                                         helper.fSubpathIndexStart,
                                                         firstCPtIdx + i, &idx);
                        }
                    }
                    break;
                }
                case SkPath::kClose_Verb:
                    break;
                case SkPath::kDone_Verb:
                    done = true;
            }
            first = false;
        }

        helper.fVertexCount = static_cast<int>(vert - base);
        helper.fIndexCount = static_cast<int>(idx - idxBase);
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
