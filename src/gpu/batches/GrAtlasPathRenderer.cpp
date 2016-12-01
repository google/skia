/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasPathRenderer.h"

#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "GrBuffer.h"
#include "GrContext.h"
#include "GrPipelineBuilder.h"
#include "GrResourceProvider.h"
#include "GrSurfacePriv.h"
#include "GrSWMaskHelper.h"
#include "GrTexturePriv.h"
#include "batches/GrVertexBatch.h"
#include "effects/GrBitmapTextGeoProc.h"
#include "effects/GrDistanceFieldGeoProc.h"

#include "SkDistanceFieldGen.h"

#define ATLAS_TEXTURE_WIDTH 2048
#define ATLAS_TEXTURE_HEIGHT 2048
#define ATLAS_TEXTURE_LOG2_WIDTH 11
#define ATLAS_TEXTURE_LOG2_HEIGHT 11
#define PLOT_WIDTH  512
#define PLOT_HEIGHT 256

#define NUM_PLOTS_X   (ATLAS_TEXTURE_WIDTH / PLOT_WIDTH)
#define NUM_PLOTS_Y   (ATLAS_TEXTURE_HEIGHT / PLOT_HEIGHT)

#ifdef DF_PATH_TRACKING
static int g_NumCachedShapes = 0;
static int g_NumFreedShapes = 0;
#endif

// only use signed distance field representation on mobile
//#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_IOS)
#define USE_SDF_PATH
//#endif

// mip levels
static const int kSmallMIP = 64;
static const int kMediumMIP = 73;
static const int kLargeMIP = 162;

// Callback to clear out internal path cache when eviction occurs
void GrAtlasPathRenderer::HandleEviction(GrBatchAtlas::AtlasID id, void* pr) {
    GrAtlasPathRenderer* dfpr = (GrAtlasPathRenderer*)pr;
    // remove any paths that use this plot
    ShapeDataList::Iter iter;
    iter.init(dfpr->fShapeList, ShapeDataList::Iter::kHead_IterStart);
    ShapeData* shapeData;
    while ((shapeData = iter.get())) {
        iter.next();
        if (id == shapeData->fID) {
            dfpr->fShapeCache.remove(shapeData->fKey);
            dfpr->fShapeList.remove(shapeData);
            delete shapeData;
#ifdef DF_PATH_TRACKING
            ++g_NumFreedPaths;
#endif
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
GrAtlasPathRenderer::GrAtlasPathRenderer() : fAtlas(nullptr) {}

GrAtlasPathRenderer::~GrAtlasPathRenderer() {
    ShapeDataList::Iter iter;
    iter.init(fShapeList, ShapeDataList::Iter::kHead_IterStart);
    ShapeData* shapeData;
    while ((shapeData = iter.get())) {
        iter.next();
        delete shapeData;
    }

#ifdef DF_PATH_TRACKING
    SkDebugf("Cached shapes: %d, freed shapes: %d\n", g_NumCachedShapes, g_NumFreedShapes);
#endif
}

////////////////////////////////////////////////////////////////////////////////
bool GrAtlasPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    if (!args.fShaderCaps->shaderDerivativeSupport()) {
        return false;
    }
    // If the shape has no key then we won't get any reuse.
    if (!args.fShape->hasUnstyledKey()) {
        return false;
    }
    // This only supports filled paths, however, the caller may apply the style to make a filled
    // path and try again.
    if (!args.fShape->style().isSimpleFill()) {
        return false;
    }
    // This does non-inverse antialiased fills.
    if (!args.fAntiAlias) {
        return false;
    }
    // TODO: Support inverse fill
    if (args.fShape->inverseFilled()) {
        return false;
    }
    // currently don't support perspective
    if (args.fViewMatrix->hasPerspective()) {
        return false;
    }

    // only support paths with bounds within kMediumMIP by kMediumMIP,
    // scaled to have bounds within 2.0f*kLargeMIP by 2.0f*kLargeMIP
    // the goal is to accelerate rendering of lots of small paths that may be scaled
    SkScalar maxScale = args.fViewMatrix->getMaxScale();
    SkRect bounds = args.fShape->styledBounds();
    SkScalar maxDim = SkMaxScalar(bounds.width(), bounds.height());

    return maxDim <= kMediumMIP && maxDim * maxScale <= 2.0f*kLargeMIP;
}

////////////////////////////////////////////////////////////////////////////////

// padding around path bounds to allow for antialiased pixels
static const SkScalar kAntiAliasPad = 1.0f;

class AtlasPathBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    typedef GrAtlasPathRenderer::ShapeData ShapeData;
    typedef SkTDynamicHash<ShapeData, ShapeData::Key> ShapeCache;
    typedef GrAtlasPathRenderer::ShapeDataList ShapeDataList;

    AtlasPathBatch(GrColor color,
                   const GrShape& shape,
                   bool antiAlias,
                   const SkMatrix& viewMatrix,
                   GrBatchAtlas* atlas,
                   ShapeCache* shapeCache, ShapeDataList* shapeList,
                   bool gammaCorrect)
            : INHERITED(ClassID()) {
        SkASSERT(shape.hasUnstyledKey());
        fBatch.fViewMatrix = viewMatrix;
        fGeoData.emplace_back(Geometry{color, shape, antiAlias});

        fAtlas = atlas;
        fShapeCache = shapeCache;
        fShapeList = shapeList;
        fGammaCorrect = gammaCorrect;

        // Compute bounds
        this->setTransformedBounds(shape.bounds(), viewMatrix, HasAABloat::kYes, IsZeroArea::kNo);
        SkScalar minSize = SkMinScalar(bounds().width(), bounds().height());

        SkRect bounds = this->bounds();
#ifdef USE_SDF_PATH
        // There is currently an issue where we may produce 2 pixels worth of AA around the path
        // when using the signed distance field representation.
        // A workaround is to outset the bounds by 1 in device space. (skbug.com/5989)
        bounds.outset(1.f, 1.f);
#endif
        this->setBounds(bounds, HasAABloat::kYes, IsZeroArea::kNo);
    }

    const char* name() const override { return "AtlasPathBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
    }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any color overrides
        if (!overrides.readsColor()) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        }
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);

        // setup batch properties
        fBatch.fColorIgnored = !overrides.readsColor();
        fBatch.fUsesLocalCoords = overrides.readsLocalCoords();
        fBatch.fCoverageIgnored = !overrides.readsCoverage();
    }

    struct FlushInfo {
        sk_sp<const GrBuffer> fVertexBuffer;
        sk_sp<const GrBuffer> fIndexBuffer;
        sk_sp<GrGeometryProcessor>   fGeometryProcessor;
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

        FlushInfo flushInfo;

        // Setup GrGeometryProcessor
        GrBatchAtlas* atlas = fAtlas;
#ifdef USE_SDF_PATH
        const SkMatrix& ctm = this->viewMatrix();
        uint32_t flags = 0;
        flags |= ctm.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;
        flags |= ctm.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
        flags |= fGammaCorrect ? kGammaCorrect_DistanceFieldEffectFlag : 0;

        GrSamplerParams params(SkShader::kRepeat_TileMode, GrSamplerParams::kBilerp_FilterMode);

        flushInfo.fGeometryProcessor = GrDistanceFieldPathGeoProc::Make(this->color(),
                                                                        this->viewMatrix(),
                                                                        atlas->getTexture(),
                                                                        params,
                                                                        flags,
                                                                        this->usesLocalCoords());
#else
        GrSamplerParams params(SkShader::kRepeat_TileMode, GrSamplerParams::kNone_FilterMode);

        SkMatrix localMatrix;
        if (this->usesLocalCoords() && !this->viewMatrix().invert(&localMatrix)) {
            SkDebugf("Cannot invert viewmatrix\n");
            return;
        }
        flushInfo.fGeometryProcessor = GrBitmapTextGeoProc::Make(this->color(),
                                                                 atlas->getTexture(),
                                                                 params,
                                                                 kA8_GrMaskFormat,
                                                                 localMatrix,
                                                                 this->usesLocalCoords());
#endif

        // allocate vertices
        size_t vertexStride = flushInfo.fGeometryProcessor->getVertexStride();
#ifdef USE_SDF_PATH
        SkASSERT(vertexStride == 2 * sizeof(SkPoint) + sizeof(GrColor));
#else
        SkASSERT(vertexStride == sizeof(SkPoint) + 2 * sizeof(uint16_t) + sizeof(GrColor));
#endif

        const GrBuffer* vertexBuffer;
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
        // Pointer to the next set of vertices to write.
        intptr_t offset = reinterpret_cast<intptr_t>(vertices);
        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];

            // get mip level
            SkScalar maxScale = this->viewMatrix().getMaxScale();
            const SkRect& bounds = args.fShape.bounds();
            SkScalar maxDim = SkMaxScalar(bounds.width(), bounds.height());
            SkScalar size = maxScale * maxDim;
            uint32_t desiredDimension;
#ifdef USE_SDF_PATH
            if (size <= kSmallMIP) {
                desiredDimension = kSmallMIP;
            } else if (size <= kMediumMIP) {
                desiredDimension = kMediumMIP;
            } else {
                desiredDimension = kLargeMIP;
            }
            // HACKHACK
//            desiredDimension = maxDim;

            const SkMatrix& ctm = SkMatrix::I();
#else
            desiredDimension = size;

            SkMatrix modifiedCtm = this->viewMatrix();
            // subtract out any integer translation
            modifiedCtm[SkMatrix::kMTransX] -= (int)modifiedCtm[SkMatrix::kMTransX];
            modifiedCtm[SkMatrix::kMTransY] -= (int)modifiedCtm[SkMatrix::kMTransY];
            const SkMatrix& ctm = modifiedCtm;
#endif
            ShapeData::Key key(args.fShape, desiredDimension, ctm);
            // check to see if path is cached
            ShapeData* shapeData = fShapeCache->find(key);
            if (nullptr == shapeData || !atlas->hasID(shapeData->fID)) {
                // Remove the stale cache entry
                if (shapeData) {
                    fShapeCache->remove(shapeData->fKey);
                    fShapeList->remove(shapeData);
                    delete shapeData;
                }
#ifdef USE_SDF_PATH
                SkScalar scale = desiredDimension/maxDim;
                // HACKHACK
//                scale = SK_Scalar1;
#else
                SkScalar scale = SK_Scalar1;
#endif
                shapeData = new ShapeData;
                if (!this->addPathToAtlas(target,
                                          &flushInfo,
                                          atlas,
                                          shapeData,
                                          args.fShape,
                                          args.fAntiAlias,
                                          desiredDimension,
                                          scale,
                                          ctm)) {
                    delete shapeData;
                    SkDebugf("Can't rasterize path\n");
                    continue;
                }
            }

            atlas->setLastUseToken(shapeData->fID, target->nextDrawToken());

            this->writePathVertices(target,
                                    atlas,
                                    offset,
                                    args.fColor,
                                    vertexStride,
                                    this->viewMatrix(),
                                    shapeData);
            offset += kVerticesPerQuad * vertexStride;
            flushInfo.fInstancesToFlush++;
        }

        this->flush(target, &flushInfo);
    }

    bool addPathToAtlas(GrVertexBatch::Target* target,
                        FlushInfo* flushInfo,
                        GrBatchAtlas* atlas,
                        ShapeData* shapeData,
                        const GrShape& shape,
                        bool antiAlias,
                        uint32_t dimension,
                        SkScalar scale,
                        const SkMatrix ctm) const {
        const SkRect& bounds = shape.bounds();

        // generate bounding rect for bitmap draw
        SkRect scaledBounds = bounds;
        // scale to mip level size
        scaledBounds.fLeft *= scale;
        scaledBounds.fTop *= scale;
        scaledBounds.fRight *= scale;
        scaledBounds.fBottom *= scale;
        // move the origin to an integer boundary (gives better devBounds fit)
        SkScalar dx = SkScalarFraction(scaledBounds.fLeft);
        SkScalar dy = SkScalarFraction(scaledBounds.fTop);
        scaledBounds.offset(-dx, -dy);
        // get integer boundary
        SkIRect devPathBounds;
        scaledBounds.roundOut(&devPathBounds);
        // pad to allow room for antialiasing
        const int intPad = SkScalarCeilToInt(kAntiAliasPad);
        // place devPathBounds at origin
        int width = devPathBounds.width();
        int height = devPathBounds.height();
        devPathBounds.fLeft = 0;
        devPathBounds.fTop = 0;
        devPathBounds.fRight = 2*intPad + width;
        devPathBounds.fBottom = 2*intPad + height;

        // draw path to bitmap
        SkMatrix drawMatrix;
        drawMatrix.setTranslate(-SkScalarFloorToInt(bounds.left()), -SkScalarFloorToInt(bounds.top()));
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
        draw.fMatrix = &drawMatrix;
        draw.fDst = dst;

        SkPath path;
        shape.asPath(&path);
        draw.drawPathCoverage(path, paint);

#ifdef USE_SDF_PATH
        // generate signed distance field
        devPathBounds.outset(SK_DistanceFieldPad, SK_DistanceFieldPad);
        width = devPathBounds.width();
        height = devPathBounds.height();
        // TODO We should really generate this directly into the plot somehow
        SkAutoSMalloc<1024> dfStorage(width * height * sizeof(unsigned char));

        // Generate signed distance field
        SkGenerateDistanceFieldFromA8Image((unsigned char*)dfStorage.get(),
                                           (const unsigned char*)dst.addr(),
                                           dst.width(), dst.height(), dst.rowBytes());
        const void* pathData = dfStorage.get();
#else
        width = devPathBounds.width();
        height = devPathBounds.height();
        const void* pathData = dst.addr();
#endif
        // add to atlas
        SkIPoint16 atlasLocation;
        GrBatchAtlas::AtlasID id;
        if (!atlas->addToAtlas(&id, target, width, height, pathData, &atlasLocation)) {
            this->flush(target, flushInfo);
            if (!atlas->addToAtlas(&id, target, width, height, pathData, &atlasLocation)) {
                return false;
            }
        }

        // add to cache
        shapeData->fKey.set(shape, dimension, ctm);
        shapeData->fScale = scale;
        shapeData->fID = id;
#ifdef USE_SDF_PATH
        //// change the scaled rect to match the size of the inset distance field
        //// TODO: leave it the same and then scale it? Something is off here.
        //scaledBounds.fRight = scaledBounds.fLeft +
        //    SkIntToScalar(devPathBounds.width() - 2*SK_DistanceFieldInset);
        //scaledBounds.fBottom = scaledBounds.fTop +
        //    SkIntToScalar(devPathBounds.height() - 2*SK_DistanceFieldInset);
        //// shift the origin to the correct place relative to the distance field
        //// need to also adjust based on the fractional translation
        //scaledBounds.offset(-SkIntToScalar(SK_DistanceFieldInset) - kAntiAliasPad + SkScalarCeilToInt(dx),
        //                    -SkIntToScalar(SK_DistanceFieldInset) - kAntiAliasPad + SkScalarCeilToInt(dy));
        //// origin we render from is inset from distance field edge
        //atlasLocation.fX += SK_DistanceFieldInset;
        //atlasLocation.fY += SK_DistanceFieldInset;

        // shift the origin to the correct place relative to the distance field
        //scaledBounds.fLeft -= kAntiAliasPad;
        //scaledBounds.fTop -= kAntiAliasPad;
        // change the scaled rect to match the size of the inset distance field
        // TODO: leave it the same and then scale it? Something is off here.
        scaledBounds.fRight = scaledBounds.fLeft +
            SkIntToScalar(devPathBounds.width() - 2 * SK_DistanceFieldPad - 2 * kAntiAliasPad);
        scaledBounds.fBottom = scaledBounds.fTop +
            SkIntToScalar(devPathBounds.height() - 2 * SK_DistanceFieldPad - 2 * kAntiAliasPad);
        // need to also adjust based on the fractional translation(?)
//        scaledBounds.offset(-kAntiAliasPad, -kAntiAliasPad);
        // origin we render from is inset from distance field edge and AA pad
        atlasLocation.fX += SK_DistanceFieldPad + kAntiAliasPad;
        atlasLocation.fY += SK_DistanceFieldPad + kAntiAliasPad;
#endif
        shapeData->fBounds = scaledBounds;
        shapeData->fAtlasLocation = atlasLocation;

        fShapeCache->add(shapeData);
        fShapeList->addToTail(shapeData);
#ifdef DF_PATH_TRACKING
        ++g_NumCachedPaths;
#endif
        return true;
    }

    void writePathVertices(GrDrawBatch::Target* target,
                           GrBatchAtlas* atlas,
                           intptr_t offset,
                           GrColor color,
                           size_t vertexStride,
                           const SkMatrix& viewMatrix,
                           const ShapeData* shapeData) const {
        GrTexture* texture = atlas->getTexture();

        SkScalar dx = shapeData->fBounds.fLeft;
        SkScalar dy = shapeData->fBounds.fTop;
        SkScalar width = shapeData->fBounds.width();
        SkScalar height = shapeData->fBounds.height();

        SkScalar invScale = 1.0f / shapeData->fScale;
        dx *= invScale;
        dy *= invScale;
        width *= invScale;
        height *= invScale;

#ifndef USE_SDF_PATH
        // add just the integer part of the translation
        // (rest of ctm is baked into cached path)
        dx += (int) viewMatrix[SkMatrix::kMTransX];
        dy += (int)viewMatrix[SkMatrix::kMTransY];
#endif

        SkPoint* positions = reinterpret_cast<SkPoint*>(offset);

        // vertex positions
        // TODO make the vertex attributes a struct
        SkRect r = SkRect::MakeXYWH(dx, dy, width, height);
        r.outset(kAntiAliasPad, kAntiAliasPad);
        positions->setRectFan(r.left(), r.top(), r.right(), r.bottom(), vertexStride);

        // colors
        for (int i = 0; i < kVerticesPerQuad; i++) {
            GrColor* colorPtr = (GrColor*)(offset + sizeof(SkPoint) + i * vertexStride);
            *colorPtr = color;
        }

#ifdef USE_SDF_PATH
        const SkScalar tx = SkIntToScalar(shapeData->fAtlasLocation.fX);
        const SkScalar ty = SkIntToScalar(shapeData->fAtlasLocation.fY);

        // vertex texture coords
        SkPoint* textureCoords = (SkPoint*)(offset + sizeof(SkPoint) + sizeof(GrColor));
        SkScalar scaledAAPad = shapeData->fScale * kAntiAliasPad;
        textureCoords->setRectFan((tx - scaledAAPad) / texture->width(),
                                  (ty - scaledAAPad) / texture->height(),
                                  (tx + shapeData->fBounds.width() + scaledAAPad) / texture->width(),
                                  (ty + shapeData->fBounds.height() + scaledAAPad)  / texture->height(),
                                  vertexStride);
#else
        int u0, v0, u1, v1;
        u0 = shapeData->fAtlasLocation.fX;
        v0 = shapeData->fAtlasLocation.fY;
        u1 = u0 + (int)width;
        v1 = v0 + (int)height;

        // normalize
        u0 *= 65535;
        u0 >>= ATLAS_TEXTURE_LOG2_WIDTH;
        u1 *= 65535;
        u1 >>= ATLAS_TEXTURE_LOG2_WIDTH;
        v0 *= 65535;
        v0 >>= ATLAS_TEXTURE_LOG2_HEIGHT;
        v1 *= 65535;
        v1 >>= ATLAS_TEXTURE_LOG2_HEIGHT;
        SkASSERT(u0 >= 0 && u0 <= 65535);
        SkASSERT(u1 >= 0 && u1 <= 65535);
        SkASSERT(v0 >= 0 && v0 <= 65535);
        SkASSERT(v1 >= 0 && v1 <= 65535);

        // vertex texture coords
        uint16_t* textureCoords = (uint16_t*)(offset + sizeof(SkPoint) + sizeof(GrColor));
        textureCoords[0] = u0;
        textureCoords[1] = v0;
        textureCoords += vertexStride / sizeof(uint16_t);
        textureCoords[0] = u0;
        textureCoords[1] = v1;
        textureCoords += vertexStride / sizeof(uint16_t);
        textureCoords[0] = u1;
        textureCoords[1] = v1;
        textureCoords += vertexStride / sizeof(uint16_t);
        textureCoords[0] = u1;
        textureCoords[1] = v0;
#endif
    }

    void flush(GrVertexBatch::Target* target, FlushInfo* flushInfo) const {
        if (flushInfo->fInstancesToFlush) {
            GrMesh mesh;
            int maxInstancesPerDraw =
                static_cast<int>(flushInfo->fIndexBuffer->gpuMemorySize() / sizeof(uint16_t) / 6);
            mesh.initInstanced(kTriangles_GrPrimitiveType, flushInfo->fVertexBuffer.get(),
                flushInfo->fIndexBuffer.get(), flushInfo->fVertexOffset, kVerticesPerQuad,
                kIndicesPerQuad, flushInfo->fInstancesToFlush, maxInstancesPerDraw);
            target->draw(flushInfo->fGeometryProcessor.get(), mesh);
            flushInfo->fVertexOffset += kVerticesPerQuad * flushInfo->fInstancesToFlush;
            flushInfo->fInstancesToFlush = 0;
        }
    }

    GrColor color() const { return fGeoData[0].fColor; }
    const SkMatrix& viewMatrix() const { return fBatch.fViewMatrix; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        AtlasPathBatch* that = t->cast<AtlasPathBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        // TODO We can position on the cpu
        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
        this->joinBounds(*that);
        return true;
    }

    struct BatchTracker {
        SkMatrix fViewMatrix;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    struct Geometry {
        GrColor fColor;
        GrShape fShape;
        bool fAntiAlias;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry> fGeoData;
    GrBatchAtlas* fAtlas;
    ShapeCache* fShapeCache;
    ShapeDataList* fShapeList;
    bool fGammaCorrect;

    typedef GrVertexBatch INHERITED;
};

bool GrAtlasPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrAtlasPathRenderer::onDrawPath");
    SkASSERT(!args.fRenderTargetContext->isUnifiedMultisampled());
    SkASSERT(args.fShape->style().isSimpleFill());

    // we've already bailed on inverse filled paths, so this is safe
    SkASSERT(!args.fShape->isEmpty());
    SkASSERT(args.fShape->hasUnstyledKey());
    if (!fAtlas) {
        fAtlas = args.fResourceProvider->makeAtlas(kAlpha_8_GrPixelConfig,
                                                   ATLAS_TEXTURE_WIDTH, ATLAS_TEXTURE_HEIGHT,
                                                   NUM_PLOTS_X, NUM_PLOTS_Y,
                                                   &GrAtlasPathRenderer::HandleEviction,
                                                   (void*)this);
        if (!fAtlas) {
            return false;
        }
    }

    sk_sp<GrDrawBatch> batch(new AtlasPathBatch(args.fPaint->getColor(),
                                                          *args.fShape,
                                                          args.fAntiAlias, *args.fViewMatrix,
                                                          fAtlas.get(), &fShapeCache, &fShapeList,
                                                          args.fGammaCorrect));
    GrPipelineBuilder pipelineBuilder(*args.fPaint);
    pipelineBuilder.setUserStencil(args.fUserStencilSettings);

    args.fRenderTargetContext->drawBatch(pipelineBuilder, *args.fClip, batch.get());

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

struct PathTestStruct {
    typedef GrAtlasPathRenderer::ShapeCache ShapeCache;
    typedef GrAtlasPathRenderer::ShapeData ShapeData;
    typedef GrAtlasPathRenderer::ShapeDataList ShapeDataList;
    PathTestStruct() : fContextID(SK_InvalidGenID), fAtlas(nullptr) {}
    ~PathTestStruct() { this->reset(); }

    void reset() {
        ShapeDataList::Iter iter;
        iter.init(fShapeList, ShapeDataList::Iter::kHead_IterStart);
        ShapeData* shapeData;
        while ((shapeData = iter.get())) {
            iter.next();
            fShapeList.remove(shapeData);
            delete shapeData;
        }
        fAtlas = nullptr;
        fShapeCache.reset();
    }

    static void HandleEviction(GrBatchAtlas::AtlasID id, void* pr) {
        PathTestStruct* dfpr = (PathTestStruct*)pr;
        // remove any paths that use this plot
        ShapeDataList::Iter iter;
        iter.init(dfpr->fShapeList, ShapeDataList::Iter::kHead_IterStart);
        ShapeData* shapeData;
        while ((shapeData = iter.get())) {
            iter.next();
            if (id == shapeData->fID) {
                dfpr->fShapeCache.remove(shapeData->fKey);
                dfpr->fShapeList.remove(shapeData);
                delete shapeData;
            }
        }
    }

    uint32_t fContextID;
    std::unique_ptr<GrBatchAtlas> fAtlas;
    ShapeCache fShapeCache;
    ShapeDataList fShapeList;
};

DRAW_BATCH_TEST_DEFINE(AtlasPathBatch) {
    static PathTestStruct gTestStruct;

    if (context->uniqueID() != gTestStruct.fContextID) {
        gTestStruct.fContextID = context->uniqueID();
        gTestStruct.reset();
        gTestStruct.fAtlas =
                context->resourceProvider()->makeAtlas(kAlpha_8_GrPixelConfig,
                                                       ATLAS_TEXTURE_WIDTH, ATLAS_TEXTURE_HEIGHT,
                                                       NUM_PLOTS_X, NUM_PLOTS_Y,
                                                       &PathTestStruct::HandleEviction,
                                                       (void*)&gTestStruct);
    }

    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrColor color = GrRandomColor(random);
    bool gammaCorrect = random->nextBool();

    // This path renderer only allows fill styles.
    GrShape shape(GrTest::TestPath(random), GrStyle::SimpleFill());
    bool antiAlias = random->nextBool();

    return new AtlasPathBatch(color,
                                        shape,
                                        antiAlias,
                                        viewMatrix,
                                        gTestStruct.fAtlas.get(),
                                        &gTestStruct.fShapeCache,
                                        &gTestStruct.fShapeList,
                                        gammaCorrect);
}

#endif
