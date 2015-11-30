
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAADistanceFieldPathRenderer.h"

#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "GrContext.h"
#include "GrPipelineBuilder.h"
#include "GrResourceProvider.h"
#include "GrSurfacePriv.h"
#include "GrSWMaskHelper.h"
#include "GrTexturePriv.h"
#include "GrVertexBuffer.h"
#include "batches/GrVertexBatch.h"
#include "effects/GrDistanceFieldGeoProc.h"

#include "SkDistanceFieldGen.h"
#include "SkRTConf.h"

#define ATLAS_TEXTURE_WIDTH 2048
#define ATLAS_TEXTURE_HEIGHT 2048
#define PLOT_WIDTH  512
#define PLOT_HEIGHT 256

#define NUM_PLOTS_X   (ATLAS_TEXTURE_WIDTH / PLOT_WIDTH)
#define NUM_PLOTS_Y   (ATLAS_TEXTURE_HEIGHT / PLOT_HEIGHT)

#ifdef DF_PATH_TRACKING
static int g_NumCachedPaths = 0;
static int g_NumFreedPaths = 0;
#endif

// mip levels
static const int kSmallMIP = 32;
static const int kMediumMIP = 73;
static const int kLargeMIP = 162;

// Callback to clear out internal path cache when eviction occurs
void GrAADistanceFieldPathRenderer::HandleEviction(GrBatchAtlas::AtlasID id, void* pr) {
    GrAADistanceFieldPathRenderer* dfpr = (GrAADistanceFieldPathRenderer*)pr;
    // remove any paths that use this plot
    PathDataList::Iter iter;
    iter.init(dfpr->fPathList, PathDataList::Iter::kHead_IterStart);
    PathData* pathData;
    while ((pathData = iter.get())) {
        iter.next();
        if (id == pathData->fID) {
            dfpr->fPathCache.remove(pathData->fKey);
            dfpr->fPathList.remove(pathData);
            delete pathData;
#ifdef DF_PATH_TRACKING
            ++g_NumFreedPaths;
#endif
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
GrAADistanceFieldPathRenderer::GrAADistanceFieldPathRenderer() : fAtlas(nullptr) {}

GrAADistanceFieldPathRenderer::~GrAADistanceFieldPathRenderer() {
    PathDataList::Iter iter;
    iter.init(fPathList, PathDataList::Iter::kHead_IterStart);
    PathData* pathData;
    while ((pathData = iter.get())) {
        iter.next();
        fPathList.remove(pathData);
        delete pathData;
    }
    delete fAtlas;

#ifdef DF_PATH_TRACKING
    SkDebugf("Cached paths: %d, freed paths: %d\n", g_NumCachedPaths, g_NumFreedPaths);
#endif
}

////////////////////////////////////////////////////////////////////////////////
bool GrAADistanceFieldPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {

    // TODO: Support inverse fill
    if (!args.fShaderCaps->shaderDerivativeSupport() || !args.fAntiAlias ||
        SkStrokeRec::kHairline_Style == args.fStroke->getStyle() ||
        args.fPath->isInverseFillType() || args.fPath->isVolatile()) {
        return false;
    }

    // currently don't support perspective
    if (args.fViewMatrix->hasPerspective()) {
        return false;
    }
    
    // only support paths with bounds within kMediumMIP by kMediumMIP,
    // scaled to have bounds within 2.0f*kLargeMIP by 2.0f*kLargeMIP
    // the goal is to accelerate rendering of lots of small paths that may be scaling
    SkScalar maxScale = args.fViewMatrix->getMaxScale();
    const SkRect& bounds = args.fPath->getBounds();
    SkScalar maxDim = SkMaxScalar(bounds.width(), bounds.height());
    // Approximate stroked size by adding the maximum of the stroke width or 2x the miter limit
    if (!args.fStroke->isFillStyle()) {
        SkScalar extraWidth = args.fStroke->getWidth();
        if (SkPaint::kMiter_Join == args.fStroke->getJoin()) {
            extraWidth = SkTMax(extraWidth, 2.0f*args.fStroke->getMiter());
        }
        maxDim += extraWidth;
    }
    
    return maxDim <= kMediumMIP && maxDim * maxScale <= 2.0f*kLargeMIP;
}

////////////////////////////////////////////////////////////////////////////////

// padding around path bounds to allow for antialiased pixels
static const SkScalar kAntiAliasPad = 1.0f;

class AADistanceFieldPathBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    typedef GrAADistanceFieldPathRenderer::PathData PathData;
    typedef SkTDynamicHash<PathData, PathData::Key> PathCache;
    typedef GrAADistanceFieldPathRenderer::PathDataList PathDataList;

    struct Geometry {
        Geometry(const SkStrokeRec& stroke) : fStroke(stroke) {
            if (!stroke.needToApply()) {
                // purify unused values to ensure binary equality
                fStroke.setStrokeParams(SkPaint::kDefault_Cap, SkPaint::kDefault_Join,
                                        SkIntToScalar(4));
                if (fStroke.getWidth() < 0) {
                    fStroke.setStrokeStyle(-1.0f);
                }
            }
        }
        SkPath fPath;
        // The unique ID of the path involved in this draw. This may be different than the ID
        // in fPath since that path may have resulted from a SkStrokeRec::applyToPath call.
        uint32_t fGenID;
        SkStrokeRec fStroke;
        bool fAntiAlias;
    };

    static GrDrawBatch* Create(const Geometry& geometry, GrColor color, const SkMatrix& viewMatrix,
                               GrBatchAtlas* atlas, PathCache* pathCache, PathDataList* pathList) {
        return new AADistanceFieldPathBatch(geometry, color, viewMatrix, atlas, pathCache,
                                            pathList);
    }

    const char* name() const override { return "AADistanceFieldPathBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color, 
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        color->setKnownFourComponents(fBatch.fColor);
        coverage->setUnknownSingleComponent();
        overrides->fUsePLSDstRead = false;
    }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any color overrides
        if (!overrides.readsColor()) {
            fBatch.fColor = GrColor_ILLEGAL;
        }
        overrides.getOverrideColorIfSet(&fBatch.fColor);

        // setup batch properties
        fBatch.fColorIgnored = !overrides.readsColor();
        fBatch.fUsesLocalCoords = overrides.readsLocalCoords();
        fBatch.fCoverageIgnored = !overrides.readsCoverage();
    }

    struct FlushInfo {
        SkAutoTUnref<const GrVertexBuffer> fVertexBuffer;
        SkAutoTUnref<const GrIndexBuffer>  fIndexBuffer;
        int fVertexOffset;
        int fInstancesToFlush;
    };

    void onPrepareDraws(Target* target) const override {
        int instanceCount = fGeoData.count();

        SkMatrix invert;
        if (this->usesLocalCoords() && !this->viewMatrix().invert(&invert)) {
            SkDebugf("Could not invert viewmatrix\n");
            return;
        }

        uint32_t flags = 0;
        flags |= this->viewMatrix().isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;

        GrTextureParams params(SkShader::kRepeat_TileMode, GrTextureParams::kBilerp_FilterMode);

        // Setup GrGeometryProcessor
        GrBatchAtlas* atlas = fAtlas;
        SkAutoTUnref<GrGeometryProcessor> dfProcessor(
                GrDistanceFieldPathGeoProc::Create(this->color(),
                                                   this->viewMatrix(),
                                                   atlas->getTexture(),
                                                   params,
                                                   flags,
                                                   this->usesLocalCoords()));

        target->initDraw(dfProcessor, this->pipeline());

        FlushInfo flushInfo;

        // allocate vertices
        size_t vertexStride = dfProcessor->getVertexStride();
        SkASSERT(vertexStride == 2 * sizeof(SkPoint));

        const GrVertexBuffer* vertexBuffer;
        void* vertices = target->makeVertexSpace(vertexStride,
                                                 kVerticesPerQuad * instanceCount,
                                                 &vertexBuffer,
                                                 &flushInfo.fVertexOffset);
        flushInfo.fVertexBuffer.reset(SkRef(vertexBuffer));
        flushInfo.fIndexBuffer.reset(target->resourceProvider()->refQuadIndexBuffer());
        if (!vertices || !flushInfo.fIndexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        flushInfo.fInstancesToFlush = 0;
        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];

            // get mip level
            SkScalar maxScale = this->viewMatrix().getMaxScale();
            const SkRect& bounds = args.fPath.getBounds();
            SkScalar maxDim = SkMaxScalar(bounds.width(), bounds.height());
            SkScalar size = maxScale * maxDim;
            uint32_t desiredDimension;
            if (size <= kSmallMIP) {
                desiredDimension = kSmallMIP;
            } else if (size <= kMediumMIP) {
                desiredDimension = kMediumMIP;
            } else {
                desiredDimension = kLargeMIP;
            }

            // check to see if path is cached
            PathData::Key key(args.fGenID, desiredDimension, args.fStroke);
            PathData* pathData = fPathCache->find(key);
            if (nullptr == pathData || !atlas->hasID(pathData->fID)) {
                // Remove the stale cache entry
                if (pathData) {
                    fPathCache->remove(pathData->fKey);
                    fPathList->remove(pathData);
                    delete pathData;
                }
                SkScalar scale = desiredDimension/maxDim;
                pathData = new PathData;
                if (!this->addPathToAtlas(target,
                                          dfProcessor,
                                          this->pipeline(),
                                          &flushInfo,
                                          atlas,
                                          pathData,
                                          args.fPath,
                                          args.fGenID,
                                          args.fStroke,
                                          args.fAntiAlias,
                                          desiredDimension,
                                          scale)) {
                    SkDebugf("Can't rasterize path\n");
                    return;
                }
            }

            atlas->setLastUseToken(pathData->fID, target->currentToken());

            // Now set vertices
            intptr_t offset = reinterpret_cast<intptr_t>(vertices);
            offset += i * kVerticesPerQuad * vertexStride;
            SkPoint* positions = reinterpret_cast<SkPoint*>(offset);
            this->writePathVertices(target,
                                    atlas,
                                    this->pipeline(),
                                    dfProcessor,
                                    positions,
                                    vertexStride,
                                    this->viewMatrix(),
                                    args.fPath,
                                    pathData);
            flushInfo.fInstancesToFlush++;
        }

        this->flush(target, &flushInfo);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

    AADistanceFieldPathBatch(const Geometry& geometry, GrColor color, const SkMatrix& viewMatrix,
                             GrBatchAtlas* atlas,
                             PathCache* pathCache, PathDataList* pathList)
        : INHERITED(ClassID()) {
        fBatch.fColor = color;
        fBatch.fViewMatrix = viewMatrix;
        fGeoData.push_back(geometry);

        fAtlas = atlas;
        fPathCache = pathCache;
        fPathList = pathList;

        // Compute bounds
        fBounds = geometry.fPath.getBounds();
        viewMatrix.mapRect(&fBounds);
    }

    bool addPathToAtlas(GrVertexBatch::Target* target,
                        const GrGeometryProcessor* dfProcessor,
                        const GrPipeline* pipeline,
                        FlushInfo* flushInfo,
                        GrBatchAtlas* atlas,
                        PathData* pathData,
                        const SkPath& path,
                        uint32_t genID,
                        const SkStrokeRec& stroke,
                        bool antiAlias,
                        uint32_t dimension,
                        SkScalar scale) const {
        const SkRect& bounds = path.getBounds();

        // generate bounding rect for bitmap draw
        SkRect scaledBounds = bounds;
        // scale to mip level size
        scaledBounds.fLeft *= scale;
        scaledBounds.fTop *= scale;
        scaledBounds.fRight *= scale;
        scaledBounds.fBottom *= scale;
        // move the origin to an integer boundary (gives better results)
        SkScalar dx = SkScalarFraction(scaledBounds.fLeft);
        SkScalar dy = SkScalarFraction(scaledBounds.fTop);
        scaledBounds.offset(-dx, -dy);
        // get integer boundary
        SkIRect devPathBounds;
        scaledBounds.roundOut(&devPathBounds);
        // pad to allow room for antialiasing
        devPathBounds.outset(SkScalarCeilToInt(kAntiAliasPad), SkScalarCeilToInt(kAntiAliasPad));
        // move origin to upper left corner
        devPathBounds.offsetTo(0,0);

        // draw path to bitmap
        SkMatrix drawMatrix;
        drawMatrix.setTranslate(-bounds.left(), -bounds.top());
        drawMatrix.postScale(scale, scale);
        drawMatrix.postTranslate(kAntiAliasPad, kAntiAliasPad);

        // setup bitmap backing
        SkASSERT(devPathBounds.fLeft == 0);
        SkASSERT(devPathBounds.fTop == 0);
        SkAutoPixmapStorage dst;
        if (!dst.tryAlloc(SkImageInfo::MakeA8(devPathBounds.width(),
                                              devPathBounds.height()))) {
            return false;
        }
        sk_bzero(dst.writable_addr(), dst.getSafeSize());

        // rasterize path
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(antiAlias);

        SkDraw draw;
        sk_bzero(&draw, sizeof(draw));

        SkRasterClip rasterClip;
        rasterClip.setRect(devPathBounds);
        draw.fRC = &rasterClip;
        draw.fClip = &rasterClip.bwRgn();
        draw.fMatrix = &drawMatrix;
        draw.fDst = dst;

        draw.drawPathCoverage(path, paint);

        // generate signed distance field
        devPathBounds.outset(SK_DistanceFieldPad, SK_DistanceFieldPad);
        int width = devPathBounds.width();
        int height = devPathBounds.height();
        // TODO We should really generate this directly into the plot somehow
        SkAutoSMalloc<1024> dfStorage(width * height * sizeof(unsigned char));

        // Generate signed distance field
        SkGenerateDistanceFieldFromA8Image((unsigned char*)dfStorage.get(),
                                           (const unsigned char*)dst.addr(),
                                           dst.width(), dst.height(), dst.rowBytes());

        // add to atlas
        SkIPoint16 atlasLocation;
        GrBatchAtlas::AtlasID id;
        bool success = atlas->addToAtlas(&id, target, width, height, dfStorage.get(),
                                         &atlasLocation);
        if (!success) {
            this->flush(target, flushInfo);
            target->initDraw(dfProcessor, pipeline);

            SkDEBUGCODE(success =) atlas->addToAtlas(&id, target, width, height,
                                                     dfStorage.get(), &atlasLocation);
            SkASSERT(success);

        }

        // add to cache
        pathData->fKey = PathData::Key(genID, dimension, stroke);
        pathData->fScale = scale;
        pathData->fID = id;
        // change the scaled rect to match the size of the inset distance field
        scaledBounds.fRight = scaledBounds.fLeft +
            SkIntToScalar(devPathBounds.width() - 2*SK_DistanceFieldInset);
        scaledBounds.fBottom = scaledBounds.fTop +
            SkIntToScalar(devPathBounds.height() - 2*SK_DistanceFieldInset);
        // shift the origin to the correct place relative to the distance field
        // need to also restore the fractional translation
        scaledBounds.offset(-SkIntToScalar(SK_DistanceFieldInset) - kAntiAliasPad + dx,
                            -SkIntToScalar(SK_DistanceFieldInset) - kAntiAliasPad + dy);
        pathData->fBounds = scaledBounds;
        // origin we render from is inset from distance field edge
        atlasLocation.fX += SK_DistanceFieldInset;
        atlasLocation.fY += SK_DistanceFieldInset;
        pathData->fAtlasLocation = atlasLocation;

        fPathCache->add(pathData);
        fPathList->addToTail(pathData);
#ifdef DF_PATH_TRACKING
        ++g_NumCachedPaths;
#endif
        return true;
    }

    void writePathVertices(GrDrawBatch::Target* target,
                           GrBatchAtlas* atlas,
                           const GrPipeline* pipeline,
                           const GrGeometryProcessor* gp,
                           SkPoint* positions,
                           size_t vertexStride,
                           const SkMatrix& viewMatrix,
                           const SkPath& path,
                           const PathData* pathData) const {
        GrTexture* texture = atlas->getTexture();

        SkScalar dx = pathData->fBounds.fLeft;
        SkScalar dy = pathData->fBounds.fTop;
        SkScalar width = pathData->fBounds.width();
        SkScalar height = pathData->fBounds.height();

        SkScalar invScale = 1.0f / pathData->fScale;
        dx *= invScale;
        dy *= invScale;
        width *= invScale;
        height *= invScale;

        SkFixed tx = SkIntToFixed(pathData->fAtlasLocation.fX);
        SkFixed ty = SkIntToFixed(pathData->fAtlasLocation.fY);
        SkFixed tw = SkScalarToFixed(pathData->fBounds.width());
        SkFixed th = SkScalarToFixed(pathData->fBounds.height());

        // vertex positions
        // TODO make the vertex attributes a struct
        SkRect r = SkRect::MakeXYWH(dx, dy, width, height);
        positions->setRectFan(r.left(), r.top(), r.right(), r.bottom(), vertexStride);

        // vertex texture coords
        SkPoint* textureCoords = positions + 1;
        textureCoords->setRectFan(SkFixedToFloat(texture->texturePriv().normalizeFixedX(tx)),
                                  SkFixedToFloat(texture->texturePriv().normalizeFixedY(ty)),
                                  SkFixedToFloat(texture->texturePriv().normalizeFixedX(tx + tw)),
                                  SkFixedToFloat(texture->texturePriv().normalizeFixedY(ty + th)),
                                  vertexStride);
    }

    void flush(GrVertexBatch::Target* target, FlushInfo* flushInfo) const {
        GrVertices vertices;
        int maxInstancesPerDraw = flushInfo->fIndexBuffer->maxQuads();
        vertices.initInstanced(kTriangles_GrPrimitiveType, flushInfo->fVertexBuffer,
            flushInfo->fIndexBuffer, flushInfo->fVertexOffset, kVerticesPerQuad,
            kIndicesPerQuad, flushInfo->fInstancesToFlush, maxInstancesPerDraw);
        target->draw(vertices);
        flushInfo->fVertexOffset += kVerticesPerQuad * flushInfo->fInstancesToFlush;
        flushInfo->fInstancesToFlush = 0;
    }

    GrColor color() const { return fBatch.fColor; }
    const SkMatrix& viewMatrix() const { return fBatch.fViewMatrix; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        AADistanceFieldPathBatch* that = t->cast<AADistanceFieldPathBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        // TODO we could actually probably do a bunch of this work on the CPU, ie map viewMatrix,
        // maybe upload color via attribute
        if (this->color() != that->color()) {
            return false;
        }

        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        this->joinBounds(that->bounds());
        return true;
    }

    struct BatchTracker {
        GrColor fColor;
        SkMatrix fViewMatrix;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
    GrBatchAtlas* fAtlas;
    PathCache* fPathCache;
    PathDataList* fPathList;

    typedef GrVertexBatch INHERITED;
};

bool GrAADistanceFieldPathRenderer::onDrawPath(const DrawPathArgs& args) {
    // we've already bailed on inverse filled paths, so this is safe
    if (args.fPath->isEmpty()) {
        return true;
    }

    if (!fAtlas) {
        fAtlas = args.fResourceProvider->createAtlas(kAlpha_8_GrPixelConfig,
                                                     ATLAS_TEXTURE_WIDTH, ATLAS_TEXTURE_HEIGHT,
                                                     NUM_PLOTS_X, NUM_PLOTS_Y,
                                                     &GrAADistanceFieldPathRenderer::HandleEviction,
                                                     (void*)this);
        if (!fAtlas) {
            return false;
        }
    }

    AADistanceFieldPathBatch::Geometry geometry(*args.fStroke);
    if (SkStrokeRec::kFill_Style == args.fStroke->getStyle()) {
        geometry.fPath = *args.fPath;
    } else {
        args.fStroke->applyToPath(&geometry.fPath, *args.fPath);
    }
    geometry.fAntiAlias = args.fAntiAlias;
    // Note: this is the generation ID of the _original_ path. When a new path is
    // generated due to stroking it is important that the original path's id is used
    // for caching.
    geometry.fGenID = args.fPath->getGenerationID();
 
    SkAutoTUnref<GrDrawBatch> batch(AADistanceFieldPathBatch::Create(geometry, args.fColor,
                                                                     *args.fViewMatrix, fAtlas,
                                                                     &fPathCache, &fPathList));
    args.fTarget->drawBatch(*args.fPipelineBuilder, batch);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

struct PathTestStruct {
    typedef GrAADistanceFieldPathRenderer::PathCache PathCache;
    typedef GrAADistanceFieldPathRenderer::PathData PathData;
    typedef GrAADistanceFieldPathRenderer::PathDataList PathDataList;
    PathTestStruct() : fContextID(SK_InvalidGenID), fAtlas(nullptr) {}
    ~PathTestStruct() { this->reset(); }

    void reset() {
        PathDataList::Iter iter;
        iter.init(fPathList, PathDataList::Iter::kHead_IterStart);
        PathData* pathData;
        while ((pathData = iter.get())) {
            iter.next();
            fPathList.remove(pathData);
            delete pathData;
        }
        delete fAtlas;
        fPathCache.reset();
    }

    static void HandleEviction(GrBatchAtlas::AtlasID id, void* pr) {
        PathTestStruct* dfpr = (PathTestStruct*)pr;
        // remove any paths that use this plot
        PathDataList::Iter iter;
        iter.init(dfpr->fPathList, PathDataList::Iter::kHead_IterStart);
        PathData* pathData;
        while ((pathData = iter.get())) {
            iter.next();
            if (id == pathData->fID) {
                dfpr->fPathCache.remove(pathData->fKey);
                dfpr->fPathList.remove(pathData);
                delete pathData;
            }
        }
    }

    uint32_t fContextID;
    GrBatchAtlas* fAtlas;
    PathCache fPathCache;
    PathDataList fPathList;
};

DRAW_BATCH_TEST_DEFINE(AADistanceFieldPathBatch) {
    static PathTestStruct gTestStruct;

    if (context->uniqueID() != gTestStruct.fContextID) {
        gTestStruct.fContextID = context->uniqueID();
        gTestStruct.reset();
        gTestStruct.fAtlas =
                context->resourceProvider()->createAtlas(kAlpha_8_GrPixelConfig,
                                                     ATLAS_TEXTURE_WIDTH, ATLAS_TEXTURE_HEIGHT,
                                                     NUM_PLOTS_X, NUM_PLOTS_Y,
                                                     &PathTestStruct::HandleEviction,
                                                     (void*)&gTestStruct);
    }

    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrColor color = GrRandomColor(random);

    AADistanceFieldPathBatch::Geometry geometry(GrTest::TestStrokeRec(random));
    geometry.fPath = GrTest::TestPath(random);
    geometry.fAntiAlias = random->nextBool();
    geometry.fGenID = random->nextU();

    return AADistanceFieldPathBatch::Create(geometry, color, viewMatrix,
                                            gTestStruct.fAtlas,
                                            &gTestStruct.fPathCache,
                                            &gTestStruct.fPathList);
}

#endif
