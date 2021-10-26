/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/AALinearizingConvexPathRenderer.h"

#include "include/core/SkString.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/geometry/GrAAConvexTessellator.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelperWithStencil.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

///////////////////////////////////////////////////////////////////////////////
namespace skgpu::v1 {

namespace {

static const int DEFAULT_BUFFER_SIZE = 100;

// The thicker the stroke, the harder it is to produce high-quality results using tessellation. For
// the time being, we simply drop back to software rendering above this stroke width.
static const SkScalar kMaxStrokeWidth = 20.0;

// extract the result vertices and indices from the GrAAConvexTessellator
void extract_verts(const GrAAConvexTessellator& tess,
                   const SkMatrix* localCoordsMatrix,
                   void* vertData,
                   const GrVertexColor& color,
                   uint16_t firstIndex,
                   uint16_t* idxs) {
    VertexWriter verts{vertData};
    for (int i = 0; i < tess.numPts(); ++i) {
        SkPoint lc;
        if (localCoordsMatrix) {
            localCoordsMatrix->mapPoints(&lc, &tess.point(i), 1);
        }
        verts << tess.point(i) << color << VertexWriter::If(localCoordsMatrix, lc)
              << tess.coverage(i);
    }

    for (int i = 0; i < tess.numIndices(); ++i) {
        idxs[i] = tess.index(i) + firstIndex;
    }
}

GrGeometryProcessor* create_lines_only_gp(SkArenaAlloc* arena,
                                          bool tweakAlphaForCoverage,
                                          bool usesLocalCoords,
                                          bool wideColor) {
    using namespace GrDefaultGeoProcFactory;

    Coverage::Type coverageType =
        tweakAlphaForCoverage ? Coverage::kAttributeTweakAlpha_Type : Coverage::kAttribute_Type;
    LocalCoords::Type localCoordsType =
            usesLocalCoords ? LocalCoords::kHasExplicit_Type : LocalCoords::kUnused_Type;
    Color::Type colorType =
        wideColor ? Color::kPremulWideColorAttribute_Type : Color::kPremulGrColorAttribute_Type;

    return Make(arena, colorType, coverageType, localCoordsType, SkMatrix::I());
}

class AAFlatteningConvexPathOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const SkMatrix& viewMatrix,
                            const SkPath& path,
                            SkScalar strokeWidth,
                            SkStrokeRec::Style style,
                            SkPaint::Join join,
                            SkScalar miterLimit,
                            const GrUserStencilSettings* stencilSettings) {
        return Helper::FactoryHelper<AAFlatteningConvexPathOp>(context, std::move(paint),
                                                               viewMatrix, path,
                                                               strokeWidth, style, join, miterLimit,
                                                               stencilSettings);
    }

    AAFlatteningConvexPathOp(GrProcessorSet* processorSet,
                             const SkPMColor4f& color,
                             const SkMatrix& viewMatrix,
                             const SkPath& path,
                             SkScalar strokeWidth,
                             SkStrokeRec::Style style,
                             SkPaint::Join join,
                             SkScalar miterLimit,
                             const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID()), fHelper(processorSet, GrAAType::kCoverage, stencilSettings) {
        fPaths.emplace_back(
                PathData{viewMatrix, path, color, strokeWidth, miterLimit, style, join});

        // compute bounds
        SkRect bounds = path.getBounds();
        SkScalar w = strokeWidth;
        if (w > 0) {
            w /= 2;
            SkScalar maxScale = viewMatrix.getMaxScale();
            // We should not have a perspective matrix, thus we should have a valid scale.
            SkASSERT(maxScale != -1);
            if (SkPaint::kMiter_Join == join && w * maxScale > 1.f) {
                w *= miterLimit;
            }
            bounds.outset(w, w);
        }
        this->setTransformedBounds(bounds, viewMatrix, HasAABloat::kYes, IsHairline::kNo);
    }

    const char* name() const override { return "AAFlatteningConvexPathOp"; }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        return fHelper.finalizeProcessors(caps, clip, clampType,
                                          GrProcessorAnalysisCoverage::kSingleChannel,
                                          &fPaths.back().fColor, &fWideColor);
    }

private:
    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&& appliedClip,
                             const GrDstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        GrGeometryProcessor* gp = create_lines_only_gp(arena,
                                                       fHelper.compatibleWithCoverageAsAlpha(),
                                                       fHelper.usesLocalCoords(),
                                                       fWideColor);
        if (!gp) {
            SkDebugf("Couldn't create a GrGeometryProcessor\n");
            return;
        }

        fProgramInfo = fHelper.createProgramInfoWithStencil(caps, arena, writeView, usesMSAASurface,
                                                            std::move(appliedClip), dstProxyView,
                                                            gp, GrPrimitiveType::kTriangles,
                                                            renderPassXferBarriers, colorLoadOp);
    }

    void recordDraw(GrMeshDrawTarget* target,
                    int vertexCount, size_t vertexStride, void* vertices,
                    int indexCount, uint16_t* indices) {
        if (vertexCount == 0 || indexCount == 0) {
            return;
        }
        sk_sp<const GrBuffer> vertexBuffer;
        int firstVertex;
        void* verts = target->makeVertexSpace(vertexStride, vertexCount, &vertexBuffer,
                                              &firstVertex);
        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }
        memcpy(verts, vertices, vertexCount * vertexStride);

        sk_sp<const GrBuffer> indexBuffer;
        int firstIndex;
        uint16_t* idxs = target->makeIndexSpace(indexCount, &indexBuffer, &firstIndex);
        if (!idxs) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
        memcpy(idxs, indices, indexCount * sizeof(uint16_t));
        GrSimpleMesh* mesh = target->allocMesh();
        mesh->setIndexed(std::move(indexBuffer), indexCount, firstIndex, 0, vertexCount - 1,
                         GrPrimitiveRestart::kNo, std::move(vertexBuffer), firstVertex);
        fMeshes.push_back(mesh);
    }

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        if (!fProgramInfo) {
            this->createProgramInfo(target);
            if (!fProgramInfo) {
                return;
            }
        }

        size_t vertexStride =  fProgramInfo->geomProc().vertexStride();
        int instanceCount = fPaths.count();

        int64_t vertexCount = 0;
        int64_t indexCount = 0;
        int64_t maxVertices = DEFAULT_BUFFER_SIZE;
        int64_t maxIndices = DEFAULT_BUFFER_SIZE;
        uint8_t* vertices = (uint8_t*) sk_malloc_throw(maxVertices * vertexStride);
        uint16_t* indices = (uint16_t*) sk_malloc_throw(maxIndices * sizeof(uint16_t));
        for (int i = 0; i < instanceCount; i++) {
            const PathData& args = fPaths[i];
            GrAAConvexTessellator tess(args.fStyle, args.fStrokeWidth,
                                       args.fJoin, args.fMiterLimit);

            if (!tess.tessellate(args.fViewMatrix, args.fPath)) {
                continue;
            }

            int currentVertices = tess.numPts();
            if (vertexCount + currentVertices > static_cast<int>(UINT16_MAX)) {
                // if we added the current instance, we would overflow the indices we can store in a
                // uint16_t. Draw what we've got so far and reset.
                this->recordDraw(target, vertexCount, vertexStride, vertices, indexCount, indices);
                vertexCount = 0;
                indexCount = 0;
            }
            if (vertexCount + currentVertices > maxVertices) {
                maxVertices = std::max(vertexCount + currentVertices, maxVertices * 2);
                if (maxVertices * vertexStride > SK_MaxS32) {
                    sk_free(vertices);
                    sk_free(indices);
                    return;
                }
                vertices = (uint8_t*) sk_realloc_throw(vertices, maxVertices * vertexStride);
            }
            int currentIndices = tess.numIndices();
            if (indexCount + currentIndices > maxIndices) {
                maxIndices = std::max(indexCount + currentIndices, maxIndices * 2);
                if (maxIndices * sizeof(uint16_t) > SK_MaxS32) {
                    sk_free(vertices);
                    sk_free(indices);
                    return;
                }
                indices = (uint16_t*) sk_realloc_throw(indices, maxIndices * sizeof(uint16_t));
            }

            const SkMatrix* localCoordsMatrix = nullptr;
            SkMatrix ivm;
            if (fHelper.usesLocalCoords()) {
                if (!args.fViewMatrix.invert(&ivm)) {
                    ivm = SkMatrix::I();
                }
                localCoordsMatrix = &ivm;
            }

            extract_verts(tess, localCoordsMatrix, vertices + vertexStride * vertexCount,
                          GrVertexColor(args.fColor, fWideColor), vertexCount,
                          indices + indexCount);
            vertexCount += currentVertices;
            indexCount += currentIndices;
        }
        if (vertexCount <= SK_MaxS32 && indexCount <= SK_MaxS32) {
            this->recordDraw(target, vertexCount, vertexStride, vertices, indexCount, indices);
        }
        sk_free(vertices);
        sk_free(indices);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo || fMeshes.isEmpty()) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
        for (int i = 0; i < fMeshes.count(); ++i) {
            flushState->drawMesh(*fMeshes[i]);
        }
    }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override {
        AAFlatteningConvexPathOp* that = t->cast<AAFlatteningConvexPathOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return CombineResult::kCannotCombine;
        }

        fPaths.push_back_n(that->fPaths.count(), that->fPaths.begin());
        fWideColor |= that->fWideColor;
        return CombineResult::kMerged;
    }

#if GR_TEST_UTILS
    SkString onDumpInfo() const override {
        SkString string;
        for (const auto& path : fPaths) {
            string.appendf(
                    "Color: 0x%08x, StrokeWidth: %.2f, Style: %d, Join: %d, "
                    "MiterLimit: %.2f\n",
                    path.fColor.toBytes_RGBA(), path.fStrokeWidth, path.fStyle, path.fJoin,
                    path.fMiterLimit);
        }
        string += fHelper.dumpInfo();
        return string;
    }
#endif

    struct PathData {
        SkMatrix fViewMatrix;
        SkPath fPath;
        SkPMColor4f fColor;
        SkScalar fStrokeWidth;
        SkScalar fMiterLimit;
        SkStrokeRec::Style fStyle;
        SkPaint::Join fJoin;
    };

    SkSTArray<1, PathData, true> fPaths;
    Helper fHelper;
    bool fWideColor;

    SkTDArray<GrSimpleMesh*> fMeshes;
    GrProgramInfo*           fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};

}  // anonymous namespace

///////////////////////////////////////////////////////////////////////////////////////////////////

PathRenderer::CanDrawPath
AALinearizingConvexPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    if (GrAAType::kCoverage != args.fAAType) {
        return CanDrawPath::kNo;
    }
    if (!args.fShape->knownToBeConvex()) {
        return CanDrawPath::kNo;
    }
    if (args.fShape->style().pathEffect()) {
        return CanDrawPath::kNo;
    }
    if (args.fShape->inverseFilled()) {
        return CanDrawPath::kNo;
    }
    if (args.fShape->bounds().width() <= 0 && args.fShape->bounds().height() <= 0) {
        // Stroked zero length lines should draw, but this PR doesn't handle that case
        return CanDrawPath::kNo;
    }
    const SkStrokeRec& stroke = args.fShape->style().strokeRec();

    if (stroke.getStyle() == SkStrokeRec::kStroke_Style ||
        stroke.getStyle() == SkStrokeRec::kStrokeAndFill_Style) {
        if (!args.fViewMatrix->isSimilarity()) {
            return CanDrawPath::kNo;
        }
        SkScalar strokeWidth = args.fViewMatrix->getMaxScale() * stroke.getWidth();
        if (strokeWidth < 1.0f && stroke.getStyle() == SkStrokeRec::kStroke_Style) {
            return CanDrawPath::kNo;
        }
        if (strokeWidth > kMaxStrokeWidth ||
            !args.fShape->knownToBeClosed() ||
            stroke.getJoin() == SkPaint::Join::kRound_Join) {
            return CanDrawPath::kNo;
        }
        return CanDrawPath::kYes;
    }
    if (stroke.getStyle() != SkStrokeRec::kFill_Style) {
        return CanDrawPath::kNo;
    }
    // This can almost handle perspective. It would need to use 3 component explicit local coords
    // when there are FPs that require them. This is difficult to test because AAConvexPathRenderer
    // takes almost all filled paths that could get here. So just avoid perspective fills.
    if (args.fViewMatrix->hasPerspective()) {
        return CanDrawPath::kNo;
    }
    return CanDrawPath::kYes;
}

bool AALinearizingConvexPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fContext->priv().auditTrail(),
                              "AALinearizingConvexPathRenderer::onDrawPath");
    SkASSERT(args.fSurfaceDrawContext->numSamples() <= 1);
    SkASSERT(!args.fShape->isEmpty());
    SkASSERT(!args.fShape->style().pathEffect());

    SkPath path;
    args.fShape->asPath(&path);
    bool fill = args.fShape->style().isSimpleFill();
    const SkStrokeRec& stroke = args.fShape->style().strokeRec();
    SkScalar strokeWidth = fill ? -1.0f : stroke.getWidth();
    SkPaint::Join join = fill ? SkPaint::Join::kMiter_Join : stroke.getJoin();
    SkScalar miterLimit = stroke.getMiter();

    GrOp::Owner op = AAFlatteningConvexPathOp::Make(
            args.fContext, std::move(args.fPaint), *args.fViewMatrix, path, strokeWidth,
            stroke.getStyle(), join, miterLimit, args.fUserStencilSettings);
    args.fSurfaceDrawContext->addDrawOp(args.fClip, std::move(op));
    return true;
}

} // namespace skgpu::v1

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(AAFlatteningConvexPathOp) {
    SkMatrix viewMatrix = GrTest::TestMatrixPreservesRightAngles(random);
    const SkPath& path = GrTest::TestPathConvex(random);

    SkStrokeRec::Style styles[3] = { SkStrokeRec::kFill_Style,
                                     SkStrokeRec::kStroke_Style,
                                     SkStrokeRec::kStrokeAndFill_Style };

    SkStrokeRec::Style style = styles[random->nextU() % 3];

    SkScalar strokeWidth = -1.f;
    SkPaint::Join join = SkPaint::kMiter_Join;
    SkScalar miterLimit = 0.5f;

    if (SkStrokeRec::kFill_Style != style) {
        strokeWidth = random->nextRangeF(1.0f, 10.0f);
        if (random->nextBool()) {
            join = SkPaint::kMiter_Join;
        } else {
            join = SkPaint::kBevel_Join;
        }
        miterLimit = random->nextRangeF(0.5f, 2.0f);
    }
    const GrUserStencilSettings* stencilSettings = GrGetRandomStencil(random, context);
    return skgpu::v1::AAFlatteningConvexPathOp::Make(context, std::move(paint), viewMatrix, path,
                                                     strokeWidth, style, join, miterLimit,
                                                     stencilSettings);
}

#endif
