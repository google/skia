/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrDrawAtlasOp.h"

#include "include/core/SkRSXform.h"
#include "include/private/GrRecordingContext.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

namespace {

class DrawAtlasOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    DrawAtlasOp(const Helper::MakeArgs&, const SkPMColor4f& color,
                const SkMatrix& viewMatrix, GrAAType, int spriteCount, const SkRSXform* xforms,
                const SkRect* rects, const SkColor* colors);

    const char* name() const override { return "DrawAtlasOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fHelper.visitProxies(func);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override;
#endif

    FixedFunctionFlags fixedFunctionFlags() const override;

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override;

private:
    void onPrepareDraws(Target*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    const SkPMColor4f& color() const { return fColor; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool hasColors() const { return fHasColors; }
    int quadCount() const { return fQuadCount; }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps&) override;

    struct Geometry {
        SkPMColor4f fColor;
        SkTArray<uint8_t, true> fVerts;
    };

    SkSTArray<1, Geometry, true> fGeoData;
    Helper fHelper;
    SkMatrix fViewMatrix;
    SkPMColor4f fColor;
    int fQuadCount;
    bool fHasColors;

    typedef GrMeshDrawOp INHERITED;
};

static sk_sp<GrGeometryProcessor> make_gp(const GrShaderCaps* shaderCaps,
                                          bool hasColors,
                                          const SkPMColor4f& color,
                                          const SkMatrix& viewMatrix) {
    using namespace GrDefaultGeoProcFactory;
    Color gpColor(color);
    if (hasColors) {
        gpColor.fType = Color::kPremulGrColorAttribute_Type;
    }

    return GrDefaultGeoProcFactory::Make(shaderCaps, gpColor, Coverage::kSolid_Type,
                                         LocalCoords::kHasExplicit_Type, viewMatrix);
}

DrawAtlasOp::DrawAtlasOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color,
                         const SkMatrix& viewMatrix, GrAAType aaType, int spriteCount,
                         const SkRSXform* xforms, const SkRect* rects, const SkColor* colors)
        : INHERITED(ClassID()), fHelper(helperArgs, aaType), fColor(color) {
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
            SkColor color = colors[spriteIndex];
            if (paintAlpha != 255) {
                color = SkColorSetA(color, SkMulDiv255Round(SkColorGetA(color), paintAlpha));
            }
            GrColor grColor = SkColorToPremulGrColor(color);

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

    this->setTransformedBounds(bounds, viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
}

#ifdef SK_DEBUG
SkString DrawAtlasOp::dumpInfo() const {
    SkString string;
    for (const auto& geo : fGeoData) {
        string.appendf("Color: 0x%08x, Quads: %d\n", geo.fColor.toBytes_RGBA(),
                       geo.fVerts.count() / 4);
    }
    string += fHelper.dumpInfo();
    string += INHERITED::dumpInfo();
    return string;
}
#endif

void DrawAtlasOp::onPrepareDraws(Target* target) {
    // Setup geometry processor
    sk_sp<GrGeometryProcessor> gp(make_gp(target->caps().shaderCaps(),
                                          this->hasColors(),
                                          this->color(),
                                          this->viewMatrix()));

    int instanceCount = fGeoData.count();
    size_t vertexStride = gp->vertexStride();

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

        size_t allocSize = args.fVerts.count();
        memcpy(vertPtr, args.fVerts.begin(), allocSize);
        vertPtr += allocSize;
    }
    helper.recordDraw(target, std::move(gp));
}

void DrawAtlasOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    fHelper.executeDrawsAndUploads(this, flushState, chainBounds);
}

GrOp::CombineResult DrawAtlasOp::onCombineIfPossible(GrOp* t, const GrCaps& caps) {
    DrawAtlasOp* that = t->cast<DrawAtlasOp>();

    if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
        return CombineResult::kCannotCombine;
    }

    // We currently use a uniform viewmatrix for this op.
    if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
        return CombineResult::kCannotCombine;
    }

    if (this->hasColors() != that->hasColors()) {
        return CombineResult::kCannotCombine;
    }

    if (!this->hasColors() && this->color() != that->color()) {
        return CombineResult::kCannotCombine;
    }

    fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
    fQuadCount += that->quadCount();

    return CombineResult::kMerged;
}

GrDrawOp::FixedFunctionFlags DrawAtlasOp::fixedFunctionFlags() const {
    return fHelper.fixedFunctionFlags();
}

GrProcessorSet::Analysis DrawAtlasOp::finalize(
        const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
        GrClampType clampType) {
    GrProcessorAnalysisColor gpColor;
    if (this->hasColors()) {
        gpColor.setToUnknown();
    } else {
        gpColor.setToConstant(fColor);
    }
    auto result = fHelper.finalizeProcessors(caps, clip, hasMixedSampledCoverage, clampType,
                                             GrProcessorAnalysisCoverage::kNone, &gpColor);
    if (gpColor.isConstant(&fColor)) {
        fHasColors = false;
    }
    return result;
}

} // anonymous namespace

std::unique_ptr<GrDrawOp> GrDrawAtlasOp::Make(GrRecordingContext* context,
                                              GrPaint&& paint,
                                              const SkMatrix& viewMatrix,
                                              GrAAType aaType,
                                              int spriteCount,
                                              const SkRSXform* xforms,
                                              const SkRect* rects,
                                              const SkColor* colors) {
    return GrSimpleMeshDrawOpHelper::FactoryHelper<DrawAtlasOp>(context, std::move(paint),
                                                                viewMatrix, aaType,
                                                                spriteCount, xforms,
                                                                rects, colors);
}

#if GR_TEST_UTILS

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

static void randomize_params(uint32_t count, SkRandom* random, SkTArray<SkRSXform>* xforms,
                             SkTArray<SkRect>* texRects, SkTArray<GrColor>* colors,
                             bool hasColors) {
    for (uint32_t v = 0; v < count; v++) {
        xforms->push_back(random_xform(random));
        texRects->push_back(random_texRect(random));
        if (hasColors) {
            colors->push_back(GrRandomColor(random));
        }
    }
}

GR_DRAW_OP_TEST_DEFINE(DrawAtlasOp) {
    uint32_t spriteCount = random->nextRangeU(1, 100);

    SkTArray<SkRSXform> xforms(spriteCount);
    SkTArray<SkRect> texRects(spriteCount);
    SkTArray<GrColor> colors;

    bool hasColors = random->nextBool();

    randomize_params(spriteCount, random, &xforms, &texRects, &colors, hasColors);

    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrAAType aaType = GrAAType::kNone;
    if (numSamples > 1 && random->nextBool()) {
        aaType = GrAAType::kMSAA;
    }

    return GrDrawAtlasOp::Make(context, std::move(paint), viewMatrix, aaType, spriteCount,
                               xforms.begin(), texRects.begin(),
                               hasColors ? colors.begin() : nullptr);
}

#endif
