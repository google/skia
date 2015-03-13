
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAADistanceFieldPathRenderer.h"

#include "GrBatch.h"
#include "GrBatchTarget.h"
#include "GrBufferAllocPool.h"
#include "GrContext.h"
#include "GrPipelineBuilder.h"
#include "GrSurfacePriv.h"
#include "GrSWMaskHelper.h"
#include "GrTexturePriv.h"
#include "effects/GrDistanceFieldTextureEffect.h"

#include "SkDistanceFieldGen.h"
#include "SkRTConf.h"

#define ATLAS_TEXTURE_WIDTH 1024
#define ATLAS_TEXTURE_HEIGHT 2048
#define PLOT_WIDTH  256
#define PLOT_HEIGHT 256

#define NUM_PLOTS_X   (ATLAS_TEXTURE_WIDTH / PLOT_WIDTH)
#define NUM_PLOTS_Y   (ATLAS_TEXTURE_HEIGHT / PLOT_HEIGHT)

#ifdef DF_PATH_TRACKING
static int g_NumCachedPaths = 0;
static int g_NumFreedPaths = 0;
#endif

// mip levels
static const int kSmallMIP = 32;
static const int kMediumMIP = 78;
static const int kLargeMIP = 192;

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
            SkDELETE(pathData);
#ifdef DF_PATH_TRACKING
            ++g_NumFreedPaths;
#endif
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
GrAADistanceFieldPathRenderer::GrAADistanceFieldPathRenderer(GrContext* context)
    : fContext(context)
    , fAtlas(NULL) {
}

GrAADistanceFieldPathRenderer::~GrAADistanceFieldPathRenderer() {
    PathDataList::Iter iter;
    iter.init(fPathList, PathDataList::Iter::kHead_IterStart);
    PathData* pathData;
    while ((pathData = iter.get())) {
        iter.next();
        fPathList.remove(pathData);
        SkDELETE(pathData);
    }
    SkDELETE(fAtlas);

#ifdef DF_PATH_TRACKING
    SkDebugf("Cached paths: %d, freed paths: %d\n", g_NumCachedPaths, g_NumFreedPaths);
#endif
}

////////////////////////////////////////////////////////////////////////////////
bool GrAADistanceFieldPathRenderer::canDrawPath(const GrDrawTarget* target,
                                                const GrPipelineBuilder* pipelineBuilder,
                                                const SkMatrix& viewMatrix,
                                                const SkPath& path,
                                                const SkStrokeRec& stroke,
                                                bool antiAlias) const {
    
    // TODO: Support inverse fill
    // TODO: Support strokes
    if (!target->caps()->shaderDerivativeSupport() || !antiAlias || path.isInverseFillType()
        || path.isVolatile() || SkStrokeRec::kFill_Style != stroke.getStyle()) {
        return false;
    }

    // currently don't support perspective
    if (viewMatrix.hasPerspective()) {
        return false;
    }
    
    // only support paths smaller than 64x64, scaled to less than 256x256
    // the goal is to accelerate rendering of lots of small paths that may be scaling
    SkScalar maxScale = viewMatrix.getMaxScale();
    const SkRect& bounds = path.getBounds();
    SkScalar maxDim = SkMaxScalar(bounds.width(), bounds.height());
    return maxDim < 64.f && maxDim * maxScale < 256.f;
}


GrPathRenderer::StencilSupport
GrAADistanceFieldPathRenderer::onGetStencilSupport(const GrDrawTarget*,
                                                   const GrPipelineBuilder*,
                                                   const SkPath&,
                                                   const SkStrokeRec&) const {
    return GrPathRenderer::kNoSupport_StencilSupport;
}

////////////////////////////////////////////////////////////////////////////////

// padding around path bounds to allow for antialiased pixels
static const SkScalar kAntiAliasPad = 1.0f;

class AADistanceFieldPathBatch : public GrBatch {
public:
    typedef GrAADistanceFieldPathRenderer::PathData PathData;
    typedef SkTDynamicHash<PathData, PathData::Key> PathCache;
    typedef GrAADistanceFieldPathRenderer::PathDataList PathDataList;

    struct Geometry {
        Geometry(const SkStrokeRec& stroke) : fStroke(stroke) {}
        SkPath fPath;
        SkStrokeRec fStroke;
        bool fAntiAlias;
        PathData* fPathData;
    };

    static GrBatch* Create(const Geometry& geometry, GrColor color, const SkMatrix& viewMatrix,
                           GrBatchAtlas* atlas, PathCache* pathCache, PathDataList* pathList) {
        return SkNEW_ARGS(AADistanceFieldPathBatch, (geometry, color, viewMatrix,
                                                     atlas, pathCache, pathList));
    }

    const char* name() const SK_OVERRIDE { return "AADistanceFieldPathBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setKnownFourComponents(fBatch.fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineInfo& init) SK_OVERRIDE {
        // Handle any color overrides
        if (init.fColorIgnored) {
            fBatch.fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            fBatch.fColor = init.fOverrideColor;
        }

        // setup batch properties
        fBatch.fColorIgnored = init.fColorIgnored;
        fBatch.fUsesLocalCoords = init.fUsesLocalCoords;
        fBatch.fCoverageIgnored = init.fCoverageIgnored;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) SK_OVERRIDE {
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
                GrDistanceFieldNoGammaTextureEffect::Create(this->color(),
                                                            this->viewMatrix(),
                                                            atlas->getTexture(),
                                                            params,
                                                            flags,
                                                            false));

        this->initDraw(batchTarget, dfProcessor, pipeline);

        // allocate vertices
        size_t vertexStride = dfProcessor->getVertexStride();
        SkASSERT(vertexStride == 2 * sizeof(SkPoint));

        int vertexCount = GrBatchTarget::kVertsPerRect * instanceCount;

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void* vertices = batchTarget->vertexPool()->makeSpace(vertexStride,
                                                              vertexCount,
                                                              &vertexBuffer,
                                                              &firstVertex);

        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        // We may have to flush while uploading path data to the atlas, so we set up the draw here
        const GrIndexBuffer* quadIndexBuffer = batchTarget->quadIndexBuffer();
        int maxInstancesPerDraw = quadIndexBuffer->maxQuads();

        GrDrawTarget::DrawInfo drawInfo;
        drawInfo.setPrimitiveType(kTriangles_GrPrimitiveType);
        drawInfo.setStartVertex(0);
        drawInfo.setStartIndex(0);
        drawInfo.setVerticesPerInstance(GrBatchTarget::kVertsPerRect);
        drawInfo.setIndicesPerInstance(GrBatchTarget::kIndicesPerRect);
        drawInfo.adjustStartVertex(firstVertex);
        drawInfo.setVertexBuffer(vertexBuffer);
        drawInfo.setIndexBuffer(quadIndexBuffer);

        int instancesToFlush = 0;
        for (int i = 0; i < instanceCount; i++) {
            Geometry& args = fGeoData[i];

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
            // TODO: handle stroked vs. filled version of same path
            PathData::Key key = { args.fPath.getGenerationID(), desiredDimension };
            args.fPathData = fPathCache->find(key);
            if (NULL == args.fPathData || !atlas->hasID(args.fPathData->fID)) {
                // Remove the stale cache entry
                if (args.fPathData) {
                    fPathCache->remove(args.fPathData->fKey);
                    fPathList->remove(args.fPathData);
                    SkDELETE(args.fPathData);
                }
                SkScalar scale = desiredDimension/maxDim;
                args.fPathData = SkNEW(PathData);
                if (!this->addPathToAtlas(batchTarget,
                                          dfProcessor,
                                          pipeline,
                                          &drawInfo,
                                          &instancesToFlush,
                                          maxInstancesPerDraw,
                                          atlas,
                                          args.fPathData,
                                          args.fPath,
                                          args.fStroke,
                                          args.fAntiAlias,
                                          desiredDimension,
                                          scale)) {
                    SkDebugf("Can't rasterize path\n");
                    return;
                }
            }

            atlas->setLastRefToken(args.fPathData->fID, batchTarget->currentToken());

            // Now set vertices
            intptr_t offset = reinterpret_cast<intptr_t>(vertices);
            offset += i * GrBatchTarget::kVertsPerRect * vertexStride;
            SkPoint* positions = reinterpret_cast<SkPoint*>(offset);
            this->drawPath(batchTarget,
                           atlas,
                           pipeline,
                           dfProcessor,
                           positions,
                           vertexStride,
                           this->viewMatrix(),
                           args.fPath,
                           args.fPathData);
            instancesToFlush++;
        }

        this->flush(batchTarget, dfProcessor, pipeline, &drawInfo, instancesToFlush,
                    maxInstancesPerDraw);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    AADistanceFieldPathBatch(const Geometry& geometry, GrColor color, const SkMatrix& viewMatrix,
                             GrBatchAtlas* atlas,
                             PathCache* pathCache, PathDataList* pathList) {
        this->initClassID<AADistanceFieldPathBatch>();
        fBatch.fColor = color;
        fBatch.fViewMatrix = viewMatrix;
        fGeoData.push_back(geometry);
        fGeoData.back().fPathData = NULL;

        fAtlas = atlas;
        fPathCache = pathCache;
        fPathList = pathList;
    }

    bool addPathToAtlas(GrBatchTarget* batchTarget,
                        const GrGeometryProcessor* dfProcessor,
                        const GrPipeline* pipeline,
                        GrDrawTarget::DrawInfo* drawInfo,
                        int* instancesToFlush,
                        int maxInstancesPerDraw,
                        GrBatchAtlas* atlas,
                        PathData* pathData,
                        const SkPath& path,
                        const SkStrokeRec&
                        stroke, bool antiAlias,
                        uint32_t dimension,
                        SkScalar scale) {
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
        // Now translate so the bound's UL corner is at the origin
        drawMatrix.postTranslate(-devPathBounds.fLeft * SK_Scalar1,
                                 -devPathBounds.fTop * SK_Scalar1);
        SkIRect pathBounds = SkIRect::MakeWH(devPathBounds.width(),
                                             devPathBounds.height());

        SkBitmap bmp;
        const SkImageInfo bmImageInfo = SkImageInfo::MakeA8(pathBounds.fRight,
                                                            pathBounds.fBottom);
        if (!bmp.tryAllocPixels(bmImageInfo)) {
            return false;
        }

        sk_bzero(bmp.getPixels(), bmp.getSafeSize());

        // rasterize path
        SkPaint paint;
        if (stroke.isHairlineStyle()) {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(SK_Scalar1);
        } else {
            if (stroke.isFillStyle()) {
                paint.setStyle(SkPaint::kFill_Style);
            } else {
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setStrokeJoin(stroke.getJoin());
                paint.setStrokeCap(stroke.getCap());
                paint.setStrokeWidth(stroke.getWidth());
            }
        }
        paint.setAntiAlias(antiAlias);

        SkDraw draw;
        sk_bzero(&draw, sizeof(draw));

        SkRasterClip rasterClip;
        rasterClip.setRect(pathBounds);
        draw.fRC = &rasterClip;
        draw.fClip = &rasterClip.bwRgn();
        draw.fMatrix = &drawMatrix;
        draw.fBitmap = &bmp;

        draw.drawPathCoverage(path, paint);

        // generate signed distance field
        devPathBounds.outset(SK_DistanceFieldPad, SK_DistanceFieldPad);
        int width = devPathBounds.width();
        int height = devPathBounds.height();
        // TODO We should really generate this directly into the plot somehow
        SkAutoSMalloc<1024> dfStorage(width * height * sizeof(unsigned char));

        // Generate signed distance field
        {
            SkAutoLockPixels alp(bmp);

            SkGenerateDistanceFieldFromA8Image((unsigned char*)dfStorage.get(),
                                               (const unsigned char*)bmp.getPixels(),
                                               bmp.width(), bmp.height(), bmp.rowBytes());
        }

        // add to atlas
        SkIPoint16 atlasLocation;
        GrBatchAtlas::AtlasID id;
        bool success = atlas->addToAtlas(&id, batchTarget, width, height, dfStorage.get(),
                                         &atlasLocation);
        if (!success) {
            this->flush(batchTarget, dfProcessor, pipeline, drawInfo, *instancesToFlush,
                        maxInstancesPerDraw);
            this->initDraw(batchTarget, dfProcessor, pipeline);
            *instancesToFlush = 0;

            SkDEBUGCODE(success =) atlas->addToAtlas(&id, batchTarget, width, height,
                                                     dfStorage.get(), &atlasLocation);
            SkASSERT(success);

        }

        // add to cache
        pathData->fKey.fGenID = path.getGenerationID();
        pathData->fKey.fDimension = dimension;
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

    void drawPath(GrBatchTarget* target,
                  GrBatchAtlas* atlas,
                  const GrPipeline* pipeline,
                  const GrGeometryProcessor* gp,
                  SkPoint* positions,
                  size_t vertexStride,
                  const SkMatrix& viewMatrix,
                  const SkPath& path,
                  const PathData* pathData) {
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

    void initDraw(GrBatchTarget* batchTarget,
                  const GrGeometryProcessor* dfProcessor,
                  const GrPipeline* pipeline) {
        batchTarget->initDraw(dfProcessor, pipeline);

        // TODO remove this when batch is everywhere
        GrPipelineInfo init;
        init.fColorIgnored = fBatch.fColorIgnored;
        init.fOverrideColor = GrColor_ILLEGAL;
        init.fCoverageIgnored = fBatch.fCoverageIgnored;
        init.fUsesLocalCoords = this->usesLocalCoords();
        dfProcessor->initBatchTracker(batchTarget->currentBatchTracker(), init);
    }

    void flush(GrBatchTarget* batchTarget,
               const GrGeometryProcessor* dfProcessor,
               const GrPipeline* pipeline,
               GrDrawTarget::DrawInfo* drawInfo,
               int instanceCount,
               int maxInstancesPerDraw) {
        while (instanceCount) {
            drawInfo->setInstanceCount(SkTMin(instanceCount, maxInstancesPerDraw));
            drawInfo->setVertexCount(drawInfo->instanceCount() * drawInfo->verticesPerInstance());
            drawInfo->setIndexCount(drawInfo->instanceCount() * drawInfo->indicesPerInstance());

            batchTarget->draw(*drawInfo);

            drawInfo->setStartVertex(drawInfo->startVertex() + drawInfo->vertexCount());
            instanceCount -= drawInfo->instanceCount();
       }
    }

    GrColor color() const { return fBatch.fColor; }
    const SkMatrix& viewMatrix() const { return fBatch.fViewMatrix; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }

    bool onCombineIfPossible(GrBatch* t) SK_OVERRIDE {
        AADistanceFieldPathBatch* that = t->cast<AADistanceFieldPathBatch>();

        // TODO we could actually probably do a bunch of this work on the CPU, ie map viewMatrix,
        // maybe upload color via attribute
        if (this->color() != that->color()) {
            return false;
        }

        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
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
};

bool GrAADistanceFieldPathRenderer::onDrawPath(GrDrawTarget* target,
                                               GrPipelineBuilder* pipelineBuilder,
                                               GrColor color,
                                               const SkMatrix& viewMatrix,
                                               const SkPath& path,
                                               const SkStrokeRec& stroke,
                                               bool antiAlias) {
    // we've already bailed on inverse filled paths, so this is safe
    if (path.isEmpty()) {
        return true;
    }

    SkASSERT(fContext);

    if (!fAtlas) {
        // Create a new atlas
        GrSurfaceDesc desc;
        desc.fFlags = kNone_GrSurfaceFlags;
        desc.fWidth = ATLAS_TEXTURE_WIDTH;
        desc.fHeight = ATLAS_TEXTURE_HEIGHT;
        desc.fConfig = kAlpha_8_GrPixelConfig;

        // We don't want to flush the context so we claim we're in the middle of flushing so as to
        // guarantee we do not recieve a texture with pending IO
        GrTexture* texture = fContext->refScratchTexture(desc, GrContext::kApprox_ScratchTexMatch,
                                                         true);
        if (texture) {
            fAtlas = SkNEW_ARGS(GrBatchAtlas, (texture, NUM_PLOTS_X, NUM_PLOTS_Y));
        } else {
            return false;
        }
        fAtlas->registerEvictionCallback(&GrAADistanceFieldPathRenderer::HandleEviction,
                                         (void*)this);
    }

    AADistanceFieldPathBatch::Geometry geometry(stroke);
    geometry.fPath = path;
    geometry.fAntiAlias = antiAlias;

    SkAutoTUnref<GrBatch> batch(AADistanceFieldPathBatch::Create(geometry, color, viewMatrix,
                                                                 fAtlas, &fPathCache, &fPathList));

    SkRect bounds = path.getBounds();
    viewMatrix.mapRect(&bounds);
    target->drawBatch(pipelineBuilder, batch, &bounds);

    return true;
}

