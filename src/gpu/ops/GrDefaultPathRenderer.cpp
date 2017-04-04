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
#include "ops/GrRectOpFactory.h"

GrDefaultPathRenderer::GrDefaultPathRenderer(bool separateStencilSupport,
                                             bool stencilWrapOpsSupport)
    : fSeparateStencil(separateStencilSupport)
    , fStencilWrapOps(stencilWrapOpsSupport) {
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

        // compute number of vertices
        int maxVertices = 0;

        // We will use index buffers if we have multiple paths or one path with multiple contours
        bool isIndexed = instanceCount > 1;
        for (int i = 0; i < instanceCount; i++) {
            const PathData& args = fPaths[i];

            int contourCount;
            maxVertices += GrPathUtils::worstCasePointCount(args.fPath, &contourCount,
                                                            args.fTolerance);

            isIndexed = isIndexed || contourCount > 1;
        }

        if (maxVertices == 0 || maxVertices > ((int)SK_MaxU16 + 1)) {
            //SkDebugf("Cannot render path (%d)\n", maxVertices);
            return;
        }

        // determine primitiveType
        int maxIndices = 0;
        GrPrimitiveType primitiveType;
        if (this->isHairline()) {
            if (isIndexed) {
                maxIndices = 2 * maxVertices;
                primitiveType = kLines_GrPrimitiveType;
            } else {
                primitiveType = kLineStrip_GrPrimitiveType;
            }
        } else {
            if (isIndexed) {
                maxIndices = 3 * maxVertices;
                primitiveType = kTriangles_GrPrimitiveType;
            } else {
                primitiveType = kTriangleFan_GrPrimitiveType;
            }
        }

        // allocate vertex / index buffers
        const GrBuffer* vertexBuffer;
        int firstVertex;

        void* verts = target->makeVertexSpace(vertexStride, maxVertices,
                                              &vertexBuffer, &firstVertex);

        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        const GrBuffer* indexBuffer = nullptr;
        int firstIndex = 0;

        void* indices = nullptr;
        if (isIndexed) {
            indices = target->makeIndexSpace(maxIndices, &indexBuffer, &firstIndex);

            if (!indices) {
                SkDebugf("Could not allocate indices\n");
                return;
            }
        }

        // fill buffers
        int vertexOffset = 0;
        int indexOffset = 0;
        for (int i = 0; i < instanceCount; i++) {
            const PathData& args = fPaths[i];

            int vertexCnt = 0;
            int indexCnt = 0;
            if (!this->createGeom(verts,
                                  vertexOffset,
                                  indices,
                                  indexOffset,
                                  &vertexCnt,
                                  &indexCnt,
                                  args.fPath,
                                  args.fTolerance,
                                  isIndexed)) {
                return;
            }

            vertexOffset += vertexCnt;
            indexOffset += indexCnt;
            SkASSERT(vertexOffset <= maxVertices && indexOffset <= maxIndices);
        }

        GrMesh mesh;
        if (isIndexed) {
            mesh.initIndexed(primitiveType, vertexBuffer, indexBuffer, firstVertex, firstIndex,
                             vertexOffset, indexOffset);
        } else {
            mesh.init(primitiveType, vertexBuffer, firstVertex, vertexOffset);
        }
        target->draw(gp.get(), this->pipeline(), mesh);

        // put back reserves
        target->putBackIndices((size_t)(maxIndices - indexOffset));
        target->putBackVertices((size_t)(maxVertices - vertexOffset), (size_t)vertexStride);
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

    bool createGeom(void* vertices,
                    size_t vertexOffset,
                    void* indices,
                    size_t indexOffset,
                    int* vertexCnt,
                    int* indexCnt,
                    const SkPath& path,
                    SkScalar srcSpaceTol,
                    bool isIndexed) const {
            SkScalar srcSpaceTolSqd = srcSpaceTol * srcSpaceTol;

            uint16_t indexOffsetU16 = (uint16_t)indexOffset;
            uint16_t vertexOffsetU16 = (uint16_t)vertexOffset;

            uint16_t* idxBase = reinterpret_cast<uint16_t*>(indices) + indexOffsetU16;
            uint16_t* idx = idxBase;
            uint16_t subpathIdxStart = vertexOffsetU16;

            SkPoint* base = reinterpret_cast<SkPoint*>(vertices) + vertexOffset;
            SkPoint* vert = base;

            SkPoint pts[4];

            bool first = true;
            int subpath = 0;

            SkPath::Iter iter(path, false);

            bool done = false;
            while (!done) {
                SkPath::Verb verb = iter.next(pts);
                switch (verb) {
                    case SkPath::kMove_Verb:
                        if (!first) {
                            uint16_t currIdx = (uint16_t) (vert - base) + vertexOffsetU16;
                            subpathIdxStart = currIdx;
                            ++subpath;
                        }
                        *vert = pts[0];
                        vert++;
                        break;
                    case SkPath::kLine_Verb:
                        if (isIndexed) {
                            uint16_t prevIdx = (uint16_t)(vert - base) - 1 + vertexOffsetU16;
                            append_countour_edge_indices(this->isHairline(), subpathIdxStart,
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
                                     isIndexed, this->isHairline(), subpathIdxStart,
                                     (int)vertexOffset, &idx);
                        }
                        break;
                    }
                    case SkPath::kQuad_Verb:
                        add_quad(&vert, base, pts, srcSpaceTolSqd, srcSpaceTol, isIndexed,
                                 this->isHairline(), subpathIdxStart, (int)vertexOffset, &idx);
                        break;
                    case SkPath::kCubic_Verb: {
                        // first pt of cubic is the pt we ended on in previous step
                        uint16_t firstCPtIdx = (uint16_t)(vert - base) - 1 + vertexOffsetU16;
                        uint16_t numPts = (uint16_t) GrPathUtils::generateCubicPoints(
                                        pts[0], pts[1], pts[2], pts[3],
                                        srcSpaceTolSqd, &vert,
                                        GrPathUtils::cubicPointCount(pts, srcSpaceTol));
                        if (isIndexed) {
                            for (uint16_t i = 0; i < numPts; ++i) {
                                append_countour_edge_indices(this->isHairline(), subpathIdxStart,
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

            *vertexCnt = static_cast<int>(vert - base);
            *indexCnt = static_cast<int>(idx - idxBase);
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
    const GrUserStencilSettings* passes[3];
    GrDrawFace                   drawFace[3];
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
        drawFace[0] = GrDrawFace::kBoth;
    } else {
        if (single_pass_shape(shape)) {
            passCount = 1;
            if (stencilOnly) {
                passes[0] = &gDirectToStencil;
            } else {
                passes[0] = &userStencilSettings;
            }
            drawFace[0] = GrDrawFace::kBoth;
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
                    drawFace[0] = drawFace[1] = GrDrawFace::kBoth;
                    break;

                case SkPath::kInverseWinding_FillType:
                    reverse = true;
                    // fallthrough
                case SkPath::kWinding_FillType:
                    if (fSeparateStencil) {
                        if (fStencilWrapOps) {
                            passes[0] = &gWindStencilSeparateWithWrap;
                        } else {
                            passes[0] = &gWindStencilSeparateNoWrap;
                        }
                        passCount = 2;
                        drawFace[0] = GrDrawFace::kBoth;
                    } else {
                        if (fStencilWrapOps) {
                            passes[0] = &gWindSingleStencilWithWrapInc;
                            passes[1] = &gWindSingleStencilWithWrapDec;
                        } else {
                            passes[0] = &gWindSingleStencilNoWrapInc;
                            passes[1] = &gWindSingleStencilNoWrapDec;
                        }
                        // which is cw and which is ccw is arbitrary.
                        drawFace[0] = GrDrawFace::kCW;
                        drawFace[1] = GrDrawFace::kCCW;
                        passCount = 3;
                    }
                    if (stencilOnly) {
                        lastPassIsBounds = false;
                        --passCount;
                    } else {
                        lastPassIsBounds = true;
                        drawFace[passCount-1] = GrDrawFace::kBoth;
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
            std::unique_ptr<GrLegacyMeshDrawOp> op(GrRectOpFactory::MakeNonAAFill(
                    paint.getColor(), viewM, bounds, nullptr, &localMatrix));

            SkASSERT(GrDrawFace::kBoth == drawFace[p]);
            GrPipelineBuilder pipelineBuilder(std::move(paint), aaType);
            pipelineBuilder.setDrawFace(drawFace[p]);
            pipelineBuilder.setUserStencil(passes[p]);
            renderTargetContext->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip,
                                                     std::move(op));
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
            pipelineBuilder.setDrawFace(drawFace[p]);
            pipelineBuilder.setUserStencil(passes[p]);
            renderTargetContext->addLegacyMeshDrawOp(std::move(pipelineBuilder), clip,
                                                     std::move(op));
        }
    }
    return true;
}

bool GrDefaultPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // This can draw any path with any simple fill style but doesn't do coverage-based antialiasing.
    return GrAAType::kCoverage != args.fAAType &&
           (args.fShape->style().isSimpleFill() ||
            IsStrokeHairlineOrEquivalent(args.fShape->style(), *args.fViewMatrix, nullptr));
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

DRAW_OP_TEST_DEFINE(DefaultPathOp) {
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
