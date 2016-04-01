/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawAtlasBatch.h"
#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "SkGr.h"
#include "SkRandom.h"
#include "SkRSXform.h"

void GrDrawAtlasBatch::initBatchTracker(const GrXPOverridesForBatch& overrides) {
    SkASSERT(fGeoData.count() == 1);
    // Handle any color overrides
    if (!overrides.readsColor()) {
        fGeoData[0].fColor = GrColor_ILLEGAL;
    }
    if (overrides.getOverrideColorIfSet(&fGeoData[0].fColor) && fHasColors) {
        size_t vertexStride = sizeof(SkPoint) + sizeof(SkPoint) +
                             (this->hasColors() ? sizeof(GrColor) : 0);
        uint8_t* currVertex = fGeoData[0].fVerts.begin();
        for (int i = 0; i < 4*fQuadCount; ++i) {
            *(reinterpret_cast<GrColor*>(currVertex + sizeof(SkPoint))) = fGeoData[0].fColor;
            currVertex += vertexStride;
        }
    }

    // setup batch properties
    fColorIgnored = !overrides.readsColor();
    fColor = fGeoData[0].fColor;
    // We'd like to assert this, but we can't because of GLPrograms test
    //SkASSERT(init.readsLocalCoords());
    fCoverageIgnored = !overrides.readsCoverage();
}

static const GrGeometryProcessor* set_vertex_attributes(bool hasColors,
                                                        GrColor color,
                                                        const SkMatrix& viewMatrix,
                                                        bool coverageIgnored) {
    using namespace GrDefaultGeoProcFactory;
    Color gpColor(color);
    if (hasColors) {
        gpColor.fType = Color::kAttribute_Type;
    }

    Coverage coverage(coverageIgnored ? Coverage::kNone_Type : Coverage::kSolid_Type);
    LocalCoords localCoords(LocalCoords::kHasExplicit_Type);
    return GrDefaultGeoProcFactory::Create(gpColor, coverage, localCoords, viewMatrix);
}

void GrDrawAtlasBatch::onPrepareDraws(Target* target) const {
    // Setup geometry processor
    SkAutoTUnref<const GrGeometryProcessor> gp(set_vertex_attributes(this->hasColors(),
                                                                     this->color(),
                                                                     this->viewMatrix(),
                                                                     this->coverageIgnored()));

    int instanceCount = fGeoData.count();
    size_t vertexStride = gp->getVertexStride();
    SkASSERT(vertexStride == sizeof(SkPoint) + sizeof(SkPoint)
             + (this->hasColors() ? sizeof(GrColor) : 0));

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
    helper.recordDraw(target, gp);
}

GrDrawAtlasBatch::GrDrawAtlasBatch(const Geometry& geometry, const SkMatrix& viewMatrix,
                                   int spriteCount, const SkRSXform* xforms, const SkRect* rects,
                                   const SkColor* colors)
    : INHERITED(ClassID()) {
    SkASSERT(xforms);
    SkASSERT(rects);

    fViewMatrix = viewMatrix;
    Geometry& installedGeo = fGeoData.push_back(geometry);

    // Figure out stride and offsets
    // Order within the vertex is: position [color] texCoord
    size_t texOffset = sizeof(SkPoint);
    size_t vertexStride = 2*sizeof(SkPoint);
    fHasColors = SkToBool(colors);
    if (colors) {
        texOffset += sizeof(GrColor);
        vertexStride += sizeof(GrColor);
    }

    // Compute buffer size and alloc buffer
    fQuadCount = spriteCount;
    int allocSize = static_cast<int>(4*vertexStride*spriteCount);
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

            *(reinterpret_cast<GrColor*>(currVertex+sizeof(SkPoint))) = grColor;
            *(reinterpret_cast<GrColor*>(currVertex+vertexStride+sizeof(SkPoint))) = grColor;
            *(reinterpret_cast<GrColor*>(currVertex+2*vertexStride+sizeof(SkPoint))) = grColor;
            *(reinterpret_cast<GrColor*>(currVertex+3*vertexStride+sizeof(SkPoint))) = grColor;
        }

        // Copy position and uv to verts
        *(reinterpret_cast<SkPoint*>(currVertex)) = quad[0];
        *(reinterpret_cast<SkPoint*>(currVertex+texOffset)) = SkPoint::Make(currRect.fLeft,
                                                                            currRect.fTop);
        bounds.growToInclude(quad[0].fX, quad[0].fY);
        currVertex += vertexStride;

        *(reinterpret_cast<SkPoint*>(currVertex)) = quad[1];
        *(reinterpret_cast<SkPoint*>(currVertex+texOffset)) = SkPoint::Make(currRect.fRight,
                                                                            currRect.fTop);
        bounds.growToInclude(quad[1].fX, quad[1].fY);
        currVertex += vertexStride;

        *(reinterpret_cast<SkPoint*>(currVertex)) = quad[2];
        *(reinterpret_cast<SkPoint*>(currVertex+texOffset)) = SkPoint::Make(currRect.fRight,
                                                                            currRect.fBottom);
        bounds.growToInclude(quad[2].fX, quad[2].fY);
        currVertex += vertexStride;

        *(reinterpret_cast<SkPoint*>(currVertex)) = quad[3];
        *(reinterpret_cast<SkPoint*>(currVertex+texOffset)) = SkPoint::Make(currRect.fLeft,
                                                                            currRect.fBottom);
        bounds.growToInclude(quad[3].fX, quad[3].fY);
        currVertex += vertexStride;
    }

    viewMatrix.mapRect(&bounds);
    // Outset for a half pixel in each direction to account for snapping in non-AA case
    bounds.outset(0.5f, 0.5f);
    this->setBounds(bounds);
}

bool GrDrawAtlasBatch::onCombineIfPossible(GrBatch* t, const GrCaps& caps) {
    GrDrawAtlasBatch* that = t->cast<GrDrawAtlasBatch>();

    if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                that->bounds(), caps)) {
        return false;
    }

    // We currently use a uniform viewmatrix for this batch
    if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
        return false;
    }

    if (this->hasColors() != that->hasColors()) {
        return false;
    }

    if (!this->hasColors() && this->color() != that->color()) {
        return false;
    }

    if (this->color() != that->color()) {
        fColor = GrColor_ILLEGAL;
    }
    fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
    fQuadCount += that->quadCount();

    this->joinBounds(that->bounds());
    return true;
}

#ifdef GR_TEST_UTILS

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

static void randomize_params(uint32_t count, SkRandom* random,
                             SkTArray<SkRSXform>* xforms,
                             SkTArray<SkRect>* texRects,
                             SkTArray<GrColor>* colors, bool hasColors) {
    for (uint32_t v = 0; v < count; v++) {
        xforms->push_back(random_xform(random));
        texRects->push_back(random_texRect(random));
        if (hasColors) {
            colors->push_back(GrRandomColor(random));
        }
    }
}

DRAW_BATCH_TEST_DEFINE(GrDrawAtlasBatch) {
    uint32_t spriteCount = random->nextRangeU(1, 100);

    SkTArray<SkRSXform> xforms(spriteCount);
    SkTArray<SkRect> texRects(spriteCount);
    SkTArray<GrColor> colors;

    bool hasColors = random->nextBool();

    randomize_params(spriteCount,
                     random,
                     &xforms,
                     &texRects,
                     &colors, hasColors);

    SkMatrix viewMatrix = GrTest::TestMatrix(random);

    GrDrawAtlasBatch::Geometry geometry;
    geometry.fColor = GrRandomColor(random);
    return GrDrawAtlasBatch::Create(geometry, viewMatrix, spriteCount, xforms.begin(),
                                    texRects.begin(), hasColors ? colors.begin() : nullptr);
}

#endif
