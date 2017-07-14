/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawAtlasOp.h"
#include "GrDrawOpTest.h"
#include "GrOpFlushState.h"
#include "SkGr.h"
#include "SkRSXform.h"
#include "SkRandom.h"

static sk_sp<GrGeometryProcessor> make_gp(bool hasColors,
                                          GrColor color,
                                          const SkMatrix& viewMatrix) {
    using namespace GrDefaultGeoProcFactory;
    Color gpColor(color);
    if (hasColors) {
        gpColor.fType = Color::kPremulGrColorAttribute_Type;
    }

    return GrDefaultGeoProcFactory::Make(gpColor, Coverage::kSolid_Type,
                                         LocalCoords::kHasExplicit_Type, viewMatrix);
}

GrDrawAtlasOp::GrDrawAtlasOp(const Helper::MakeArgs& helperArgs, GrColor color,
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

    SkRect bounds;
    bounds.setLargestInverted();
    int paintAlpha = GrColorUnpackA(installedGeo.fColor);
    for (int spriteIndex = 0; spriteIndex < spriteCount; ++spriteIndex) {
        // Transform rect
        SkPoint quad[4];
        const SkRect& currRect = rects[spriteIndex];
        xforms[spriteIndex].toQuad(currRect.width(), currRect.height(), quad);

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
        *(reinterpret_cast<SkPoint*>(currVertex)) = quad[0];
        *(reinterpret_cast<SkPoint*>(currVertex + texOffset)) =
                SkPoint::Make(currRect.fLeft, currRect.fTop);
        bounds.growToInclude(quad[0].fX, quad[0].fY);
        currVertex += vertexStride;

        *(reinterpret_cast<SkPoint*>(currVertex)) = quad[1];
        *(reinterpret_cast<SkPoint*>(currVertex + texOffset)) =
                SkPoint::Make(currRect.fRight, currRect.fTop);
        bounds.growToInclude(quad[1].fX, quad[1].fY);
        currVertex += vertexStride;

        *(reinterpret_cast<SkPoint*>(currVertex)) = quad[2];
        *(reinterpret_cast<SkPoint*>(currVertex + texOffset)) =
                SkPoint::Make(currRect.fRight, currRect.fBottom);
        bounds.growToInclude(quad[2].fX, quad[2].fY);
        currVertex += vertexStride;

        *(reinterpret_cast<SkPoint*>(currVertex)) = quad[3];
        *(reinterpret_cast<SkPoint*>(currVertex + texOffset)) =
                SkPoint::Make(currRect.fLeft, currRect.fBottom);
        bounds.growToInclude(quad[3].fX, quad[3].fY);
        currVertex += vertexStride;
    }

    this->setTransformedBounds(bounds, viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
}

SkString GrDrawAtlasOp::dumpInfo() const {
    SkString string;
    for (const auto& geo : fGeoData) {
        string.appendf("Color: 0x%08x, Quads: %d\n", geo.fColor, geo.fVerts.count() / 4);
    }
    string += fHelper.dumpInfo();
    string += INHERITED::dumpInfo();
    return string;
}

void GrDrawAtlasOp::onPrepareDraws(Target* target) const {
    // Setup geometry processor
    sk_sp<GrGeometryProcessor> gp(make_gp(this->hasColors(), this->color(), this->viewMatrix()));

    int instanceCount = fGeoData.count();
    size_t vertexStride = gp->getVertexStride();
    SkASSERT(vertexStride ==
             sizeof(SkPoint) + sizeof(SkPoint) + (this->hasColors() ? sizeof(GrColor) : 0));

    QuadHelper helper;
    int numQuads = this->quadCount();
    void* verts = helper.init(target, vertexStride, numQuads);
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
    helper.recordDraw(target, gp.get(), fHelper.makePipeline(target));
}

bool GrDrawAtlasOp::onCombineIfPossible(GrOp* t, const GrCaps& caps) {
    GrDrawAtlasOp* that = t->cast<GrDrawAtlasOp>();

    if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
        return false;
    }

    // We currently use a uniform viewmatrix for this op.
    if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
        return false;
    }

    if (this->hasColors() != that->hasColors()) {
        return false;
    }

    if (!this->hasColors() && this->color() != that->color()) {
        return false;
    }

    fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
    fQuadCount += that->quadCount();

    this->joinBounds(*that);
    return true;
}

GrDrawOp::FixedFunctionFlags GrDrawAtlasOp::fixedFunctionFlags() const {
    return fHelper.fixedFunctionFlags();
}

GrDrawOp::RequiresDstTexture GrDrawAtlasOp::finalize(const GrCaps& caps,
                                                     const GrAppliedClip* clip) {
    GrProcessorAnalysisColor gpColor;
    if (this->hasColors()) {
        gpColor.setToUnknown();
    } else {
        gpColor.setToConstant(fColor);
    }
    auto result =
            fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kNone, &gpColor);
    if (gpColor.isConstant(&fColor)) {
        fHasColors = false;
    }
    return result;
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

GR_DRAW_OP_TEST_DEFINE(GrDrawAtlasOp) {
    uint32_t spriteCount = random->nextRangeU(1, 100);

    SkTArray<SkRSXform> xforms(spriteCount);
    SkTArray<SkRect> texRects(spriteCount);
    SkTArray<GrColor> colors;

    bool hasColors = random->nextBool();

    randomize_params(spriteCount, random, &xforms, &texRects, &colors, hasColors);

    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrAAType aaType = GrAAType::kNone;
    if (GrFSAAType::kUnifiedMSAA == fsaaType && random->nextBool()) {
        aaType = GrAAType::kMSAA;
    }

    return GrDrawAtlasOp::Make(std::move(paint), viewMatrix, aaType, spriteCount, xforms.begin(),
                               texRects.begin(), hasColors ? colors.begin() : nullptr);
}

#endif
