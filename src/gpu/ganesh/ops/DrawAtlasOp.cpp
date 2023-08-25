/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/ops/DrawAtlasOp.h"

#include "include/core/SkRSXform.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/base/SkRandom.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDefaultGeoProcFactory.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"

using namespace skia_private;

namespace {

class DrawAtlasOpImpl final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    DrawAtlasOpImpl(GrProcessorSet*, const SkPMColor4f& color,
                    const SkMatrix& viewMatrix, GrAAType, int spriteCount, const SkRSXform* xforms,
                    const SkRect* rects, const SkColor* colors);

    const char* name() const override { return "DrawAtlasOp"; }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

    FixedFunctionFlags fixedFunctionFlags() const override;

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;

private:
    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&&,
                             const GrDstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override;

    void onPrepareDraws(GrMeshDrawTarget*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;
#if defined(GR_TEST_UTILS)
    SkString onDumpInfo() const override;
#endif

    const SkPMColor4f& color() const { return fColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool hasColors() const { return fHasColors; }
    int quadCount() const { return fQuadCount; }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps&) override;

    struct Geometry {
        SkPMColor4f fColor;
        TArray<uint8_t, true> fVerts;
    };

    STArray<1, Geometry, true> fGeoData;
    Helper fHelper;
    SkMatrix fViewMatrix;
    SkPMColor4f fColor;
    int fQuadCount;
    bool fHasColors;

    GrSimpleMesh* fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;
};

GrGeometryProcessor* make_gp(SkArenaAlloc* arena,
                             bool hasColors,
                             const SkPMColor4f& color,
                             const SkMatrix& viewMatrix) {
    using namespace GrDefaultGeoProcFactory;
    Color gpColor(color);
    if (hasColors) {
        gpColor.fType = Color::kPremulGrColorAttribute_Type;
    }

    return GrDefaultGeoProcFactory::Make(arena, gpColor, Coverage::kSolid_Type,
                                         LocalCoords::kHasExplicit_Type, viewMatrix);
}

DrawAtlasOpImpl::DrawAtlasOpImpl(GrProcessorSet* processorSet, const SkPMColor4f& color,
                                 const SkMatrix& viewMatrix, GrAAType aaType, int spriteCount,
                                 const SkRSXform* xforms, const SkRect* rects,
                                 const SkColor* colors)
        : GrMeshDrawOp(ClassID()), fHelper(processorSet, aaType), fColor(color) {
    SkASSERT(xforms);
    SkASSERT(rects);

    fViewMatrix = viewMatrix;
    Geometry& installedGeo = fGeoData.push_back();
    installedGeo.fColor = color;

    // Figure out stride and offsets
    // Order within the vertex is: position [color] texCoord
    size_t texOffset = sizeof(SkPoint);
    size_t vertexStride = 2 * sizeof(SkPoint);
    fHasColors = SkToBool(colors);
    if (colors) {
        texOffset += sizeof(GrColor);
        vertexStride += sizeof(GrColor);
    }

    // Compute buffer size and alloc buffer
    fQuadCount = spriteCount;
    int allocSize = static_cast<int>(4 * vertexStride * spriteCount);
    installedGeo.fVerts.reset(allocSize);
    uint8_t* currVertex = installedGeo.fVerts.begin();

    SkRect bounds = SkRectPriv::MakeLargestInverted();
    // TODO4F: Preserve float colors
    int paintAlpha = GrColorUnpackA(installedGeo.fColor.toBytes_RGBA());
    for (int spriteIndex = 0; spriteIndex < spriteCount; ++spriteIndex) {
        // Transform rect
        SkPoint strip[4];
        const SkRect& currRect = rects[spriteIndex];
        xforms[spriteIndex].toTriStrip(currRect.width(), currRect.height(), strip);

        // Copy colors if necessary
        if (colors) {
            // convert to GrColor
            SkColor spriteColor = colors[spriteIndex];
            if (paintAlpha != 255) {
                spriteColor = SkColorSetA(spriteColor,
                                          SkMulDiv255Round(SkColorGetA(spriteColor), paintAlpha));
            }
            GrColor grColor = SkColorToPremulGrColor(spriteColor);

            *(reinterpret_cast<GrColor*>(currVertex + sizeof(SkPoint))) = grColor;
            *(reinterpret_cast<GrColor*>(currVertex + vertexStride + sizeof(SkPoint))) = grColor;
            *(reinterpret_cast<GrColor*>(currVertex + 2 * vertexStride + sizeof(SkPoint))) =
                    grColor;
            *(reinterpret_cast<GrColor*>(currVertex + 3 * vertexStride + sizeof(SkPoint))) =
                    grColor;
        }

        // Copy position and uv to verts
        *(reinterpret_cast<SkPoint*>(currVertex)) = strip[0];
        *(reinterpret_cast<SkPoint*>(currVertex + texOffset)) =
                SkPoint::Make(currRect.fLeft, currRect.fTop);
        SkRectPriv::GrowToInclude(&bounds, strip[0]);
        currVertex += vertexStride;

        *(reinterpret_cast<SkPoint*>(currVertex)) = strip[1];
        *(reinterpret_cast<SkPoint*>(currVertex + texOffset)) =
                SkPoint::Make(currRect.fLeft, currRect.fBottom);
        SkRectPriv::GrowToInclude(&bounds, strip[1]);
        currVertex += vertexStride;

        *(reinterpret_cast<SkPoint*>(currVertex)) = strip[2];
        *(reinterpret_cast<SkPoint*>(currVertex + texOffset)) =
                SkPoint::Make(currRect.fRight, currRect.fTop);
        SkRectPriv::GrowToInclude(&bounds, strip[2]);
        currVertex += vertexStride;

        *(reinterpret_cast<SkPoint*>(currVertex)) = strip[3];
        *(reinterpret_cast<SkPoint*>(currVertex + texOffset)) =
                SkPoint::Make(currRect.fRight, currRect.fBottom);
        SkRectPriv::GrowToInclude(&bounds, strip[3]);
        currVertex += vertexStride;
    }

    this->setTransformedBounds(bounds, viewMatrix, HasAABloat::kNo, IsHairline::kNo);
}

#if defined(GR_TEST_UTILS)
SkString DrawAtlasOpImpl::onDumpInfo() const {
    SkString string;
    for (const auto& geo : fGeoData) {
        string.appendf("Color: 0x%08x, Quads: %d\n", geo.fColor.toBytes_RGBA(),
                       geo.fVerts.size() / 4);
    }
    string += fHelper.dumpInfo();
    return string;
}
#endif

void DrawAtlasOpImpl::onCreateProgramInfo(const GrCaps* caps,
                                          SkArenaAlloc* arena,
                                          const GrSurfaceProxyView& writeView,
                                          bool usesMSAASurface,
                                          GrAppliedClip&& appliedClip,
                                          const GrDstProxyView& dstProxyView,
                                          GrXferBarrierFlags renderPassXferBarriers,
                                          GrLoadOp colorLoadOp) {
    // Setup geometry processor
    GrGeometryProcessor* gp = make_gp(arena,
                                      this->hasColors(),
                                      this->color(),
                                      this->viewMatrix());

    fProgramInfo = fHelper.createProgramInfo(caps, arena, writeView, usesMSAASurface,
                                             std::move(appliedClip), dstProxyView, gp,
                                             GrPrimitiveType::kTriangles, renderPassXferBarriers,
                                             colorLoadOp);
}

void DrawAtlasOpImpl::onPrepareDraws(GrMeshDrawTarget* target) {
    if (!fProgramInfo) {
        this->createProgramInfo(target);
    }

    int instanceCount = fGeoData.size();
    size_t vertexStride = fProgramInfo->geomProc().vertexStride();

    int numQuads = this->quadCount();
    QuadHelper helper(target, vertexStride, numQuads);
    void* verts = helper.vertices();
    if (!verts) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    uint8_t* vertPtr = reinterpret_cast<uint8_t*>(verts);
    for (int i = 0; i < instanceCount; i++) {
        const Geometry& args = fGeoData[i];

        size_t allocSize = args.fVerts.size();
        memcpy(vertPtr, args.fVerts.begin(), allocSize);
        vertPtr += allocSize;
    }

    fMesh = helper.mesh();
}

void DrawAtlasOpImpl::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fProgramInfo || !fMesh) {
        return;
    }

    flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
    flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
    flushState->drawMesh(*fMesh);
}

GrOp::CombineResult DrawAtlasOpImpl::onCombineIfPossible(GrOp* t,
                                                         SkArenaAlloc*,
                                                         const GrCaps& caps) {
    auto that = t->cast<DrawAtlasOpImpl>();

    if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
        return CombineResult::kCannotCombine;
    }

    // We currently use a uniform viewmatrix for this op.
    if (!SkMatrixPriv::CheapEqual(this->viewMatrix(), that->viewMatrix())) {
        return CombineResult::kCannotCombine;
    }

    if (this->hasColors() != that->hasColors()) {
        return CombineResult::kCannotCombine;
    }

    if (!this->hasColors() && this->color() != that->color()) {
        return CombineResult::kCannotCombine;
    }

    fGeoData.push_back_n(that->fGeoData.size(), that->fGeoData.begin());
    fQuadCount += that->quadCount();

    return CombineResult::kMerged;
}

GrDrawOp::FixedFunctionFlags DrawAtlasOpImpl::fixedFunctionFlags() const {
    return fHelper.fixedFunctionFlags();
}

GrProcessorSet::Analysis DrawAtlasOpImpl::finalize(const GrCaps& caps,
                                                   const GrAppliedClip* clip,
                                                   GrClampType clampType) {
    GrProcessorAnalysisColor gpColor;
    if (this->hasColors()) {
        gpColor.setToUnknown();
    } else {
        gpColor.setToConstant(fColor);
    }
    auto result = fHelper.finalizeProcessors(caps, clip, clampType,
                                             GrProcessorAnalysisCoverage::kNone, &gpColor);
    if (gpColor.isConstant(&fColor)) {
        fHasColors = false;
    }
    return result;
}

} // anonymous namespace

namespace skgpu::ganesh::DrawAtlasOp {

GrOp::Owner Make(GrRecordingContext* context,
                 GrPaint&& paint,
                 const SkMatrix& viewMatrix,
                 GrAAType aaType,
                 int spriteCount,
                 const SkRSXform* xforms,
                 const SkRect* rects,
                 const SkColor* colors) {
    return GrSimpleMeshDrawOpHelper::FactoryHelper<DrawAtlasOpImpl>(context, std::move(paint),
                                                                    viewMatrix, aaType,
                                                                    spriteCount, xforms,
                                                                    rects, colors);
}

}  // namespace skgpu::ganesh::DrawAtlasOp

#if defined(GR_TEST_UTILS)
#include "src/gpu/ganesh/GrDrawOpTest.h"

static SkRSXform random_xform(SkRandom* random) {
    static const SkScalar kMinExtent = -100.f;
    static const SkScalar kMaxExtent = 100.f;
    static const SkScalar kMinScale = 0.1f;
    static const SkScalar kMaxScale = 100.f;
    static const SkScalar kMinRotate = -SK_ScalarPI;
    static const SkScalar kMaxRotate = SK_ScalarPI;

    SkRSXform xform = SkRSXform::MakeFromRadians(random->nextRangeScalar(kMinScale, kMaxScale),
                                                 random->nextRangeScalar(kMinRotate, kMaxRotate),
                                                 random->nextRangeScalar(kMinExtent, kMaxExtent),
                                                 random->nextRangeScalar(kMinExtent, kMaxExtent),
                                                 random->nextRangeScalar(kMinExtent, kMaxExtent),
                                                 random->nextRangeScalar(kMinExtent, kMaxExtent));
    return xform;
}

static SkRect random_texRect(SkRandom* random) {
    static const SkScalar kMinCoord = 0.0f;
    static const SkScalar kMaxCoord = 1024.f;

    SkRect texRect = SkRect::MakeLTRB(random->nextRangeScalar(kMinCoord, kMaxCoord),
                                      random->nextRangeScalar(kMinCoord, kMaxCoord),
                                      random->nextRangeScalar(kMinCoord, kMaxCoord),
                                      random->nextRangeScalar(kMinCoord, kMaxCoord));
    texRect.sort();
    return texRect;
}

static void randomize_params(uint32_t count, SkRandom* random, TArray<SkRSXform>* xforms,
                             TArray<SkRect>* texRects, TArray<GrColor>* colors,
                             bool hasColors) {
    for (uint32_t v = 0; v < count; v++) {
        xforms->push_back(random_xform(random));
        texRects->push_back(random_texRect(random));
        if (hasColors) {
            colors->push_back(GrTest::RandomColor(random));
        }
    }
}

GR_DRAW_OP_TEST_DEFINE(DrawAtlasOp) {
    uint32_t spriteCount = random->nextRangeU(1, 100);

    TArray<SkRSXform> xforms(spriteCount);
    TArray<SkRect> texRects(spriteCount);
    TArray<GrColor> colors;

    bool hasColors = random->nextBool();

    randomize_params(spriteCount, random, &xforms, &texRects, &colors, hasColors);

    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrAAType aaType = GrAAType::kNone;
    if (numSamples > 1 && random->nextBool()) {
        aaType = GrAAType::kMSAA;
    }

    return skgpu::ganesh::DrawAtlasOp::Make(context,
                                            std::move(paint),
                                            viewMatrix,
                                            aaType,
                                            spriteCount,
                                            xforms.begin(),
                                            texRects.begin(),
                                            hasColors ? colors.begin() : nullptr);
}

#endif
