/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAALinearizingConvexPathRenderer.h"
#include "GrAAConvexTessellator.h"
#include "GrContext.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawOpTest.h"
#include "GrGeometryProcessor.h"
#include "GrOpFlushState.h"
#include "GrPathUtils.h"
#include "GrProcessor.h"
#include "GrStyle.h"
#include "SkGeometry.h"
#include "SkPathPriv.h"
#include "SkString.h"
#include "SkTraceEvent.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "ops/GrMeshDrawOp.h"
#include "ops/GrSimpleMeshDrawOpHelper.h"

static const int DEFAULT_BUFFER_SIZE = 100;

// The thicker the stroke, the harder it is to produce high-quality results using tessellation. For
// the time being, we simply drop back to software rendering above this stroke width.
static const SkScalar kMaxStrokeWidth = 20.0;

GrAALinearizingConvexPathRenderer::GrAALinearizingConvexPathRenderer() {
}

///////////////////////////////////////////////////////////////////////////////

GrPathRenderer::CanDrawPath
GrAALinearizingConvexPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
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
    return CanDrawPath::kYes;
}

// extract the result vertices and indices from the GrAAConvexTessellator
static void extract_verts(const GrAAConvexTessellator& tess,
                          void* vertices,
                          size_t vertexStride,
                          GrColor color,
                          uint16_t firstIndex,
                          uint16_t* idxs,
                          bool tweakAlphaForCoverage) {
    intptr_t verts = reinterpret_cast<intptr_t>(vertices);

    for (int i = 0; i < tess.numPts(); ++i) {
        *((SkPoint*)((intptr_t)verts + i * vertexStride)) = tess.point(i);
    }

    // Make 'verts' point to the colors
    verts += sizeof(SkPoint);
    for (int i = 0; i < tess.numPts(); ++i) {
        if (tweakAlphaForCoverage) {
            SkASSERT(SkScalarRoundToInt(255.0f * tess.coverage(i)) <= 255);
            unsigned scale = SkScalarRoundToInt(255.0f * tess.coverage(i));
            GrColor scaledColor = (0xff == scale) ? color : SkAlphaMulQ(color, scale);
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = scaledColor;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
            *reinterpret_cast<float*>(verts + i * vertexStride + sizeof(GrColor)) =
                    tess.coverage(i);
        }
    }

    for (int i = 0; i < tess.numIndices(); ++i) {
        idxs[i] = tess.index(i) + firstIndex;
    }
}

static sk_sp<GrGeometryProcessor> create_lines_only_gp(bool tweakAlphaForCoverage,
                                                       const SkMatrix& viewMatrix,
                                                       bool usesLocalCoords) {
    using namespace GrDefaultGeoProcFactory;

    Coverage::Type coverageType;
    if (tweakAlphaForCoverage) {
        coverageType = Coverage::kSolid_Type;
    } else {
        coverageType = Coverage::kAttribute_Type;
    }
    LocalCoords::Type localCoordsType =
            usesLocalCoords ? LocalCoords::kUsePosition_Type : LocalCoords::kUnused_Type;
    return MakeForDeviceSpace(Color::kPremulGrColorAttribute_Type, coverageType, localCoordsType,
                              viewMatrix);
}

namespace {

class AAFlatteningConvexPathOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID
    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          const SkPath& path,
                                          SkScalar strokeWidth,
                                          SkStrokeRec::Style style,
                                          SkPaint::Join join,
                                          SkScalar miterLimit,
                                          const GrUserStencilSettings* stencilSettings) {
        return Helper::FactoryHelper<AAFlatteningConvexPathOp>(std::move(paint), viewMatrix, path,
                                                               strokeWidth, style, join, miterLimit,
                                                               stencilSettings);
    }

    AAFlatteningConvexPathOp(const Helper::MakeArgs& helperArgs,
                             GrColor color,
                             const SkMatrix& viewMatrix,
                             const SkPath& path,
                             SkScalar strokeWidth,
                             SkStrokeRec::Style style,
                             SkPaint::Join join,
                             SkScalar miterLimit,
                             const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID()), fHelper(helperArgs, GrAAType::kCoverage, stencilSettings) {
        fPaths.emplace_back(
                PathData{color, viewMatrix, path, strokeWidth, style, join, miterLimit});

        // compute bounds
        SkRect bounds = path.getBounds();
        SkScalar w = strokeWidth;
        if (w > 0) {
            w /= 2;
            // If the half stroke width is < 1 then we effectively fallback to bevel joins.
            if (SkPaint::kMiter_Join == join && w > 1.f) {
                w *= miterLimit;
            }
            bounds.outset(w, w);
        }
        this->setTransformedBounds(bounds, viewMatrix, HasAABloat::kYes, IsZeroArea::kNo);
    }

    const char* name() const override { return "AAFlatteningConvexPathOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fHelper.visitProxies(func);
    }

    SkString dumpInfo() const override {
        SkString string;
        for (const auto& path : fPaths) {
            string.appendf(
                    "Color: 0x%08x, StrokeWidth: %.2f, Style: %d, Join: %d, "
                    "MiterLimit: %.2f\n",
                    path.fColor, path.fStrokeWidth, path.fStyle, path.fJoin, path.fMiterLimit);
        }
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                GrPixelConfigIsClamped dstIsClamped) override {
        return fHelper.xpRequiresDstTexture(caps, clip, dstIsClamped,
                                            GrProcessorAnalysisCoverage::kSingleChannel,
                                            &fPaths.back().fColor);
    }

private:
    void draw(GrMeshDrawOp::Target* target, const GrGeometryProcessor* gp,
              const GrPipeline* pipeline, int vertexCount, size_t vertexStride, void* vertices,
              int indexCount, uint16_t* indices) const {
        if (vertexCount == 0 || indexCount == 0) {
            return;
        }
        const GrBuffer* vertexBuffer;
        GrMesh mesh(GrPrimitiveType::kTriangles);
        int firstVertex;
        void* verts = target->makeVertexSpace(vertexStride, vertexCount, &vertexBuffer,
                                              &firstVertex);
        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }
        memcpy(verts, vertices, vertexCount * vertexStride);

        const GrBuffer* indexBuffer;
        int firstIndex;
        uint16_t* idxs = target->makeIndexSpace(indexCount, &indexBuffer, &firstIndex);
        if (!idxs) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
        memcpy(idxs, indices, indexCount * sizeof(uint16_t));
        mesh.setIndexed(indexBuffer, indexCount, firstIndex, 0, vertexCount - 1);
        mesh.setVertexData(vertexBuffer, firstVertex);
        target->draw(gp, pipeline, mesh);
    }

    void onPrepareDraws(Target* target) override {
        const GrPipeline* pipeline = fHelper.makePipeline(target);

        // Setup GrGeometryProcessor
        sk_sp<GrGeometryProcessor> gp(create_lines_only_gp(fHelper.compatibleWithAlphaAsCoverage(),
                                                           this->viewMatrix(),
                                                           fHelper.usesLocalCoords()));
        if (!gp) {
            SkDebugf("Couldn't create a GrGeometryProcessor\n");
            return;
        }

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(fHelper.compatibleWithAlphaAsCoverage()
                         ? vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr)
                         : vertexStride ==
                                   sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));

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
                this->draw(target, gp.get(), pipeline, vertexCount, vertexStride, vertices,
                           indexCount, indices);
                vertexCount = 0;
                indexCount = 0;
            }
            if (vertexCount + currentVertices > maxVertices) {
                maxVertices = SkTMax(vertexCount + currentVertices, maxVertices * 2);
                if (maxVertices * vertexStride > SK_MaxS32) {
                    sk_free(vertices);
                    sk_free(indices);
                    return;
                }
                vertices = (uint8_t*) sk_realloc_throw(vertices, maxVertices * vertexStride);
            }
            int currentIndices = tess.numIndices();
            if (indexCount + currentIndices > maxIndices) {
                maxIndices = SkTMax(indexCount + currentIndices, maxIndices * 2);
                if (maxIndices * sizeof(uint16_t) > SK_MaxS32) {
                    sk_free(vertices);
                    sk_free(indices);
                    return;
                }
                indices = (uint16_t*) sk_realloc_throw(indices, maxIndices * sizeof(uint16_t));
            }

            extract_verts(tess, vertices + vertexStride * vertexCount, vertexStride, args.fColor,
                          vertexCount, indices + indexCount,
                          fHelper.compatibleWithAlphaAsCoverage());
            vertexCount += currentVertices;
            indexCount += currentIndices;
        }
        if (vertexCount <= SK_MaxS32 && indexCount <= SK_MaxS32) {
            this->draw(target, gp.get(), pipeline, vertexCount, vertexStride, vertices, indexCount,
                       indices);
        }
        sk_free(vertices);
        sk_free(indices);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        AAFlatteningConvexPathOp* that = t->cast<AAFlatteningConvexPathOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }

        fPaths.push_back_n(that->fPaths.count(), that->fPaths.begin());
        this->joinBounds(*that);
        return true;
    }

    const SkMatrix& viewMatrix() const { return fPaths[0].fViewMatrix; }

    struct PathData {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkPath fPath;
        SkScalar fStrokeWidth;
        SkStrokeRec::Style fStyle;
        SkPaint::Join fJoin;
        SkScalar fMiterLimit;
    };

    SkSTArray<1, PathData, true> fPaths;
    Helper fHelper;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

bool GrAALinearizingConvexPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrAALinearizingConvexPathRenderer::onDrawPath");
    SkASSERT(GrFSAAType::kUnifiedMSAA != args.fRenderTargetContext->fsaaType());
    SkASSERT(!args.fShape->isEmpty());
    SkASSERT(!args.fShape->style().pathEffect());

    SkPath path;
    args.fShape->asPath(&path);
    bool fill = args.fShape->style().isSimpleFill();
    const SkStrokeRec& stroke = args.fShape->style().strokeRec();
    SkScalar strokeWidth = fill ? -1.0f : stroke.getWidth();
    SkPaint::Join join = fill ? SkPaint::Join::kMiter_Join : stroke.getJoin();
    SkScalar miterLimit = stroke.getMiter();

    std::unique_ptr<GrDrawOp> op = AAFlatteningConvexPathOp::Make(
            std::move(args.fPaint), *args.fViewMatrix, path, strokeWidth, stroke.getStyle(), join,
            miterLimit, args.fUserStencilSettings);
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(AAFlatteningConvexPathOp) {
    SkMatrix viewMatrix = GrTest::TestMatrixPreservesRightAngles(random);
    SkPath path = GrTest::TestPathConvex(random);

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
    return AAFlatteningConvexPathOp::Make(std::move(paint), viewMatrix, path, strokeWidth, style,
                                          join, miterLimit, stencilSettings);
}

#endif
